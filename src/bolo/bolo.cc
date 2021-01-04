#include "bolo.h"

#include <fstream>
#include <string>

#include "compress.h"
#include "crypto.h"
#include "tar.h"
#include "util.h"

namespace bolo {
using namespace std::string_literals;

Result<std::unique_ptr<Bolo>, std::string> Bolo::LoadFromJsonFile(const fs::path &path) try {
  std::ifstream f(path);
  if (!f.is_open()) {
    return Err("failed to open " + path.string());
  }

  json config;
  try {
    f >> config;
  } catch (const json::parse_error &e) {
    return Err("json parse_error: "s + e.what());
  }

  BackupFileId next_id;
  BackupList list;
  fs::path backup_dir;

  try {
    config.at("next_id").get_to(next_id);
    config.at("backup_list").get_to(list);
    backup_dir = config.at("backup_dir").get<std::string>();
    backup_dir = backup_dir.lexically_normal();
  } catch (const json::out_of_range &e) {
    return Err("json out_of_range: "s + e.what());
  } catch (const json::type_error &e) {
    return Err("json type_error: "s + e.what());
  }

  fs::create_directories(backup_dir);
  // if backup_path exists
  if (fs::exists(backup_dir)) {
    // check if backup_path is a dir
    if (fs::status(backup_dir).type() != fs::file_type::directory)
      return Err("the backup_path is not a directory: "s + backup_dir.string());
  } else {
    return Err("failed to create backup directory: "s + backup_dir.string());
  }
  return Ok(std::unique_ptr<Bolo>(new Bolo(path, config, list, next_id, backup_dir)));
} catch (const fs::filesystem_error &e) {
  return Err("filesystem error: "s + e.what());
}

Result<BackupFile, std::string> Bolo::Backup(const fs::path &path, bool is_compressed,
                                             bool is_encrypted, const std::string &key) try {
  auto id = NextId();
  // remove '/' in directory path
  auto p = path.string().back() == '/' ? path.parent_path() : path;

  std::string filename = p.lexically_relative(p.parent_path());

  // backup filename = filename + id
  auto backup_path = (backup_dir_ / (filename + std::to_string(id)));

  auto file = BackupFile{
      id, filename, p, backup_path, GetTimestamp(), is_compressed, is_encrypted,
  };

  backup_files_[file.id] = file;

  std::string err;
  if (auto ins = BackupImpl(file, key)) {
    err = ins.error();
    goto clean;
  }

  if (auto ins = UpdateConfig()) {
    err = ins.error();
    goto clean;
  }

  return Ok(file);

clean:
  backup_files_.erase(file.id);
  fs::remove(file.backup_path);
  return Err(err);
} catch (const fs::filesystem_error &e) {
  return Err("filesystem error: "s + e.what());
}

// `f` has already being inserted into config
Insidious<std::string> Bolo::BackupImpl(BackupFile &f, const std::string &key) {
  auto temp = MakeTemp();
  if (auto tar = bolo_tar::Tar::Open(temp)) {
    if (auto res = tar.value()->Append(f.path)) return Danger("tar error: "s + res.error());
  } else {
    return Danger(tar.error());
  }

  if (f.is_compressed) {
    auto t = MakeTemp();
    std::ifstream ifs(temp, std::ios_base::binary);
    if (!ifs.good()) return Danger("failded to open "s + temp);
    std::ofstream ofs(t, std::ios_base::binary | std::ios_base::trunc);
    if (!ofs.good()) return Danger("failded to open "s + t);

    if (auto ins = bolo_compress::Compress(ifs, ofs, bolo_compress::Scheme::DEFLATE))
      return Danger("compression error: "s + ins.error());

    // update temp
    temp = t;
  }

  if (f.is_encrypted) {
    if (key == "") return Danger("the file is encrypted, but the key is empty"s);

    auto t = MakeTemp();
    std::ifstream ifs(temp, std::ios_base::binary);
    if (!ifs.good()) return Danger("failded to open "s + temp);
    std::ofstream ofs(t, std::ios_base::binary | std::ios_base::trunc);
    if (!ofs.good()) return Danger("failded to open "s + t);

    if (auto ins = bolo_crypto::Encrypt(ifs, ofs, key))
      return Danger("encrypt error: "s + ins.error());

    // update temp
    temp = t;
  }

  // rename temp
  fs::rename(temp, f.backup_path);
  return Safe;
}

// 删除一个备份文件
Insidious<std::string> Bolo::Remove(BackupFileId id) try {
  if (backup_files_.find(id) == backup_files_.end())
    return Danger("No backup file with a BackupFileId of "s + std::to_string(id));

  auto file = backup_files_[id];

  fs::remove(file.backup_path);

  backup_files_.erase(id);

  return UpdateConfig();
} catch (const fs::filesystem_error &e) {
  return Danger("filesystem error: "s + e.what());
}

// 更新一个备份文件
Insidious<std::string> Bolo::Update(BackupFileId id, const std::string &key) try {
  if (backup_files_.find(id) == backup_files_.end())
    return Danger("No backup file with a BackupFileId of "s + std::to_string(id));

  BackupFile &file = backup_files_[id];

  if (file.is_encrypted && key == "") return Danger("the file is encrypted, but the key is empty"s);

  if (auto ins = BackupImpl(file, key)) return ins;

  if (auto ins = UpdateConfig()) return ins;

  // update timestamp
  file.timestamp = GetTimestamp();
  return Safe;
} catch (const fs::filesystem_error &e) {
  return Danger("filesystem error: "s + e.what());
}

Insidious<std::string> Bolo::Restore(BackupFileId id, const fs::path &restore_dir,
                                     const std::string &key) try {
  using bolo_tar::Tar;

  // check if the restore dir exists
  if (!fs::exists(restore_dir)) return Danger("directory does not exist: "s + restore_dir.string());
  if (!fs::is_directory(restore_dir))
    return Danger("the restore_path should be a directory: "s + restore_dir.string());

  // check if the file id is right
  if (backup_files_.find(id) == backup_files_.end())
    return Danger("No backup file with a BackupFileId of "s + std::to_string(id));
  const BackupFile &file = backup_files_[id];

  // check if the backup file exists
  if (!fs::exists(file.backup_path))
    return Danger("backup file does not exist: "s + file.backup_path);

  // check is there is a conflict file in the restore_dir
  auto restore_path = restore_dir / file.filename;
  if (fs::exists(restore_path))
    return Danger("file `"s + file.filename + "` already exists in `" + restore_dir.string() + "`");

  if (file.is_encrypted && key == "") return Danger("the file is encrypted, but the key is empty"s);

  std::string temp = file.backup_path;

  if (file.is_encrypted) {
    auto t = MakeTemp();

    std::ifstream ifs(temp, std::ios_base::binary);
    if (!ifs.good()) return Danger("failded to open "s + file.backup_path);
    std::ofstream ofs(t, std::ios_base::binary | std::ios_base::trunc);
    if (!ofs.good()) return Danger("failded to open "s + temp);

    if (auto ins = bolo_crypto::Decrypt(ifs, ofs, key))
      return Danger("decrypto error: "s + ins.error());

    temp = t;
  }

  if (file.is_compressed) {
    auto t = MakeTemp();

    std::ifstream ifs(temp, std::ios_base::binary);
    if (!ifs.good()) return Danger("failded to open "s + file.backup_path);
    std::ofstream ofs(t, std::ios_base::binary | std::ios_base::trunc);
    if (!ofs.good()) return Danger("failded to open "s + temp);

    if (auto ins = bolo_compress::Uncompress(ifs, ofs, bolo_compress::Scheme::DEFLATE))
      return Danger("compression error: "s + ins.error());

    temp = t;
  }

  if (auto tar = Tar::Open(temp)) {
    if (auto res = tar.value()->Extract(restore_dir)) return Danger("tar error: "s + res.error());
  } else {
    return Danger("tar error: "s + tar.error());
  }

  return Safe;
} catch (const fs::filesystem_error &e) {
  return Danger("filesystem error: "s + e.what());
}

Insidious<std::string> Bolo::UpdateConfig() {
  try {
    config_["next_id"] = next_id_;
    config_["backup_list"] = json(backup_files_);
    config_["backup_dir"] = backup_dir_.string();
  } catch (const json::type_error &e) {
    return Danger("json type_error: "s + e.what());
  }

  std::ofstream f(config_file_path_);
  if (!f.is_open()) return Danger("failed to open config file: "s + config_file_path_.string());

  f << config_.dump(4);
  if (!f.good()) return Danger("failed to write to config file: "s + config_file_path_.string());
  return Safe;
}

};  // namespace bolo
