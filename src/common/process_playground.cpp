#include "answer_common.hpp"
#include "docker_end_answer.hpp"
#include "logging.hpp"

using namespace watchman::internal;

namespace {
template<typename GetMessage, typename GetUserOutput>
watchman::Response processPlaygroundTemplate(std::string const & message, GetMessage getMessage,
                                             GetUserOutput getUserOutput) {
    ExitCodes const exitCode = getExitCode(message);
    switch (exitCode) {
    case ExitCodes::Ok: {
        // pattern: user's_output user_code_ok_f936a25e user_solution_ok_f936a25e
        auto usersOutput = getMessage(message);
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
}  // namespace

namespace watchman::lunix::playground {

std::string getMessageOutput(std::string const & messages) {
    size_t const userCodeEndIndex = messages.find(kUserCodeSeparator);
    std::string usersOutput = userCodeEndIndex == 0 ? std::string{}
                                                    : messages.substr(0, userCodeEndIndex);

    return usersOutput;
}

std::string getUserOutput(std::string const & message) {
    return message.substr(0, message.size() - kRNEscapeSequence.size()
                                 - getStringLength(ExitCodes::UserError));
}

Response processPlayground(std::string const & message) {
    return processPlaygroundTemplate(
        message, [](auto && message) { return lunix::playground::getMessageOutput(message); },
        [](auto && message) { return lunix::playground::getUserOutput(message); });
}
}  // namespace watchman::lunix::playground

namespace watchman::mac::playground {

std::string getMessageOutput(std::string const & messages) {
    size_t const userCodeEndIndex = messages.find(kUserCodeSeparator);
    std::string usersOutput = userCodeEndIndex > kRNEscapeSequence.size()
                                ? messages.substr(0, userCodeEndIndex - kRNEscapeSequence.size())
                                : std::string{};

    return usersOutput;
}

std::string getUserOutput(std::string const & message) {
    return message.substr(0, message.size() - getStringLength(ExitCodes::UserError));
}

Response processPlayground(std::string const & message) {
    return processPlaygroundTemplate(
        message, [](auto && message) { return mac::playground::getMessageOutput(message); },
        [](auto && message) { return mac::playground::getUserOutput(message); });
}
}  // namespace watchman::mac::playground

namespace watchman {

Response getPlaygroungResponse(std::string const & message) {
    if (hasEscapeSequence(message)) {
        return mac::playground::processPlayground(message);
    }

    return lunix::playground::processPlayground(message);
}

}  // namespace watchman
