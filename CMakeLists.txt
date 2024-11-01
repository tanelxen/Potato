cmake_minimum_required(VERSION 3.28)
project(TryOpenGL)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)

# Configure GLFW
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

set(CMAKE_XCODE_GENERATE_SCHEME ON)

# Thirdparty libraries
add_subdirectory( vendor/glad )
add_subdirectory( vendor/glfw )
add_subdirectory( vendor/imgui )

add_executable( ${PROJECT_NAME}
        src/main.cpp

        vendor/imgui/backends/imgui_impl_glfw.cpp
        vendor/imgui/backends/imgui_impl_opengl3.cpp
        vendor/stb_image.h
        vendor/stb_image_write.h

        src/imguiStyling.cpp
        src/Shader.cpp
        src/Shader.h
        src/Camera.cpp
        src/Camera.h
        src/Quake3Bsp.cpp
        src/Quake3Bsp.h
        src/Quake3Types.h
        src/Grid.cpp
        src/Grid.h
        src/Cube.cpp
        src/Cube.h
        src/BrushTool.cpp
        src/BrushTool.h
        src/World.cpp
        src/World.h
        src/CubeGeometry.cpp
        src/CubeGeometry.h
        src/Q3BspMesh.cpp
        src/Q3BspMesh.h
        src/KeyValueCollection.cpp
        src/KeyValueCollection.h
        src/TextureAtlas.cpp
        src/TextureAtlas.h
        src/Q3MapScene.cpp
        src/Q3MapScene.h
        src/Q3BspCollision.cpp
        src/Q3BspCollision.h
)

target_include_directories(${PROJECT_NAME} PRIVATE vendor/glm)

target_link_libraries(${PROJECT_NAME} PRIVATE glad glfw imgui)

#file(COPY assets DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}")

add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}/assets
)
