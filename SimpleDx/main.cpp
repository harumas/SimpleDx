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

	wchar_t title[] = L"DX12テスト";

	if (SimpleDx::Initialize(1280, 720, title) == -1)
		return 0;

	MSG msg = {};

	SimpleDx::SetUpData();

	unsigned int frame = 0;

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

		SimpleDx::Refresh(frame);
		//++frame;
	}

	SimpleDx::Dispose();

	return 0;
}

