#include <string>
#include <vector>

#include "result.h"

#pragma once

namespace bolo_tar {
enum class TarError {};
// @inputs: paths of input files
// @output: path of output file
// Returns Result<>(Ok(true)) on success; returns Result<>(Err(TarError)) on error
bolo::Result<bool, TarError> Tar(const std::vector<std::string> &inputs, const std::string &output);

// @input: the path of input file
// @ouput: the path directory to place files being tared
bolo::Result<bool, TarError> Untar(const std::string &input, const std::string &output);
};  // namespace bolo_tar
