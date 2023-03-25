#include "common.hpp"

#include "logging.hpp"
#include "tar_to_stream.hpp"

#include <fstream>

namespace watchman {
bool makeTar(std::string_view archiveName, std::string_view sourceCode) {
    std::ofstream stream(archiveName.data(), std::ios::binary | std::ios::trunc);
    if (!stream.is_open()) {
        Log::error("Failed creating tar file: {}", archiveName.data());
        return false;
    }

    constexpr size_t kSuffix = 4;
    std::string const tempName = std::string{archiveName}.substr(0, archiveName.size() - kSuffix);
    tar::tar_to_stream(stream, tempName, sourceCode.data(), sourceCode.size());
    tar::tar_to_stream_tail(stream);
    return true;
}

LogDuration::LogDuration(std::string operation)
    : m_operation(std::move(operation))
    , m_start(std::chrono::system_clock::now()) {}

LogDuration::~LogDuration() {
    Log::info("Duration for {} is {} milliseconds", m_operation,
              std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()
                                                                    - m_start)
                  .count());
}
};  // namespace watchman
