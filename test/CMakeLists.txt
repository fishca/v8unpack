# Test CMakeLists для проверки новой архитектуры

cmake_minimum_required(VERSION 3.10)
project(V8UnpackArchitectureTest)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Пути к boost
find_package(Boost REQUIRED COMPONENTS filesystem system)

# Пути к исходным файлам
set(SRC_DIR ${CMAKE_SOURCE_DIR}/../src)
set(TEST_DIR ${CMAKE_SOURCE_DIR}/../test)

# Включаемые директории
include_directories(
    ${SRC_DIR}
    ${SRC_DIR}/app
    ${SRC_DIR}/commands
    ${SRC_DIR}/metadata
    ${SRC_DIR}/fileops
    ${SRC_DIR}/utils
    ${SRC_DIR}/SystemClasses
    ${Boost_INCLUDE_DIRS}
)

# Собираем все новые cpp файлы архитектуры
set(ARCHITECTURE_SOURCES
    # App layer
    ${SRC_DIR}/app/Application.cpp
    ${SRC_DIR}/app/ArgumentParser.cpp
    ${SRC_DIR}/app/CommandBridge.cpp

    # Metadata layer
    ${SRC_DIR}/metadata/MetadataAnalyzer.cpp
    ${SRC_DIR}/metadata/RegexRegistry.cpp

    # FileOps layer
    ${SRC_DIR}/fileops/BatchProcessor.cpp

    # Utils layer
    ${SRC_DIR}/utils/ErrorCodes.cpp
    ${SRC_DIR}/utils/StringConverters.cpp

    # Command system
    ${SRC_DIR}/commands/Command.cpp
    ${SRC_DIR}/commands/CommandFactory.cpp
    ${SRC_DIR}/commands/CommandRegistry.cpp
    ${SRC_DIR}/commands/ConsoleMessageRegistrator.cpp

    # Concrete commands
    ${SRC_DIR}/commands/concrete/UnpackCommand.cpp
    ${SRC_DIR}/commands/concrete/PackCommand.cpp
    ${SRC_DIR}/commands/concrete/ParseCommand.cpp
    ${SRC_DIR}/commands/concrete/ListCommand.cpp
    ${SRC_DIR}/commands/concrete/BuildCommand.cpp
    ${SRC_DIR}/commands/concrete/HelpCommand.cpp
    ${SRC_DIR}/commands/concrete/DeflateCommand.cpp
    ${SRC_DIR}/commands/concrete/InflateCommand.cpp
    ${SRC_DIR}/commands/concrete/BatCommand.cpp
    ${SRC_DIR}/commands/concrete/ExampleCommand.cpp

    # System Classes (needed for commands)
    ${SRC_DIR}/SystemClasses/String.cpp
    ${SRC_DIR}/SystemClasses/System.Classes.cpp
    ${SRC_DIR}/SystemClasses/System.cpp
    ${SRC_DIR}/SystemClasses/System.SysUtils.cpp
    ${SRC_DIR}/SystemClasses/System.IOUtils.cpp
)

# Тест новой архитектуры
add_executable(test_new_architecture
    ${TEST_DIR}/test_new_main.cpp
    ${ARCHITECTURE_SOURCES}
)

target_link_libraries(test_new_architecture
    ${Boost_LIBRARIES}
)

# Демо приложения новой архитектуры
add_executable(new_main_demo
    ${SRC_DIR}/new_main.cpp
    ${ARCHITECTURE_SOURCES}
)

target_link_libraries(new_main_demo
    ${Boost_LIBRARIES}
)

# Выводим сообщение при конфигурации
message(STATUS "=== V8Unpack Architecture Test Configuration ===")
message(STATUS "Source directory: ${SRC_DIR}")
message(STATUS "Architecture sources: ${ARCHITECTURE_SOURCES}")
message(STATUS "Boost includes: ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost libraries: ${Boost_LIBRARIES}")

# Простой тест основных компонентов
set(SIMPLE_SOURCES
    ${SRC_DIR}/app/ArgumentParser.cpp
    ${SRC_DIR}/metadata/MetadataAnalyzer.cpp
    ${SRC_DIR}/metadata/RegexRegistry.cpp
    ${SRC_DIR}/utils/ErrorCodes.cpp
    ${SRC_DIR}/utils/StringConverters.cpp
    ${SRC_DIR}/SystemClasses/String.cpp
)

add_executable(simple_test ../test/simple_test.cpp ${SIMPLE_SOURCES})
target_link_libraries(simple_test ${Boost_LIBRARIES})
target_compile_definitions(simple_test PRIVATE BOOST_FILESYSTEM_NO_DEPRECATED)
