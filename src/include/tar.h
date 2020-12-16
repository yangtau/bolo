#pragma once

#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "result.h"

namespace bolo_tar {

class Tar;
using TarResult = bolo::Result<bool, std::string>;

class Tar {
 public:
  struct TarFile {
    std::string filename;
    int size;
    std::filesystem::perms perms;
    std::filesystem::file_type type;
  };

  static bolo::Result<std::shared_ptr<Tar>, std::string> Open(const std::filesystem::path &);

  Tar(Tar &&t) : fs_(std::move(t.fs_)) {}
  Tar(const Tar &) = delete;

  TarResult Write();
  TarResult Append(const std::filesystem::path &);
  bolo::Result<std::vector<TarFile>, std::string> List();

  // input path should be a directory
  TarResult Extract(const std::filesystem::path &);

 private:
  explicit Tar(std::fstream &&fs) : fs_(std::move(fs)) {}
  TarResult AppendImpl(const std::filesystem::path &, const std::filesystem::path &);
  TarResult AppendFile(const std::filesystem::path &, const std::filesystem::path &);
  TarResult AppendDirectory(const std::filesystem::path &, const std::filesystem::path &);
  TarResult ExtractFile(const std::filesystem::path &, int);

 private:
  std::fstream fs_;
};

};  // namespace bolo_tar
