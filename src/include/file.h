#pragma once

#include <string>

namespace bolo {
struct BackupConfig {
  bool is_compressed;    // 是否压缩
  bool is_cloud_backup;  // 是否云端备份
  bool is_encrypted;     // 是否加密
  // TODO(yangtau): 云备份信息 云目录
  // TODO(yangtau): 加密信息 密钥
};

struct BackupFile {
  std::string origin;  // 文件初始目录
  std::string backup;  // 本地备份目录
  BackupConfig config;
};
};  // namespace bolo
