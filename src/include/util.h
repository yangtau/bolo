#pragma once

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <filesystem>
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

inline std::string MakeTemp() {
  auto dir = std::filesystem::temp_directory_path() / "bolo_XXXXXXXXX";
  char buf[512];
  std::strncpy(buf, dir.c_str(), sizeof(buf));

  mktemp(buf);
  return buf;
}

};  // namespace bolo
