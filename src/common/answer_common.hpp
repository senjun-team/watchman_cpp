#pragma once

#include <map>
#include <string>
#include <string_view>

namespace watchman::internal {
// For all exit statuses of 'timeout' util see "man timeout"
const std::map<std::string_view, int32_t> kTimeoutUtilCodes{
    {"124\r\n", 124},  // if COMMAND times out, and --preserve-status is not specified
    {"125\r\n", 125},  // if the timeout command itself fails
    {"137\r\n", 137}   // if COMMAND (or timeout itself) is sent the KILL (9)
};

enum class ExitCodes : int32_t {
    Ok,         // no errors
    UserError,  // error: syntax / building / bad running user code
    TestError,  // test case failed
    Unknown     // trash answer from container
};

struct ContainerMessages {
    std::string usersOutput;
    std::string testsOutput;
};

// 3 bytes: code\r\n
std::string_view constexpr kLinuxEscapeSequence = "\r\n";
std::string_view constexpr kCodeTestsSeparator = "user_code_ok_f936a25e";

std::string_view constexpr kWrongDockerImage = "Maybe wrong docker image?";

struct MarkToStatusCode {
    std::string_view string;
    ExitCodes code;
};

std::vector<MarkToStatusCode> const kCorrelations{
    {"user_solution_ok_f936a25e", ExitCodes::Ok},
    {"user_solution_error_f936a25e", ExitCodes::UserError},
    {"tests_cases_error_f936a25e", ExitCodes::TestError}};

size_t getStringLength(ExitCodes code);

ExitCodes getSequencedExitCode(std::string const & containerOutput);

// debug function for macos purposes
void removeEscapeSequences(std::string & string);

bool hasLinuxEscape(std::string const & output);

}  // namespace watchman::internal