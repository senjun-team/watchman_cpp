#include "docker.h"
#include "unifex/execute.hpp"
#include "restinio/all.hpp"

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
                          .request_handler([](const restinio::request_handle_t &req) {
                              return restinio::request_accepted();
                          }));
    return 0;
}
