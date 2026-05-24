#include "messages.hpp"

const std::map<shader_precompiler::Error::ErrorCodes, std::string> EN_MESSAGES
{
    {
        shader_precompiler::Error::ErrorCodes::UNEXPECTED_CHAR,
        "Unexpected char '{}'"
    },

    {
        shader_precompiler::Error::ErrorCodes::LEVEL_DIRECTIVE,
        "{}"
    },
    {
        shader_precompiler::Error::ErrorCodes::UNEXPECTED_DIRECTIVE,
        "Unexpected directive '{}'"
    },
    {
        shader_precompiler::Error::ErrorCodes::UNEXPECTED_MACRO_NAME,
        "Unexpected macro name '{}'"
    },
    {
        shader_precompiler::Error::ErrorCodes::UNEXPECTED_START_TOKEN,
        "Unexpected start token '{}'"
    },
};

std::string shader_precompiler::locales::getString(shader_precompiler::locales::Locales locale, shader_precompiler::Error::ErrorCodes code) {
    try {
        return EN_MESSAGES.at(code);
    }
    catch (...) {
        return "invalid localization";
    }
}