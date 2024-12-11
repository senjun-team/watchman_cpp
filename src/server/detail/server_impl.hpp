#pragma once

#include "common/config.hpp"
#include "core/service.hpp"

#include <restinio/all.hpp>
#include <unifex/async_scope.hpp>
#include <unifex/static_thread_pool.hpp>

namespace watchman::detail {

class AsyncContext {
public:
    explicit AsyncContext(size_t poolSize);
    void schedule(std::function<void()> f);

private:
    unifex::static_thread_pool m_threadPool;
    unifex::async_scope m_scope;
};

class ServerImpl {
public:
    explicit ServerImpl(Config && config);
    ~ServerImpl();
    void start();

private:
    std::string processRequest(std::string_view handle, std::string const & body);
    void internalProcessRequest(restinio::request_handle_t req);

    // handles
    std::string processCheck(std::string const & body);
    std::string processPlayground(std::string const & body);
    std::string processPractice(std::string const & body);

    Service m_service;
    AsyncContext m_context;
};
}  // namespace watchman::detail
