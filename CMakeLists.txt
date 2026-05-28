cmake_minimum_required (VERSION 3.21)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project (ShaderPrecompiler VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_TESTS "Build tests" OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/arc/${CMAKE_BUILD_TYPE})

include(FetchContent)

FetchContent_Declare(
	argparse
	GIT_REPOSITORY	https://github.com/p-ranav/argparse.git
	GIT_TAG 	3eda91b2e1ce7d569f84ba295507c4cd8fd96910 #tag/3.2
)
FetchContent_MakeAvailable(argparse)

FetchContent_Declare(
	fmt
	GIT_REPOSITORY	https://github.com/fmtlib/fmt.git
	GIT_TAG 	407c905e45ad75fc29bf0f9bb7c5c2fd3475976f #tag/12.1.0
)
FetchContent_MakeAvailable(fmt)

add_library(${PROJECT_NAME}Core STATIC
     
    "src/precompiler.cpp"
 "src/utils/string_utils.cpp" "src/lexer.cpp" "src/ast.cpp" "src/minimazer.cpp" "src/messages.cpp" "src/diagnostic_reporters.cpp" "src/semantic.cpp")

add_library(${PROJECT_NAME}::Core ALIAS ${PROJECT_NAME}Core)

target_include_directories(${PROJECT_NAME}Core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/ShaderPrecompiler>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

add_executable(${PROJECT_NAME} "src/main.cpp")

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/ShaderPrecompiler>
)

target_compile_definitions(${PROJECT_NAME} PUBLIC
    APPVERSION="${PROJECT_VERSION}"
    APPNAME="${PROJECT_NAME}"
    APPAUTHOR="MaserPlay"
)
target_link_libraries(${PROJECT_NAME}Core PUBLIC fmt::fmt)
target_link_libraries(${PROJECT_NAME} PUBLIC argparse ${PROJECT_NAME}::Core)

if (BUILD_TESTS)
    add_subdirectory(tests)
endif()