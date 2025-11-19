cmake_minimum_required (VERSION 3.10)

# Настройка политик CMake
cmake_policy(SET CMP0074 NEW)
if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

project (v8unpack)

set (v8unpack_VERSION_MAJOR 3)
set (v8unpack_VERSION_MINOR 0)

if(WIN32)
set(RC_FILE "VersionInfo.rc")
else()
set(RC_FILE "")
endif()

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

# Linux-specific definition
if(UNIX)
add_definitions(-D__LINUX)
endif()

set (V8UNPACK_SOURCES
src/main.cpp
src/binarydecimalnumber.cpp
src/common.cpp
src/EctoSoftTree.cpp
src/ExactStructureBuilder.cpp
src/logger.cpp
src/mdCommand.cpp
src/mdForm.cpp
src/mdLang.cpp
src/mdMoxel.cpp
src/mdObject.cpp
src/messageregistration.cpp
src/parse_tree.cpp
src/placeholder216.cpp
src/StringUtils.cpp
src/THashedStringList.cpp
src/TMSTree.cpp
src/tree.cpp
src/treeparser.cpp
src/TStringList.cpp
src/utils.cpp
src/V8File.cpp
src/VersionFile.cpp
# SystemClasses files
src/SystemClasses/String.cpp
src/SystemClasses/System.Classes.cpp
src/SystemClasses/System.SysUtils.cpp
src/SystemClasses/System.IOUtils.cpp
src/SystemClasses/System.cpp
src/SystemClasses/TStream.cpp
src/SystemClasses/TMemoryStream.cpp
src/SystemClasses/TFileStream.cpp
src/SystemClasses/TStreamReader.cpp
src/SystemClasses/TStreamWriter.cpp
src/SystemClasses/GetTickCount.cpp
)
set (V8UNPACK_HEADERS
src/binarydecimalnumber.h
src/common.h
src/EctoSoftTree.h
src/guids.h
src/logger.h
src/mdCommand.h
src/mdForm.h
src/mdLang.h
src/mdMoxel.h
src/mdObject.h
src/messageregistration.h
src/NodeTypes.h
src/parse_tree.h
src/StringUtils.h
src/THashedStringList.h
src/TMSTree.h
src/tree.h
src/TStringList.h
src/V8File.h
src/version.h
src/VersionFile.h
src/ExactStructureBuilder.hpp
src/treeparser.hpp
)

if((MSVC) AND (NOT DISABLE_SINGLE_FILE))

foreach(flag_var
CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
SIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)

MATCHES "/MD")
g(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
dif(${flag_var} MATCHES "/MD")

endforeach(flag_var)

set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB:MSVCRT")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:MSVCRT")
set (CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:MSVCRTD")

endif()

set(CMAKE_CXX_STANDARD 14)
add_executable (v8unpack ${V8UNPACK_SOURCES} ${V8UNPACK_HEADERS} ${RC_FILE})

# Include directories for source files
include_directories (src src/SystemClasses)

# КАСТОМНЫЙ ПОИСК BOOST - Обход проблемы FindBoost.cmake
message(STATUS "Starting custom Boost search...")

# Проверка переменных окружения
if(DEFINED ENV{BOOST_ROOT})
    set(BOOST_ROOT $ENV{BOOST_ROOT})
    message(STATUS "BOOST_ROOT from environment: ${BOOST_ROOT}")
endif()

# Пути поиска Boost
set(BOOST_SEARCH_PATHS
    ${BOOST_ROOT}
    /usr/include
    /usr/local/include
    /opt/local/include
    C:/boost
    C:/Boost
    C:/local/boost
    C:/Libraries/boost_1_75_0
    C:/Libraries/boost_1_70_0
    C:/Libraries/boost_1_60_0
)

# Поиск Boost headers
find_path(Boost_INCLUDE_DIR
    NAMES boost/version.hpp
    PATHS ${BOOST_SEARCH_PATHS}
    PATH_SUFFIXES boost
    DOC "Boost include directory"
)

if(Boost_INCLUDE_DIR)
    message(STATUS "Found Boost include directory: ${Boost_INCLUDE_DIR}")
    include_directories(${Boost_INCLUDE_DIR})
else()
    message(FATAL_ERROR "Boost headers not found. Please install Boost or set BOOST_ROOT environment variable.")
endif()

# Поиск Boost libraries
set(BOOST_LIB_SEARCH_PATHS
    ${BOOST_ROOT}/lib
    ${Boost_INCLUDE_DIR}/../lib
    /usr/lib
    /usr/local/lib
    /opt/local/lib
    C:/boost/lib
    C:/Boost/lib
    C:/local/boost/lib
    C:/Libraries/boost_1_75_0/lib
    C:/Libraries/boost_1_70_0/lib
    C:/Libraries/boost_1_60_0/lib
)

find_library(BOOST_FILESYSTEM_LIBRARY
    NAMES boost_filesystem boost_filesystem-mt
    PATHS ${BOOST_LIB_SEARCH_PATHS}
    DOC "Boost filesystem library"
)

find_library(BOOST_SYSTEM_LIBRARY
    NAMES boost_system boost_system-mt
    PATHS ${BOOST_LIB_SEARCH_PATHS}
    DOC "Boost system library"
)

find_library(BOOST_IOSTREAMS_LIBRARY
    NAMES boost_iostreams boost_iostreams-mt
    PATHS ${BOOST_LIB_SEARCH_PATHS}
    DOC "Boost iostreams library"
)

if(NOT BOOST_FILESYSTEM_LIBRARY OR NOT BOOST_SYSTEM_LIBRARY OR NOT BOOST_IOSTREAMS_LIBRARY)
    message(FATAL_ERROR "Boost libraries not found. Please install Boost or set BOOST_ROOT environment variable.")
endif()

message(STATUS "Found Boost libraries successfully")

# Линковка с Boost
target_link_libraries(v8unpack 
    ${BOOST_FILESYSTEM_LIBRARY} 
    ${BOOST_SYSTEM_LIBRARY} 
    ${BOOST_IOSTREAMS_LIBRARY}
)

# Поиск ZLIB
find_package (ZLIB REQUIRED)
include_directories (${ZLIB_INCLUDE_DIRS})
target_link_libraries (v8unpack ${ZLIB_LIBRARIES})
