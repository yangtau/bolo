#include "bolo.h"

#include <fstream>

#include "util.h"

namespace bolo {

Result<Bolo, std::string> Bolo::LoadFromJsonFile(const std::string &path) {
  std::ifstream f(path);
  if (!f.is_open()) {
    return Err("failed to open " + path);
  }

  json config;
  try {
    f >> config;
  } catch (const json::parse_error &e) {
    return Err(std::string("json parse_error: ") + e.what());
  }

  BackupFileId next_id;
  BackupList list;

  try {
    config.at("next_id").get_to(next_id);
    config.at("backup_list").get_to(list);
  } catch (const json::out_of_range &e) {
    return Err(std::string("json out_of_range: ") + e.what());
  } catch (const json::type_error &e) {
    return Err(std::string("json type_error: ") + e.what());
  }

  return Ok(Bolo(path, config, list, next_id));
}

Result<BackupFile, std::string> Bolo::Backup(const std::string &path,
                                             const std::string &backup_path, bool is_compressed,
                                             bool is_encrypted) {
  auto file = BackupFile{NextId(), path, backup_path, GetTimestamp(), is_compressed, is_encrypted};

  backup_files_[file.id] = file;

  if (!UpdateConfig()) {
    return Err<std::string>("failed to update config");
  }

  // TODO: backup

  if (is_compressed) {
    // TODO:
  }
  if (is_encrypted) {
    // TODO:
  }

  // TODO: if failed in the last three phases, rollback the change in config

  return Ok(file);
}

// 删除一个备份文件
bool Bolo::Remove(BackupFileId id) {
  if (backup_files_.find(id) == backup_files_.end()) return false;

  // TODO: remove backup

  backup_files_.erase(id);

  return UpdateConfig();
}

// 更新一个备份文件
bool Bolo::Update(BackupFileId id) {
  if (backup_files_.find(id) == backup_files_.end()) return false;

  // TODO: update backup

  BackupFile &file = backup_files_[id];
  file.timestamp = GetTimestamp();

  return UpdateConfig();
}

bool Bolo::UpdateConfig() {
  try {
    config_["next_id"] = next_id_;
    config_["backup_list"] = json(backup_files_);
  } catch (const json::type_error &e) {
    return false;
  }

  std::ofstream f(config_file_path_);
  if (!f.is_open()) return false;

  f << config_.dump(4);

  return static_cast<bool>(f);
}

};  // namespace bolo
