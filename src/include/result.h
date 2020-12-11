#pragma once

#include <functional>
#include <type_traits>
#include <variant>

namespace bolo {

// Either
template <typename T, typename U>
class Either : private std::variant<T, U> {
 public:
  Either(const T &t) : std::variant<T, U>(t) {}
  Either(T &&t) : std::variant<T, U>(std::move(t)) {}
  Either(const U &u) : std::variant<T, U>(u) {}
  Either(U &&u) : std::variant<T, U>(std::move(u)) {}

  bool HoldsLeft() const { return std::holds_alternative<T>(*this); }
  bool HoldsRight() const { return std::holds_alternative<U>(*this); }

  const T &left() const { return std::get<T>(*this); }
  const U &right() const { return std::get<U>(*this); }
};

// Ok
template <typename T>
struct Ok {
  T value;
  explicit Ok(const T &t) : value{t} {}
  explicit Ok(T &&t) : value{std::move(t)} {}
};

// Err
template <typename E>
struct Err {
  E value;
  explicit Err(const E &e) : value{e} {}
  explicit Err(E &&e) : value{std::move(e)} {}
};

// Result
template <typename T, typename E>
class Result : private Either<Ok<T>, Err<E>> {
  using Ei = Either<Ok<T>, Err<E>>;

 public:
  Result(const Ok<T> &ok) : Ei(ok) {}
  Result(Ok<T> &&ok) : Ei(std::move(ok)) {}
  Result(const Err<E> &err) : Ei(err) {}
  Result(Err<E> &&err) : Ei(std::move(err)) {}

  explicit operator bool() const { return this->HoldsLeft(); }
  bool operator==(const Result<T, E> &r) const {
    if (r && *this) return r.value() == this->value();
    if (!r && !*this) return r.error() == this->error();  // Nothing
    return false;
  }

  const T &value() const { return this->left().value; }
  const T &value_or(const T &default_value) const { return *this ? value() : default_value; }

  E error() const { return this->right().value; }

  // Function: T -> U
  // Map: Function -> Result<U, E>
  template <typename Function, typename U = typename std::result_of<Function(T)>::type>
  auto Map(Function fn) const -> Result<U, E> {
    if (this->HoldsLeft()) return Ok(fn((value())));
    return Err(error());
  }

  // Function: T -> U
  // Map: Function -> U -> Result<U, E>
  template <typename Function, typename U = typename std::result_of<Function(T)>::type>
  auto MapOr(Function fn, const U &default_value) const -> Result<U, E> {
    if (this->HoldsLeft()) return Ok(fn((value())));
    return Ok(default_value);
  }

  // Function: T -> Result<U, E>
  // >>=: Function -> Result<U, E>
  template <typename Function, typename R = typename std::result_of<Function(T)>::type>
  auto operator>>=(Function fn) const -> R {
    if (*this) return fn((value()));
    return Err(error());
  }

  // Returns res if the result is Err, otherwise returns this
  Result<T, E> operator|(const Result<T, E> &res) const { return *this ? *this : res; }

  // Returns res if the result is Ok, otherwise return this
  Result<T, E> operator&(const Result<T, E> &res) const { return *this ? res : *this; }
};

// Nothing
namespace details {
struct nothing_t {};
};  // namespace details

constexpr details::nothing_t Nothing;

// Maybe
template <typename T>
class Maybe : private Either<T, details::nothing_t> {
  using E = Either<T, details::nothing_t>;

 public:
  explicit Maybe(const T &t) : E(t) {}
  explicit Maybe(T &&t) : E(std::move(t)) {}
  Maybe(const details::nothing_t &) : E(Nothing) {}

  explicit operator bool() const { return this->HoldsLeft(); }
  bool operator==(const Maybe<T> &m) const {
    if (m && *this) return m.value() == this->value();
    if (!m && !*this) return true;  // Nothing
    return false;
  }

  const T &value() const { return this->left(); }
  const T &value_or(const T &default_value) const { return *this ? value() : default_value; }

  // Map: Function(T->U) -> Maybe<U>
  template <typename Function, typename U = typename std::result_of<Function(T)>::type>
  auto Map(Function fn) -> Maybe<U> {
    if (this->HoldsLeft()) return Maybe<U>(fn(value()));
    return *this;  // Nothing
  }

  // Function: T -> U
  // MapOr: Function -> U -> Maybe<U>
  template <typename Function, typename U = typename std::result_of<Function(T)>::type>
  auto MapOr(Function fn, const U &u) -> Maybe<U> {
    if (this->HoldsLeft()) return Maybe<U>(fn(value()));
    return Maybe<U>(u);
  }

  // >>=: Function (T -> Maybe<U>) -> Maybe<U>
  template <typename Function, typename M = typename std::result_of<Function(T)>::type>
  auto operator>>=(Function fn) const -> M {
    if (*this) return fn((value()));
    return Nothing;
  }

  // Returns res if there is Nothing, otherwise returns this
  Maybe<T> operator|(const Maybe<T> &res) const { return *this ? *this : res; }

  // Returns res if the is Just something, otherwise return this
  Maybe<T> operator&(const Maybe<T> &res) const { return *this ? res : *this; }
};

template <typename T>
inline Maybe<T> Just(const T &t) {
  return Maybe<T>(t);
}

template <typename T>
inline Maybe<T> Just(T &&t) {
  return Maybe<T>(std::move(t));
}

;

};  // namespace bolo
