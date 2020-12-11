#pragma once

#include <functional>
#include <utility>
#include <variant>

namespace bolo {

template <typename T, typename E>
class Result {
 public:
  static auto Ok(const T& t) { return Result(t); }
  static auto Ok(T&& t) { return Result(std::move(t)); }
  static auto Err(const E& e) { return Result(e); }
  static auto Err(E&& e) { return Result(std::move(e)); }
  explicit operator bool() const { return std::holds_alternative<T>(result_); }
  T value() const { return std::get<T>(result_); }
  T value_or(T default_value) const { return *this ? std::get<T>(result_) : default_value; }
  E error() const { return std::get<E>(result_); }

  // Function: T -> U
  // Map: Function -> Result<U, E>
  template <typename Function>
  auto Map(Function fn) const -> Result<decltype(fn(T())), E> {
    return *this ? Ok(fn(value())) : Err(error());
  }

  // Function: T -> U
  // Map: Function -> U -> Result<U, E>
  template <typename Function>
  auto MapOr(Function fn, decltype(fn(T())) default_value) const -> Result<decltype(fn(T())), E> {
    using U = decltype(fn(T()));
    return *this ? Result<U, E>::Ok(fn(value())) : Result<U, E>::Ok(default_value);
  }

  // Function: T -> Result<U, E>
  // >>=: Function -> Result<U, E>
  template <typename Function>
  auto operator>>=(Function fn) const -> decltype(fn(T())) {
    using U = decltype(fn(T()));  // Result<U, E>
    return *this ? fn((value())) : U::Err(error());
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
