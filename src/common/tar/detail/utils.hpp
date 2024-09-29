#include <algorithm>
#include <iomanip>
#include <sstream>
#include <string>

namespace tar::detail {
auto const getStringStream = [](auto && value, uint32_t size) -> std::string {
    std::stringstream ss;
    // use convertion to octal intentionally
    ss << std::oct << std::setw(static_cast<int>(size)) << std::setfill('0') << value;
    return ss.str();
};

auto const fillElement = [](std::string const & from, auto & elem) -> void {
    for (size_t i = 0, size = from.size(); i < size; ++i) {
        elem[i] = from[i];
    }
};

auto const copyToHeaderField = [](auto const & from, auto & to) -> void {
    std::copy(from.begin(), from.begin() + std::min(from.size(), sizeof(to) - 1), to.begin());
};

}  // namespace tar::detail
