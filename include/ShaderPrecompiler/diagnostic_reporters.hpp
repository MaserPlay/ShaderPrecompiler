#pragma once

#include <iostream>
#include <fmt/format.h>

#include "messages.hpp"

namespace shader_precompiler {
	class PrintDiagnostic : public IDiagnosticReporter {
		bool hasErrors_{ false };
		locales::Locales locale;
		std::ostream& out;
	public:
		PrintDiagnostic(locales::Locales locale, std::ostream& out = std::cout) : locale(locale), out(out) {}
		void report(const Error& error) override;
	};
};