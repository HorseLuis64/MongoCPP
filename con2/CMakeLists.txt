cmake_minimum_required(VERSION 3.16)
project(MongoExample)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Buscar todos los headers y fuentes
file(GLOB_RECURSE HEADERS "include/*.h" "../../*.h")
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Incluir headers de MongoDB y Pistache
include_directories(
    include
    ${CMAKE_SOURCE_DIR}
    /usr/local/include/mongocxx/v_noabi
    /usr/local/include/bsoncxx/v_noabi
    /usr/local/include    # Pistache headers
)

# Carpetas donde buscar librerías
link_directories(/usr/local/lib)

# Crear ejecutable
add_executable(MongoExample ${SOURCES} ${HEADERS})

# Incluir solo tus headers
target_include_directories(MongoExample PRIVATE include)

# Enlazar librerías
target_link_libraries(MongoExample PRIVATE
    mongocxx
    bsoncxx
    pthread
    pistache
)
