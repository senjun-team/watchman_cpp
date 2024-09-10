#pragma once

#include <filesystem>
#include <fstream>
#include <string>
#include "common/common.hpp"

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

inline std::string getJson(std::string const & path) {
    std::ifstream file(getAssetPath(path));
    std::stringstream json;
    json << file.rdbuf();
    return json.str();
}

std::string const kFilesStructureAssets = "files_structure.json";

std::string const kCppProject = "cpp_playground.json";
std::string const kCppProjectCompileError = "cpp_playground_compile_error.json";

std::string const kPythonProject = "python_playground.json";

std::string const kGoProject = "go_playground.json";

std::string const kRustProject = "rust_playground.json";

std::string const kHaskellProject = "haskell_playground.json";

std::string const kPythonPracticeRun = "practice/python_run.json";
std::string const kPythonPracticeTest = "practice/python_test.json";
