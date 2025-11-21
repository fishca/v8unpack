# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

v8Unpack is a console utility for unpacking, packing, and parsing 1C:Enterprise configuration files (*.cf, *.epf, *.erf). It's a cross-platform C++ application that handles the proprietary 1C file format with compression and container structures.

Fork of the original project by Denis Demidov (disa_da2@mail.ru):
- Original project: https://www.assembla.com/spaces/V8Unpack/team
- Original SVN repo: http://svn2.assembla.com/svn/V8Unpack/

## Build System

### Dependencies
- CMake 3.10 or higher
- Boost libraries (1.53+): filesystem, system, iostreams
- zlib
- C++14 compiler (GCC 9+ on Linux, Visual Studio 2022 on Windows)

### Build Commands

#### Linux/Ubuntu
```bash
# Install dependencies
sudo apt-add-repository ppa:dmpas/e8
sudo apt-get update
sudo apt-get install v8unpack

# Build from source
mkdir build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE="${VCPKG_DIR}/scripts/buildsystems/vcpkg.cmake" ..
cmake --build . --config Release
```

#### Windows (Visual Studio)
```bash
# Build with CMake
cmake -G "Visual Studio 14" -DBOOST_ROOT="%BOOST_ROOT%" -DBOOST_LIBRARYDIR="%BOOST_LIBRARYDIR%" -DZLIB_LIBRARY=path/to/zlibstatic.lib -DZLIB_INCLUDE_DIR=path/to/zlib .
cmake --build . --config Release
```

## Core Architecture

### File Format Handlers

The project handles two major 1C file format versions:

1. **Format 1.5** (32-bit): Traditional format using 32-bit addressing
   - File header: `stFileHeader` (16 bytes)
   - Block header: `stBlockHeader` (31 bytes)
   - Element addresses: `stElemAddr` (12 bytes)
   - Page size: 512 bytes (default)

2. **Format 1.6** (64-bit): Introduced in 1C:Enterprise 8.3.16+
   - File header: `stFileHeader64` (20 bytes)
   - Block header: `stBlockHeader64` (55 bytes)
   - Element addresses: `stElemAddr64` (24 bytes)
   - Base offset: 0x1359 ("magic offset")
   - Supports configurations without compatibility mode

### Key Classes

**CV8File** (`src/V8File.h`, `src/V8File.cpp`):
- Main container class representing a 1C file
- Manages file header, element addresses, and element data
- Handles both packed and unpacked data
- Methods: `GetData()`, `LoadFileFromFolder()`

**CV8Elem** (`src/V8File.h`, `src/V8File.cpp`):
- Represents individual elements within a V8 file
- Each element has: header (metadata), data (content), and optional nested CV8File
- Can be recursively packed/unpacked
- Supports deflate/inflate compression

**BracketParser** (`src/tree.h`):
- Parses 1C configuration metadata structure (bracket-based format)
- Builds tree representation with nodes: OBJECT, STRING, NUMBER
- Handles GUIDs and quoted strings
- Used for parsing configuration metadata

**tree** (`src/parse_tree.h`):
- Alternative tree parser for 1C text format
- State machine-based parser for configuration structure files
- Supports both value and delimiter modes

### Metadata Structure

1C configurations use GUIDs to identify different object types. The project maps these to metadata directories:

- `GUID_Catalogs` → "Справочники" (Catalogs)
- `GUID_Documents` → "Документы" (Documents)
- `GUID_Reports` → "Отчеты" (Reports)
- `GUID_DataProcessors` → "Обработки" (Data Processors)
- `GUID_CommonModules` → "Общие модули" (Common Modules)
- `GUID_CommonForms` → "Общие формы" (Common Forms)
- And many more metadata types (see `src/guids.h` and `src/main.cpp:299-345`)

### SystemClasses Layer

The `src/SystemClasses/` directory provides VCL-like abstractions:
- `TStream`, `TMemoryStream`, `TFileStream`: Stream handling
- `String.hpp`: String utilities with UTF-8 support
- `System.Classes.cpp`, `System.SysUtils.cpp`: System utilities
- These classes bridge Delphi/VCL idioms to cross-platform C++

### Data Flow

1. **Unpack**: `.cf` → `CV8File::LoadFile()` → Parse headers → Extract elements → Decompress data → Save to folder
2. **Pack**: Folder → `CV8File::LoadFileFromFolder()` → Build elements → Compress data → Write `.cf` file
3. **Parse**: `.cf` → Unpack → Parse metadata using `BracketParser` → Extract configuration structure → Save parsed output
4. **Build**: Source folder → Parse metadata → Rebuild container structure → Pack into `.cf` file

## Common Commands

### Basic Operations
```bash
# Unpack a .cf file
v8unpack -U[NPACK] input.cf output_dir [block_name]

# Pack a directory into .cf file
v8unpack -PA[CK] input_dir output.cf

# Inflate (decompress) a data file
v8unpack -I[NFLATE] input.data output_file

# Deflate (compress) a file
v8unpack -D[EFLATE] input_file output.data

# Parse configuration structure
v8unpack -P[ARSE] input_file output_dir [block_name1 block_name2 ...]

# Build .cf file from parsed structure
v8unpack -B[UILD] [-N[OPACK]] input_dir output.cf

# List files in .cf container
v8unpack -LISTFILES|-LF input.cf

# Process list file (batch operations)
v8unpack -L[IST] listfile
```

### Example Workflow
```bash
# Full unpack workflow
v8unpack -unpack 1Cv8.cf 1Cv8.unp
v8unpack -undeflate 1Cv8.unp/metadata.data 1Cv8.unp/metadata.data.und
v8unpack -unpack 1Cv8.unp/metadata.data.und 1Cv8.unp/metadata.unp

# Full pack workflow
v8unpack -pack 1Cv8.unp/metadata.unp 1Cv8.unp/metadata_new.data.und
v8unpack -deflate 1Cv8.unp/metadata_new.data.und 1Cv8.unp/metadata.data
v8unpack -pack 1Cv8.und 1Cv8_new.cf
```

## Important Implementation Notes

### Format Detection
- Check for V8 format 1.5: Signature 0x7fffffff at element addresses
- Check for V8 format 1.6: Signature 0xffffffffffffffff + offset 0x1359
- Functions: `IsV8File()` (format 1.5), `IsV8File16()` (format 1.6)

### Compression
- Uses zlib for deflate/inflate operations
- Not all elements are compressed (check before inflating)
- Helper function: `try_inflate()` attempts inflation safely

### String Encoding
- UTF-8 is primary encoding for file I/O
- Wide strings (wstring) used internally for metadata
- BOM handling: `removeBOM()` for UTF-8, `removeBOMutf16()` for UTF-16

### Memory Management
- Version 3.0 optimization: Root container now built dynamically with streaming to file
- Previous version 2.0 built entire container in memory (caused segfault for large configs)
- Use `Dispose()` methods to free resources when done

## CI/CD

- AppVeyor builds for Ubuntu and Windows
- Ubuntu: GCC-9, vcpkg for dependencies, creates .deb package
- Windows: Visual Studio 2022, creates .exe, .msi (WiX), and Chocolatey .nupkg
- Build status: https://ci.appveyor.com/project/dmpas/v8unpack/branch/master

## Encoding and Locale

The codebase uses Russian comments and metadata names. Key considerations:
- `std::locale::global(std::locale(""))` set in main()
- UTF-8 handling via Boost.Locale and custom converters
- Wide string conversions: `wstringToString()`, `WStringToString()`, `wstring_to_utf8()`
