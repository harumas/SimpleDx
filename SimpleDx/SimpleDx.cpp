#include "SimpleDx.h"

HWND SimpleDx::hwnd = {};
WNDCLASSEX SimpleDx::wnd = {};

ID3D12Device* SimpleDx::dev = nullptr;
IDXGIFactory6* SimpleDx::dxgiFactory = nullptr;
IDXGISwapChain4* SimpleDx::swapChain = nullptr;
ID3D12CommandAllocator* SimpleDx::cmdAllocator = nullptr;
ID3D12GraphicsCommandList* SimpleDx::cmdList = nullptr;
ID3D12CommandQueue* SimpleDx::cmdQueue = nullptr;
std::vector<ID3D12Resource*> SimpleDx::backBuffers;
ID3D12DescriptorHeap* SimpleDx::rtvHeaps = nullptr;
ID3D12Fence* SimpleDx::fence = nullptr;
UINT64 SimpleDx::fenceVal = 0;



LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (msg == WM_DESTROY)
	{
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}

int SimpleDx::Initialize(long width, long height, wchar_t* title)
{
	HRESULT result = S_OK;
	wnd = {};

	wnd.cbSize = sizeof(WNDCLASSEX);
	wnd.lpfnWndProc = (WNDPROC)WindowProcedure;//コールバック関数の指定
	wnd.lpszClassName = title;//アプリケーションクラス名(適当でいいです)
	wnd.hInstance = GetModuleHandle(0);//ハンドルの取得
	RegisterClassEx(&wnd);//アプリケーションクラス(こういうの作るからよろしくってOSに予告する)

	RECT wrc = { 0, 0, width, height };

	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);//ウィンドウのサイズはちょっと面倒なので関数を使って補正する
	//ウィンドウオブジェクトの生成
	hwnd = CreateWindow(wnd.lpszClassName,//クラス名指定
		title,//タイトルバーの文字
		WS_OVERLAPPEDWINDOW,//タイトルバーと境界線があるウィンドウです
		CW_USEDEFAULT,//表示X座標はOSにお任せします
		CW_USEDEFAULT,//表示Y座標はOSにお任せします
		wrc.right - wrc.left,//ウィンドウ幅
		wrc.bottom - wrc.top,//ウィンドウ高
		nullptr,//親ウィンドウハンドル
		nullptr,//メニューハンドル
		wnd.hInstance,//呼び出しアプリケーションハンドル
		nullptr);//追加パラメータ

	if (CreateDevice() == -1)
	{
		return -1;
	}

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};

	swapChainDesc.Width = width;
	swapChainDesc.Height = height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //ピクセルフォーマット
	swapChainDesc.Stereo = false; //ステレオ表示フラグ
	swapChainDesc.SampleDesc.Count = 1; //マルチサンプルの指定
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	swapChainDesc.BufferCount = 2; //ダブルバッファーなので2

	//バックバッファーは伸び縮み可能
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	//フリップ後は速やかに破棄
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	//特に指定なし
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	//ウィンドウとスクリーンの切替可能
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	result = dxgiFactory->CreateSwapChainForHwnd(cmdQueue, hwnd, &swapChainDesc, nullptr, nullptr, (IDXGISwapChain1**)&swapChain);

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};

	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; //レンダーターゲットビューなのでRTV
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 2; //表裏の2つ
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE; //特に指定なし

	result = dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvHeaps));

	DXGI_SWAP_CHAIN_DESC swDesc = {};
	result = swapChain->GetDesc(&swDesc);

	backBuffers = std::vector <ID3D12Resource*>(swDesc.BufferCount);
	D3D12_CPU_DESCRIPTOR_HANDLE handle = rtvHeaps->GetCPUDescriptorHandleForHeapStart();

	for (UINT i = 0; i < swDesc.BufferCount; i++)
	{
		result = swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffers[i]));
		dev->CreateRenderTargetView(backBuffers[i], nullptr, handle);
		handle.ptr += dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}


	result = dev->CreateFence(fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));

	ShowWindow(hwnd, SW_SHOW);

	return 0;
}


