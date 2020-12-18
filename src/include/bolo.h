#pragma once
#include <string>
#include <unordered_map>
#include <utility>

#include "backup_file.h"
#include "result.h"
#include "types.h"
#include "util.h"

namespace bolo {

class Bolo {
 public:
  // Input:
  //   path: json config 配置路径
  // Returns bolo: Result<Bolo> on success;
  // Return error massage: Result<std::string> on error
  static Result<Bolo, std::string> LoadFromJsonFile(const std::string &path);

  // 添加一个备份文件
  Result<BackupFile, std::string> Backup(const std::string &path, bool is_compressed,
                                         bool is_encrypted);

  // 删除一个备份文件
  Insidious<std::string> Remove(BackupFileId id);

  // 更新一个备份文件
  Insidious<std::string> Update(BackupFileId id);

  // 恢复一个备份文件
  // restore_path 是恢复位置的文件夹路径
  Insidious<std::string> Restore(BackupFileId id, const std::string &restore_path);

  Maybe<BackupFile> GetBackupFile(BackupFileId id) {
    if (backup_files_.find(id) != backup_files_.end()) return Just(backup_files_[id]);
    return Nothing;
  }

 private:
  template <typename ConfigPath, typename Json, typename BackupFileMap>
  Bolo(ConfigPath &&config_path, Json &&config, BackupFileMap &&m, BackupFileId next_id)
      : config_file_path_(std::forward<ConfigPath>(config_path)),
        config_(std::forward<Json>(config)),
        backup_files_(std::forward<BackupFileMap>(m)),
        next_id_(next_id) {}

  // 更新配置文件:
  //     备份文件列表更新, 或者其他配置信息更新; 配置更新后, 必须将配置持久化成功后才返回 true
  Insidious<std::string> UpdateConfig();
  BackupFileId NextId() { return next_id_++; }

  PropertyWithGetter(std::string, config_file_path);  // 配置文件路径
  PropertyWithGetter(std::string, backup_path);       // 备份文件夹路径
  PropertyWithGetter(json, config);                   // 配置
  PropertyWithGetter(BackupList, backup_files);       // 备份文件列表
  PropertyWithGetter(BackupFileId, next_id);          // 下一个备份文件 id
};
};  // namespace bolo
