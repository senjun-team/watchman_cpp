#pragma once
// https://github.com/Armchair-Software/tar_to_stream

// Tar format
// https://man.freebsd.org/cgi/man.cgi?query=tar&sektion=5&format=html

#include <algorithm>
#include <array>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

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

/// Read a "file" in memory, and write it as a TAR archive to the stream
struct TarHeader {
    // offset
    std::array<char, 100> name = {};  //   0    filename
    std::array<char, 8> mode = {};    // 100    file mode: 0000644 etc
    std::array<char, 8> uid =
        {};  // 108    user id, ascii representation of octal value: "0001750" (for UID 1000)
    std::array<char, 8> gid =
        {};  // 116    group id, ascii representation of octal value: "0001750" (for GID 1000)
    std::array<char, 12> size = {};   // 124    file size, ascii representation of octal value
    std::array<char, 12> mtime = {};  // 136    modification time, seconds since epoch
    std::array<char, 8> checksum = {' ', ' ', ' ', ' ',
                                    ' ', ' ', ' ', ' '};  // 148 chksum field set to 8 spaces.
    char typeflag = '0';                                  // 156    '0' is a regular file
    std::array<char, 100> linkname = {};                  // 157    null bytes when not a link
    std::array<char, 6> magic = {
        'u', 's', 't',
        'a', 'r', ' '};  // 257    format: Unix Standard TAR: "ustar ", not null-terminated
    std::array<char, 2> version = {};   // 263    " "
    std::array<char, 32> uname = {};    // 265    user name
    std::array<char, 32> gname = {};    // 297    group name
    std::array<char, 8> devmajor = {};  // 329    null bytes
    std::array<char, 8> devminor = {};  // 337    null bytes
    std::array<char, 155> prefix = {};  // 345    null bytes
    std::array<char, 12> padding = {};  // 500    padding to reach 512 block size
};

template<typename T>
void tar_to_stream(T & stream,              /// stream to write to, e.g. ostream or ofstream
                   TarInfo const & info) {  /// file owner group name
    TarHeader header;

    auto strFilemode = fileModeToString(info.filemode);
    uint32_t const fileModePadding = sizeof(header.mode) - 1 - strFilemode.size();
    constexpr auto kNullCharacter = '0';
    strFilemode.insert(strFilemode.begin(), fileModePadding,
                       kNullCharacter);  // zero-pad the file mode

    auto copyToHeaderField = [](auto const & from, auto & to) -> void {
        std::copy(from.begin(), from.begin() + std::min(from.size(), sizeof(to) - 1), to.begin());
    };

    copyToHeaderField(info.filename, header.name);
    copyToHeaderField(strFilemode, header.mode);
    copyToHeaderField(info.uname, header.uname);
    copyToHeaderField(info.gname, header.gname);

    header.typeflag = fileTypeToChar(info.fileType);

    auto const getStringStream = [](auto && value, uint32_t size) -> std::string {
        std::stringstream ss;
        // use convertion to octal intentionally
        ss << std::oct << std::setw(static_cast<int>(size)) << std::setfill('0') << value;
        return ss.str();
    };

    auto fillElement = [](std::string const & from, auto & elem) -> void {
        for (size_t i = 0, size = from.size(); i < size; ++i) {
            elem[i] = from[i];
        }
    };

    // Use a stringstream to convert the size to an octal string
    fillElement(getStringStream(info.data.size(), sizeof(header.size) - 1), header.size);
    fillElement(getStringStream(info.mtime, sizeof(header.mtime) - 1), header.mtime);
    fillElement(getStringStream(info.uid, sizeof(header.uid) - 1), header.uid);
    fillElement(getStringStream(info.gid, sizeof(header.gid) - 1), header.gid);

    // Calculate the checksum, as it is used by tar
    uint32_t checksumValue = 0;
    constexpr auto kTarHeaderSize = sizeof(TarHeader);
    for (uint32_t i = 0; i != kTarHeaderSize; ++i) {
        checksumValue += reinterpret_cast<uint8_t *>(&header)[i];
    }

    fillElement(getStringStream(checksumValue, sizeof(header.checksum) - 2), header.checksum);

    stream << std::string_view(header.name.data(), kTarHeaderSize);
    if (info.fileType == FileType::RegularFile) {
        uint32_t const padding =
            info.data.size() == kTarHeaderSize
                ? 0
                : kTarHeaderSize - static_cast<uint32_t>(info.data.size() % kTarHeaderSize);
        stream << std::string_view(info.data) << std::string(padding, '\0');
    }
}

template<typename T>
void tar_to_stream_tail(T & stream, uint32_t tailLength = 512u * 2u) {
    /// TAR archives expect a tail of null bytes at the end - min of 512 * 2, but implementations
    /// often add more
    stream << std::string(tailLength, '\0');
}
}  // namespace tar

#pragma clang diagnostic pop
