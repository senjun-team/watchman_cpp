#pragma once

#include <filesystem>
#include <string>

struct ServiceParams {
    std::string const config = std::string{TEST_DATA_DIR} + "config.json";
};

ServiceParams const kParams;

namespace fs = std::filesystem;

inline fs::path getAssetPath(std::string const & assetName) {
    fs::path dir(TEST_DATA_DIR);
    fs::path asset(assetName);
    return dir / asset;
}

std::string const kFilesStructureAssets = "files_structure.json";
