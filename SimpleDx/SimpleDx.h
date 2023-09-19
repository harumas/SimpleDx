#pragma once
#include <Windows.h> 
#include <d3d12.h>
#include <dxgi1_6.h>
#include <vector>
#include <string>


#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

class SimpleDx
{
public:
	SimpleDx() = delete;
	~SimpleDx() = delete;

	static int Initialize(long width, long height, wchar_t* title);
	static void Dispose();
	static void Refresh();

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
	static	UINT64 fenceVal;

	static int CreateDevice();
};
