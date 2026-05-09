# Style Guide
- All header files should have `#pragma once` guards to prevent multiple inclusion.
- With few exceptions, place code in a namespace reflect module structure (e.g. `precompiler`, ... ).
## Function using
- Raw `new` / `delete` usage is discouraged. Prefer smart pointers.
- You must use `constexpr`, not `#define`
## Includes
- Include headers in the following order: Related header, C system headers, C++ standard library headers, other libraries' headers, your project's headers.
- Do not use . or .. in paths in `#include`.
- Separate each non-empty group with one blank line.
## Naming style
- All classes must have `PascalCase` naming style
- All function names must be in `camelCase` and should preferably be verb-based (e.g. `createResource`, `loadTexture` ).
- All variables must have `camelCase` naming style
- All namespaces must have `snake_case` naming style
- Constants (constexpr variables) must have `UPPER_SNAKE_CASE` naming style
- Values in `enum class` must have `UPPER_SNAKE_CASE` naming style
- Private member variables and functions may use trailing underscore (`_`).
## File locations and names
- Every header file does must have `.hpp` extension
- Every source file does must have `.cpp` extension