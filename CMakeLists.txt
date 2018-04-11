cmake_minimum_required(VERSION 2.8)

project(storage)

#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

option(BUILD_EXAMPLE "Build example code" OFF)

if(NOT TARGET mor)
    add_subdirectory(../mor mor)
endif()

if(BUILD_EXAMPLE)
    add_subdirectory(example)
endif()

file(GLOB CPP_FILES storage/backend.cpp)
file(GLOB HPP_FILES storage/backend.h)

find_package (sqlite3)
if (SQLITE3_FOUND)
  set(CPP_FILES ${CPP_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/storage/sqlite.cpp)
  include_directories(${SQLITE3_INCLUDE_DIRS})
  set(LIBRARIES_FOR_BACKEND ${LIBRARIES_FOR_BACKEND} ${SQLITE3_LIBRARIES})
endif (SQLITE3_FOUND)

set(PostgreSQL_ROOT "${PostgreSQL_ROOT}")
find_package(PostgreSQL)
if (PostgreSQL_FOUND)
  set(CPP_FILES ${CPP_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/storage/postgresql.cpp)
  include_directories(${POSTGRESQL_INCLUDE_DIR})
  set(LIBRARIES_FOR_BACKEND ${LIBRARIES_FOR_BACKEND} ${POSTGRESQL_LIBRARIES})
endif (PostgreSQL_FOUND)

message(STATUS "LIBRARIES_FOR_BACKEND: ${LIBRARIES_FOR_BACKEND}")
if(NOT LIBRARIES_FOR_BACKEND)
    message(FATAL_ERROR "LIBRARIES_FOR_BACKEND is empty")
endif()

add_library(${PROJECT_NAME} ${CPP_FILES} ${HPP_FILES})
target_link_libraries(${PROJECT_NAME} mor ${LIBRARIES_FOR_BACKEND})
target_include_directories(${PROJECT_NAME} PUBLIC ./)

install(TARGETS ${PROJECT_NAME} DESTINATION lib)
install(FILES ${HPP_FILES} DESTINATION include/${PROJECT_NAME})
