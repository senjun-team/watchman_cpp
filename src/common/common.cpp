#include "common.hpp"

#include "tar_to_stream.h"

#include <fstream>

namespace watchman {
void makeTar(std::string_view archiveName, std::string_view sourceCode) {
    std::ofstream stream(archiveName.data(), std::ios::binary);
    constexpr size_t kSuffix = 4;
    std::string const tempName = std::string{archiveName}.substr(0, archiveName.size() - kSuffix);
    tar::tar_to_stream(stream, tempName, sourceCode.data(), sourceCode.size());
    tar::tar_to_stream_tail(stream);
}

};  // namespace watchman
