#pragma once

#include <string>

#include "lib/jsonlib.h"
#include "types.h"

namespace bolo {
struct BackupFile {
  std::string path;         // 文件初始目录
  std::string backup_path;  // 本地备份目录
  Timestamp timestamp;      // 备份的时间
  BackupFileId id;
  bool is_compressed;       // 是否压缩
  bool is_encrypted;        // 是否加密
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(BackupFile, path, backup_path, timestamp, is_compressed,
                                   is_encrypted);
};  // namespace bolo
