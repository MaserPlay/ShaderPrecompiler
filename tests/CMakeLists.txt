cmake_minimum_required(VERSION 3.21)
project(ShaderPrecompilerTests)

# GoogleTest requires at least C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_TLS_VERIFY FALSE)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
  ${PROJECT_NAME}
  "TestsMain.cpp"
)

target_link_libraries(
  ${PROJECT_NAME}
  GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(${PROJECT_NAME})

# �������� ���������� ���� (���� ����� �� ����������)
    
target_compile_definitions(${PROJECT_NAME} PUBLIC
    APPVERSION="${PROJECT_VERSION}"
    APPNAME="${PROJECT_NAME}"
    APPAUTHOR="MaserPlay"
)

# target_link_libraries(${PROJECT_NAME} GameEngine::GameEngine)

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}../src>
)