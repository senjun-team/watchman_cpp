#include "service.hpp"

#include "common/logging.hpp"

namespace watchman {

struct StringContainers {
    static std::string_view constexpr python = "python";
    static std::string_view constexpr rust = "rust";
};

size_t constexpr kDockerTimeout = 124;
size_t constexpr kDockerMemoryKill = 137;

static std::string const kUserSourceFile = "/home/code_runner/solution";
static std::string const kUserSourceFileTests = "/home/code_runner/solution_tests";

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

    if (!runTaskParams.sourceTest.empty()) {
        if (auto result = container.runCode(runTaskParams.sourceTest); !result.isValid()) {
            return {.sourceCode = Response::kSuccessCode,
                    .testsCode = result.code,
                    .output = std::move(result.output)};
        }
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

detail::Container::DockerAnswer detail::Container::runCode(std::string const & code) {
    std::string const archive = "archive.tar";
    if (!makeTar(archive, code)) {
        Log::error("Internal error! Couldn't create an archive");
        return {};
    }

    if (!dockerWrapper.putArchive({id, kUserSourceFile, archive})) {
        Log::error("Internal error! Couldn't put an archive to container");
        return {};
    }

    std::vector<std::string> const args{"sh", "run.sh", "archive"};
    auto result = dockerWrapper.exec({args, id});
    if (!static_cast<bool>(std::remove(archive.c_str()))) {
        Log::error("Internal error! Couldn't remove an archive from host");
        return {};
    }

    return {result.exitCode, result.output};
}

detail::Container::DockerAnswer detail::Container::clean() {
    // TODO may be here we need to remove user code from container or in runCode
    return {.code = Response::kSuccessCode};
}

detail::Container::Container() = default;

bool detail::Container::DockerAnswer::isValid() const { return code == Response::kSuccessCode; }

}  // namespace watchman
