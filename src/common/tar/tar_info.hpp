#pragma once

#include <string>

namespace tar {
enum class Filemode {
    ReadWriteExecute, // for directiories
    ReadWrite
    // etc ...
};

enum class FileType {
    RegularFile,
    Directory
    // etc ...
};

struct FileInfo {
    std::string const & archiveName;  /// name of the file to write in tar
    std::string const & data;         /// data for writing
    FileType fileType = FileType::RegularFile;
    Filemode filemode = Filemode::ReadWriteExecute;
    uint64_t mtime = 0;          /// file modification time, in seconds since epoch
    uint32_t uid = 0;            /// file owner user ID
    uint32_t gid = 0;            /// file owner group ID
    std::string uname = "root";  /// file owner username
    std::string gname = "root";  /// file owner group name
};
}  // namespace tar
