cmake_minimum_required(VERSION 3.9)
project(emulator)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

set(source_dir ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(header_ext .h)
set(source_ext .cpp)

# Targets
set(exec_target emulator)

# ! В БУДУЩЕМ УКАЗАТЬ КОНКРЕТНЫЕ СПИСКИ ! 
file(GLOB_RECURSE sources ${source_dir}/*${source_ext})
file(GLOB_RECURSE headers ${source_dir}/*${header_ext})

# Create executable
add_executable(${exec_target} main${source_ext} ${sources})

# Set headers
foreach(_header ${headers})
    get_filename_component(_include_dir ${_header} DIRECTORY)
    target_include_directories(${exec_target} PRIVATE ${_include_dir})
endforeach()