#pragma once
#include "GameCore.h"

using namespace GameCore;

class D3DInitApp final : public IGameApp
{

public:
    D3DInitApp() = default;

    virtual void Startup() override;
    virtual void Cleanup() override;

    virtual void Update(float deltaT) override;
    virtual void RenderScene() override;
    virtual void SetWindowTitle() override;
private:

    static constexpr  UINT FrameCount = 2;

    //渲染管道对象
    com_ptr<IDXGISwapChain3> m_swapChain;
    com_ptr<IDXGIAdapter1> m_HardwareAdapter;
    com_ptr<IDXGIOutput> m_output;
    com_ptr<IDXGIDebug> m_debug;

    com_ptr<ID3D12Device> m_device;
    com_ptr<ID3D12Resource> m_renderTargets[FrameCount];
    com_ptr<ID3D12CommandAllocator> m_commandAllocator;
    com_ptr<ID3D12CommandQueue> m_commandQueue;
    com_ptr<ID3D12DescriptorHeap> m_rtvHeap;
    com_ptr<ID3D12PipelineState> m_pipelineState;
    com_ptr<ID3D12GraphicsCommandList> m_commandList;
    UINT m_rtvDescriptorSize;

    //同步对象
    UINT m_frameIndex;
    HANDLE m_fenceEvent;
    com_ptr<ID3D12Fence> m_fence;
    UINT64 m_fenceValue;

    //是否启用软件模拟设备
    bool m_useWarpDevice = false;

    void LoadPipeline();
    void LoadAssert();
    void PopulateCommandList();
    void WaitForPreviousFrame();
};