#include "utils.hpp"

namespace tar::detail {

inline std::string fileModeToString(Filemode mode) {
    switch (mode) {
    case Filemode::ReadWriteExecute: return std::string{"644"};
    case Filemode::ReadWrite: return std::string{"755"};
    }

    std::string const errorMessage =
        "Unknown filemode: " + std::to_string(static_cast<uint32_t>(mode));
    throw std::logic_error(errorMessage);
}

inline char fileTypeToChar(FileType fileType) {
    switch (fileType) {
    case FileType::RegularFile: return '0';
    case FileType::Directory: return '5';
    }

    std::string const errorMessage =
        "Unknown filetype: " + std::to_string(static_cast<uint32_t>(fileType));
    throw std::logic_error(errorMessage);
}
}  // namespace tar::detail
