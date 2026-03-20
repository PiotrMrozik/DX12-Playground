set( HEADER_FILES
    Include/Sample.h
)

set( SRC_FILES
    Source/main.cpp
    Source/Sample.cpp
)

set( SHADER_FILES
    Shaders/VertexShader.hlsl
    Shaders/PixelShader.hlsl
)

source_group( "Shaders" FILES ${SHADER_FILES} )

set_source_files_properties( Shaders/VertexShader.hlsl PROPERTIES
    VS_SHADER_TYPE Vertex
    VS_SHADER_MODEL 5.1
    VS_SHADER_ENTRYPOINT main
    VS_SHADER_OBJECT_FILE_NAME "$(OutDir)%(Filename).cso"
)

set_source_files_properties( Shaders/PixelShader.hlsl PROPERTIES
    VS_SHADER_TYPE Pixel
    VS_SHADER_MODEL 5.1
    VS_SHADER_ENTRYPOINT main
    VS_SHADER_OBJECT_FILE_NAME "$(OutDir)%(Filename).cso"
)

add_executable( Sample WIN32
    ${HEADER_FILES}
    ${SRC_FILES}
    ${SHADER_FILES}
)

target_include_directories( Sample
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include
)

target_link_libraries( Sample
    Engine
    D3DCompiler.lib
    Shlwapi.lib
)