#include "answer_common.hpp"

namespace watchman::internal {

std::vector const kEscapePatterns{kEscapedCodeTestsSeparator, kEscapedUserCodeSeparator};

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

std::string removeEscapeSequences(std::string const & string) {
    std::string noEscapeString(string);
    size_t constexpr size = kRNEscapeSequence.size();
    while (size_t const from = noEscapeString.find(kRNEscapeSequence)) {
        if (from == std::string::npos) {
            break;
        }
        noEscapeString.erase(from, size);
    }
    return noEscapeString;
}

std::string removeEscapeSequencesTender(std::string const & string) {
    std::string noExcessEscapeSymbols(string);
    for (auto const pattern : kEscapePatterns) {
        size_t const beginPattern = noExcessEscapeSymbols.find(pattern);
        if (beginPattern == std::string::npos) {
            continue;
        }

        size_t const beginRN = beginPattern + pattern.size() - kRNEscapeSequence.size();
        noExcessEscapeSymbols.erase(beginRN, kRNEscapeSequence.size());
    }
    return noExcessEscapeSymbols;
}

bool hasEscapeSequence(std::string const & output) { return output.ends_with(kRNEscapeSequence); }

}  // namespace watchman::internal
