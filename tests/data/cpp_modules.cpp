#include "ut.hpp"

import std;

namespace ut = boost::ut;

int main() {
    using namespace ut;

    "check user input"_test = [] {
        std::string str = R"(#INJECT-b585472fa)";
        expect(true);
    };
}