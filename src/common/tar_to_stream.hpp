#pragma once
// https://github.com/Armchair-Software/tar_to_stream

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <string>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

namespace tar {

template<typename T>
void tar_to_stream(T & stream,                    /// stream to write to, e.g. ostream or ofstream
                   std::string const & filename,  /// name of the file to write
                   char const * data,             /// pointer to the data in this archive segment
                   uint64_t size,                 /// size of the data
                   uint64_t mtime = 0,            /// file modification time, in seconds since epoch
                   std::string filemode = "644",  /// file mode
                   uint32_t uid = 0u,             /// file owner user ID
                   uint32_t gid = 0u,             /// file owner group ID
                   std::string const & uname = "root",    /// file owner username
                   std::string const & gname = "root") {  /// file owner group name
    /// Read a "file" in memory, and write it as a TAR archive to the stream
    struct {                  // offset
        char name[100] = {};  //   0    filename
        char mode[8] = {};    // 100    file mode: 0000644 etc
        char uid[8] =
            {};  // 108    user id, ascii representation of octal value: "0001750" (for UID 1000)
        char gid[8] =
            {};  // 116    group id, ascii representation of octal value: "0001750" (for GID 1000)
        char size[12] = {};              // 124    file size, ascii representation of octal value
        char mtime[12] = "00000000000";  // 136    modification time, seconds since epoch
        char chksum[8] = {' ', ' ', ' ', ' ', ' ',
                          ' ', ' ', ' '};  // 148    checksum: six octal bytes followed by null and
                                           // ' '.  Checksum is the octal sum of all bytes in the
                                           // header, with chksum field set to 8 spaces.
        char typeflag = '0';               // 156    '0'
        char linkname[100] = {};           // 157    null bytes when not a link
        char magic[6] = {
            'u', 's', 't',
            'a', 'r', ' '};     // 257    format: Unix Standard TAR: "ustar ", not null-terminated
        char version[2] = " ";  // 263    " "
        char uname[32] = {};    // 265    user name
        char gname[32] = {};    // 297    group name
        char devmajor[8] = {};  // 329    null bytes
        char devminor[8] = {};  // 337    null bytes
        char prefix[155] = {};  // 345    null bytes
        char padding[12] = {};  // 500    padding to reach 512 block size
    } header;                   // 512

    filemode.insert(filemode.begin(), 7 - filemode.size(), '0');  // zero-pad the file mode

    std::copy(filename.begin(), filename.begin() + sizeof(header.name) - 1, header.name);
    std::copy(filemode.begin(), filemode.begin() + sizeof(header.mode) - 1, header.mode);
    std::copy(uname.begin(), uname.begin() + sizeof(header.uname) - 1, header.uname);
    std::copy(gname.begin(), gname.begin() + sizeof(header.gname) - 1, header.gname);

    // Дима, ты можешь это сделать более изящным в соответствии со своим видением прекрасного.
    if (data == nullptr) {
        header.typeflag = '5';
    }

    auto const getStringStream = [](auto && value, uint32_t size) -> std::stringstream {
        std::stringstream ss;
        // use convertion to octal intentionally
        ss << std::oct << std::setw(static_cast<int>(size)) << std::setfill('0') << value;
        return ss;
    };

    if (size != 0) {
        // Use a stringstream to convert the size to an octal string
        getStringStream(size, sizeof(header.size) - 1) >> header.size;
        getStringStream(mtime, sizeof(header.mtime) - 1) >> header.mtime;
        getStringStream(uid, sizeof(header.uid) - 1) >> header.uid;
        getStringStream(gid, sizeof(header.gid) - 1) >> header.gid;

        // Calculate the checksum, as it is used by tar
        uint32_t checksum_value = 0;
        for (uint32_t i = 0; i != sizeof(header); ++i) {
            checksum_value += reinterpret_cast<uint8_t *>(&header)[i];
        }

        getStringStream(checksum_value, sizeof(header.chksum) - 2) >> header.chksum;
    } else {
        // If the size is 0, then we need to fill in a size of 00000000000
        for (uint32_t i = 0; i < sizeof(header.size); ++i) {
            header.size[i] = '0';
        }
    }

    auto const padding = size == 512u ? 0 : 512u - static_cast<unsigned int>(size % 512);

    stream << std::string_view(header.name, sizeof(header));
    if (data != nullptr) {
        stream << std::string_view(data, size);
        stream << std::string(padding, '\0');
    }
}

template<typename T>
void tar_to_stream_tail(T & stream, unsigned int tail_length = 512u * 2u) {
    /// TAR archives expect a tail of null bytes at the end - min of 512 * 2, but implementations
    /// often add more
    stream << std::string(tail_length, '\0');
}
}  // namespace tar

#pragma clang diagnostic pop
