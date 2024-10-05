#pragma once
// https://github.com/Armchair-Software/tar_to_stream

// Tar format
// https://man.freebsd.org/cgi/man.cgi?query=tar&sektion=5&format=html

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

    auto const strFilemode = detail::getStringFilemode(info.filemode);

    detail::TarHeader header;
    detail::toArrayFromString(info.archiveName, header.name);
    detail::toArrayFromString(strFilemode, header.mode);
    detail::toArrayFromString(info.uname, header.uname);
    detail::toArrayFromString(info.gname, header.gname);

    header.typeflag = detail::fileTypeToChar(info.fileType);

    // Use a stringstream to convert the size to an octal string
    detail::toArrayFromString(detail::getString(info.data.size(), sizeof(header.size) - 1),
                              header.size);
    detail::toArrayFromString(detail::getString(info.mtime, sizeof(header.mtime) - 1),
                              header.mtime);
    detail::toArrayFromString(detail::getString(info.uid, sizeof(header.uid) - 1), header.uid);
    detail::toArrayFromString(detail::getString(info.gid, sizeof(header.gid) - 1), header.gid);

    // Calculate the checksum, as it is used by tar
    uint32_t checksumValue = 0;
    for (uint32_t i = 0; i != detail::kTarHeaderSize; ++i) {
        checksumValue += reinterpret_cast<uint8_t *>(&header)[i];
    }

    detail::toArrayFromString(detail::getString(checksumValue, sizeof(header.checksum) - 2),
                              header.checksum);
    detail::fillStream(stream, info, header);
}

template<typename T>
void tar_to_stream_tail(T & stream, uint32_t tailLength = 512u * 2u) {
    /// TAR archives expect a tail of null bytes at the end - min of 512 * 2, but implementations
    /// often add more
    stream << std::string(tailLength, detail::kNullTerminator);
}
}  // namespace tar

#pragma clang diagnostic pop
