cmake_minimum_required(VERSION 3.31)

project(glint)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

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

find_package(Vulkan REQUIRED)

set(IMGUI_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/external/imgui")

file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE IMGUI_SOURCES "${IMGUI_INCLUDE_DIR}/*.cpp")

add_executable(${PROJECT_NAME} 
  ${SOURCES}
  ${IMGUI_SOURCES}
)

target_compile_definitions(${PROJECT_NAME} PRIVATE 
  ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets/"
  PROJ_NAME="${PROJECT_NAME}"
)

set(STB_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/external/stb")

set(DISCORD_SDK_ROOT "${CMAKE_SOURCE_DIR}/external/discord_social_sdk")
set(DISCORD_SDK_LIB_DIR "${DISCORD_SDK_ROOT}/lib/release")
set(DISCORD_SDK_BIN_DIR "${DISCORD_SDK_ROOT}/bin/release")
set(DISCORD_SDK_INCLUDE_DIR "${DISCORD_SDK_ROOT}/include")

set(NLOHMANN_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/external/nlohmann")

target_include_directories(${PROJECT_NAME} PRIVATE 
  ${PROJECT_SOURCE_DIR}/include
  ${STB_INCLUDE_DIR}
  ${IMGUI_INCLUDE_DIR}
  ${DISCORD_SDK_INCLUDE_DIR}
  ${NLOHMANN_INCLUDE_DIR}

  ${Vulkan_INCLUDE_DIRS}
  ${glm_SOURCE_DIR}
  ${vma_SOURCE_DIR}/include
  ${FFMPEG_DIR}/include
)

target_link_directories(${PROJECT_NAME} PRIVATE
  ${FFMPEG_DIR}/lib
)

set(DISCORD_LIB_PATH "${DISCORD_SDK_LIB_DIR}/discord_partner_sdk.lib")
set(DISCORD_SHARED_LIB "${DISCORD_SDK_BIN_DIR}/discord_partner_sdk.dll")

target_link_libraries(${PROJECT_NAME} PRIVATE 
  glfw
  fmt::fmt
  Vulkan::Vulkan
  avcodec.lib
  avformat.lib
  avutil.lib
  swscale.lib
  swresample.lib
  ${DISCORD_LIB_PATH}
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${DISCORD_SHARED_LIB}"
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
)

if(WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(${PROJECT_NAME} PRIVATE -static-libgcc -static-libstdc++)
endif()

if(WIN32)
    set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/glint.rc")
    target_sources(${PROJECT_NAME} PRIVATE ${APP_ICON_RESOURCE_WINDOWS})
endif()