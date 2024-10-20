#include "project.hpp"

#include "tar/tar_creator.hpp"

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

std::string makeProjectTar(Project const & project) {
    std::string resultTar;

    {
        tar::Creator<std::ostringstream> creator(resultTar);
        for (auto const & pathContent : project.pathsContents) {
            if (pathContent.isDir) {
                creator.addFile({pathContent.path, pathContent.content, tar::FileType::Directory,
                                 tar::Filemode::ReadWrite, 0, 1000, 1000, "code_runner",
                                 "code_runner"});
            } else {
                creator.addFile({pathContent.path, pathContent.content, tar::FileType::RegularFile,
                                 tar::Filemode::ReadWriteExecute, 0, 1000, 1000, "code_runner",
                                 "code_runner"});
            }
        }
    }

    return resultTar;
}

}  // namespace watchman
