##cmake_minimum_required(VERSION 3.13)
##project(WebApp CXX)
##
##set(CMAKE_CXX_STANDARD 17)
##
##
##file(GLOB_RECURSE HEADERS "include/*.h" "include/*.hpp")
##
##add_executable(main main.cpp ${HEADERS})
##
##target_include_directories(main PRIVATE ${CMAKE_SOURCE_DIR}/include)
##
##target_link_options(main PRIVATE "-sFETCH=1")

cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_COMPILER "em++")

file(GLOB_RECURSE HEADERS "include/*.h" "include/*.hpp")

add_executable(frontend main.cpp ${HEADERS})

target_include_directories(frontend PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
# Forzar salida a wasm + js + html
set_target_properties(frontend PROPERTIES
    OUTPUT_NAME "main"
    SUFFIX ".html"
)

target_link_options(frontend PRIVATE
    "-sFETCH"
    "-sUSE_ES6_IMPORT_META=0"
    "-sALLOW_MEMORY_GROWTH=1"
    "--shell-file" "${CMAKE_CURRENT_SOURCE_DIR}/index.html"
)

