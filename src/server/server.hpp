#pragma once

#include "core/service.hpp"

#include <string>

namespace watchman {
class Server {
public:
    explicit Server(Config && config);
    void start(size_t threadPoolSize);

private:
    std::string processRequest(std::string_view handle, std::string const & body);

    // handles
    std::string processCheck(std::string const & body);
    std::string processPlayground(std::string const & body);
    std::string processPractice(std::string const & body);

    Service m_service;
};
}  // namespace watchman
