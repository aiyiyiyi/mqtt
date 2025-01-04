cmake_minimum_required(VERSION 3.16)

project(myproject C CXX)

#add_executable(globals globals.c)
#add_executable(PUBACK PUBACK.c connect_and_connack.c globals.c)
#add_executable(PUBLISH PUBLISH.c connect_and_connack.c globals.c)
#add_executable(CONNECT connect_and_connack.c globals.c)
add_executable(main main.c connect_and_connack.c PUBACK.c PUBLISH.c globals.c tcp.c init.c)

# add_library(mylib SHARED ncJSON.c)

# target_link_libraries(client mylib)
# target_link_libraries(server mylib)

# target_include_directories(mylib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -Wshadow -Wuninitialized -Wunused -Wextra -Wformat -fsanitize=undefined -fsanitize=address -fsanitize=leak  -fno-omit-frame-pointer -ggdb ")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -Wall -Werror -Wshadow -Wuninitialized -Wunused  -Wextra -Wformat  -fsanitize=undefined -fsanitize=address -fno-omit-frame-pointer -ggdb ")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address -fsanitize=leak -fno-omit-frame-pointer")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/output/)
    message(STATUS "The CMAKE_INSTALL_PREFIX is not set. Using default value: ${CMAKE_INSTALL_PREFIX}")
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

set(CMAKE_LIBRARY_OUTPUT_DIRECTOPY ${PROJECT_BINARY}/libso)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/liba)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#install(TARGETS client server DESTINATION lib/x86_64)

