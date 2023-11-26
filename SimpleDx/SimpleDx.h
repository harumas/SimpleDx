#pragma once
#include <Windows.h>
#include <tchar.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include <vector>
#include <string>

#include <d3dcompiler.h>
#ifdef _DEBUG
#include <iostream>
#endif


#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")

using namespace DirectX;


class SimpleDx
{
public:
	SimpleDx() = delete;
	~SimpleDx() = delete;

	static int Initialize(long width, long height, wchar_t* title);
	static void Dispose();
	static void SetUpData();
	static void Refresh(unsigned char frame);

private:
	static HWND hwnd;
	static WNDCLASSEX wnd;

	static ID3D12Device* dev;
	static IDXGIFactory6* dxgiFactory;
	static IDXGISwapChain4* swapChain;
	static ID3D12CommandAllocator* cmdAllocator;
	static ID3D12GraphicsCommandList* cmdList;
	static ID3D12CommandQueue* cmdQueue;
	static ID3D12DescriptorHeap* rtvHeaps;
	static std::vector<ID3D12Resource*> backBuffers;
	static ID3D12Fence* fence;
	static UINT64 fenceVal;
	static long windowWidth;
	static long windowHeight;
	static ID3D12PipelineState* pipelineState;
	static D3D12_VIEWPORT viewport;
	static D3D12_RECT scissorrect;
	static ID3D12RootSignature* rootSignature;
	static D3D12_INDEX_BUFFER_VIEW ibView;
	static D3D12_VERTEX_BUFFER_VIEW vbView;


	static int CreateDevice();
};

struct Vertex
{
	XMFLOAT3 pos;
	XMFLOAT2 uv;
};
