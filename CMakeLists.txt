cmake_minimum_required (VERSION 3.10)

cmake_policy(SET CMP0074 NEW)

project (v8unpack)

set (v8unpack_VERSION_MAJOR 3)
set (v8unpack_VERSION_MINOR 0)

if(WIN32)
	set(RC_FILE "VersionInfo.rc")
else()
	set(RC_FILE "")
endif()

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

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
	src/utils.cpp
	src/V8File.h
	src/version.h
	src/VersionFile.h
	src/zconf.h 
	src/zlib.h 
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

SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "C:/Libraries/boost_1_60_0")
SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "C:/Libraries/boost_1_60_0/lib32-msvc-15.0")


# For Linux, use shared libraries. For Windows MSVC, static libraries are configured in appveyor.yml
if(WIN32 AND MSVC)
	set (Boost_USE_STATIC_LIBS ON)
	set (Boost_USE_MULTITHREADED OFF)
	set (Boost_USE_STATIC_RUNTIME ON)
else()
	# Linux: use system shared libraries
	set (Boost_USE_STATIC_LIBS OFF)
	set (Boost_USE_MULTITHREADED OFF)
	set (Boost_USE_STATIC_RUNTIME OFF)
endif()

find_package (Boost 1.53 REQUIRED COMPONENTS filesystem system iostreams property_tree locale)


include_directories (${Boost_INCLUDE_DIRS})
include_directories (${ZLIB_INCLUDE_DIRS})
include_directories (src/SystemClasses)

target_link_libraries (v8unpack ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})
