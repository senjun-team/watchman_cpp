#include "docker.h"
#include <restinio/all.hpp>
#include <unifex/execute.hpp>

int main() {
    Docker client;
    auto images = client.list_images();
    for (auto &image: images.GetObject()) {
        std::cout << jsonToString(image.name) << ':';
        std::cout << jsonToString(image.value) << std::endl;
    }

    restinio::run(restinio::on_thread_pool(4)
                          .port(8050)
                          .address("0.0.0.0")
                          .request_handler(
                                  [&images](
                                          const restinio::request_handle_t &req) -> restinio::request_handling_status_t {

                                      std::string result;
                                      for (auto &image: images.GetObject()) {
                                          std::cout << jsonToString(image.name) << ':';
                                          std::cout << jsonToString(image.value) << std::endl;
                                          result += jsonToString(image.value) += ' ';
                                      }
                                      req->create_response()
                                              .append_header(restinio::http_field::version,
                                                             std::to_string(req->header().http_major()))
                                              .append_header(restinio::http_field::content_type, "application/json")
                                              .append_header(restinio::http_field::status_uri,
                                                             std::to_string(200))
                                              .set_body(result)
                                              .done();
                                      return restinio::request_accepted();
                                  }));
    return 0;
}
