cmake_minimum_required(VERSION 3.21.1)
project(pragma VERSION 1.0)

# Generate a config.h file with some settings like DEBUG etc...
configure_file(config.h.in config.h)
add_executable(${PROJECT_NAME} src/main.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_BINARY_DIR})

# Download all git submodules
find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
# update submodules
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if(GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT
        )
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT} please checkout submodules")
        endif()
    endif()
endif()

# add the glfw and glm to the project
add_subdirectory(external/glfw)
add_subdirectory(external/glm)

find_package(Vulkan REQUIRED)

target_include_directories(${PROJECT_NAME}
    PRIVATE ${Vulkan_INCLUDE_DIRS}
    PRIVATE ${PROJECT_SOURCE_DIR}/external/glfw/include
)

target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan glm glfw)

# uncomment to have install
# install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})
# install(FILES "${PROJECT_BINARY_DIR}/Progconfig.hpp" DESTINATION include)