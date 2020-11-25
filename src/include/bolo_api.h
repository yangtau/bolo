#pragma once
#include <optional>
#include <string>
#include <vector>

#include "file.h"

namespace bolo {
/// 获取所有备份文件
// Return: 如果成功，返回所有备份文件的信息
std::optional<std::vector<BackupFile>> GetBackup(const std::string& token);

/// 添加备份文件
// @path: 备份源文件目录
// @config: 备份配置
// Return: 如果成功，返回该备份文件生成的 BackupFile
std::optional<BackupFile> Backup(const std::string& token, const std::string& path,
                                 const BackupConfig& config);

/// 恢复文件备份
// Return: 返回是否成功
bool Restore(const BackupFile& file);
};  // namespace bolo
