#include "service.hpp"

#include "common/logging.hpp"

namespace watchman {

size_t constexpr kDockerTimeout = 124;
size_t constexpr kDockerMemoryKill = 137;

std::string_view constexpr kUserSourceFile = "/home/code_runner/solution";
std::string_view constexpr kUserSourceFileTests = "/home/code_runner/solution_tests";

Service::Service(std::string const & host)
    : m_containerController(host) {}

detail::ContainerController::ContainerController(std::string const & host)
    : m_hostDocker(host) {
    readConfig();
}

void detail::ContainerController::readConfig() {
    // TODO fill hash maps from config
}

Response watchman::Service::runTask(watchman::RunTaskParams const & runTaskParams) {
    auto container = getReadyContainer(getContainerType(runTaskParams.containerType));
    if (auto result = container.runCode(runTaskParams.sourceRun); result.code != 0) {
        return {.sourceCode = result.code, .output = std::move(result.output)};
    }

    if (auto result = container.runCode(runTaskParams.sourceTest); result.code != 0) {
        return {.testsCode = result.code, .output = std::move(result.output)};
    }

    if (auto result = container.clean(); result.code != 0) {
        Log::error("Error while removing files for {}", container.id);
        return {.sourceCode = 0, .testsCode = 0, .output = std::move(result.output)};
    }

    return {0, 0, "Success"};
}

detail::Container Service::getReadyContainer(detail::Container::Type type) { return {}; }

detail::Container::Type Service::getContainerType(const std::string & type) {
    return detail::Container::Type::Python;
}

detail::Container::DockerAnswer detail::Container::runCode(std::string const & code) { return {}; }

detail::Container::DockerAnswer detail::Container::clean() { return {}; }

detail::Container::Container() = default;
}  // namespace watchman
