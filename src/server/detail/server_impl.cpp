#include "server/detail/server_impl.hpp"

#include "common/common.hpp"
#include "common/config.hpp"
#include "common/logging.hpp"
#include "core/parser.hpp"

#include <unifex/then.hpp>

namespace watchman::detail {

constexpr size_t kPort = 8000;
std::string const kIpAddress = "0.0.0.0";

constexpr std::string_view kCheck = "/check";
constexpr std::string_view kPlayground = "/playground";
constexpr std::string_view kPractice = "/practice";

std::optional<Api> getApi(std::string_view handle) {
    if (handle == kCheck) {
        return Api::Check;
    }

    if (handle == kPlayground) {
        return Api::Playground;
    }

    if (handle == kPractice) {
        return Api::Practice;
    }

    return std::nullopt;
}

AsyncContext::AsyncContext(size_t poolSize)
    : m_threadPool(poolSize) {}

void AsyncContext::schedule(std::function<void()> f) {
    m_scope.detached_spawn(unifex::schedule(m_threadPool.get_scheduler())
                           | unifex::then([f = std::move(f)]() { f(); }));
}

ServerImpl::ServerImpl(Config && config)
    : m_service(std::move(config))
    , m_context(config.threadPoolSize) {}

ServerImpl::~ServerImpl() = default;

void ServerImpl::internalProcessRequest(restinio::request_handle_t req) {
    LogDuration const duration("Request handling");
    try {
        std::string const result = processRequest(req->header().path(), req->body());
        req->create_response()
            .append_header(restinio::http_field::version,
                           std::to_string(req->header().http_major()))
            .append_header(restinio::http_field::content_type, "application/json")
            .append_header(restinio::http_field::status_uri,
                           std::to_string(restinio::status_code::ok.raw_code()))
            .set_body(result)
            .done();

        Log::info("request handled successfully: {}", result);
    }
    catch (std::exception const & ex) {
        req->create_response()
            .append_header(restinio::http_field::version,
                           std::to_string(req->header().http_major()))
            .append_header(restinio::http_field::content_type, "application/json")
            .append_header(restinio::http_field::status_uri,
                           std::to_string(restinio::status_code::internal_server_error.raw_code()))
            .set_body(R"({"error":"something went wrong"})")
            .done();

        Log::warning("Exception while processing: {}", ex.what());
    }
}

void ServerImpl::start() {
    Log::info("Watchman working on {} port", kPort);
    restinio::run(restinio::on_this_thread()
                      .port(kPort)
                      .address(kIpAddress)
                      .request_handler([this](restinio::request_handle_t const & req)
                                           -> restinio::request_handling_status_t {
                          if (restinio::http_method_post() != req->header().method()) {
                              LogDuration const duration("Request handling");
                              Log::error("error while handling: {}", req->body());
                              return restinio::request_rejected();
                          }

                          m_context.schedule([this, req = req]() { internalProcessRequest(req); });
                          return restinio::request_accepted();
                      }));
}

std::string ServerImpl::processRequest(std::string_view apiString, std::string const & body) {
    Log::info("Processing handle {}, body:\n {}", apiString, body);

    auto api = getApi(apiString);
    if (!api.has_value()) {
        auto const errorMesage = fmt::format("Unknowon api: {}", apiString);
        Log::info(fmt::runtime(errorMesage));
        return makeErrorJson(errorMesage);
    }

    try {
        switch (*api) {
        case Api::Check: return processCheck(body);
        case Api::Playground: return processPlayground(body);
        case Api::Practice: return processPractice(body);
        }

    } catch (std::exception const & exception) {
        auto const errorMesage =
            fmt::format("Error while processing handle `{}` {}", apiString, exception.what());
        Log::info(fmt::runtime(errorMesage));
        return makeErrorJson(errorMesage);
    }
    throw std::logic_error{"process request logic error"};
}

std::string ServerImpl::processCheck(std::string const & body) {
    auto const params = parseTask(body);
    if (params.taskLauncherType == TaskLauncherType::UNKNOWN) {
        return {};
    }

    return makeJsonCourse(m_service.runTask(params));
}

std::string ServerImpl::processPlayground(std::string const & body) {
    auto const params = parsePlayground(body);
    if (params.taskLauncherType == TaskLauncherType::UNKNOWN) {
        return {};
    }

    return makeJsonPlayground(m_service.runPlayground(params));
}

std::string ServerImpl::processPractice(std::string const & body) {
    auto const params = parsePractice(body);
    if (params.taskLauncherType == TaskLauncherType::UNKNOWN) {
        return {};
    }

    return makeJsonPlayground(m_service.runPractice(params));
}

}  // namespace watchman::detail
