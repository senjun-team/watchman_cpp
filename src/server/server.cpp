#include "server.hpp"

#include "common/common.hpp"
#include "common/logging.hpp"

#include <restinio/all.hpp>

namespace watchman {

constexpr size_t kThreadCount = 4;
constexpr size_t kPort = 8000;
std::string const kIpAddress = "0.0.0.0";

Server::Server(std::string const & dockerHost, std::string const & configPath)
    : m_service(dockerHost, configPath) {}

void Server::start() {
    Log::info("Watchman working on {} port", kPort);
    restinio::run(restinio::on_thread_pool(kThreadCount)
                      .port(kPort)
                      .address(kIpAddress)
                      .request_handler([this](restinio::request_handle_t const & req)
                                           -> restinio::request_handling_status_t {
                          LogDuration duration("Request handling");
                          if (restinio::http_method_post() != req->header().method()) {
                              Log::error("error while handling: {}", req->body());
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

                          Log::info("request handled successfully: {}", result.output);
                          return restinio::request_accepted();
                      }));
}

Response Server::processRequest(std::string const & body) {
    Log::info("Processing body:\n {}", body);
    auto const params = parse(body);
    if (params.containerType.empty()) {
        return {};
    }

    if (params.sourceRun.empty()) {
        return {};
    }

    return m_service.runTask(params);
}

}  // namespace watchman
