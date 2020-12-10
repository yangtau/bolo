#pragma once

#include <functional>
#include <utility>
#include <variant>

namespace bolo {

template <typename T, typename E>
class Result {
 public:
  static auto Ok(const T& t) { return Result(t); }
  static auto Ok(T&& t) { return Result(std::move<T>(t)); }
  static auto Err(const E& e) { return Result(e); }
  static auto Err(E&& e) { return Result(std::move<E>(e)); }
  explicit operator bool() const { return std::holds_alternative<T>(result_); }
  T value() const { return std::get<T>(result_); }
  T value_or(T default_value) const { return *this ? std::get<T>(result_) : default_value; }
  E error() const { return std::get<E>(result_); }

  template <typename Function>
  auto Map(Function fn) const -> Result<decltype(fn(T())), E> {
    if (*this) {
      return Result(fn(value()));
    } else {
      return Result(error());
    }
  }

  template <typename Function>
  auto MapOr(Function fn, decltype(fn(T())) default_value) const {
    using U = decltype(fn(T()));
    if (*this) {
      return Result(fn(value()));
    } else {
      return Result<U, E>(default_value);
    }
  }

  template <typename Function>
  auto operator>>=(Function fn) const -> decltype(Map(fn)) {
    return Map(fn);
  }

 private:
  explicit Result(T t) : result_{t} {
    static_assert(!std::is_same<T, E>::value, "Result: is_same<T, E> == true");
  }

  explicit Result(E e) : result_{e} {
    static_assert(!std::is_same<T, E>::value, "Result: is_same<T, E> == true");
  }

  const std::variant<T, E> result_;
};
};  // namespace bolo
