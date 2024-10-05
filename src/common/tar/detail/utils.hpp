#include <algorithm>
#include <iomanip>
#include <sstream>
#include <string>

namespace tar::detail {
auto const getString = [](auto && value, uint32_t size) -> std::string {
    std::stringstream ss;
    // use convertion to octal intentionally
    ss << std::oct << std::setw(static_cast<int>(size)) << std::setfill('0') << value;
    return ss.str();
};

auto const toArrayFromString = [](auto const & from, auto & toArray) -> void {
    std::copy(from.begin(), from.begin() + std::min(from.size(), sizeof(toArray) - 1),
              toArray.begin());
};

}  // namespace tar::detail
