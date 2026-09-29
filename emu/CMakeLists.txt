cmake_minimum_required(VERSION 3.20)
project(8kto_emu VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# generates `compile_commands.json` for clangd / LSP
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# compiler flags
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -Werror -Wpedantic \
-Wshadow -Wnon-virtual-dtor -Wold-style-cast \
-Wcast-align -Wunused -Woverloaded-virtual \
-Wconversion -Wsign-conversion -Wnull-dereference \
-Wdouble-promotion -Wformat=2 -Wmisleading-indentation \
-Wduplicated-cond -Wduplicated-branches -Wlogical-op \
-Wuseless-cast -fsanitize=address,undefined,leak")

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall -Wextra -Werror -Wpedantic \
-Wshadow -Wnon-virtual-dtor -Wold-style-cast \
-Wcast-align -Wunused -Woverloaded-virtual \
-Wconversion -Wsign-conversion -Wnull-dereference \
-Wdouble-promotion -Wformat=2 -Wmisleading-indentation \
-Wduplicated-cond -Wduplicated-branches -Wlogical-op \
-Wuseless-cast -fsanitize=address,undefined,leak")

# glob all .cpp files in src/
file(GLOB SOURCES "src/*.cpp")

# glob all .hpp files in src/ (optional, for IDEs)
file(GLOB HEADERS "src/*.hpp")

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /permissive-)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -Wshadow -Wconversion -O2)
endif()
