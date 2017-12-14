cmake_minimum_required(VERSION 2.8)

project(storage)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

if(EXISTS ${PROJECT_SOURCE_DIR}/deps/mor/CMakeLists.txt AND NOT TARGET mor)
    add_subdirectory(deps/mor)
endif()

add_subdirectory(test)

file(GLOB CPP_FILES src/backend.cpp)
file(GLOB HPP_FILES src/backend.h)

find_package (sqlite3)
if (SQLITE3_FOUND)
  set(CPP_FILES ${CPP_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/src/sqlite.cpp)
  include_directories(${SQLITE3_INCLUDE_DIRS})
  set(LIBRARIES_FOR_BACKEND ${LIBRARIES_FOR_BACKEND} sqlite3)
endif (SQLITE3_FOUND)

find_package (PostgreSQL)
if (POSTGRESQL_FOUND)
  set(CPP_FILES ${CPP_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/src/postgresql.cpp)
  include_directories(${POSTGRESQL_INCLUDE_DIR})
  set(LIBRARIES_FOR_BACKEND ${LIBRARIES_FOR_BACKEND} ${POSTGRESQL_LIBRARIES})
endif (POSTGRESQL_FOUND)
message(STATUS "CPP_FILES ${CPP_FILES}")
message(STATUS "LIBRARIES_FOR_BACKEND ${LIBRARIES_FOR_BACKEND}")

add_library(${PROJECT_NAME} ${CPP_FILES} ${HPP_FILES})
target_link_libraries(${PROJECT_NAME} mor pq ${LIBRARIES_FOR_BACKEND})
target_include_directories(${PROJECT_NAME} PUBLIC src)

install(TARGETS ${PROJECT_NAME} DESTINATION lib)
install(FILES ${HPP_FILES} DESTINATION include/${PROJECT_NAME})
