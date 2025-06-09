cmake_minimum_required(VERSION 3.31)

project(glint)

include(FetchContent)

FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG 3.3.9
)

FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 11.2.0
)

FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG 1.0.1
)

FetchContent_Declare(
  vma
  GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
  GIT_TAG master
)

FetchContent_MakeAvailable(glfw fmt glm vma)

set(FFMPEG_DIR "${CMAKE_BINARY_DIR}/_deps/ffmpeg")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg -g")

find_package(Vulkan REQUIRED)

file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(${PROJECT_NAME} ${SOURCES})

target_compile_definitions(${PROJECT_NAME} PRIVATE ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets/")

target_include_directories(${PROJECT_NAME} PRIVATE 
  ${PROJECT_SOURCE_DIR}/include
  ${Vulkan_INCLUDE_DIRS}
  ${glm_SOURCE_DIR}
  ${vma_SOURCE_DIR}/include
  ${FFMPEG_DIR}/include
)

target_link_directories(${PROJECT_NAME} PRIVATE
  ${FFMPEG_DIR}/lib
)

target_link_libraries(${PROJECT_NAME} PRIVATE 
  glfw
  fmt::fmt
  Vulkan::Vulkan
  avcodec.lib
  avformat.lib
  avutil.lib
  swscale.lib
  swresample.lib
)

if(WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(${PROJECT_NAME} PRIVATE -static-libgcc -static-libstdc++)
endif()

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)