function(subdirectories_list result parent_directory)
    file(GLOB children "${parent_directory}/*")

    foreach(child ${children})
        if(IS_DIRECTORY ${child})
            list(APPEND subdirectories ${child})
        endif()
    endforeach()

    set("${result}"
        ${subdirectories}
        PARENT_SCOPE)
endfunction()

# Discovers gtest tests inside the "test" directory, adds a custom target to build all tests for the
# given target
function(setup_tests target_name)
    if(NOT BUILD_TESTS)
        return()
    endif()

    include(GoogleTest)

    file(GLOB test_sources ${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp)

    list(LENGTH test_sources sources_count)
    if(sources_count EQUAL 0)
        return()
    endif()

    set(test_targets "")
    foreach(test_path IN LISTS test_sources)
        get_filename_component(test_name ${test_path} NAME_WE)
        set(test_executable "${test_name}_executable")

        add_executable(${test_executable} ${test_path})
        target_link_libraries(${test_executable} PRIVATE ${target_name} gtest_main)
        gtest_add_tests(TARGET ${test_executable})
        list(APPEND test_targets ${test_executable})
    endforeach()

    add_custom_target(
        "${target_name}_tests"
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target ${test_targets}
        DEPENDS ${target_name})
endfunction()

# Sets up a generic executable project. Any private dependencies to be linked are passed by
# ${ARGV1}. Public dependencies are passed by ${ARGV2}.
function(setup_project project_name)
    set(private_dependencies ${ARGV1})
    set(public_dependencies ${ARGV2})

    file(GLOB source_files "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
    add_executable(${project_name} ${source_files})
    target_include_directories(${project_name} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(${project_name} PRIVATE ${private_dependencies})
    target_link_libraries(${project_name} PUBLIC ${public_dependencies})

    setup_tests(${project_name})
endfunction()

# Sets up a generic library. Any private dependencies to be linked are passed by ${ARGV1}. Public
# dependencies are passed by ${ARGV2}.
function(setup_library library_name)
    set(private_dependencies ${ARGV1})
    set(public_dependencies ${ARGV2})
    set(private_include ${ARGV3})
    set(public_include ${ARGV4})

    file(GLOB sources ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
    add_library(${library_name} STATIC ${sources})
    target_include_directories(${library_name} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
                                                      ${public_include}})
    target_link_libraries(${library_name} PRIVATE ${private_dependencies})
    target_link_libraries(${library_name} PUBLIC ${public_dependencies})

    setup_tests(${library_name})
endfunction()

function(setup_generic_opengl_project project_name)
    set(private_dependencies ${ARGV1})
    list(APPEND private_dependencies "Boilerplate-OpenGL;glad;glfw")
    setup_project(${project_name} "${private_dependencies}")
endfunction()
