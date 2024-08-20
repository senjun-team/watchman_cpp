#include "project.hpp"

#include "tar_to_stream.hpp"

#include <filesystem>
#include <sstream>

namespace fs = std::filesystem;
namespace watchman {

void recursiveFillAbsolutePaths(Directory const & directory, fs::path const & currentPath,
                                std::vector<PathContent> & paths) {
    for (auto & file : directory.files) {
        paths.push_back({currentPath / file.name, file.content});
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

std::ostringstream makeProjectTar(Project const & project) {
    std::ostringstream stream(project.name, std::ios::binary | std::ios::trunc);

    for (auto const & pathContent : project.pathsContents) {
        tar::tar_to_stream(stream, pathContent.path, pathContent.content.data(),
                           pathContent.content.size());
    }
    tar::tar_to_stream_tail(stream);
    return stream;
}

}  // namespace watchman
