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
			CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
			CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)

		if(${flag_var} MATCHES "/MD")
			string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
		endif(${flag_var} MATCHES "/MD")

	endforeach(flag_var)

	set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB:MSVCRT")
	set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:MSVCRT")
	set (CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:MSVCRTD")

endif()

set(CMAKE_CXX_STANDARD 14)
add_executable (v8unpack ${V8UNPACK_SOURCES} ${V8UNPACK_HEADERS} ${RC_FILE})

# Include directories for source files
include_directories (src src/SystemClasses)

# Поиск Boost
if(CMAKE_TOOLCHAIN_FILE MATCHES "vcpkg")
    # Для vcpkg сначала пробуем стандартный find_package
    find_package(Boost COMPONENTS filesystem system iostreams CONFIG QUIET)
    if(NOT Boost_FOUND)
        # Если не нашли через CONFIG, пробуем через модуль FindBoost
        find_package(Boost COMPONENTS filesystem system iostreams QUIET)
    endif()

    if(Boost_FOUND)
        message(STATUS "Found Boost via vcpkg toolchain")
        include_directories(${Boost_INCLUDE_DIRS})
        target_link_libraries(v8unpack ${Boost_LIBRARIES})
    else()
        # Fallback: используем кастомный поиск для vcpkg
        message(WARNING "Standard Boost search with vcpkg failed, using custom search")

        # Improved vcpkg Boost search logic
        # Check common vcpkg installation patterns
        if(DEFINED ENV{VCPKG_ROOT})
            set(VCPKG_ROOT $ENV{VCPKG_ROOT})
        endif()

        # Determine target triplet - defaults to x64-windows on Windows
        if(NOT VCPKG_TARGET_TRIPLET)
            if(WIN32)
                if(CMAKE_SIZEOF_VOID_P EQUAL 8)
                    set(VCPKG_TARGET_TRIPLET "x64-windows")
                else()
                    set(VCPKG_TARGET_TRIPLET "x86-windows")
                endif()
            else()
                if(CMAKE_SIZEOF_VOID_P EQUAL 8)
                    set(VCPKG_TARGET_TRIPLET "x64-linux")
                else()
                    set(VCPKG_TARGET_TRIPLET "x86-linux")
                endif()
            endif()
        endif()

        # Common vcpkg installation paths for Boost headers
        set(BOOST_SEARCH_PATHS
            # Default vcpkg installed location
            C:/Tools/vcpkg/installed/${VCPKG_TARGET_TRIPLET}/include
            # Alternative vcpkg locations
            C:/vcpkg/installed/${VCPKG_TARGET_TRIPLET}/include
            ${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/include
            # Relative to current directory
            ${CMAKE_CURRENT_BINARY_DIR}/../../vcpkg/installed/${VCPKG_TARGET_TRIPLET}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/../vcpkg/installed/${VCPKG_TARGET_TRIPLET}/include
            # Standard package location (for when using existing vcpkg installation)
            C:/Tools/vcpkg/packages/boost-headers_x64-windows/include
            C:/Tools/vcpkg/packages/boost-headers_x86-windows/include
        )

        find_path(Boost_INCLUDE_DIR
            NAMES boost/version.hpp
            PATHS ${BOOST_SEARCH_PATHS}
            DOC "Boost include directory from vcpkg"
        )

        if(Boost_INCLUDE_DIR)
            message(STATUS "Found Boost include directory via vcpkg custom search: ${Boost_INCLUDE_DIR}")
            include_directories(${Boost_INCLUDE_DIR})

            # Library paths relative to found include - try multiple vcpkg patterns
            set(BOOST_LIB_SEARCH_PATHS
                ${Boost_INCLUDE_DIR}/../lib
                C:/Tools/vcpkg/installed/${VCPKG_TARGET_TRIPLET}/lib
                C:/vcpkg/installed/${VCPKG_TARGET_TRIPLET}/lib
                ${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/lib
                ${CMAKE_CURRENT_BINARY_DIR}/../../vcpkg/installed/${VCPKG_TARGET_TRIPLET}/lib
                ${CMAKE_CURRENT_SOURCE_DIR}/../vcpkg/installed/${VCPKG_TARGET_TRIPLET}/lib
                # Try the package directories directly
                C:/Tools/vcpkg/packages/boost-filesystem_${VCPKG_TARGET_TRIPLET}/lib
                C:/Tools/vcpkg/packages/boost-system_${VCPKG_TARGET_TRIPLET}/lib
                C:/Tools/vcpkg/packages/boost-iostreams_${VCPKG_TARGET_TRIPLET}/lib
                ${CMAKE_CURRENT_BINARY_DIR}/../../vcpkg/packages/boost-filesystem_${VCPKG_TARGET_TRIPLET}/lib
                ${CMAKE_CURRENT_SOURCE_DIR}/../vcpkg/packages/boost-filesystem_${VCPKG_TARGET_TRIPLET}/lib
            )

            message(STATUS "Searching for Boost libraries in: ${BOOST_LIB_SEARCH_PATHS}")

            # Try to find what boost library files actually exist
            file(GLOB_RECURSE BOOST_LIB_FILES
                "C:/Tools/vcpkg/packages/boost-filesystem*/lib/*.lib"
                "C:/Tools/vcpkg/packages/boost-system*/lib/*.lib"
                "C:/Tools/vcpkg/packages/boost-iostreams*/lib/*.lib"
                "C:/Tools/vcpkg/installed/${VCPKG_TARGET_TRIPLET}/lib/*.lib"
            )
            if(BOOST_LIB_FILES)
                message(STATUS "Found boost library files: ${BOOST_LIB_FILES}")
            else()
                message(WARNING "No boost library files found in common vcpkg locations")
            endif()

            # Find Boost libraries - try different naming patterns
            find_library(BOOST_FILESYSTEM_LIBRARY
                NAMES boost_filesystem-vc142-mt-x64-1_89 boost_filesystem-vc142-mt boost_filesystem boost_filesystem-vc142-mt-gd-x64-1_89 libboost_filesystem libboost_filesystem-vc142-mt
                PATHS ${BOOST_LIB_SEARCH_PATHS}
                DOC "Boost filesystem library from vcpkg"
            )

            find_library(BOOST_SYSTEM_LIBRARY
                NAMES boost_system-vc142-mt-x64-1_89 boost_system-vc142-mt boost_system-vc142-mt-gd-x64-1_89 boost_system libboost_system libboost_system-vc142-mt
                PATHS ${BOOST_LIB_SEARCH_PATHS}
                DOC "Boost system library from vcpkg"
                NO_DEFAULT_PATH
            )

            find_library(BOOST_IOSTREAMS_LIBRARY
                NAMES boost_iostreams-vc142-mt-x64-1_89 boost_iostreams-vc142-mt boost_iostreams-vc142-mt-gd-x64-1_89 boost_iostreams libboost_iostreams libboost_iostreams-vc142-mt
                PATHS ${BOOST_LIB_SEARCH_PATHS}
                DOC "Boost iostreams library from vcpkg"
            )

            # Debug: check if boost_system library exists in package directory
            if(NOT BOOST_SYSTEM_LIBRARY)
                file(GLOB_RECURSE BOOST_SYSTEM_FILES "C:/Tools/vcpkg/packages/boost-system*/lib/*.lib")
                message(STATUS "Boost system files found: ${BOOST_SYSTEM_FILES}")
            endif()

            # Check which libraries were actually found
            set(FOUND_LIBS "")
            if(BOOST_FILESYSTEM_LIBRARY)
                list(APPEND FOUND_LIBS ${BOOST_FILESYSTEM_LIBRARY})
            endif()
            if(BOOST_SYSTEM_LIBRARY)
                list(APPEND FOUND_LIBS ${BOOST_SYSTEM_LIBRARY})
            else()
                message(WARNING "Boost system library not found - this may be expected in newer Boost versions where it's header-only")
            endif()
            if(BOOST_IOSTREAMS_LIBRARY)
                list(APPEND FOUND_LIBS ${BOOST_IOSTREAMS_LIBRARY})
            endif()

            # Require filesystem and iostreams, but make system optional
            if(BOOST_FILESYSTEM_LIBRARY AND BOOST_IOSTREAMS_LIBRARY)
                message(STATUS "Found Boost libraries via custom search: ${FOUND_LIBS}")
                target_link_libraries(v8unpack ${FOUND_LIBS})
            else()
                message(FATAL_ERROR "Required Boost libraries not found. filesystem=${BOOST_FILESYSTEM_LIBRARY}, iostreams=${BOOST_IOSTREAMS_LIBRARY}")
            endif()
        else()
            message(FATAL_ERROR "Boost include directory not found. Please ensure Boost is properly installed via vcpkg.")
        endif()
    endif()
else()
    # КАСТОМНЫЙ ПОИСК BOOST - Обход проблемы FindBoost.cmake
    # Настройки поиска Boost
    set (Boost_USE_STATIC_LIBS OFF)
    set (Boost_USE_MULTITHREADED ON)
    set (Boost_USE_STATIC_RUNTIME OFF)

    # Поиск Boost с множественными путями и fallback
    message(STATUS "Starting custom Boost search...")

# Поиск Boost с множественными путями и fallback
message(STATUS "Starting custom Boost search...")

# Проверка переменных окружения
if(NOT BOOST_ROOT AND DEFINED ENV{BOOST_ROOT})
    set(BOOST_ROOT $ENV{BOOST_ROOT})
    message(STATUS "BOOST_ROOT from environment: ${BOOST_ROOT}")
endif()
if(NOT BOOST_LIBRARYDIR AND DEFINED ENV{BOOST_LIBRARYDIR})
    set(BOOST_LIBRARYDIR $ENV{BOOST_LIBRARYDIR})
    message(STATUS "BOOST_LIBRARYDIR from environment: ${BOOST_LIBRARYDIR}")
endif()
if(NOT BOOST_INCLUDEDIR AND DEFINED ENV{BOOST_INCLUDEDIR})
    set(BOOST_INCLUDEDIR $ENV{BOOST_INCLUDEDIR})
    message(STATUS "BOOST_INCLUDEDIR from environment: ${BOOST_INCLUDEDIR}")
endif()

if(DEFINED ENV{VCPKG_ROOT})
    set(VCPKG_ROOT $ENV{VCPKG_ROOT})
    message(STATUS "VCPKG_ROOT from environment: ${VCPKG_ROOT}")
endif()
if(DEFINED ENV{VCPKG_DIR})
    set(VCPKG_DIR $ENV{VCPKG_DIR})
    message(STATUS "VCPKG_DIR from environment: ${VCPKG_DIR}")
endif()

# Пути поиска Boost
set(BOOST_SEARCH_PATHS
    ${BOOST_ROOT}
    ${BOOST_INCLUDEDIR}/..
    /usr/include
    /usr/local/include
    /opt/local/include
    /homebrew/include
    C:/boost
    C:/Boost
    C:/local/boost
    C:/Libraries/boost_1_75_0
    C:/Libraries/boost_1_70_0
    C:/Libraries/boost_1_65_1
    C:/Libraries/boost_1_60_0
    ${VCPKG_ROOT}/installed/x64-windows/include
    ${VCPKG_ROOT}/installed/x86-windows/include
    ${VCPKG_ROOT}/installed/x64-linux/include
    ${VCPKG_DIR}/installed/x64-linux/include
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

    # Извлечение версии Boost из version.hpp
    file(READ "${Boost_INCLUDE_DIR}/boost/version.hpp" BOOST_VERSION_CONTENT)
    if(BOOST_VERSION_CONTENT MATCHES "#define BOOST_VERSION[ \\t]+([0-9]+)")
        set(BOOST_VERSION ${CMAKE_MATCH_1})
        math(EXPR BOOST_VERSION_MAJOR "${BOOST_VERSION} / 100000")
        math(EXPR BOOST_VERSION_MINOR "${BOOST_VERSION} / 100 % 1000")
        message(STATUS "Detected Boost version: ${BOOST_VERSION_MAJOR}.${BOOST_VERSION_MINOR}")
    endif()

    include_directories(${Boost_INCLUDE_DIR})
else()
    # Попытка стандартного find_package если кастомный поиск не удался
    message(WARNING "Custom Boost search failed, trying standard find_package(Boost)")
    find_package(Boost QUIET COMPONENTS filesystem system iostreams)
    if(Boost_FOUND)
        message(STATUS "Found Boost via find_package")
        include_directories(${Boost_INCLUDE_DIRS})
        target_link_libraries(v8unpack ${Boost_LIBRARIES})
        set(BOOST_FOUND_VIA_PACKAGE TRUE)
    else()
        message(FATAL_ERROR "Boost headers not found. Please install Boost or set BOOST_ROOT environment variable.")
    endif()
endif()

if(NOT BOOST_FOUND_VIA_PACKAGE)
# Поиск Boost libraries
set(BOOST_LIB_SEARCH_PATHS
    ${BOOST_LIBRARYDIR}
    ${BOOST_ROOT}/lib
    ${Boost_INCLUDE_DIR}/../lib
    /usr/lib
    /usr/local/lib
    /opt/local/lib
    /homebrew/lib
    C:/boost/lib
    C:/Boost/lib
    C:/local/boost/lib
    C:/Libraries/boost_1_75_0/lib32-msvc-15.0
    C:/Libraries/boost_1_70_0/lib32-msvc-15.0
    C:/Libraries/boost_1_65_1/lib32-msvc-14.1
    C:/Libraries/boost_1_60_0/lib32-msvc-15.0
    ${VCPKG_ROOT}/installed/x64-windows/lib
    ${VCPKG_ROOT}/installed/x86-windows/lib
    ${VCPKG_ROOT}/installed/x64-linux/lib
    ${VCPKG_DIR}/installed/x64-linux/lib
)

# Поиск требуемых библиотек Boost
find_library(BOOST_FILESYSTEM_LIBRARY
    NAMES boost_filesystem boost_filesystem-mt boost_filesystem-vc142 boost_filesystem-vc142-mt libboost_filesystem libboost_filesystem-mt libboost_filesystem-vc142 libboost_filesystem-vc142-mt boost_filesystem-mgw9-mt boost_filesystem-mgw9
    PATHS ${BOOST_LIB_SEARCH_PATHS}
    DOC "Boost filesystem library"
)

find_library(BOOST_SYSTEM_LIBRARY
    NAMES boost_system boost_system-mt boost_system-vc142 boost_system-vc142-mt libboost_system libboost_system-mt libboost_system-vc142 libboost_system-vc142-mt boost_system-mgw9-mt boost_system-mgw9
    PATHS ${BOOST_LIB_SEARCH_PATHS}
    DOC "Boost system library"
)

find_library(BOOST_IOSTREAMS_LIBRARY
    NAMES boost_iostreams boost_iostreams-mt boost_iostreams-vc142 boost_iostreams-vc142-mt libboost_iostreams libboost_iostreams-mt libboost_iostreams-vc142 libboost_iostreams-vc142-mt boost_iostreams-mgw9-mt boost_iostreams-mgw9
    PATHS ${BOOST_LIB_SEARCH_PATHS}
    DOC "Boost iostreams library"
)

# Проверка найденных библиотек
if(NOT BOOST_FILESYSTEM_LIBRARY)
    message(FATAL_ERROR "Boost filesystem library not found")
endif()
if(NOT BOOST_SYSTEM_LIBRARY)
    message(FATAL_ERROR "Boost system library not found")
endif()
if(NOT BOOST_IOSTREAMS_LIBRARY)
    message(FATAL_ERROR "Boost iostreams library not found")
endif()

message(STATUS "Found Boost libraries:")
message(STATUS "  Filesystem: ${BOOST_FILESYSTEM_LIBRARY}")
message(STATUS "  System: ${BOOST_SYSTEM_LIBRARY}")
message(STATUS "  IOStreams: ${BOOST_IOSTREAMS_LIBRARY}")

# Линковка с Boost
target_link_libraries(v8unpack 
    ${BOOST_FILESYSTEM_LIBRARY} 
    ${BOOST_SYSTEM_LIBRARY} 
    ${BOOST_IOSTREAMS_LIBRARY}
)
endif()

    # Поиск ZLIB
    find_package (ZLIB REQUIRED)
    include_directories (${ZLIB_INCLUDE_DIRS})
    target_link_libraries(v8unpack ${ZLIB_LIBRARIES})
endif()
