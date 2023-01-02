#include "docker.h"
#include "unifex/execute.hpp"

int main() {
    Docker client;
    auto images = client.list_images();
    for (auto &image: images.GetObject()) {
        std::cout << jsonToString(image.name) << ':';
        std::cout << jsonToString(image.value) << std::endl;
    }

    return 0;
}
