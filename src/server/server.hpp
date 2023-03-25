#pragma once
#include "core/service.hpp"
#include "parser.hpp"

#include <string>

namespace watchman {
class Server {
public:
    Server(std::string const & dockerHost, std::string const & configPath);
    void start();

private:
    std::string processRequest(std::string const & body);
    size_t const kDockerTimeoutCode = 124;
    size_t const kDockerMemoryKill = 137;
    Service m_service;
};
}  // namespace watchman
