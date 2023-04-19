#include "D3D12Init.h"

void D3DInitApp::Startup()
{
	LoadPipeline();
	LoadAssert();
}

void D3DInitApp::Cleanup()
{
	WaitForPreviousFrame();

	DXGIGetDebugInterface1(NULL, WINRT_IID_PPV_ARGS(m_debug));
	m_debug->ReportLiveObjects(DXGI_DEBUG_DXGI, (DXGI_DEBUG_RLO_FLAGS)(DXGI_DEBUG_RLO_IGNORE_INTERNAL | DXGI_DEBUG_RLO_DETAIL));

	CloseHandle(m_fenceEvent);
}

void D3DInitApp::Update(float deltaT)
{
	SetWindowTitle();
}

void D3DInitApp::RenderScene()
{
	//分配指令
	PopulateCommandList();

	//执行指令
	ID3D12CommandList* ppCommandLists[] = { m_commandList.get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	//页面翻转
	ThrowIfFailed(m_swapChain->Present(1, 0));

	//执行同步
	WaitForPreviousFrame();
}

void D3DInitApp::SetWindowTitle()
{
	DXGI_ADAPTER_DESC1 desc;
	m_HardwareAdapter->GetDesc1(&desc);

	wstring Title = GetGameClassName();
	wstring AdapterInfo = Title + L"  Adapter:" + desc.Description;

	SetWindowText(GetHwnd(), AdapterInfo.c_str());
}

void D3DInitApp::LoadPipeline()
{
	UINT dxgiFactoryFlags = 0;

	//1.启用调试层
#if defined(_DEBUG)
	{
		com_ptr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(WINRT_IID_PPV_ARGS(debugController))))
		{
			debugController->EnableDebugLayer();

			//开启附加调试层
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}
#endif

	//2.创建设备
	com_ptr<IDXGIFactory4> factory;
	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, WINRT_IID_PPV_ARGS(factory)));
	if (m_useWarpDevice)
	{
		com_ptr<IDXGIAdapter> warpAdapter;
		ThrowIfFailed(factory->EnumWarpAdapter(WINRT_IID_PPV_ARGS(warpAdapter)));

		ThrowIfFailed(D3D12CreateDevice(
			warpAdapter.get(),
			D3D_FEATURE_LEVEL_11_0,
			WINRT_IID_PPV_ARGS(m_device)
		));
	}
	else
	{
		GetHardwareAdapter(factory.get(), m_HardwareAdapter.put());
		ThrowIfFailed(D3D12CreateDevice(
			m_HardwareAdapter.get(),
			D3D_FEATURE_LEVEL_11_0,
			WINRT_IID_PPV_ARGS(m_device)
		));
	}

	//设置显存相关
	com_ptr<IDXGIAdapter3> dxgiAdapter3 = nullptr;
	m_HardwareAdapter->QueryInterface(WINRT_IID_PPV_ARGS(dxgiAdapter3));
	DXGI_QUERY_VIDEO_MEMORY_INFO memInfo;
	if (SUCCEEDED(dxgiAdapter3->SetVideoMemoryReservation(NULL, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, MEM_GiB(1))))
	{
		dxgiAdapter3->QueryVideoMemoryInfo(NULL, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &memInfo);
	}

	//获取output相关
	POINT mousePosition;
	GetCursorPos(&mousePosition);
	HMONITOR monitor = MonitorFromPoint(mousePosition, MONITOR_DEFAULTTOPRIMARY);

	UINT index = 0;
	DXGI_OUTPUT_DESC outputDesc;
	com_ptr<IDXGIOutput> output = nullptr;
	while (SUCCEEDED(dxgiAdapter3->EnumOutputs(index, output.put())))
	{
		ZeroMemory(&outputDesc, sizeof(DXGI_OUTPUT_DESC));

		if (SUCCEEDED(output->GetDesc(&outputDesc)))
		{
			if ((outputDesc.Monitor == monitor))
			{
				m_output = output;
				break;
			}
			else
			{
				output = nullptr;
			}
		}
		index++;
	}

	//创建命令队列
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, WINRT_IID_PPV_ARGS(m_commandQueue)));

	//创建交换链
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = FrameCount;
	swapChainDesc.Width = GetDisplayWidth();
	swapChainDesc.Height = GetDisplayHeight();
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	com_ptr<IDXGISwapChain1> swapChain;
	ThrowIfFailed(factory->CreateSwapChainForHwnd(
		m_commandQueue.get(), // 交换链需要一个命令队列来强制刷新
		GetHwnd(),
		&swapChainDesc,
		nullptr,
		nullptr,
		swapChain.put()
	));

	ThrowIfFailed(factory->MakeWindowAssociation(GetHwnd(), DXGI_MWA_NO_ALT_ENTER));//禁用alt+enter全屏

	m_swapChain = swapChain.as<IDXGISwapChain3>();

	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	//创建渲染器目标视图 (RTV) 描述符堆
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = FrameCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, WINRT_IID_PPV_ARGS(m_rtvHeap)));

	m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	//创建帧资源（每个帧的渲染器目标视图）。
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT n = 0; n < FrameCount; n++)
	{
		ThrowIfFailed(m_swapChain->GetBuffer(n, WINRT_IID_PPV_ARGS(m_renderTargets[n])));
		m_device->CreateRenderTargetView(m_renderTargets[n].get(), nullptr, rtvHandle);
		rtvHandle.Offset(1, m_rtvDescriptorSize);
	}

	//创建命令分配器。
	ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, WINRT_IID_PPV_ARGS(m_commandAllocator)));
}

void D3DInitApp::LoadAssert()
{
	//创建命令列表。关闭命令列表。
	ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.get(), nullptr, WINRT_IID_PPV_ARGS(m_commandList)));
	ThrowIfFailed(m_commandList->Close());

	//创建围栏。(围栏用于将 CPU 与 GPU 同步 (多引擎同步) 。)
	ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, WINRT_IID_PPV_ARGS(m_fence)));
	m_fenceValue = 1;
	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (m_fenceEvent == nullptr)
	{
		ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
	}
}

void D3DInitApp::PopulateCommandList()
{
	//allocators的reset方法必须要在其关联的commandlists在gpu上执行完所有指令之后才能调用，应用程序应该通过fence来决定gpu的执行进度
	ThrowIfFailed(m_commandAllocator->Reset());

	//当ExecuteCommandList方法执行之后，commandlist在进行下一次指令记录之前必须要先调用reset()
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.get(), m_pipelineState.get()));

	//将后台缓冲转换为rtv
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);

	//清理后台缓冲
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	//将后台缓冲转换为 present.
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	ThrowIfFailed(m_commandList->Close());
}

void D3DInitApp::WaitForPreviousFrame()
{
	//发送同步指令
	const UINT64 fence = m_fenceValue;
	ThrowIfFailed(m_commandQueue->Signal(m_fence.get(), fence));
	m_fenceValue++;

	// 等待同步完成
	if (m_fence->GetCompletedValue() < fence)
	{
		ThrowIfFailed(m_fence->SetEventOnCompletion(fence, m_fenceEvent));
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}

	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}
