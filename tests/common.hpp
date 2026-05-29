#pragma once

#include <gtest/gtest.h>
#include <array>

#include "lexer.hpp"
#include "ast.hpp"
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

inline bool areEqual(
    const std::vector<std::unique_ptr<shader_precompiler::ast::nodes::Node>>& a,
    const std::vector<std::unique_ptr<shader_precompiler::ast::nodes::Node>>& b)
{
    if (a.size() != b.size())
        return false;

    return std::equal(
        a.begin(), a.end(),
        b.begin(),
        [](const auto& lhs, const auto& rhs) {

            if (!lhs || !rhs)
                return lhs == rhs;

            return *lhs == *rhs;
        }
    );
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
#define ASSERT_TRUE_AST(rightTree, tree) ASSERT_TRUE(areEqual(rightTree, tree)) << shader_precompiler::ast::toDebugString(tree) << "RIGHT:\n" << shader_precompiler::ast::toDebugString(rightTree);
#define EXPECT_TEXT(base, target) EXPECT_EQ(base.text, target) << ::testing::PrintToString(base.toDebugString());
#define EXPECT_TYPE(base, target) EXPECT_EQ(base.type, target) << ::testing::PrintToString(base.toDebugString());