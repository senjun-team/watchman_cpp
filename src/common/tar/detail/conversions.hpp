#pragma once

// TODO remove cd ".."
#include "../tar_info.hpp"
#include "consts.hpp"

#include <stdexcept>

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

auto const getStringFilemode = [](Filemode filemode) -> std::string {
    auto strFilemode = detail::fileModeToString(filemode);
    uint32_t const fileModePadding = sizeof(TarHeader::mode) - 1 - strFilemode.size();
    strFilemode.insert(strFilemode.begin(), fileModePadding,
                       detail::kNullCharacter);  // zero-pad the file mode
    return strFilemode;
};

template<typename T>
void fillStream(T & stream, FileInfo const & info, TarHeader const & header) {
    stream << std::string_view(header.name.data(), detail::kTarHeaderSize);
    // TODO add other types?
    if (info.fileType == FileType::RegularFile) {
        uint32_t const padding =
            info.data.size() == detail::kTarHeaderSize
                ? 0
                : detail::kTarHeaderSize
                      - static_cast<uint32_t>(info.data.size() % detail::kTarHeaderSize);
        stream << info.data << std::string(padding, detail::kNullTerminator);
    }
}

}  // namespace tar::detail
