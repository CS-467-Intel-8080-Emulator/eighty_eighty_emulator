> This document was generated with the help of Claude (Claude Code, model Claude
> Opus 5.5). The accompanying transcript documents the GenAI interaction that
> led to this document.
> https://drive.google.com/drive/folders/1OnKHKWv8dSEi50XEmH_wwSpZn72022D8?usp=drive_link

# Code Formatting

Formatting rules (indentation, braces, spacing, includes) are defined in a
shared `.clang-format` config file. The formatter can be run manually (see
[Usage](#usage)) or automatically on save using a VS Code autoformatter (see
[VS Code](#vs-code-optional)).

See the project [style guide](STYLE.md) for more information.

## Install

The CI uses `clang-format` version **23.1.5**, the latest
[PyPI distribution](https://pypi.org/project/clang-format/) at the time of
authoring. Install the pinned version using
[pipx](https://pipx.pypa.io/stable/installation/):

```sh
pipx install clang-format==23.1.5
```

This installs `clang-format` and `git clang-format`.

## Usage

### Format changes

`git clang-format` formats only the lines that have changed since a previous
commit.

```sh
# Format staged changes (then `git add` the fixes)
git clang-format

# Format all changes on your branch since it split from main
# This is what the CI checks when you submit a PR
git clang-format $(git merge-base origin/main HEAD)

# Preview: show the diff without changing any files
git clang-format --diff $(git merge-base origin/main HEAD)
```

In PowerShell, use `(git merge-base origin/main HEAD)` without the `$`.

### Format files

`clang-format` formats entire files. `-i` edits files in place; without it, the
formatted file prints to the terminal.

```sh
# Format one or more files
clang-format -i src/core/cpu.cpp src/core/cpu.hpp
```

## VS Code (optional)

To format C++ on save with the
[C/C++ extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools),
add these to your user or workspace settings (**Preferences: Open User/Workspace
Settings (JSON)**):

```jsonc
{
    "[cpp]": {
        "editor.defaultFormatter": "ms-vscode.cpptools",
        "editor.formatOnSave": true,
        "editor.formatOnSaveMode": "modifications"
    },

    "C_Cpp.formatting": "clangFormat",
    "C_Cpp.clang_format_style": "file"
}
```

Use `"editor.formatOnSaveMode": "modifications"` to format only modified lines
and avoid changing existing code. Use `"file"` when refactoring or authoring new
files.

By default, the Microsoft C/C++ extension uses the version of `clang-format` on
your environment PATH, or the version bundled with the extension; whichever is
newer. To always use the pinned version, specify the path explicitly (find it
with `which clang-format` or `where.exe clang-format`):

```jsonc
"C_Cpp.clang_format_path": "/Users/you/.local/bin/clang-format"
```
