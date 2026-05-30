#pragma once

#include <iostream>
#include <array>
#include <fmt/format.h>

#include "messages.hpp"

namespace shader_precompiler {
	class PrintDiagnostic : public IDiagnosticReporter {
		bool hasErrors_{ false };
		locales::Locales locale;
		std::ostream& out;
	public:
		PrintDiagnostic(locales::Locales locale = locales::Locales::ENGLISH, std::ostream& out = std::cout) : locale(locale), out(out) {}
		void report(const Error& error) override;
	};

	class CalcDiagnostic : public shader_precompiler::IDiagnosticReporter {
		shader_precompiler::IDiagnosticReporter& pr;
		std::array<std::size_t, 4> errors{};
	public:
		CalcDiagnostic(shader_precompiler::IDiagnosticReporter& pr) : pr(pr), errors() {}

		void report(const shader_precompiler::Error& error) override {
			pr.report(error);
			errors[(unsigned short)error.level]++;
		}

		std::size_t getErrorsCount(shader_precompiler::Error::Level level) const {
			return errors.at((unsigned short)level);
		}
	};
};