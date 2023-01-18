#include <gtest/gtest.h>

#include "core/docker_wrapper.hpp"
#include "docker.hpp"
#include "rapidjson/document.h"
#include "server/server.hpp"

TEST(Simple, W) {
    rapidjson::Document document;
    Docker docker;
    watchman::Server server;
    watchman::DockerWrapper const dockerWrapper;
    auto const containers = dockerWrapper.getAllContainers();
}