#include "logging.hpp"

#include <spdlog/sinks/ringbuffer_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace watchman {

std::shared_ptr<spdlog::logger> Log::m_log;

class ImplementLogger : public Log {
public:
    ImplementLogger()
        : Log() {}
} m_spdlogger;

Log::Log() { init(""); }

void Log::init(std::string const & log_file, uint32_t concurrency, uint32_t sink) {
    std::array<spdlog::sink_ptr, toUnderlying(SinkType::Count)> sinks;
    size_t sinkCount{0};
    auto const isMt = (concurrency & toUnderlying(ConcurrencyType::MultiThreaded)) != 0;
    if ((sink & toUnderlying(SinkType::StdOut)) != 0) {
        if (isMt) {
            sinks[sinkCount] = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        } else {
            sinks[sinkCount] = std::make_shared<spdlog::sinks::stdout_color_sink_st>();
        }
        ++sinkCount;
    }
    if ((sink & toUnderlying(SinkType::StdErr)) != 0) {
        if (isMt) {
            sinks[sinkCount] = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
        } else {
            sinks[sinkCount] = std::make_shared<spdlog::sinks::stderr_color_sink_st>();
        }
        ++sinkCount;
    }

    if (!log_file.empty()) {
        sinks[sinkCount++] =
            std::make_shared<spdlog::sinks::rotating_file_sink_mt>(log_file, 1024 * 1024 * 10, 3);
    }
    if ((concurrency & toUnderlying(ConcurrencyType::Async)) != 0) {
        assert(!isMt);
        spdlog::init_thread_pool(8192, 1);
        m_log = std::make_shared<spdlog::async_logger>(
            "async_logger", sinks.begin(), sinks.begin() + sinkCount, spdlog::thread_pool(),
            spdlog::async_overflow_policy::block);
    } else {
        m_log = std::make_shared<spdlog::logger>("sync_logger", sinks.begin(),
                                                 sinks.begin() + sinkCount);
    }
}

}  // namespace watchman
