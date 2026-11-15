> This document was generated with the help of Claude (Claude Code, model Claude
> Opus 5.5). The accompanying transcript documents the GenAI interaction that
> led to this style guide.
> https://drive.google.com/drive/folders/1F1GXk-Lc0R8Pw2vfJegA5LrB7_Lp38Ph?usp=drive_link

# Eighty Eighty Emulator C++ Style Guide

> **Status: Draft.** All standards subject to team discussion. Tooling has not
> yet been implemented, and existing code has not been updated according to
> these conventions.

This style guide is based on
[Google C++ Style](https://google.github.io/styleguide/cppguide.html), adjusted
to match the conventions this team already uses. Formatting rules
(indentation, braces, spacing, includes) are enforced by `clang-format`. Naming,
semantics, and architectural organization are enforced in code review.

## Files

- Code is generally organized into separate header and C++ files. Header and C++
  files are placed into the same directory; no separate includes folder.
    - Headers use `.hpp`; source files use `.cpp`: `cpu.hpp`, `cpu.cpp`.
    - Interfaces are header-only: `memory_bus.hpp`, `io_bus.hpp`.
- File names are `snake_case`: `memory_bus.hpp`, `invaders_machine.cpp`.
- Test files are prefixed with `test_`: `test_shift_register.cpp`,
  `tests/spec/test_arithmetic.cpp`.
- Committed files use Unix-style line endings.
- Every file ends with a single newline.

## Header guards

Header files should use `#define` header guards instead of the non-standard
`#pragma once`. The format of the symbol name should be `<FEATURE>_HPP`.

```cpp
#ifndef CPU_HPP
#define CPU_HPP

// ...

#endif
```

## Includes

Group includes in the following order at the top of the file, with one blank
line between groups, sorted alphabetically within each group:

1. The file's own header (for `.cpp` files only)
2. C++ standard library
3. Third-party libraries (SDL, Catch2)
4. Other project headers

```cpp
#include "invaders_machine.hpp"

#include <cstdint>
#include <vector>

#include <SDL3/SDL.h>

#include "core/cpu.hpp"
#include "platform/machine.hpp"
```

- Use `<...>` for standard and third-party headers, `"..."` for project headers.
- Write project include paths relative to `src/` (`"core/cpu.hpp"`), not with
  `../` or a leading `src/`. A file may include a header from its own directory
  by name alone (`"timing.hpp"`).
- Include all headers/libraries explicitly rather than relying on cascading includes.

## Formatting

- Indent using 4 spaces.
- Aim for line length of around 80 characters with a maximum of 100; exceed this
  limit only when necessary.
- Opening braces go on the same line as the statement, function, or class:

    ```cpp
    void CPU::push_word(MemoryBus& memory, uint16_t value) {
        ...
    }
    ```

- Braces may be omitted for single-line bodies of `if`, `for`, and `while`
  statements.
- `return` statements should be placed on their own lines except in the case of
  single-line function definitions which may be placed inline.

    ```cpp
    const char* title() const override { return "Space Invaders"; }`
    ```

- Place one space after control keywords: `if (x)`, not `if(x)`.
- Attach `*` and `&` to the type: `MemoryBus& memory`, `IOBus* io`.
- Left-align `public:`/`private:` with the `class` keyword.

## Naming

| Token                                   | Style                               | Example                            |
| --------------------------------------- | ----------------------------------- | ---------------------------------- |
| Types (classes, structs, enums)         | `PascalCase`                        | `ShiftRegister`, `InvadersMachine` |
| Functions and methods                   | `snake_case`                        | `push_word`, `read_byte`           |
| Variables and parameters                | `snake_case`                        | `frame_cycles`, `addr`             |
| Private data members                    | `snake_case_` (trailing underscore) | `renderer_`, `cpu_`                |
| Public struct fields                    | `snake_case`                        | `RomFile::name`                    |
| Constants (`constexpr`, `static const`) | `UPPER_SNAKE_CASE`                  | `CYCLES_PER_FRAME`, `FLAG_CARRY`   |
| Namespaces                              | `snake_case`                        | `invaders`, `spec`                 |

- Put acronyms in type names in all-caps: `CPU`, `IOBus`, `InvadersIO`.

## Comments and documentation

- In general, prefer `//` over `/* */` except for long comment blocks (10+
  lines) describing emulated hardware behavior, documenting mappings, etc.
- Document classes and public member functions in the header file with a `//`
  block directly above the declaration.

```cpp
// Port I/O for IN/OUT. The machine (e.g. Space Invaders) implements this with
// its own devices: input ports, the shift register, sound triggers.
class IOBus {
public:
    virtual ~IOBus() = default;

    // IN port: value placed in A
    virtual uint8_t in(uint8_t port) = 0;

    // OUT port: A is sent to the port
    virtual void out(uint8_t port, uint8_t value) = 0;
};
```

- Short, self-explanatory functions (simple getters and setters, `reset()`,
  etc.) don't need a comment.
- When behavior, implementation, values, etc. come from an official document or
  other reference, cite the source and provide links where possible:

```cpp
// Intel 8080 Microcomputer Systems User's Manual, DAA instruction:
// adjusts the accumulator to two BCD digits after an addition.
```

```cpp
namespace invaders {
    // Clocks (from MAME https://github.com/mamedev/mame/blob/master/src/mame/midw8080/mw8080bw.h)
    // ...
}
```

- Mark unfinished work with `TODO: ` and include the GitHub issue number, if
  applicable.

```cpp
// TODO: ports 3 and 5 trigger sounds (#48)
```

### GenAI usage

- Cite GenAI usage according to the
  [CS 467 course policy](https://canvas.oregonstate.edu/courses/2061602/assignments/syllabus#gen-ai-policy).
- GenAI citations, where relevant, should be placed at the top of the file,
  before includes.
