#pragma once

#include <chrono>
#include <string>
#include <vector>

namespace watchman {

std::string const kFilenameTask = "task";
std::string const kFilenameTaskTests = "task_tests";

enum class Api { Check, Playground, Practice };

inline std::string requiredApiField(Api api) {
    switch (api) {
    case Api::Check: return "courses";
    case Api::Playground: return "playground";
    case Api::Practice: return "practice";
    }

    return {};
}

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
