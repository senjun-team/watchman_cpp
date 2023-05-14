#include "server.hpp"

#include "common/common.hpp"
#include "common/logging.hpp"

#include <restinio/all.hpp>

namespace watchman {

constexpr size_t kPort = 8000;
std::string const kIpAddress = "0.0.0.0";

Server::Server(std::string const & dockerHost, Config && config)
    : m_service(dockerHost, std::move(config)) {}

void Server::start(size_t threadPoolSize) {
    Log::info("Watchman working on {} port with the {} threads", kPort, threadPoolSize);
    restinio::run(restinio::on_thread_pool(threadPoolSize)
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
                              .set_body(result)
                              .done();

                          Log::info("request handled successfully: {}", result);
                          return restinio::request_accepted();
                      }));
}

std::string Server::processRequest(std::string const & body) {
    Log::info("Processing body:\n {}", body);
    auto const params = parse(body);
    if (params.containerType.empty()) {
        return {};
    }

    if (params.sourceRun.empty()) {
        return {};
    }

    auto const makeJson = [this](Response && response) -> std::string {
        rapidjson::StringBuffer stringBuffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(stringBuffer);

        writer.StartObject();
        writer.Key("error_code");
        writer.Int64(response.sourceCode);

        if (response.sourceCode == kDockerTimeoutCode) {
            writer.Key("output");
            writer.String("Timeout");
            writer.EndObject();
            return stringBuffer.GetString();
        }

        if (response.sourceCode == kDockerMemoryKill) {
            writer.Key("output");
            writer.String("Out of memory");
            writer.EndObject();
            return stringBuffer.GetString();
        }

        writer.Key("output");
        writer.String(response.output);

        if (response.sourceCode == 0) {
            writer.Key("tests_error_code");
            writer.Int64(response.testsCode);
            writer.Key("tests_error");
            writer.String(response.testsOutput);
            writer.EndObject();
            return stringBuffer.GetString();
        }

        writer.EndObject();
        return stringBuffer.GetString();
    };
    return makeJson(m_service.runTask(params));
}

}  // namespace watchman
