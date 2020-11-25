#include <optional>

#include "buffer.h"

#pragma once

namespace bolo {
std::optional<Buffer> Compress(const Buffer &buf);
std::optional<Buffer> Uncompress(const Buffer &buf);
};  // namespace bolo
