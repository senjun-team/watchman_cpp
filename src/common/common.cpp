#include "common.hpp"

#include "logging.hpp"
#include "structarus/tar_creator.hpp"

#include <sstream>
#include <thread>

namespace watchman {

size_t getCpuCount() {
    constexpr size_t kDefCpuCount = 4;
    const auto cpuCount = static_cast<size_t>(std::thread::hardware_concurrency());
    return cpuCount > 0 ? cpuCount : kDefCpuCount;
}

std::string makeTar(std::vector<CodeFilename> && data) {
    std::string stringTar;
    {
        tar::Creator<std::ostringstream> creator(stringTar);

        for (auto const & element : data) {
            if (!element.code.empty() && element.filename == kFilenameTask
                || element.filename == kFilenameTaskTests) {
                creator.addFile({element.filename, element.code, tar::FileType::RegularFile});
            }
        }
    }
    return stringTar;
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
