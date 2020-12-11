#define CATCH_CONFIG_MAIN

#include <catch.h>
#include <result.h>

#include <string>

using namespace bolo;

TEST_CASE("Result", "test") {
  int v{20};
  auto err_msg = std::string{"Unexpected Error"};
  auto square = [](int a) { return a * a; };
  auto div3 = [](int a) -> double { return a / 3.0; };

  auto r1 = Result<int, std::string>::Ok(v);
  auto r2 = Result<int, std::string>::Err(err_msg);

  SECTION("bool") {
    REQUIRE(static_cast<bool>(r1));

    REQUIRE(static_cast<bool>(r2) == false);
    REQUIRE(!r2);
  }

  SECTION("value") { REQUIRE(r1.value() == v); }
  SECTION("value_or") { REQUIRE(r2.value_or(v) == v); }
  SECTION("error") { REQUIRE(r2.error() == err_msg); }
  SECTION("map") {
    auto r = r1.Map(square).Map(square);
    ;
    REQUIRE(r.value() == v * v * v * v);

    auto r3 = r2.Map(square);

    REQUIRE(!r3);
    REQUIRE(r3.error() == err_msg);
    REQUIRE(r3.value_or(v) == v);
  }

  SECTION("map_or") {
    auto r = r2.MapOr(square, v);
    REQUIRE(static_cast<bool>(r));

    REQUIRE(r2.Map(square).MapOr(square, v).value() == v);
  }

  SECTION(">>=") {
    auto fn = [](int a) -> Result<double, std::string> {
      using R = Result<double, std::string>;
      return a == 0 ? R::Err("get 0") : R::Ok(a * 3.0);
    };

    auto r = r1 >>= fn;
    REQUIRE(static_cast<bool>(r));
    REQUIRE(r.value() == v * 3.0);

    auto r3 = Result<int, std::string>::Ok(0);
    auto r4 = r3 >>= fn;
    REQUIRE(!r4);
    REQUIRE(r4.error() == "get 0");
  }
}
