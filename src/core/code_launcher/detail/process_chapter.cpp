#include "common/detail/answer_common.hpp"
#include "common/logging.hpp"
#include "core/code_launcher/response.hpp"

namespace watchman::detail {

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

Response processChapter(std::string const & message) {
    ExitCodes const exitCode = getExitCode(message);
    switch (exitCode) {
    case ExitCodes::Ok: {
        // pattern: user's_output user_code_ok_f936a25e user_solution_ok_f936a25e
        auto [usersOutput, testsOutput] = getMessageOutput(ExitCodes::Ok, message);
        return {kSuccessCode, std::move(usersOutput), std::move(testsOutput)};
    }

    case ExitCodes::UserError: {
        // pattern: user's_output user_solution_error_f936a25e
        return {watchman::kUserCodeError, getUserOutput(message), std::nullopt};
    }

    case ExitCodes::TestError: {
        // pattern: user's_output user_code_ok_f936a25e tests_cases_error_f936a25e
        auto [usersOutput, testsOutput] = getMessageOutput(ExitCodes::TestError, message);
        return {watchman::kTestsError, std::move(usersOutput), std::move(testsOutput)};
    }

    default:
        Log::error(kWrongDockerImage);
        return {static_cast<int32_t>(exitCode), "Internal error", ""};
    }
}

}  // namespace watchman::detail

namespace watchman {

Response getCourseResponse(std::string const & message) {
    if (detail::hasEscapeSequence(message)) {
        auto noEscapeMessage = detail::removeEscapeSequencesTender(message);
        return detail::processChapter(noEscapeMessage);
    }

    return detail::processChapter(message);
}
}  // namespace watchman
