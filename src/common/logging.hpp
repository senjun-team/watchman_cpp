#pragma once

#include <spdlog/spdlog.h>

namespace watchman {

template<typename Enum>
inline constexpr std::underlying_type_t<Enum> toUnderlying(Enum e) noexcept {
    return static_cast<std::underlying_type_t<Enum>>(e);
}

class Log {
    template<typename... Args>
    using FormatString = spdlog::format_string_t<Args...>;

public:
    enum class SinkType : uint32_t { StdOut = 1U << 0U, StdErr = 1U << 1U, Count = 2 };

    enum class ConcurrencyType : uint32_t {
        SingleThreaded = 1U << 0U,
        MultiThreaded = 1U << 1U,
        Async = 1U << 2U,
        Count = 3,
    };

    static void init(std::string const & log_file,
                     uint32_t concurrency = toUnderlying(Log::ConcurrencyType::SingleThreaded),
                     uint32_t sink = toUnderlying(Log::SinkType::StdOut));

    template<typename... Args>
    static void trace(FormatString<Args...> const & fmt, Args &&... args) {
        m_log->trace(fmt, std::forward<Args>(args)...);
    }
    template<typename... Args>
    static void trace(spdlog::source_loc loc, FormatString<Args...> const & fmt, Args &&... args) {
        m_log->log(loc, spdlog::level::trace, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void debug(FormatString<Args...> const & fmt, Args &&... args) {
        m_log->debug(fmt, std::forward<Args>(args)...);
    }
    template<typename... Args>
    static void debug(spdlog::source_loc loc, FormatString<Args...> const & fmt, Args &&... args) {
        m_log->log(loc, spdlog::level::debug, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void info(FormatString<Args...> const & fmt, Args &&... args) {
        m_log->info(fmt, std::forward<Args>(args)...);
    }
    template<typename... Args>
    static void info(spdlog::source_loc loc, FormatString<Args...> const & fmt, Args &&... args) {
        m_log->log(loc, spdlog::level::info, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void warning(FormatString<Args...> const & fmt, Args &&... args) {
        m_log->warn(fmt, std::forward<Args>(args)...);
    }
    template<typename... Args>
    static void warning(spdlog::source_loc loc, FormatString<Args...> const & fmt,
                        Args &&... args) {
        m_log->log(loc, spdlog::level::warn, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void error(FormatString<Args...> const & fmt, Args &&... args) {
        m_log->error(fmt, std::forward<Args>(args)...);
    }
    template<typename... Args>
    static void error(spdlog::source_loc loc, FormatString<Args...> const & fmt, Args &&... args) {
        m_log->log(loc, spdlog::level::err, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void critical(FormatString<Args...> const & fmt, Args &&... args) {
        m_log->critical(fmt, std::forward<Args>(args)...);
    }
    template<typename... Args>
    static void critical(spdlog::source_loc loc, FormatString<Args...> const & fmt,
                         Args &&... args) {
        m_log->log(loc, spdlog::level::critical, fmt, std::forward<Args>(args)...);
    }

    static void setLevel(spdlog::level::level_enum level) { m_log->set_level(level); }

    static void flushOn(spdlog::level::level_enum level) { m_log->flush_on(level); }

protected:
    Log();

private:
    static std::shared_ptr<spdlog::logger> m_log;
};

}  // namespace watchman
