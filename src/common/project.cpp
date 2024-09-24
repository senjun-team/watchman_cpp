#include "project.hpp"

#include "common/logging.hpp"
#include "tar_to_stream.hpp"

#include <filesystem>
#include <sstream>

namespace fs = std::filesystem;
namespace watchman {

void recursiveFillAbsolutePaths(Directory const & directory, fs::path const & currentPath,
                                std::vector<PathContent> & paths) {
    paths.push_back({currentPath.string() + "/", ""});
    paths.back().isDir = true;

    for (auto & file : directory.files) {
        paths.push_back({currentPath / file.name, file.content});
        if (file.isMain) {
            paths.back().isMain = true;
        }
    }

    for (auto & subDirectory : directory.directories) {
        recursiveFillAbsolutePaths(subDirectory, currentPath / subDirectory.name, paths);
    }
}

std::vector<PathContent> getPathsToFiles(Directory const & rootDirectory) {
    std::vector<PathContent> paths;
    recursiveFillAbsolutePaths(rootDirectory, rootDirectory.name, paths);
    return paths;
}

std::string getMainFile(std::vector<PathContent> const & pathContents) {
    for (auto const & p : pathContents) {
        if (p.isMain) {
            return p.path;
        }
    }
    throw std::logic_error{"There's no main file"};
}

std::ostringstream makeProjectTar(Project const & project) {
    std::ostringstream stream(project.name, std::ios::binary | std::ios::trunc);

    for (auto const & pathContent : project.pathsContents) {
        tar::tar_to_stream(
            stream, pathContent.path, pathContent.isDir ? nullptr : pathContent.content.data(),
            pathContent.isDir ? 0 : pathContent.content.size(), 0,
            pathContent.isDir ? tar::Filemode::ReadWrite : tar::Filemode::ReadWriteExecute, 1000,
            1000, "code_runner", "code_runner");
    }
    tar::tar_to_stream_tail(stream);
    return stream;
}

}  // namespace watchman
