# ffmpeg libraries
find_package(PkgConfig)

if(${PkgConfig_FOUND} AND (NOT DEFINED LIBAV_INCLUDE OR NOT DEFINED LIBAV_LIB))
    pkg_check_modules(AVCODEC REQUIRED IMPORTED_TARGET libavcodec)
    pkg_check_modules(AVFORMAT REQUIRED IMPORTED_TARGET libavformat)
    pkg_check_modules(AVUTIL REQUIRED IMPORTED_TARGET libavutil)
    pkg_check_modules(SWSCALE REQUIRED IMPORTED_TARGET libswscale)
endif()

# GCE-Math (Generalized Constant Expression Math) is a templated C++ library enabling compile-time
# computation of mathematical functions.
set(gcem_path ${PROJECT_SOURCE_DIR}/vendor/gcem)
# FIXME(poppyfanboy) I can't figure out how to change the include prefix of the existing target, so
# that when you include the library in code, it is `#include<gcem/gcem.hpp>` instead of just
# `#include<gcem.hpp>`.
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${gcem_path}/include
                        ${temp_directory}/gcem/include/gcem)
add_library(gcem INTERFACE)
target_include_directories(gcem INTERFACE ${temp_directory}/gcem/include)

# GSL
add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/gsl)

# google test (testing and mocking framework)
enable_testing()
add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/googletest)

# fmt (string format library, an implementation of c++20 std::format)
add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/fmt)

# glad (OpenGL functions loader)
add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/glad)

# GLFW (cross-platform opengl context and window creation)
set(GLFW_BUILD_DOCS
    OFF
    CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS
    OFF
    CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES
    OFF
    CACHE BOOL "" FORCE)
add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/glfw)

# stb (image loader)
set(stb_path ${PROJECT_SOURCE_DIR}/vendor/stb)
file(COPY ${stb_path}/stb_image.h DESTINATION ${temp_directory}/stb/include/stb/)
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${temp_directory}/stb/include/)

# glm (math library)
set(glm_path ${PROJECT_SOURCE_DIR}/vendor/glm/)
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${glm_path}/glm
                        ${temp_directory}/glm/include/glm)
add_library(glm INTERFACE)
target_include_directories(glm INTERFACE ${temp_directory}/glm/include/)
target_compile_definitions(glm INTERFACE GLM_FORCE_SILENT_WARNINGS)

# assimp (model loader)
set(ASSIMP_BUILD_TESTS OFF)
add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/assimp)

# Sparsepp (hash map optimized in terms of memory for small number of items)
set(sparsepp_path ${PROJECT_SOURCE_DIR}/vendor/sparsepp)
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${sparsepp_path}/sparsepp
                        ${temp_directory}/sparsepp/include/sparsepp)
add_library(sparsepp INTERFACE)
target_include_directories(sparsepp INTERFACE ${temp_directory}/sparsepp/include)
