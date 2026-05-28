#pragma once

#include <gtest/gtest.h>
#include <array>

#include "lexer.hpp"
#include "diagnostic_reporters.hpp"

inline std::vector<std::string> tokensToDebugStrings(
    const std::vector<shader_precompiler::lexer::Token>& tokens)
{
    std::vector<std::string> result;

    for (const auto& token : tokens) {
        result.push_back(token.toDebugString());
    }

    return result;
}

class SemanticDiagnostic : public shader_precompiler::IDiagnosticReporter {
	shader_precompiler::PrintDiagnostic pr;
	std::array<std::size_t, 4> errors{};
	SemanticDiagnostic(const SemanticDiagnostic&) = delete;
	SemanticDiagnostic(const SemanticDiagnostic&&) = delete;
public:
	SemanticDiagnostic() : pr(shader_precompiler::locales::Locales::ENGLISH), errors() {}

	void report(const shader_precompiler::Error& error) override {
		pr.report(error);
		errors[(unsigned short)error.level]++;
	}

	std::size_t getErrorsCount(shader_precompiler::Error::Level level) const {
		return errors.at((unsigned short)level);
	}
};

#define ASSERT_SIZE(base, target) ASSERT_EQ(base.size(), target) << ::testing::PrintToString(tokensToDebugStrings(base));
#define ASSERT_EQ_AST(rightTree, tree) ASSERT_EQ(rightTree, tree) << (tree).toDebugString(0) << "RIGHT:\n" << (rightTree).toDebugString(0);
#define EXPECT_TEXT(base, target) EXPECT_EQ(base.text, target) << ::testing::PrintToString(base.toDebugString());
#define EXPECT_TYPE(base, target) EXPECT_EQ(base.type, target) << ::testing::PrintToString(base.toDebugString());