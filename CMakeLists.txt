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

add_definitions (-DBOOST_ALL_NO_LIB)

# Include directories for source files
include_directories (src src/SystemClasses)

# Windows-specific Boost paths (only if needed)
if(WIN32)
	# Проверка переменных окружения Boost
	if(DEFINED ENV{BOOST_ROOT})
		set(BOOST_ROOT $ENV{BOOST_ROOT})
		message(STATUS "Using BOOST_ROOT from environment: ${BOOST_ROOT}")
		list(APPEND CMAKE_PREFIX_PATH ${BOOST_ROOT})
	endif()
	if(DEFINED ENV{BOOST_LIBRARYDIR})
		set(BOOST_LIBRARYDIR $ENV{BOOST_LIBRARYDIR})
		message(STATUS "Using BOOST_LIBRARYDIR from environment: ${BOOST_LIBRARYDIR}")
		list(APPEND CMAKE_LIBRARY_PATH ${BOOST_LIBRARYDIR})
	endif()
	if(DEFINED ENV{BOOST_INCLUDEDIR})
		set(BOOST_INCLUDEDIR $ENV{BOOST_INCLUDEDIR})
		message(STATUS "Using BOOST_INCLUDEDIR from environment: ${BOOST_INCLUDEDIR}")
		list(APPEND CMAKE_INCLUDE_PATH ${BOOST_INCLUDEDIR})
	endif()

	# Автоматическое определение путей vcpkg для Windows
	if(DEFINED ENV{VCPKG_ROOT})
		set(VCPKG_ROOT $ENV{VCPKG_ROOT})
		message(STATUS "Using VCPKG_ROOT from environment: ${VCPKG_ROOT}")
	elseif(DEFINED ENV{VCPKG_ROOT_DIR})
		set(VCPKG_ROOT $ENV{VCPKG_ROOT_DIR})
		message(STATUS "Using VCPKG_ROOT_DIR from environment: ${VCPKG_ROOT}")
	else()
		# Поиск vcpkg в стандартных местах
		find_path(VCPKG_ROOT patterns/names.txt PATHS
			"$ENV{USERPROFILE}/vcpkg"
			"C:/vcpkg"
			"D:/vcpkg"
			"E:/vcpkg"
			PATH_SUFFIXES installed/x64-windows installed/x86-windows
			DOC "Path to vcpkg installation"
		)
		if(VCPKG_ROOT)
			message(STATUS "Found vcpkg at: ${VCPKG_ROOT}")
		endif()
	endif()
	
	# Добавление путей vcpkg в поиск
	if(VCPKG_ROOT)
		list(APPEND CMAKE_PREFIX_PATH "${VCPKG_ROOT}/installed/x64-windows/share")
		list(APPEND CMAKE_PREFIX_PATH "${VCPKG_ROOT}/installed/x86-windows/share")
		list(APPEND CMAKE_PREFIX_PATH "${VCPKG_ROOT}/installed/x64-windows")
		list(APPEND CMAKE_PREFIX_PATH "${VCPKG_ROOT}/installed/x86-windows")
		
		# Добавление модулей CMake в путь поиска
		list(APPEND CMAKE_MODULE_PATH "${VCPKG_ROOT}/installed/x64-windows/share/cmake")
		list(APPEND CMAKE_MODULE_PATH "${VCPKG_ROOT}/installed/x86-windows/share/cmake")
		list(APPEND CMAKE_MODULE_PATH "${VCPKG_ROOT}/installed/x64-windows/lib/cmake/boost")
		list(APPEND CMAKE_MODULE_PATH "${VCPKG_ROOT}/installed/x86-windows/lib/cmake/boost")
	endif()
	
	# Альтернативные пути для модулей CMake
	list(APPEND CMAKE_MODULE_PATH "C:/boost/lib/cmake" "C:/Boost/lib/cmake" "C:/local/boost/lib/cmake")
	
	# Дополнительные пути для стандартного FindBoost модуля
	list(APPEND CMAKE_MODULE_PATH "C:/Program Files/CMake/share/cmake-3.28/Modules" "C:/Program Files/CMake/share/cmake-3.27/Modules" "C:/Program Files/CMake/share/cmake-3.26/Modules")
	
	# Альтернативные пути для Boost
	list(APPEND CMAKE_PREFIX_PATH "C:/boost" "C:/Boost" "C:/local/boost" "C:/Libraries/boost_1_75_0" "C:/Libraries/boost_1_70_0" "C:/Libraries/boost_1_65_1")
endif()

set (Boost_USE_STATIC_LIBS OFF)
set (Boost_USE_MULTITHREADED ON)
set (Boost_USE_STATIC_RUNTIME OFF)

find_package (Boost 1.53 REQUIRED COMPONENTS filesystem system iostreams)

include_directories (${Boost_INCLUDE_DIRS})

target_link_libraries (v8unpack ${Boost_LIBRARIES})

find_package (ZLIB REQUIRED)

include_directories (${ZLIB_INCLUDE_DIRS})
