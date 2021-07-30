# Options:
#
# * BUILD_TESTS =  ON / OFF
# * LIBAV_INCLUDE and LIBAV_LIB for ffmpeg libraries to explicitly hint paths

cmake_minimum_required(VERSION 3.16)
project(Learn-OpenGL)

set(temp_directory ${PROJECT_SOURCE_DIR}/temp)
execute_process(COMMAND ${CMAKE_COMMAND} -E remove_directory ${temp_directory})
file(MAKE_DIRECTORY ${temp_directory})

set(CMAKE_CXX_STANDARD 20)

include(${PROJECT_SOURCE_DIR}/scripts/UtilityFunctions.cmake)
include(${PROJECT_SOURCE_DIR}/scripts/DependenciesSetup.cmake)

# libraries
subdirectories_list(libraries_list "${PROJECT_SOURCE_DIR}/libraries")
foreach(library ${libraries_list})
    add_subdirectory(${library})
endforeach()

# projects supposedly dependent on the libraries above
subdirectories_list(projects_list "${PROJECT_SOURCE_DIR}/projects")
foreach(project ${projects_list})
    add_subdirectory(${project})
endforeach()
