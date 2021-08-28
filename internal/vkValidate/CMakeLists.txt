set(proj vkValidate)
add_library(${proj} src/vulkanValidation.cpp)

target_include_directories(${proj} PUBLIC src/)

#message(STATUS "='${Vulkan_INCLUDE_DIRS}'")

target_include_directories(${proj}
    PRIVATE ${Vulkan_INCLUDE_DIRS}
    PRIVATE ${PROJECT_SOURCE_DIR}/external/glfw/include
)

target_link_libraries(${proj} Vulkan::Vulkan glm glfw)

# list(APPEND vkValidateIncludes src/headers/vulkanValidation.hpp)

set (vkVIncludeDir ${CMAKE_CURRENT_SOURCE_DIR}/src/ PARENT_SCOPE) 


# message(STATUS "='${vkValidateIncludes}'")
# message(STATUS ${CMAKE_CURRENT_SOURCE_DIR})

# foreach(kek ${vkValidateIncludes})
#   message(STATUS "='${kek}'")
# endforeach()