#pragma once

#include <type_traits>

namespace bolo {
template <typename...>
using void_t = void;

template <class T, class R = void, class = void>
struct is_callable : std::false_type {};

template <class T>
struct is_callable<T, void, void_t<std::result_of_t<T>>> : std::true_type {};

template <class T, class R>
struct is_callable<T, R, void_t<std::result_of_t<T>>>
    : std::is_convertible<std::result_of_t<T>, R> {};

template <typename TSignature>
class function_view;

template <typename TReturn, typename... TArgs>
class function_view<TReturn(TArgs...)> final {
 private:
  using signature_type = TReturn(void*, TArgs...);

  void* _ptr;
  TReturn (*_erased_fn)(void*, TArgs...);

 public:
  template <typename T, typename = std::enable_if_t<is_callable<T&(TArgs...)>{}>>
  function_view(T&& x) noexcept : _ptr{(void*)std::addressof(x)} {
    _erased_fn = [](void* ptr, TArgs... xs) -> TReturn {
      return (*reinterpret_cast<std::add_pointer_t<T>>(ptr))(std::forward<TArgs>(xs)...);
    };
  }

  decltype(auto) operator()(TArgs... xs) const
      noexcept(noexcept(_erased_fn(_ptr, std::forward<TArgs>(xs)...))) {
    return _erased_fn(_ptr, std::forward<TArgs>(xs)...);
  }
};
};  // namespace bolo
