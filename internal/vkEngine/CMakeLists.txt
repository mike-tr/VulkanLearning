set(proj vkEngine)
set(includeDir ${proj}IncludeDirs)

add_library(${proj} src/engine.cpp src/vulkanDevice.cpp src/vulkanWSI.cpp)
target_compile_features(${proj} PRIVATE cxx_std_20)

target_include_directories(${proj} PUBLIC src/)
target_include_directories(${proj} PRIVATE ${vkVIncludeDir})

# add include directories
target_include_directories(${proj}
    PRIVATE ${Vulkan_INCLUDE_DIRS}
    PRIVATE ${PROJECT_SOURCE_DIR}/external/glfw/include
)

# add dependencies to this subproject
target_link_libraries(${proj} Vulkan::Vulkan glm glfw vkValidate)

# create variable that holds reference to location of all source files ( includes )
set (${includeDir} ${CMAKE_CURRENT_SOURCE_DIR}/src/ PARENT_SCOPE) 