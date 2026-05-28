#pragma once

#include <fmt/format.h>

#include "messages.hpp"

namespace shader_precompiler {
	class PrintDiagnostic : public IDiagnosticReporter {
		bool hasErrors_{ false };
		locales::Locales locale;
	public:
		PrintDiagnostic(locales::Locales locale) : locale(locale) {}
		void report(const Error& error) override;
	};
};