void SimpleDx::Refresh()
{
	//DirectX処理
	//バックバッファのインデックスを取得
	UINT bbIdx = swapChain->GetCurrentBackBufferIndex();

	D3D12_RESOURCE_BARRIER BarrierDesc = {};
	BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	BarrierDesc.Transition.pResource = backBuffers[bbIdx];
	BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	cmdList->ResourceBarrier(1, &BarrierDesc);

	//レンダーターゲットを指定
	auto rtvH = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
	rtvH.ptr += static_cast<ULONG_PTR>(bbIdx * dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
	cmdList->OMSetRenderTargets(1, &rtvH, false, nullptr);

	//画面クリア
	float clearColor[] = { 1.0f,1.0f,0.0f,1.0f };//黄色
	cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);

	BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	cmdList->ResourceBarrier(1, &BarrierDesc);

	//命令のクローズ
	cmdList->Close();


	//コマンドリストの実行
	ID3D12CommandList* cmdlists[] = { cmdList };
	cmdQueue->ExecuteCommandLists(1, cmdlists);
	////待ち
	cmdQueue->Signal(fence, ++fenceVal);

	if (fence->GetCompletedValue() != fenceVal)
	{
		auto event = CreateEvent(nullptr, false, false, nullptr);
		fence->SetEventOnCompletion(fenceVal, event);
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);
	}
	cmdAllocator->Reset();//キューをクリア
	cmdList->Reset(cmdAllocator, nullptr);//再びコマンドリストをためる準備


	//フリップ
	swapChain->Present(1, 0);
}

int SimpleDx::CreateDevice()
{
	//DXのフィーチャーレベルを定義
	D3D_FEATURE_LEVEL levels[] =
	{
			D3D_FEATURE_LEVEL_12_1,
			D3D_FEATURE_LEVEL_12_0,
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
	};

	//対応しているドライバーがなければ終了
	HRESULT result = S_OK;
	if (FAILED(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&dxgiFactory))))
	{
		if (FAILED(CreateDXGIFactory2(0, IID_PPV_ARGS(&dxgiFactory))))
		{
			return -1;
		}
	}

	//利用可能なAdapterを検索する
	//Adapter: 1つまたは複数の GPU、DAC、およびビデオメモリー
	std::vector <IDXGIAdapter*> adapters;
	IDXGIAdapter* tmpAdapter = nullptr;
	for (int i = 0; dxgiFactory->EnumAdapters(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		adapters.push_back(tmpAdapter);
	}

	for (IDXGIAdapter* adpt : adapters)
	{
		DXGI_ADAPTER_DESC adesc = {};
		adpt->GetDesc(&adesc);
		std::wstring strDesc = adesc.Description;
		if (strDesc.find(L"NVIDIA") != std::string::npos)
		{
			tmpAdapter = adpt;
			break;
		}
	}

	//Direct3Dデバイスの初期化
	//デバイスの対応しているフィーチャーレベルを検索
	D3D_FEATURE_LEVEL featureLevel;
	for (D3D_FEATURE_LEVEL l : levels)
	{
		if (D3D12CreateDevice(tmpAdapter, l, IID_PPV_ARGS(&dev)) == S_OK)
		{
			featureLevel = l;
			break;
		}
	}

	result = dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmdAllocator));
	result = dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmdAllocator, nullptr, IID_PPV_ARGS(&cmdList));

	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};

	//タイムアウトなし
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	//アダプターを1つしか使わないときは0で良い
	cmdQueueDesc.NodeMask = 0;
	//プライオリティは特に指定なし
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	//コマンドリストと合わせる
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	result = dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&cmdQueue));

	return 0;
}

void SimpleDx::Dispose()
{
	UnregisterClass(wnd.lpszClassName, wnd.hInstance);
}

