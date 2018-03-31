#include <Windows.h>

HWND hwnd = NULL;
int Width = 800;
int Height = 600;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
int messageloop();



bool InitializeWindow(HINSTANCE hInstance,
						int ShowWnd,
						int width, int height,
						bool windowed)
{
	typedef struct _WNDCLASS {
		UINT cbSize;
		UINT style;
		WNDPROC lpfnWndProc;
		int cbClsExtra;
		int cbWndTxtra;
		HANDLE hInstance;
		HICON hIcon;
		HCURSOR hCursor;
		HBRUSH hbrBackground;
		LPCTSTR lpszMenuName;
		LPCTSTR lpszClassName;
	}WNDCLASS;

	WNDCLASSEX wc;

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = NULL;
	wc.cbWndExtra = NULL;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = L"PROJECT P";
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	
	if (!RegisterClassEx(&wc)) {
		MessageBox(NULL, L"ERROR REG CLASS", L"ERROR",
			MB_OK | MB_ICONERROR);
		return 1;
	}

	hwnd = CreateWindowEx(NULL, wc.lpszClassName, L"PROJECT P",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		width, height, NULL, NULL, hInstance, NULL);

	if (!hwnd)
	{
		MessageBox(NULL, L"ERROR CREATE WINDOW", L"ERROR",
			MB_OK | MB_ICONERROR);
		return 1;
	}

	ShowWindow(hwnd, ShowWnd);
	UpdateWindow(hwnd);

	return true;
}

int WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nShowCmd)
{
	if (!InitializeWindow(hInstance, nShowCmd, Width, 1000, true))
	{
		MessageBox(0, L"Window Init ERROR", L"ERROR", MB_OK);
		return 0;
	}
	
	//messageloop();
	return 0;
}

int messageloop()
{
	while (true)
	{

	}
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_KEYDOWN:
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	default:
		break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}