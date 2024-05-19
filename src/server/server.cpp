#include "server.hpp"

#include "common/common.hpp"
#include "common/logging.hpp"
#include "parser.hpp"

#include <rapidjson/prettywriter.h>

#include <restinio/all.hpp>

namespace watchman {
constexpr size_t kPort = 8000;
std::string const kIpAddress = "0.0.0.0";

size_t constexpr kDockerTimeoutCode = 124;
size_t constexpr kDockerMemoryKill = 137;

constexpr std::string_view kCheck = "/check";
constexpr std::string_view kPlayground = "/playground";

Api getApi(std::string_view handle) {
    if (handle == kCheck) {
        return Api::Check;
    }

    if (handle == kPlayground) {
        return Api::Playground;
    }

    throw std::runtime_error{"Unkonwn api"};
}

std::string makeJsonCourse(Response && response) {
    rapidjson::StringBuffer stringBuffer;
    rapidjson::Writer writer(stringBuffer);

    writer.StartObject();
    writer.Key("status_code");
    writer.Int64(response.sourceCode);

    if (response.sourceCode == kDockerTimeoutCode) {
        writer.Key("user_code_output");
        writer.String("Timeout");
        writer.EndObject();
        return stringBuffer.GetString();
    }

    if (response.sourceCode == kDockerMemoryKill) {
        writer.Key("user_code_output");
        writer.String("Out of memory");
        writer.EndObject();
        return stringBuffer.GetString();
    }

    writer.Key("user_code_output");
    writer.String(response.output);

    writer.Key("tests_output");
    writer.String(*response.testsOutput);

    writer.EndObject();
    return stringBuffer.GetString();
}

std::string makeJsonPlayground(Response && response) {}

Server::Server(Config && config)
    : m_service(std::move(config)) {}

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

                          auto const result = processRequest(req->header().path(), req->body());
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

std::string Server::processRequest(std::string_view api, std::string const & body) {
    Log::info("Processing handle {}, body:\n {}", api, body);

    try {
        switch (getApi(api)) {
        case Api::Check: return processCheck(body);
        case Api::Playground: return processPlayground(body);
        }
    } catch (std::exception const & exception) {
        Log::error("Wrong api: {}", exception.what());
    }

    return std::string{R"({"output": "unknown handle"})"};
}

std::string Server::processCheck(std::string const & body) {
    auto const params = parse(body, Api::Check);
    if (params.containerType.empty()) {
        return {};
    }

    return makeJsonCourse(m_service.runTask(params));
}

std::string Server::processPlayground(std::string const & body) {
    auto const params = parse(body, Api::Playground);
    if (params.containerType.empty()) {
        return {};
    }

    return makeJsonPlayground(m_service.runPlayground(params));
}
}  // namespace watchman
