#pragma once

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>

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
  auto now = system_clock::now();
  return static_cast<Timestamp>(duration_cast<microseconds>(now.time_since_epoch()).count());
}

inline std::string TimestampToString(Timestamp stamp) {
  namespace ch = std::chrono;

  ch::microseconds s{static_cast<ch::microseconds>(stamp)};
  ch::system_clock::time_point p{ch::duration{s}};

  auto t = ch::system_clock::to_time_t(p);

  std::stringstream ss;
  ss << std::put_time(std::localtime(&t), "%F %T");
  return ss.str();
}

};  // namespace bolo
