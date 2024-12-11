#include "server.hpp"

#include "server/detail/server_impl.hpp"

namespace watchman {

Server::Server(Config && config)
    : m_impl(std::make_unique<detail::ServerImpl>(std::move(config))) {}

Server::~Server() = default;

void Server::start() { m_impl->start(); }

}  // namespace watchman
