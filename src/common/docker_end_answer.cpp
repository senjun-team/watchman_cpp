#include "docker_end_answer.hpp"
#include "answer_common.hpp"

#include "logging.hpp"

#include <map>

using namespace watchman::internal;
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

Response processCourse(std::string const & message) {
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

Response processPlayground(std::string const & message) {
    // todo here we need to process result from container
    // we don't need extra string tests_ok like in courses
    // we have to process answer without tests launching
    // interesting, that mac answer also has escape sequence now
    return processCourse(message);
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

Response processCourse(std::string const & message) {
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

Response processPlayground(std::string const & message) { return {}; }

}  // namespace watchman::mac

namespace watchman {

Response getCourseResponse(std::string const & message) {
    if (hasLinuxEscape(message)) {
        return linux::processCourse(message);
    }

    return mac::processCourse(message);
}

Response getPlaygroungResponse(std::string const & message) {
    if (hasLinuxEscape(message)) {
        return linux::processPlayground(message);
    }

    return mac::processPlayground(message);
}
}  // namespace watchman
