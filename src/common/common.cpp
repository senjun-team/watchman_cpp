#include "common.hpp"

#include "logging.hpp"
#include "tar_to_stream.hpp"


namespace watchman {
std::ostringstream makeTar(std::string const & sourceCode, std::string const & sourceTests) {
    std::ostringstream stream(std::ios::binary | std::ios::trunc);
    tar::tar_to_stream(stream, kFilenameTask, sourceCode.data(), sourceCode.size());
    tar::tar_to_stream(stream, kFilenameTaskTests, sourceTests.data(), sourceTests.size());
    tar::tar_to_stream_tail(stream);
    return stream;
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
