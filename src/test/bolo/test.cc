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

bool CreateConfigFile(const std::string &path, const std::string &content) {
  std::ofstream f(path, std::ofstream::trunc | std::ofstream::out);
  if (!f) return false;

  f << content;
  return !!f;
}

bool DeleteFile(const std::string &path) { return fs::remove(path); }

const auto config_path = "config.json";

Bolo LoadConfig() {
  auto res = Bolo::LoadFromJsonFile(config_path);
  REQUIRE(static_cast<bool>(res));

  auto b = res.value();
  return b;
}

TEST_CASE("Bolo", "test") {
  std::vector<std::tuple<std::string, std::string, bool, bool>> cases{
      {"path/hello", "path/world", false, true},
      {"path/java", "path/python", true, true},
      {"path/c", "path/go", false, false},
      {"path/ruby", "path/lisp", false, false},
  };

  BackupList list;

  {
    // failed to load
    REQUIRE(CreateConfigFile(config_path, "{ \"backup_list\": [], \"next_\": 0 }"));
    auto res = Bolo::LoadFromJsonFile(config_path);
    REQUIRE(!res);

    res = Bolo::LoadFromJsonFile("!hello world");
    REQUIRE(!res);
  }

  {
    // load
    REQUIRE(CreateConfigFile(config_path, "{ \"backup_list\": [], \"next_id\": 0 }"));
    auto b = LoadConfig();
    REQUIRE(b.next_id() == 0);
    REQUIRE(b.backup_files().size() == 0);

    // backup
    for (auto &[origin, des, compressed, encrypted] : cases) {
      auto res = b.Backup(origin, des, compressed, encrypted);

      REQUIRE(static_cast<bool>(res));
      auto f = res.value();

      list[f.id] = f;

      REQUIRE(f.path == origin);
      REQUIRE(f.backup_path == des);
      REQUIRE(f.is_compressed == compressed);
      REQUIRE(f.is_encrypted == encrypted);

      auto m = b.GetBackupFile(f.id);
      REQUIRE(static_cast<bool>(m));
      REQUIRE(m.value() == f);
    }
  }

  {
    // update
    auto b = LoadConfig();

    for (auto &it : list) {
      REQUIRE(b.Update(it.first));

      auto m = b.GetBackupFile(it.first);
      REQUIRE(!!m);

      auto f = m.value();

      REQUIRE(f.timestamp > it.second.timestamp);
    }
  }

  {
    // remove
    auto b = LoadConfig();

    for (auto &it : list) {
      REQUIRE(b.Remove(it.first));

      auto m = b.Remove(it.first);
      REQUIRE(!m);
    }
  }

  REQUIRE(DeleteFile(config_path));
}
