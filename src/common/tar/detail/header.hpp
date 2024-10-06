#pragma once
#include <array>

namespace tar::detail {

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

}  // namespace tar::detail
