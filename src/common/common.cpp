#include "common.hpp"

#include "logging.hpp"
#include "tar_to_stream.hpp"

#include <boost/property_tree/json_parser.hpp>

#include <filesystem>
#include <thread>

namespace fs = std::filesystem;

namespace watchman {

size_t getCpuCount() {
    constexpr size_t kDefCpuCount = 4;
    const auto cpuCount = static_cast<size_t>(std::thread::hardware_concurrency());
    return cpuCount > 0 ? cpuCount : kDefCpuCount;
}

template<typename Ptree>
Config fillConfig(Ptree const & root) {
    Config config;

    auto const & threadPoolSize = root.get_child_optional("thread-pool-size");
    if (threadPoolSize.has_value()) {
        config.threadPoolSize = threadPoolSize.value().template get_value<size_t>();
    }

    auto const & maxContainersAmount = root.get_child_optional("max-containers-amount");
    if (!maxContainersAmount.has_value()) {
        Log::error("Required field \'max-containers-amount\' is absent");
        std::terminate();
    }
    config.maxContainersAmount = maxContainersAmount.value().template get_value<uint32_t>();

    auto const & courses = root.get_child_optional("courses");
    if (!courses.has_value()) {
        Log::error("Required field \'courses\' is absent");
        std::terminate();
    }

    for (auto const & course : courses.value()) {
        auto imageName = course.second.get_child_optional("image-name");
        if (!imageName.has_value()) {
            Log::error("Required field \'image-name\' is absent in {}", course.first);
            std::terminate();
        }

        auto const launched = course.second.get_child_optional("launched");
        if (!launched.has_value()) {
            Log::error("Required field \'launched\' is absent in {}", course.first);
            std::terminate();
        }

        auto const & containerType = course.first;

        config.languages.insert({containerType + "_check",
                                 {imageName.value().template get_value<std::string>(),
                                  launched.value().template get_value<uint32_t>()}});
    }

    auto const & playground = root.get_child_optional("playground");
    if (!playground.has_value()) {
        Log::error("Required field \'playground\' is absent");
        std::terminate();
    }

    for (auto const & language : playground.value()) {
        auto imageName = language.second.get_child_optional("image-name");
        if (!imageName.has_value()) {
            Log::error("Required field \'image-name\' is absent in {}", language.first);
            std::terminate();
        }

        auto const launched = language.second.get_child_optional("launched");
        if (!launched.has_value()) {
            Log::error("Required field \'launched\' is absent in {}", language.first);
            std::terminate();
        }

        auto const & containerType = language.first;

        config.playgrounds.insert({containerType + "_playground",
                                   {imageName.value().template get_value<std::string>(),
                                    launched.value().template get_value<uint32_t>()}});
    }
    return config;
}

Config readConfig(std::string_view configPath) {
    namespace pt = boost::property_tree;
    pt::ptree loadPtreeRoot;

    try {
        pt::read_json(configPath.data(), loadPtreeRoot);
        return fillConfig(loadPtreeRoot);
    } catch (std::exception const & error) {
        Log::error("Error while reading config file: {}", error.what());
        std::terminate();
    }
}

std::ostringstream makeTar(std::vector<CodeFilename> && data) {
    std::ostringstream stream(std::ios::binary | std::ios::trunc);

    for (auto const & element : data) {
        if (!element.code.empty() && element.filename == kFilenameTask
            || element.filename == kFilenameTaskTests) {
            tar::tar_to_stream(stream, element.filename, element.code.data(), element.code.size());
        }
    }

    tar::tar_to_stream_tail(stream);
    return stream;
}

void createFile(File const & file) {
    std::ofstream osFile(file.name);
    osFile << file.content;
    osFile.close();
}

void recursiveFillAbsolutePaths(Directory const & directory, fs::path const & currentPath,
                                std::vector<PathContent> & paths) {
    for (auto & file : directory.files) {
        paths.push_back({currentPath / file.name, file.content});
    }

    for (auto & subDirectory : directory.directories) {
        recursiveFillAbsolutePaths(subDirectory, currentPath / subDirectory.name, paths);
    }
}

std::vector<PathContent> getPathsToFiles(Directory const & rootDirectory) {
    std::vector<PathContent> paths;
    recursiveFillAbsolutePaths(rootDirectory, rootDirectory.name, paths);
    return paths;
}

void fillAbsolutePaths(Directory & rootDirectory) {}

LogDuration::LogDuration(std::string operation)
    : m_operation(std::move(operation))
    , m_start(std::chrono::system_clock::now()) {}

LogDuration::~LogDuration() {
    Log::info("Duration for {} is {} milliseconds", m_operation,
              std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()
                                                                    - m_start)
                  .count());
}

bool errorCodeIsUnexpected(ErrorCode code) {
    return code != kSuccessCode && code != kUserCodeError && code != kTestsError;
}

};  // namespace watchman
