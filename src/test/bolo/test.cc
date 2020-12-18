#include <cstring>
#include <iostream>
#include <memory>
#define CATCH_CONFIG_MAIN
#include <catch.h>

#include <filesystem>
#include <fstream>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "bolo.h"

namespace fs = std::filesystem;
using namespace bolo;

const auto config_path = "config.json";
std::vector<std::tuple<std::string, bool, bool>> cases{
    {"hello/hello.txt", false, true},
    {"java.txt", true, true},
    {"hello/a/python.txt", false, false},
    {"path/ruby.txt", false, false},
};

std::string repeat(std::string input, int cnt) {
  std::string s;
  while (cnt--) s += input;
  return s;
}

std::unordered_map<std::string, std::string> content{
    {"hello/hello.txt", repeat("你好世界, 真不戳!\n", 20)},
    {"java.txt", repeat("Java is the best language", 50)},
    {"hello/a/python.txt", ""},
    {"path/ruby.txt", repeat("ruby ruby?", 10)},

};

bool CreateConfigFile(const std::string &content) {
  std::ofstream f(config_path, std::ofstream::trunc | std::ofstream::out);
  if (!f) return false;

  f << content;
  return !!f;
}

bool RemoveConfig() { return fs::remove(config_path); }

bool CreateFiles() {
  fs::create_directory("bolo_test_dir");
  fs::current_path("bolo_test_dir");

  for (auto &it : content) {
    fs::path path(it.first);
    if (path.parent_path() != fs::path("")) fs::create_directories(path.parent_path());

    std::ofstream ofs(path, std::ios_base::trunc | std::ofstream::out);
    ofs << it.second;
    if (!ofs.good()) return false;
  }

  return true;
}

bool CompareFiles(const fs::path &a, const fs::path &b) {
  if (fs::file_size(a) != fs::file_size(b)) {
    std::cerr << "size of " << a << " is different from size of " << b << std::endl;
    return false;
  }
  std::ifstream fa(a);
  std::ifstream fb(b);

  while (fa.good() && fb.good()) {
    constexpr int len = 40;
    char bufa[len] = {0};
    char bufb[len] = {0};

    fa.read(bufa, len);
    fb.read(bufb, len);

    if (std::memcmp(bufb, bufa, len) != 0) {
      for (int i = 0; i < len; i++) {
        std::cerr << "<" << std::hex << bufa[i] << "," << std::hex << bufb[i] << ">";
      }
      return false;
    }
  }

  return fa.eof() && fb.eof();
}

bool ReadString(const fs::path &p, std::string &s) {
  std::ifstream ifs(p);
  while (ifs.good()) {
    char buf[1024] = {0};
    ifs.read(buf, 1024);

    s += buf;
  }

  return ifs.eof();
}

void DeleteFiles() {
  fs::current_path("..");
  fs::remove_all("bolo_test_dir");
}

TEST_CASE("Bolo-error", "error") {
  // failed to load
  REQUIRE(CreateConfigFile("{ \"backup_list\": [], \"next_\": 0 ,\"backup_path\":\"hello\"}"));
  auto res = Bolo::LoadFromJsonFile(config_path);
  REQUIRE(!res);

  res = Bolo::LoadFromJsonFile("!hello world");
  REQUIRE(!res);

  REQUIRE(RemoveConfig());
}

TEST_CASE("Bolo", "test") {
  BackupList list;

  REQUIRE(CreateFiles());
  REQUIRE(
      CreateConfigFile("{ \"backup_list\": [], \"next_id\": 0,\"backup_dir\":\"backup_path/\" }"));

  {
    // load
    auto bolo_res = Bolo::LoadFromJsonFile(config_path);
    if (!bolo_res) std::cerr << bolo_res.error() << std::endl;
    REQUIRE(!!bolo_res);
    auto b = std::move(bolo_res.value());

    REQUIRE(b->next_id() == 0);
    REQUIRE(b->backup_files().size() == 0);
    REQUIRE(b->backup_dir() == "backup_path/");

    // backup
    for (auto &[origin, compressed, encrypted] : cases) {
      auto res = b->Backup(origin, compressed, encrypted);
      if (!res) std::cout << res.error() << std::endl;
      REQUIRE(!!(res));
      auto f = res.value();

      list[f.id] = f;

      REQUIRE(f.path == origin);
      REQUIRE(f.is_compressed == compressed);
      REQUIRE(f.is_encrypted == encrypted);

      auto m = b->GetBackupFile(f.id);
      REQUIRE(!!m);
      REQUIRE(m.value() == f);
    }
  }

  fs::path restore_dir = fs::path("restore");
  {
    // restore
    auto bolo_res = Bolo::LoadFromJsonFile(config_path);
    if (!bolo_res) std::cout << bolo_res.error() << std::endl;
    REQUIRE(!!bolo_res);
    auto b = std::move(bolo_res.value());

    fs::create_directory(restore_dir);
    for (auto &it : list) {
      auto file = restore_dir / it.second.filename;
      auto ins = b->Restore(it.first, restore_dir);
      if (ins) std::cerr << ins.error() << std::endl;
      REQUIRE(!ins);

      CompareFiles(file, it.second.path);
    }
  }

  {
    // update
    auto bolo_res = Bolo::LoadFromJsonFile(config_path);
    if (!bolo_res) std::cout << bolo_res.error() << std::endl;
    REQUIRE(!!bolo_res);
    auto b = std::move(bolo_res.value());

    for (auto &it : list) {
      auto ins = b->Update(it.first);
      if (ins) std::cerr << ins.error() << std::endl;
      REQUIRE(!ins);

      auto m = b->GetBackupFile(it.first);
      REQUIRE(!!m);

      auto f = m.value();

      REQUIRE(f.timestamp >= it.second.timestamp);

      it.second = f;
    }
  }

  {
    // remove
    auto bolo_res = Bolo::LoadFromJsonFile(config_path);
    REQUIRE(!!bolo_res);
    auto b = std::move(bolo_res.value());

    for (auto &it : list) {
      auto ins = b->Remove(it.first);
      if (ins) std::cerr << ins.error() << std::endl;
      REQUIRE(!ins);
    }
  }

  DeleteFiles();
}
