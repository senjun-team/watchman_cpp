#pragma once
#include "core/service.hpp"
#include "parser.hpp"

#include <string>

namespace watchman {
class Server {
public:
    void start();

private:
    Response processRequest(std::string const & body);
    Service m_service;
};
}  // namespace watchman
