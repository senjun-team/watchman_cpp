#include "docker_end_answer.hpp"

#include "logging.hpp"

#include <map>
#include <string_view>

namespace {
// For all exit statuses of 'timeout' util see "man timeout"
static const std::map<std::string_view, int32_t> kTimeoutUtilCodes{
    {"124\r\n", 124},  // if COMMAND times out, and --preserve-status is not specified
    {"125\r\n", 125},  // if the timeout command itself fails
    {"137\r\n", 137}   // if COMMAND (or timeout itself) is sent the KILL (9)
};

// Also exit statuses of timeout util inside container:
size_t constexpr kDockerTimeout = 124;
size_t constexpr kDockerMemoryKill = 137;
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

size_t getStringLength(ExitCodes code) {
    switch (code) {
    case ExitCodes::Ok: return kCorrelations[0].string.size();
    case ExitCodes::UserError: return kCorrelations[1].string.size();
    case ExitCodes::TestError: return kCorrelations[2].string.size();
    case ExitCodes::Unknown: break;
    }
    return -1;
}

ExitCodes getSequencedExitCode(std::string const & containerOutput) {
    // Case when timeout util returns its status code
    for (auto const & [codeStr, code] : kTimeoutUtilCodes) {
        if (containerOutput.ends_with(codeStr)) {
            return static_cast<ExitCodes>(code);
        }
    }

    for (auto const & pattern : kCorrelations) {
        if (containerOutput.find(pattern.string) != std::string::npos) {
            return pattern.code;
        }
    }

    return ExitCodes::Unknown;
}

// debug function for macos purposes
void removeEscapeSequences(std::string & string) {
    size_t constexpr size = kLinuxEscapeSequence.size();
    while (size_t const from = string.find(kLinuxEscapeSequence)) {
        if (from == std::string::npos) {
            break;
        }
        string.erase(from, size);
    }
}

bool hasLinuxEscape(std::string const & output) {
    if (output.size() < kLinuxEscapeSequence.size()) {
        return false;
    }

    return output.ends_with(kLinuxEscapeSequence);
}

}  // namespace

namespace watchman::linux {

ContainerMessages getMessageOutput(ExitCodes code, std::string const & messages) {
    size_t const userCodeEndIndex = messages.find(kCodeTestsSeparator);
    std::string usersOutput = userCodeEndIndex > kLinuxEscapeSequence.size()
                                ? messages.substr(0, userCodeEndIndex - kLinuxEscapeSequence.size())
                                : std::string{};

    size_t const from = userCodeEndIndex + kCodeTestsSeparator.size() + kLinuxEscapeSequence.size();
    size_t const amount =
        messages.size() - from - getStringLength(code) - kLinuxEscapeSequence.size();
    std::string testsOutput = messages.substr(from, amount);
    if (testsOutput.ends_with(kLinuxEscapeSequence)) {
        testsOutput = testsOutput.substr(0, testsOutput.size() - kLinuxEscapeSequence.size());
    }

    return {std::move(usersOutput), std::move(testsOutput)};
}

std::string getUserOutput(std::string const & message) {
    return message.substr(0, message.size() - kLinuxEscapeSequence.size()
                                 - getStringLength(ExitCodes::UserError));
}

Response processMessage(std::string const & message) {
    ExitCodes const exitCode = getSequencedExitCode(message);
    switch (exitCode) {
    case ExitCodes::Ok: {
        // pattern: user's_output user_code_ok_f936a25e user_solution_ok_f936a25e
        auto [usersOutput, testsOutput] = getMessageOutput(ExitCodes::Ok, message);
        return {kSuccessCode, std::move(usersOutput), std::move(testsOutput)};
    }

    case ExitCodes::UserError: {
        // pattern: user's_output user_solution_error_f936a25e
        return {kUserCodeError, getUserOutput(message), std::nullopt};
    }

    case ExitCodes::TestError: {
        // pattern: user's_output user_code_ok_f936a25e tests_cases_error_f936a25e
        auto [usersOutput, testsOutput] = getMessageOutput(ExitCodes::TestError, message);
        return {kTestsError, std::move(usersOutput), std::move(testsOutput)};
    }

    default:
        Log::error(kWrongDockerImage);
        return {static_cast<int32_t>(exitCode), "Internal error", ""};
    }
}

}  // namespace watchman::linux

namespace watchman::mac {

ContainerMessages getMessageOutput(ExitCodes code, std::string const & messages) {
    size_t const userCodeEndIndex = messages.find(kCodeTestsSeparator);
    std::string usersOutput = userCodeEndIndex == 0 ? std::string{}
                                                    : messages.substr(0, userCodeEndIndex);

    size_t const from = userCodeEndIndex + kCodeTestsSeparator.size();
    size_t const amount = messages.size() - from - getStringLength(code);
    std::string testsOutput = messages.substr(from, amount);

    return {std::move(usersOutput), std::move(testsOutput)};
}

std::string getUserOutput(std::string const & message) {
    return message.substr(0, message.size() - getStringLength(ExitCodes::UserError));
}

Response processMessage(std::string const & message) {
    ExitCodes const exitCode = getSequencedExitCode(message);
    switch (exitCode) {
    case ExitCodes::Ok: {
        // pattern: user's_output user_code_ok_f936a25e user_solution_ok_f936a25e
        auto [usersOutput, testsOutput] = getMessageOutput(ExitCodes::Ok, message);
        return {kSuccessCode, std::move(usersOutput), std::move(testsOutput)};
    }

    case ExitCodes::UserError: {
        // pattern: user's_output user_solution_error_f936a25e
        return {kUserCodeError, getUserOutput(message), std::string{}};
    }

    case ExitCodes::TestError: {
        // pattern: user's_output user_code_ok_f936a25e tests_cases_error_f936a25e
        auto [usersOutput, testsOutput] = getMessageOutput(ExitCodes::TestError, message);
        return {kTestsError, std::move(usersOutput), std::move(testsOutput)};
    }

    default:
        Log::error(kWrongDockerImage);
        return {static_cast<int32_t>(exitCode), "Internal error", ""};
    }
}

}  // namespace watchman::mac

namespace watchman {

Response getCourseResponse(std::string const & message) {
    if (hasLinuxEscape(message)) {
        return linux::processMessage(message);
    }

    return mac::processMessage(message);
}

Response getPlaygroungResponse(std::string const & message) {
    if (hasLinuxEscape(message)) {
        return linux::processMessage(message);
    }

    return mac::processMessage(message);
}
}  // namespace watchman
