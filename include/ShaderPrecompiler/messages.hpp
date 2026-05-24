#pragma once

#include <map>
#include <string>

#include "shader_precompiler.hpp"

namespace shader_precompiler::locales {

    enum class Locales
    {
        ENGLISH
    };

    std::string getString(Locales locale, shader_precompiler::Error::ErrorCodes code);
};