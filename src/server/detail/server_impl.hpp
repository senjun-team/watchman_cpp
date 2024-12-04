#pragma once

#include "common/config.hpp"
#include "core/service.hpp"

namespace watchman::detail {

class ServerImpl {
public:
    explicit ServerImpl(Config && config);
    ~ServerImpl();
    void start(size_t threadPoolSize);

    std::string processRequest(std::string_view handle, std::string const & body);

private:
    // handles
    std::string processCheck(std::string const & body);
    std::string processPlayground(std::string const & body);
    std::string processPractice(std::string const & body);

    Service m_service;
};
}  // namespace watchman::detail
