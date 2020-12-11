#pragma once
#include <iostream>

#include "result.h"

namespace bolo_encrypt {
enum class Scheme {
  Xor,
};

bolo::Maybe<std::iostream> Encrypt(std::iostream &buf, Scheme s);
bolo::Maybe<std::iostream> Dencrypt(std::iostream &buf, Scheme s);
};  // namespace bolo_encrypt
