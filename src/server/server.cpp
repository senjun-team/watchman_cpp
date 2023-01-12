#include "server.hpp"

#include "../common/common.hpp"

#include <docker.hpp>
#include <restinio/all.hpp>
#include <iostream>

namespace watchman {
void Server::start() {
    restinio::run(restinio::on_thread_pool(4).port(8050).address("0.0.0.0").request_handler(
        [this](restinio::request_handle_t const & req) -> restinio::request_handling_status_t {
            if (restinio::http_method_post() != req->header().method()) {
                return restinio::request_rejected();
            }

            auto const result = processRequest(req->body());
            createResponse(result);
            return restinio::request_accepted();
        }));
}

std::pair<Status, Text> Server::processRequest(std::string const & body) {
    std::cout << "Post accepted\n";
}

void Server::createResponse(std::pair<Status, Text> const & result) {}

}  // namespace watchman
