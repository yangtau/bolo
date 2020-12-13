#pragma once

#include <chrono>

#include "types.h"

#define PropertyWithGetter(type, var) \
 private:                             \
  type var##_;                        \
                                      \
 public:                              \
  const type &var() const { return var##_; }

namespace bolo {
inline Timestamp GetTimestamp() {
  using namespace std::chrono;
  auto now = high_resolution_clock::now();
  return static_cast<Timestamp>(duration_cast<nanoseconds>(now.time_since_epoch()).count());
}
};  // namespace bolo
