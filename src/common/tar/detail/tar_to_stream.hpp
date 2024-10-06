#pragma once

// https://github.com/Armchair-Software/tar_to_stream

// Tar format
// https://man.freebsd.org/cgi/man.cgi?query=tar&sektion=5&format=html

#include "conversions.hpp"
#include "utils.hpp"

#include <cstdint>
#include <string>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

namespace tar::detail {

// create in memory tar structure
template<typename T>
void tar_to_stream(T & stream,              /// stream to write to, e.g. ostream or ofstream
                   FileInfo const & info) {  /// file owner group name

    auto const strFilemode = detail::getStringFilemode(info.filemode);

    detail::TarHeader header;
    detail::toArrayFromString(info.archiveName, header.name);
    detail::toArrayFromString(strFilemode, header.mode);
    detail::toArrayFromString(info.uname, header.uname);
    detail::toArrayFromString(info.gname, header.gname);

    header.typeflag = detail::fileTypeToChar(info.fileType);

    detail::toArrayFromString(detail::getString(info.data.size(), sizeof(header.size) - 1),
                              header.size);
    detail::toArrayFromString(detail::getString(info.mtime, sizeof(header.mtime) - 1),
                              header.mtime);
    detail::toArrayFromString(detail::getString(info.uid, sizeof(header.uid) - 1), header.uid);
    detail::toArrayFromString(detail::getString(info.gid, sizeof(header.gid) - 1), header.gid);
    detail::toArrayFromString(
        detail::getString(detail::calculateChecksum(header), sizeof(header.checksum) - 2),
        header.checksum);

    detail::fillStream(stream, info, header);
}

template<typename T>
void tar_to_stream_tail(T & stream) {
    constexpr uint32_t kTailLengh = 1024;
    stream << std::string(kTailLengh, detail::kNullTerminator);
}
}  // namespace tar

#pragma clang diagnostic pop
