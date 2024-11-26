#pragma once

#include "common/config.hpp"

namespace watchman::detail {
class ServerImpl;
}

namespace watchman {
class Server {
public:
    explicit Server(Config && config);
    ~Server();

    void start(size_t threadPoolSize);

private:
    std::unique_ptr<detail::ServerImpl> m_impl;
};
}  // namespace watchman
