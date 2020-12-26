#define CATCH_CONFIG_MAIN

#include <catch.h>
#include <compress.h>
#include <test_util.h>

#include <fstream>
#include <iostream>
#include <string>

std::string test_dir = "__compress_test_dir__";

TEST_CASE("compress") {
  fs::create_directories(test_dir);
  fs::current_path(test_dir);

  // test1
  {
    // compress
    {
      WriteString("test1", Repeat("heellllo", 100));

      std::ifstream ifs("test1");
      std::ofstream ofs("test1.z");

      auto ins = bolo_compress::Compress(ifs, ofs, bolo_compress::Scheme::DEFLATE);
      if (ins) std::cerr << ins.error() << std::endl;
      REQUIRE(!ins);
    }

    // decompress
    {
      std::ifstream ifs("test1.z", std::ios_base::binary);
      std::ofstream ofs("test1.out", std::ios_base::binary);

      auto ins = bolo_compress::Uncompress(ifs, ofs, bolo_compress::Scheme::DEFLATE);
      if (ins) std::cerr << ins.error() << std::endl;
      REQUIRE(!ins);
    }

    auto s1 = fs::file_size("test1.z");
    auto s2 = fs::file_size("test1");
    std::cout << "origin/compress: " << s1 << "/" << s2 << "(" << s1 * 0.1 / s2 << ")" << std::endl;

    REQUIRE(CompareFiles("test1.out", "test1"));
  }

  fs::current_path("..");
  fs::remove_all(test_dir);
}
