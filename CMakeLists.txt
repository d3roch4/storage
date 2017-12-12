cmake_minimum_required(VERSION 2.8)

project(guardiao)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

add_subdirectory(../mor mor)
add_subdirectory(test)

file(GLOB CPP_FILES src/*.cpp)
file(GLOB HPP_FILES src/*.h)

add_library(${PROJECT_NAME} ${CPP_FILES} ${HPP_FILES})
target_link_libraries(${PROJECT_NAME} mor pq)
target_include_directories(${PROJECT_NAME} PUBLIC src)

install(TARGETS ${PROJECT_NAME} DESTINATION lib)
install(FILES ${HPP_FILES} DESTINATION include/${PROJECT_NAME})
