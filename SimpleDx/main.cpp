#include <Windows.h> 
#include <tchar.h> 
#include <string.h> 
#include "SimpleDx.h"
#ifdef _DEBUG 
#include <iostream> 
#endif using namespace std;


#ifdef _DEBUG 
int main()
{
	#else 
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	#endif 

	wchar_t title[] = L"DX12ƒeƒXƒg";

	if (SimpleDx::Initialize(1280, 720, title) == -1)
		return 0;

	XMFLOAT3 vertices[] =
	{
		{-1.0f,-1.0f,0.0f},
		{-1.0f,1.0f,0.0f},
		{1.0f,-1.0f,0.0f},
	};

	MSG msg = {};

	while (true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_QUIT)
		{
			break;
		}
	}

	SimpleDx::Dispose();

	return 0;
}

