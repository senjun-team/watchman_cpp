#pragma once
#include "../core/service.hpp"
#include "parser.hpp"

#include <string>

namespace watchman {
class Server {
public:
    void start();

private:
    std::pair<Status, Text> processRequest(std::string const & body);
    void createResponse(std::pair<Status, Text> const & result);
    Service m_service;
    Parser m_parser;
};
}  // namespace watchman
