#include <optional>

#include "buffer.h"

#pragma once

namespace bolo_compress {
class Compressor {
  virtual std::optional<bolo_buffer::Buffer> Compress(const bolo_buffer::Buffer &buf);
};
};  // namespace bolo_compress
