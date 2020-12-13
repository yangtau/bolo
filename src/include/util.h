#pragma once

#define PropertyWithGetter(type, var) \
 private:                             \
  type var##_;                        \
                                      \
 public:                              \
  const type &var() const { return var##_; }\
