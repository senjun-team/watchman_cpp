#include "service.hpp"

#include "common/logging.hpp"

namespace watchman {

struct StringContainers {
    static std::string_view constexpr python = "python";
    static std::string_view constexpr rust = "rust";
};

size_t constexpr kDockerTimeout = 124;
size_t constexpr kDockerMemoryKill = 137;

std::string_view constexpr kUserSourceFile = "/home/code_runner/solution";
std::string_view constexpr kUserSourceFileTests = "/home/code_runner/solution_tests";

Service::Service(std::string const & host)
    : m_containerController(host) {}

detail::ContainerController::ContainerController(std::string host)
    : m_hostDocker(std::move(host)) {
    readConfig();
}

void detail::ContainerController::readConfig() {
    // TODO fill hash maps from config
}

detail::Container & detail::ContainerController::getReadyContainer(detail::Container::Type type) {
    std::unique_lock lock(m_mutex);
    auto & containers = m_containers.at(type);

    // TODO rethink this awful loop and condition
    while (true) {
        for (auto & container : containers) {
            if (!container.isReserved) {
                container.isReserved = true;
                return container;
            }
        }

        m_containerFree.wait(lock, [&containers]() {
            for (auto const & container : containers) {
                if (!container.isReserved) {
                    return true;
                }
            }

            return false;
        });
    }
}
void detail::ContainerController::containerReleased() { m_containerFree.notify_all(); }

Response watchman::Service::runTask(watchman::RunTaskParams const & runTaskParams) {
    auto const containerType = getContainerType(runTaskParams.containerType);
    if (containerType == detail::Container::Type::Unknown) {
        Log::warning("Unknown container type is provided");
        return {.sourceCode = Response::kInvalidCode,
                .testsCode = Response::kInvalidCode,
                .output = fmt::format("Error: unknown container type \'{}\'",
                                      runTaskParams.containerType)};
    }

    auto & container = getReadyContainer(containerType);  // here we have got a race
    if (auto result = container.runCode(runTaskParams.sourceRun); !result.isValid()) {
        return {.sourceCode = result.code,
                .testsCode = Response::kInvalidCode,
                .output = std::move(result.output)};
    }

    if (auto result = container.runCode(runTaskParams.sourceTest); !result.isValid()) {
        return {.sourceCode = Response::kSuccessCode,
                .testsCode = result.code,
                .output = std::move(result.output)};
    }

    if (auto result = container.clean(); !result.isValid()) {
        Log::error("Error while removing files for {}", container.id);
        return {.sourceCode = Response::kSuccessCode,
                .testsCode = Response::kSuccessCode,
                .output = std::move(result.output)};
    }
    m_containerController.containerReleased();

    return {Response::kSuccessCode, Response::kSuccessCode, "Success"};
}

detail::Container & Service::getReadyContainer(detail::Container::Type type) {
    return m_containerController.getReadyContainer(type);
}

detail::Container::Type Service::getContainerType(std::string const & type) {
    if (type == StringContainers::python) {
        return detail::Container::Type::Python;
    }

    if (type == StringContainers::rust) {
        return detail::Container::Type::Rust;
    }

    return detail::Container::Type::Unknown;
}

detail::Container::DockerAnswer detail::Container::runCode(std::string const & code) { return {}; }

detail::Container::DockerAnswer detail::Container::clean() { return {}; }

detail::Container::Container() = default;

bool detail::Container::DockerAnswer::isValid() const { return code == Response::kSuccessCode; }

}  // namespace watchman
