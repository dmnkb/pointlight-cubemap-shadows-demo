cmake_minimum_required(VERSION 3.20.0)

project(PointLightShadow)

set(CMAKE_CXX_STANDARD 23)

file(GLOB_RECURSE MY_SOURCES src/*.cpp)

add_executable(${PROJECT_NAME} ${MY_SOURCES})

# Add vendored dependencies
add_subdirectory(vendor/glfw EXCLUDE_FROM_ALL)
add_subdirectory(vendor/glad EXCLUDE_FROM_ALL)
add_subdirectory(vendor/glm EXCLUDE_FROM_ALL)

# Set up include + link
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
    ${ASSIMP_INCLUDE_DIR}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE glfw 
    PRIVATE glad 
    PRIVATE glm
)
