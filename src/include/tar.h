#include <string>
#include <vector>

#pragma once

namespace bolo {
bool Tar(const std::vector<std::string> &inputs, const std::string &output);
bool Untar(const std::string &input, const std::string &output);
};  // namespace bolo
