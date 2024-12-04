#include <gtest/gtest.h>

#include "../service/common.hpp"
#include "common/detail/project_utils.hpp"
#include "core/parser.hpp"

#include <fstream>
#include <sstream>
#include <structarus/tar_creator.hpp>

TEST(Parser, emptyTestString) {
    watchman::Response response;
    response.output = "lalala";
    response.sourceCode = 0;
    response.testsOutput = std::nullopt;

    ASSERT_NE(makeJsonPlayground(std::move(response)), std::string{""});
}

TEST(Parser, notEmptyTestString) {
    watchman::Response response;
    response.output = "lalala";
    response.sourceCode = 0;
    response.testsOutput = "not null";

    ASSERT_NE(makeJsonCourse(std::move(response)), std::string{""});
}

TEST(Parser, DirectoriesParser) {
    std::ifstream file(getAssetPath(kFilesStructureAssets));
    std::stringstream json;
    json << file.rdbuf();

    watchman::detail::Directory rootDirectory = watchman::detail::jsonToDirectory(json.str());
    ASSERT_EQ(rootDirectory.name, "watchman_cpp_dir");
    ASSERT_EQ(rootDirectory.files.size(), 3u);
    ASSERT_EQ(rootDirectory.directories.size(), 1u);

    auto const & src = rootDirectory.directories.back();
    ASSERT_EQ(src.name, "src");
    ASSERT_EQ(src.files.size(), 2u);
    ASSERT_EQ(src.directories.size(), 1u);

    auto const & core = src.directories.back();

    ASSERT_EQ(core.name, "core");
    ASSERT_EQ(core.files.size(), 4u);
    ASSERT_TRUE(core.directories.empty());
}

TEST(Parser, FillPaths) {
    std::ifstream file(getAssetPath(kFilesStructureAssets));
    std::stringstream json;
    json << file.rdbuf();

    watchman::detail::Directory rootDirectory = watchman::detail::jsonToDirectory(json.str());
    auto paths = getPathsToFiles(rootDirectory);
    ASSERT_EQ(paths.size(), 12u);
}

TEST(Parser, TarDir) {
    std::ifstream file(getAssetPath(kFilesStructureAssets));
    std::stringstream json;
    json << file.rdbuf();

    watchman::detail::Directory rootDirectory = watchman::detail::jsonToDirectory(json.str());
    auto pathsContents = getPathsToFiles(rootDirectory);

    {
        std::string tarName = "my_tarball.tar";
        tar::Creator<std::ofstream> creator(tarName);
        for (auto const & pathContent : pathsContents) {
            if (pathContent.isDir) {
                creator.addFile({pathContent.path, pathContent.content, tar::FileType::Directory,
                                 tar::Filemode::ReadWrite});
            } else {
                creator.addFile({pathContent.path, pathContent.content, tar::FileType::RegularFile,
                                 tar::Filemode::ReadWriteExecute});
            }
        }
    }
    ASSERT_TRUE(true);
}
