#include "answer_common.hpp"
#include "docker_end_answer.hpp"
#include "logging.hpp"

namespace watchman::internal {

std::string getMessageOutput(std::string const & messages) {
    size_t const userCodeEndIndex = messages.find(kUserCodeSeparator);
    std::string usersOutput = userCodeEndIndex == 0 ? std::string{}
                                                    : messages.substr(0, userCodeEndIndex);

    return usersOutput;
}

Response processPlayground(std::string const & message) {
    ExitCodes const exitCode = getExitCode(message);
    switch (exitCode) {
    case ExitCodes::Ok: {
        // pattern: user's_output user_code_ok_f936a25e user_solution_ok_f936a25e
        auto usersOutput = getMessageOutput(message);
        return {watchman::kSuccessCode, std::move(usersOutput), std::nullopt};
    }

    case ExitCodes::UserError: {
        // pattern: user's_output user_solution_error_f936a25e
        return {watchman::kUserCodeError, getUserOutput(message), std::nullopt};
    }

    default:
        watchman::Log::error(kWrongDockerImage);
        return {static_cast<int32_t>(exitCode), "Internal error", ""};
    }
}
}  // namespace watchman::internal

namespace watchman {

Response getPlaygroungResponse(std::string const & message) {
    if (internal::hasEscapeSequence(message)) {
        std::string messageWithoutEscapeSequence = internal::removeEscapeSequencesTender(message);
        return internal::processPlayground(messageWithoutEscapeSequence);
    }

    return internal::processPlayground(message);
}

}  // namespace watchman
