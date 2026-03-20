# vcpkg provides directx-headers (find_package works after toolchain is loaded)
find_package(directx-headers CONFIG REQUIRED)

set( HEADER_FILES 
    Include/DX12LibPCH.h
    Include/Application.h
    Include/CommandQueue.h
    Include/Game.h
    Include/Helpers.h
    Include/HighResolutionClock.h
   # resource.h
    Include/Window.h
)

set( SOURCE_FILES
    Source/Application.cpp
    Source/CommandQueue.cpp
    Source/Game.cpp
    Source/HighResolutionClock.cpp
    Source/Window.cpp
)


set( SHADER_FILES
    Shaders/VertexShader.hlsl
    Shaders/PixelShader.hlsl
)

source_group( "Resources\\Shaders" FILES ${SHADER_FILES} )

set_source_files_properties( shaders/VertexShader.hlsl PROPERTIES 
    VS_SHADER_TYPE Vertex
    VS_SHADER_MODEL 5.1
)

set_source_files_properties( shaders/PixelShader.hlsl PROPERTIES 
    VS_SHADER_TYPE Pixel
    VS_SHADER_MODEL 5.1
)

add_library( Engine STATIC
    ${HEADER_FILES}
    ${SOURCE_FILES}
    ${SHADER_FILES}
    ${RESOURCE_FILES}
)

target_include_directories( Engine
    PUBLIC Include
)

target_link_libraries( Engine
    PUBLIC d3d12.lib
    PUBLIC dxgi.lib
    PUBLIC dxguid.lib
    PUBLIC Microsoft::DirectX-Headers
)