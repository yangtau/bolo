#include "bolo.h"

#include <fstream>
#include <string>

#include "util.h"

namespace bolo {
using namespace std::string_literals;

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
  std::string backup_path;

  try {
    config.at("next_id").get_to(next_id);
    config.at("backup_list").get_to(list);
    config.at("backup_path").get_to(list);
  } catch (const json::out_of_range &e) {
    return Err("json out_of_range: "s + e.what());
  } catch (const json::type_error &e) {
    return Err("json type_error: "s + e.what());
  }

  // TODO: make backup_path dir

  return Ok(Bolo(path, config, list, next_id));
}

Result<BackupFile, std::string> Bolo::Backup(const std::string &path, bool is_compressed,
                                             bool is_encrypted) {
  auto file = BackupFile{NextId(), path, backup_path_, GetTimestamp(), is_compressed, is_encrypted};

  backup_files_[file.id] = file;

  if (!UpdateConfig()) {
    return Err("failed to update config"s);
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
Insidious<std::string> Bolo::Remove(BackupFileId id) {
  if (backup_files_.find(id) == backup_files_.end()) return Danger(""s);

  // TODO: remove backup

  backup_files_.erase(id);

  return UpdateConfig();
}

// 更新一个备份文件
Insidious<std::string> Bolo::Update(BackupFileId id) {
  if (backup_files_.find(id) == backup_files_.end())
    return Danger("No backup file with a BackupFileId of "s + std::to_string(id));

  // TODO: update backup

  BackupFile &file = backup_files_[id];
  file.timestamp = GetTimestamp();

  return UpdateConfig();
}

Insidious<std::string> Bolo::UpdateConfig() {
  try {
    config_["next_id"] = next_id_;
    config_["backup_list"] = json(backup_files_);
  } catch (const json::type_error &e) {
    return Danger("json type_error: "s + e.what());
  }

  std::ofstream f(config_file_path_);
  if (!f.is_open()) return Danger("failed to open config file: "s + config_file_path_);

  f << config_.dump(4);
  if (!f.good()) return Danger("failed to write to config file: "s + config_file_path_);
  return Safe;
}

};  // namespace bolo
