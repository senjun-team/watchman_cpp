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

std::string const kCppProject = "cpp_playground.json";
std::string const kCppProjectCompileError = "cpp_playground_compile_error.json";

std::string const kPythonProject = "python_playground.json";

std::string const kGoProject = "go_playground.json";

std::string const kRustProject = "rust_playground.json";
