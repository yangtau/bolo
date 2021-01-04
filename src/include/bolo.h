#pragma once
#include <filesystem>
#include <string>
#include <unordered_map>
#include <utility>

#include "backup_file.h"
#include "result.h"
#include "types.h"
#include "util.h"

namespace bolo {
namespace fs = std::filesystem;

class Bolo {
 public:
  Bolo(const Bolo &) = delete;
  Bolo(Bolo &&) = default;

  // Input:
  //   path: json config 配置路径
  // Returns bolo: Result<Bolo> on success;
  // Return error massage: Result<std::string> on error
  static Result<std::unique_ptr<Bolo>, std::string> LoadFromJsonFile(const fs::path &path);

  // 添加一个备份文件
  Result<BackupFile, std::string> Backup(const fs::path &path, bool is_compressed,
                                         bool is_encrypted, bool enable_cloud = false,
                                         const std::string &key = "");

  // 删除一个备份文件
  Insidious<std::string> Remove(BackupFileId id);

  // 更新一个备份文件
  Insidious<std::string> Update(BackupFileId id, const std::string &key = "");

  // 恢复一个备份文件
  // restore_path 是恢复位置的文件夹路径
  Insidious<std::string> Restore(BackupFileId id, const fs::path &restore_path,
                                 const std::string &key = "");

  Maybe<BackupFile> GetBackupFile(BackupFileId id) {
    if (backup_files_.find(id) != backup_files_.end()) return Just(backup_files_[id]);
    return Nothing;
  }

 private:
  template <typename Json, typename BackupFileMap>
  Bolo(const std::string &config_path, Json &&config, BackupFileMap &&m, BackupFileId next_id,
       const std::string &backup_dir)
      : config_file_path_{config_path},
        config_(std::forward<Json>(config)),
        backup_files_(std::forward<BackupFileMap>(m)),
        next_id_(next_id),
        backup_dir_{backup_dir} {}

  // 更新配置文件:
  //     备份文件列表更新, 或者其他配置信息更新; 配置更新后, 必须将配置持久化成功后才返回 true
  Insidious<std::string> UpdateConfig();
  Insidious<std::string> BackupImpl(BackupFile &file, const std::string &key);
  BackupFileId NextId() { return next_id_++; }

  PropertyWithGetter(fs::path, config_file_path);  // 配置文件路径
  PropertyWithGetter(json, config);                // 配置
  PropertyWithGetter(BackupList, backup_files);    // 备份文件列表
  PropertyWithGetter(BackupFileId, next_id);       // 下一个备份文件 id
  PropertyWithGetter(fs::path, backup_dir);        // 备份文件夹路径
};
};  // namespace bolo
