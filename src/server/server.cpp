#include "server.hpp"

#include "../common/common.hpp"

#include <docker.hpp>
#include <restinio/all.hpp>

namespace watchman {
void Server::start() {
    restinio::run(restinio::on_thread_pool(4).port(8050).address("0.0.0.0").request_handler(
        [this](restinio::request_handle_t const & req) -> restinio::request_handling_status_t {
            if (restinio::http_method_post() != req->header().method()) {
                return restinio::request_rejected();
            }

            auto const result = processRequest(req->body());
            req->create_response()
                .append_header(restinio::http_field::version,
                               std::to_string(req->header().http_major()))
                .append_header(restinio::http_field::content_type, "application/json")
                .append_header(restinio::http_field::status_uri,
                               std::to_string(restinio::status_code::ok.raw_code()))
                .set_body(result.output)
                .done();
            return restinio::request_accepted();
        }));
}

Response Server::processRequest(std::string const & body) {
    auto const params = m_parser.parse(body);
    return m_service.runTask(params);
}

}  // namespace watchman
