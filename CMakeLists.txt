cmake_minimum_required(VERSION 2.8)

project(storage)

#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} $ENV{CXXFLAGS} $ENV{CPPFLAGS}")

option(BUILD_EXAMPLE "Build example code" OFF)

if(NOT TARGET mor)
    add_subdirectory(../mor mor)
endif()
if(NOT TARGET d3util)
    add_subdirectory(../d3util d3util)
endif()

if(BUILD_EXAMPLE)
    add_subdirectory(example)
endif()

file(GLOB CPP_FILES storage/backend.cpp storage/expression.cpp)
file(GLOB HPP_FILES storage/backend.h storage/expression.h storage/query.h)

#find_package (sqlite3)
if (SQLITE3_FOUND)
  set(CPP_FILES ${CPP_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/storage/sqlite.cpp)
  include_directories(${SQLITE3_INCLUDE_DIRS})
  set(LIBRARIES_FOR_BACKEND ${LIBRARIES_FOR_BACKEND} ${SQLITE3_LIBRARIES})
endif (SQLITE3_FOUND)

find_package(PostgreSQL)
if (TRUE)
  set(CPP_FILES ${CPP_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/storage/postgresql.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/storage/postgresql.h)
  include_directories(${PostgreSQL_INCLUDE_DIRS})
  set(LIBRARIES_FOR_BACKEND ${LIBRARIES_FOR_BACKEND} PRIVATE pq)
endif ()

message(STATUS "LIBRARIES_FOR_BACKEND: ${LIBRARIES_FOR_BACKEND}")
if(NOT LIBRARIES_FOR_BACKEND)
    message(FATAL_ERROR "LIBRARIES_FOR_BACKEND is empty")
endif()

add_compile_definitions(BOOST_STACKTRACE_GNU_SOURCE_NOT_REQUIRED=1)
find_package(Boost REQUIRED COMPONENTS system)
include_directories(src ${Boost_INCLUDE_DIRS})

add_library(${PROJECT_NAME} ${CPP_FILES} ${HPP_FILES})
target_link_libraries(${PROJECT_NAME} ${LIBRARIES_FOR_BACKEND} mor d3util )
target_include_directories(${PROJECT_NAME} PUBLIC ./)

install(TARGETS ${PROJECT_NAME} DESTINATION lib)
install(FILES ${HPP_FILES} DESTINATION include/${PROJECT_NAME})
