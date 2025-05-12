cmake_minimum_required(VERSION 3.28)
project(TryOpenGL)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)

# Configure GLFW
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

set(CMAKE_XCODE_GENERATE_SCHEME OFF)

# Thirdparty libraries
add_subdirectory( vendor/glad )
add_subdirectory( vendor/glfw )
add_subdirectory( vendor/imgui )

add_executable( ${PROJECT_NAME}
        src/main.cpp

        #vendor/imgui/backends/imgui_impl_glfw.cpp
        #vendor/imgui/backends/imgui_impl_opengl3.cpp
        vendor/stb_image.h

        #src/imguiStyling.cpp
        src/Shader.cpp
        src/Shader.h
        src/Camera.cpp
        src/Camera.h
        src/Quake3Bsp.cpp
        src/Quake3Bsp.h
        src/Quake3Types.h
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
        src/PlayerMovement.cpp
        src/PlayerMovement.h
        
        src/SourceBSPTypes.h
        src/SourceBSPAsset.cpp
        src/SourceBSPAsset.h
        src/SourceBspMesh.cpp
        src/SourceBspMesh.h
        src/SourceMapScene.cpp
        src/SourceMapScene.h
        
        src/Player.cpp
        src/Player.h
               
        src/GoldSrcMDLAsset.cpp
        src/GoldSrcMDLAsset.h
               
        src/GoldSrcModel.cpp
        src/GoldSrcModel.h
        src/StudioRenderer.cpp
        src/StudioRenderer.h
        
        src/Q3Bezier.cpp
        src/Q3Bezier.h
        
        src/Q3LightGrid.cpp
        src/Q3LightGrid.h
        
        src/Monster.cpp
        src/Monster.h
        
        src/Application.cpp
        src/Application.h
        src/Input.cpp
        src/Input.h
        
        src/HUD.cpp
        src/HUD.h
        src/DebugRenderer.cpp
        src/DebugRenderer.h
        src/Cube.cpp
        src/Cube.h
        
        src/miniaudio.c
        src/ma_reverb_node.cpp
        src/verblib.cpp
        src/verblib.h
)


#set(SHADERS_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders")
#file(GLOB_RECURSE SHADER_FILES "${SHADERS_FOLDER}/*")
#source_group("Shaders" FILES ${SHADER_FILES})
#add_custom_target(Shaders SOURCES ${SHADER_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE vendor/glm)

target_link_libraries(${PROJECT_NAME} PRIVATE glad glfw imgui)

#file(COPY assets DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}")

add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}/assets
)
