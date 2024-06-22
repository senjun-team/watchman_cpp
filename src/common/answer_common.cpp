#include "answer_common.hpp"

namespace watchman::internal {

size_t getStringLength(ExitCodes code) {
    switch (code) {
    case ExitCodes::Ok: return kCorrelations[0].string.size();
    case ExitCodes::UserError: return kCorrelations[1].string.size();
    case ExitCodes::TestError: return kCorrelations[2].string.size();
    case ExitCodes::Unknown: break;
    }
    return -1;
}

ExitCodes getExitCode(std::string const & containerOutput) {
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
    size_t constexpr size = kRNEscapeSequence.size();
    while (size_t const from = string.find(kRNEscapeSequence)) {
        if (from == std::string::npos) {
            break;
        }
        string.erase(from, size);
    }
}

bool hasEscapeSequence(std::string const & output) {
    return output.ends_with(kRNEscapeSequence);
}

}  // namespace watchman::internal
