#include "bolo.h"

#include <fstream>
#include <string>

#include "tar.h"
#include "util.h"

namespace bolo {
using namespace std::string_literals;

Result<std::unique_ptr<Bolo>, std::string> Bolo::LoadFromJsonFile(const fs::path &path) {
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
}

Result<BackupFile, std::string> Bolo::Backup(const fs::path &path, bool is_compressed,
                                             bool is_encrypted) {
  auto id = NextId();
  std::string filename = path.filename();

  // backup filename = filename + id
  auto backup_path = (backup_dir_ / (filename + std::to_string(id)));

  auto file = BackupFile{
      id, filename, path, backup_path, GetTimestamp(), is_compressed, is_encrypted,
  };

  backup_files_[file.id] = file;

  auto ins = BackupImpl(file);
  if (ins) {
    backup_files_.erase(file.id);
    std::error_code err;
    fs::remove(file.backup_path, err);
    return Err(ins.error());
  }

  return Ok(file);
}

Insidious<std::string> Bolo::BackupImpl(const BackupFile &f) {
  {
    using bolo_tar::Tar;
    auto tar = Tar::Open(f.backup_path);
    if (!tar) return Danger(tar.error());

    auto res = tar.value()->Append(f.path);

    if (res) {
      backup_files_.erase(f.id);
      return Danger("tar error: "s + res.error());
    }

    if (f.is_compressed) {
      // TODO:
    }
    if (f.is_encrypted) {
      // TODO:
    }
  }

  return UpdateConfig();
}

// 删除一个备份文件
Insidious<std::string> Bolo::Remove(BackupFileId id) {
  if (backup_files_.find(id) == backup_files_.end())
    return Danger("No backup file with a BackupFileId of "s + std::to_string(id));

  auto file = backup_files_[id];

  std::error_code err;
  fs::remove(file.backup_path, err);

  backup_files_.erase(id);

  return UpdateConfig();
}

// 更新一个备份文件
Insidious<std::string> Bolo::Update(BackupFileId id) {
  if (backup_files_.find(id) == backup_files_.end())
    return Danger("No backup file with a BackupFileId of "s + std::to_string(id));

  BackupFile &file = backup_files_[id];
  file.timestamp = GetTimestamp();

  return BackupImpl(file);
}

Insidious<std::string> Bolo::Restore(BackupFileId id, const fs::path &restore_dir) {
  using bolo_tar::Tar;

  if (!fs::exists(restore_dir)) return Danger("directory does not exist: "s + restore_dir.string());
  if (!fs::is_directory(restore_dir))
    return Danger("the restore_path should be a directory: "s + restore_dir.string());

  if (backup_files_.find(id) == backup_files_.end())
    return Danger("No backup file with a BackupFileId of "s + std::to_string(id));
  const BackupFile &file = backup_files_[id];

  if (!fs::exists(file.backup_path))
    return Danger("backup file does not exist: "s + file.backup_path);
  auto restore_path = restore_dir / file.filename;
  if (fs::exists(restore_path))
    return Danger("file `"s + file.filename + "` already exists in `" + restore_dir.string() + "`");

  if (file.is_compressed) {
    // TODO
  }
  if (file.is_encrypted) {
  }

  if (auto tar = Tar::Open(file.backup_path))
    return tar.value()->Extract(restore_dir);
  else
    return Danger(tar.error());
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
