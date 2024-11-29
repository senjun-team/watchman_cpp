#pragma once


#include <chrono>

namespace watchman {

std::string const kFilenameTask = "task";
std::string const kFilenameTaskTests = "task_tests";

enum class Api { Check, Playground, Practice };

size_t getCpuCount();

struct CodeFilename {
    std::string code;
    std::string filename;
};

std::string makeTar(std::vector<CodeFilename> && data);

class LogDuration {
public:
    LogDuration(std::string operation);
    ~LogDuration();

private:
    std::string const m_operation;
    std::chrono::system_clock::time_point m_start;
};

}  // namespace watchman
