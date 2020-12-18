#pragma once

#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "result.h"

namespace bolo_tar {

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

  bolo::Insidious<std::string> Write();
  bolo::Insidious<std::string> Append(const std::filesystem::path &);
  bolo::Result<std::vector<TarFile>, std::string> List();

  // input path should be a directory
  bolo::Insidious<std::string> Extract(const std::filesystem::path &);

 private:
  explicit Tar(std::fstream &&fs) : fs_(std::move(fs)) {}
  bolo::Insidious<std::string> AppendImpl(const std::filesystem::path &,
                                          const std::filesystem::path &);
  bolo::Insidious<std::string> AppendFile(const std::filesystem::path &,
                                          const std::filesystem::path &);
  bolo::Insidious<std::string> AppendDirectory(const std::filesystem::path &,
                                               const std::filesystem::path &);
  bolo::Insidious<std::string> ExtractFile(const std::filesystem::path &, int);

 private:
  std::fstream fs_;
};

};  // namespace bolo_tar
