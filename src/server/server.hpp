#pragma once

#include "common/common.hpp"
#include "core/service.hpp"
#include "parser.hpp"

#include <string>

namespace watchman {
class Server {
public:
    explicit Server(Config && config);
    void start(size_t threadPoolSize);

private:
    std::string processRequest(std::string const & body);
    Service m_service;
};
}  // namespace watchman
