#include <algorithm>
#include <iomanip>
#include <sstream>
#include <string>

namespace tar {
enum class Filemode {
    ReadWriteExecute,
    ReadWrite
    // etc ...
};

enum class FileType {
    RegularFile,
    Directory
    // etc ...
};

namespace detail {
struct TarInfo {
    std::string const & filename;  /// name of the file to write in tar
    std::string const & data;      /// pointer to the data in this archive segment
    FileType fileType = FileType::RegularFile;
    Filemode filemode = Filemode::ReadWriteExecute;  /// file mode
    uint64_t mtime = 0;          /// file modification time, in seconds since epoch
    uint32_t uid = 0;            /// file owner user ID
    uint32_t gid = 0;            /// file owner group ID
    std::string uname = "root";  /// file owner username
    std::string gname = "root";  /// file owner group name
};

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

}  // namespace detail
}  // namespace tar