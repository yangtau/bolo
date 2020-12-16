#include <catch.h>
#include <result.h>

#include <string>

using namespace bolo;

TEST_CASE("Result", "test") {
  int v{20};
  auto err_msg = std::string{"Unexpected Error"};
  auto square = [](int a) { return a * a; };

  Result<int, std::string> r1 = Ok(v);
  Result<int, std::string> r2 = Err(err_msg);

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
      if (a == 0)
        return Err<std::string>("get 0");
      else
        return Ok(a * 3.0);
    };

    auto r = r1 >>= fn;
    REQUIRE(static_cast<bool>(r));
    REQUIRE(r.value() == v * 3.0);

    Result<int, std::string> r3 = Ok(0);
    auto r4 = r3 >>= fn;
    REQUIRE(!r4);
    REQUIRE(r4.error() == "get 0");
  }

  SECTION("|") {
    REQUIRE((r1 | Ok(2)).value() == r1.value());
    REQUIRE((r1 | Err<std::string>("")).value() == r1.value());

    REQUIRE((r2 | Ok(2)).value() == 2);
    REQUIRE((r2 | Err<std::string>("")).value_or(0) == 0);
  }
  SECTION("&") {
    REQUIRE(!(r2 & Ok(2)));
    REQUIRE((r2 & Err<std::string>("")).error() == err_msg);

    REQUIRE((r1 & Ok(2)).value() == 2);
    REQUIRE((r1 & Err<std::string>("")).value_or(0) == 0);
  }

  SECTION("==") {
    REQUIRE(r1 == Ok(v));
    REQUIRE(r2 == Err(err_msg));
  }
}

TEST_CASE("Maybe", "test") {
  Maybe<int> m1 = Just(2);
  Maybe<int> m2 = Nothing;

  SECTION("value") {
    REQUIRE(static_cast<bool>(m1));
    REQUIRE(m1.value() == 2);

    REQUIRE(!m2);
    REQUIRE(m2.value_or(2) == 2);
  }

  SECTION("map map_or") {
    REQUIRE(m1.Map([](int a) { return a + 100; }).value() == 102);
    REQUIRE(m2.Map([](int a) { return a + 100; }) == Nothing);

    REQUIRE(m2.MapOr([](int a) { return a + 100; }, 100) == Just(100));
    REQUIRE(m1.MapOr([](int a) { return a + 100; }, 100) == Just(102));
  }

  SECTION(">>=") {
    auto div = [](int a) -> Maybe<int> {
      if (a == 0) return Nothing;
      return Just(a + 100);
    };

    REQUIRE((m1 >>= div) == Just(102));
    REQUIRE((Just(0) >>= div) == Nothing);
  }

  SECTION("|&") {
    REQUIRE((Just(20) | Nothing) == Just(20));
    REQUIRE((Just(20) | Just(30)) == Just(20));
    REQUIRE((Maybe<int>(Nothing) | Just(30)) == Just(30));
    REQUIRE((Maybe<int>(Nothing) | Nothing) == Nothing);

    REQUIRE((Just(20) & Just(30)) == Just(30));
    REQUIRE((Maybe<int>(Nothing) & Just(30)) == Nothing);
    REQUIRE((Just(30) & Nothing) == Nothing);
    REQUIRE((Maybe<int>(Nothing) & Nothing) == Nothing);
  }
}
