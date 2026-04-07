# vcpkg provides directx-headers (find_package works after toolchain is loaded)
find_package(directx-headers CONFIG REQUIRED)

file( GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS
    Include/*.h
)

set( SOURCE_FILES
    Source/Application.cpp
    Source/CommandQueue.cpp
    Source/Game.cpp
    Source/HighResolutionClock.cpp
    Source/Window.cpp
    Source/ECS/Entity/EntityManager.cpp
    Source/ECS/Component/ComponentManager.cpp
    Source/ECS/System/SystemManager.cpp
    Source/ECS/World.cpp
    Source/ECS/Prefabs/MeshObjectPrefab.cpp
    Source/Camera/OrbitCamera.cpp
    Source/Mesh/MeshRegistry.cpp
    Source/Mesh/Primitives.cpp
)

add_library( Engine STATIC
    ${HEADER_FILES}
    ${SOURCE_FILES}
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
