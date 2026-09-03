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
        shader_precompiler::Error::ErrorCodes::UNEXPECTED_TOKEN,
        "Unexpected token '{}'"
    },
    {
        shader_precompiler::Error::ErrorCodes::CANT_OPEN_FILE,
        "Can`t open file '{}'"
    },
    {
        shader_precompiler::Error::ErrorCodes::TOKENS_AFTER_DIRECTIVE_AND_BEFORE_NEW_LINE,
        "TOKENS_AFTER_DIRECTIVE_AND_BEFORE_NEW_LINE '{}'"
    },
    {
        shader_precompiler::Error::ErrorCodes::UNEXPECTED_START_TOKEN,
        "Unexpected start token '{}'"
    },
    {
        shader_precompiler::Error::ErrorCodes::NO_CLOSE_ATTRIBUTE_TOKEN,
        "No closed attribute token"
    },
    {
        shader_precompiler::Error::ErrorCodes::NO_CLOSE_BRACKET_TOKEN,
        "No closed bracket token"
    },
    {
        shader_precompiler::Error::ErrorCodes::TYPE_ALONE,
        "Type alone '{}'"
    },
    {
        shader_precompiler::Error::ErrorCodes::UNDEFINDED_TYPE,
        "Undefinded type '{}'"
    },
    {
        shader_precompiler::Error::ErrorCodes::UNDEFINDED_VARIABLE,
        "Undefinded variable '{}'"
    },
    {
        shader_precompiler::Error::ErrorCodes::UNDEFINDED_FUNCTION,
        "Undefinded function '{}'"
    },
    {
        shader_precompiler::Error::ErrorCodes::TOKEN_IS_NULL,
        "Token is null '{}'"
    },
    {
        shader_precompiler::Error::ErrorCodes::REDEFINITION_VARIABLE,
        "Redefinition variable '{}'"
    },
    {
        shader_precompiler::Error::ErrorCodes::REDEFINITION_FUNCTION,
        "Redefinition function '{}'"
    },
    {
        shader_precompiler::Error::ErrorCodes::LAYOUT_IN_INVALID_PARAMS,
        "[[__glsl_layout_in(  )]] invalid params"
    },{
        shader_precompiler::Error::ErrorCodes::NO_DIRECTIVE_NAME,
        "Directive has no name"
    },
    {
        shader_precompiler::Error::ErrorCodes::DELETING_BY_DEFINE,
        "Macro '{}' expands to nothing"
    },
    {
        shader_precompiler::Error::ErrorCodes::MULTIPLE_ATTRIBUTES,
        "Multiple conflicting attributes on the same declaration"
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