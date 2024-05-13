cmake_minimum_required (VERSION 2.8)
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
	src/V8File.cpp 
	src/utils.cpp 
	src/placeholder216.cpp 
	src/VersionFile.cpp 
	src/atree.cpp 
	src/MetaObject.cpp 
	src/NodeTypes.cpp 
	src/v8MetaCommand.cpp 
	src/v8MetaDoc.cpp 
	src/v8MetaForm.cpp
	src/v8MetaMoxel.cpp
	src/v8MetaObject.cpp
	src/v8Tree.cpp 
	src/NodeTypes.cpp
	src/v8MetaCommonModules.cpp
	src/v8MetaInterfaces.cpp
)
set (V8UNPACK_HEADERS 
	src/V8File.h 
	src/version.h 
	src/zconf.h 
	src/zlib.h 
	src/VersionFile.h
	src/atree.h 
	src/MetaObject.h 
	src/NodeTypes.h 
	src/v8MetaCommand.h 
	src/v8MetaDoc.h 
	src/v8MetaForm.h
	src/v8MetaMoxel.h
	src/v8MetaObject.h
	src/v8Tree.h 
	src/v8constants.h
	src/tree_utils.h
	src/tree.h
	src/NodeTypes.h
	src/v8MetaCommonModules.h
	src/v8MetaInterfaces.h
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
set (Boost_USE_STATIC_LIBS ON)
set (Boost_USE_MULTITHREADED OFF)
set (Boost_USE_STATIC_RUNTIME ON)

find_package (Boost 1.53 REQUIRED COMPONENTS filesystem system iostreams)

include_directories (${Boost_INCLUDE_DIRS})
target_link_libraries (v8unpack ${Boost_LIBRARIES})


find_package (ZLIB REQUIRED)

include_directories (${ZLIB_INCLUDE_DIRS})
target_link_libraries (v8unpack ${ZLIB_LIBRARIES})

