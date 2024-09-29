#pragma once
// https://github.com/Armchair-Software/tar_to_stream

// Tar format
// https://man.freebsd.org/cgi/man.cgi?query=tar&sektion=5&format=html

#include "detail/consts.hpp"
#include "detail/conversions.hpp"
#include "detail/utils.hpp"

#include <cstdint>
#include <string>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

namespace tar {

template<typename T>
void tar_to_stream(T & stream,              /// stream to write to, e.g. ostream or ofstream
                   TarInfo const & info) {  /// file owner group name
    detail::TarHeader header;

    auto strFilemode = detail::fileModeToString(info.filemode);
    uint32_t const fileModePadding = sizeof(header.mode) - 1 - strFilemode.size();
    strFilemode.insert(strFilemode.begin(), fileModePadding,
                       detail::kNullCharacter);  // zero-pad the file mode

    detail::copyToHeaderField(info.filename, header.name);
    detail::copyToHeaderField(strFilemode, header.mode);
    detail::copyToHeaderField(info.uname, header.uname);
    detail::copyToHeaderField(info.gname, header.gname);

    header.typeflag = detail::fileTypeToChar(info.fileType);

    // Use a stringstream to convert the size to an octal string
    detail::fillElement(detail::getStringStream(info.data.size(), sizeof(header.size) - 1),
                        header.size);
    detail::fillElement(detail::getStringStream(info.mtime, sizeof(header.mtime) - 1),
                        header.mtime);
    detail::fillElement(detail::getStringStream(info.uid, sizeof(header.uid) - 1), header.uid);
    detail::fillElement(detail::getStringStream(info.gid, sizeof(header.gid) - 1), header.gid);

    // Calculate the checksum, as it is used by tar
    uint32_t checksumValue = 0;
    for (uint32_t i = 0; i != detail::kTarHeaderSize; ++i) {
        checksumValue += reinterpret_cast<uint8_t *>(&header)[i];
    }

    detail::fillElement(detail::getStringStream(checksumValue, sizeof(header.checksum) - 2),
                        header.checksum);

    stream << std::string_view(header.name.data(), detail::kTarHeaderSize);
    if (info.fileType == FileType::RegularFile) {
        uint32_t const padding =
            info.data.size() == detail::kTarHeaderSize
                ? 0
                : detail::kTarHeaderSize
                      - static_cast<uint32_t>(info.data.size() % detail::kTarHeaderSize);
        stream << std::string_view(info.data) << std::string(padding, detail::kNullTerminator);
    }
}

template<typename T>
void tar_to_stream_tail(T & stream, uint32_t tailLength = 512u * 2u) {
    /// TAR archives expect a tail of null bytes at the end - min of 512 * 2, but implementations
    /// often add more
    stream << std::string(tailLength, detail::kNullTerminator);
}
}  // namespace tar

#pragma clang diagnostic pop
