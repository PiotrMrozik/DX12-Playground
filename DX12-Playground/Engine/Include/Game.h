/**
 * @brief Abstract base class for DirectX 12 demos.
 */
#pragma once

#include <memory>
#include <string>

#include <directx/d3d12.h>
#include <wrl.h>

#include <Events.h>
#include <Window.h>

class CommandQueue;

class Game : public std::enable_shared_from_this<Game>
{
public:
    Game(const std::wstring& name, int width, int height, bool vSync);
    virtual ~Game();

    int GetClientWidth() const
    {
        return m_Width;
    }
    int GetClientHeight() const
    {
        return m_Height;
    }

    /**
     * Initialize the DirectX runtime, create the window, and allocate
     * shared GPU resources (MSAA heaps).
     */
    virtual bool Initialize();

    virtual bool LoadContent() = 0;
    virtual void UnloadContent() = 0;
    virtual void Destroy();

protected:
    friend class Window;

    // ---------------------------------------------------------------
    // Event handlers — all have default implementations
    // ---------------------------------------------------------------
    virtual void OnUpdate(UpdateEventArgs& e);
    virtual void OnRender(RenderEventArgs& e);

    // Escape = quit, Alt+Enter / F11 = fullscreen, V = vsync toggle
    virtual void OnKeyPressed(KeyEventArgs& e);
    virtual void OnKeyReleased(KeyEventArgs& e);
    virtual void OnMouseMoved(MouseMotionEventArgs& e);
    virtual void OnMouseButtonPressed(MouseButtonEventArgs& e);
    virtual void OnMouseButtonReleased(MouseButtonEventArgs& e);

    // Mouse wheel adjusts m_FoV (clamped to [12, 90])
    virtual void OnMouseWheel(MouseWheelEventArgs& e);

    // Updates dimensions, viewport, scissor rect, and MSAA resources
    virtual void OnResize(ResizeEventArgs& e);
    virtual void OnWindowDestroy();

    // ---------------------------------------------------------------
    // Helpers (A, B, D)
    // ---------------------------------------------------------------

    // Insert a transition barrier for a single resource (A)
    void TransitionResource(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
                            Microsoft::WRL::ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES beforeState,
                            D3D12_RESOURCE_STATES afterState);

    // Create a default-heap buffer and upload data via a staging buffer (B)
    void UpdateBufferResource(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
                              ID3D12Resource** pDestinationResource, ID3D12Resource** pIntermediateResource,
                              size_t numElements, size_t elementSize, const void* bufferData,
                              D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);

    // ExecuteCommandList + Present + WaitForFenceValue in one call (D)
    void Present(std::shared_ptr<CommandQueue> commandQueue,
                 Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList);

    // Recreate MSAA colour RT and MSAA depth buffer at the given size (E)
    void ResizeMSAAResources(int width, int height);

    // ---------------------------------------------------------------
    // Shared state
    // ---------------------------------------------------------------

    // Viewport and scissor rect, kept in sync with the window size (C)
    D3D12_VIEWPORT m_Viewport;
    D3D12_RECT m_ScissorRect;

    // Per-frame fence values for triple-buffered sync (D)
    uint64_t m_FenceValues[Window::BufferCount] = {};

    // MSAA intermediate colour render target (E)
    Microsoft::WRL::ComPtr<ID3D12Resource> m_MSAARenderTarget;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_MSAARTVHeap;

    // MSAA depth buffer (E)
    Microsoft::WRL::ComPtr<ID3D12Resource> m_DepthBuffer;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DSVHeap;

    // MSAA parameters — queried in Initialize() (E)
    UINT m_MSAASampleCount = 4;
    UINT m_MSAAQualityLevel = 0;

    // Field of view in degrees, adjusted by mouse wheel (J)
    float m_FoV = 45.0f;

    // Set to true at the end of LoadContent; guards resize callbacks (E)
    bool m_ContentLoaded = false;

    std::shared_ptr<Window> m_pWindow;

private:
    std::wstring m_Name;
    int m_Width;
    int m_Height;
    bool m_vSync;
};
