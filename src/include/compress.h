#pragma once

#include <istream>

#include "result.h"

namespace bolo_compress {
enum class Scheme {
  DEFLATE,
};

bolo::Maybe<std::iostream> Compress(std::iostream &buf, Scheme s);
bolo::Maybe<std::iostream> Uncompress(std::iostream &buf, Scheme s);
};  // namespace bolo_compress
