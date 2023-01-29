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
            sinks[sinkCount++] = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        } else {
            sinks[sinkCount++] = std::make_shared<spdlog::sinks::stdout_color_sink_st>();
        }
    }
    if ((sink & toUnderlying(SinkType::StdErr)) != 0) {
        if (isMt) {
            sinks[sinkCount++] = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
        } else {
            sinks[sinkCount++] = std::make_shared<spdlog::sinks::stderr_color_sink_st>();
        }
    }

    if (!log_file.empty()) {
        constexpr size_t kLogFileMaxSize = 1024 * 1024 * 10;
        constexpr size_t kRotatingLogFileCount = 3;
        sinks[sinkCount++] = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            log_file, kLogFileMaxSize, kRotatingLogFileCount);
    }
    if ((concurrency & toUnderlying(ConcurrencyType::Async)) != 0) {
        assert(!isMt);
        constexpr size_t kQueueSize = 8192;
        constexpr size_t kThreadCount = 8192;
        spdlog::init_thread_pool(kQueueSize, kThreadCount);
        m_log = std::make_shared<spdlog::async_logger>(
            "async_logger", sinks.begin(), sinks.begin() + sinkCount, spdlog::thread_pool(),
            spdlog::async_overflow_policy::block);
    } else {
        m_log = std::make_shared<spdlog::logger>("sync_logger", sinks.begin(),
                                                 sinks.begin() + sinkCount);
    }
}

}  // namespace watchman
