#pragma once

#include <ctime>

#include "types.h"

#define PropertyWithGetter(type, var) \
 private:                             \
  type var##_;                        \
                                      \
 public:                              \
  const type &var() const { return var##_; }

namespace bolo {
inline Timestamp GetTimestamp() { return static_cast<Timestamp>(std::time(NULL)); }
};  // namespace bolo
