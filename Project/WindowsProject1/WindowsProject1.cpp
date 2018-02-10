// WindowsProject1.cpp: 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#include "WindowsProject1.h"
#include "resource.h"

#include <time.h>
#include <list>

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

WCHAR lpszText[10] = L"TEXT";
std::list<WCHAR> wCharLst;

// 이 코드 모듈에 들어 있는 함수의 정방향 선언입니다.
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
	srand(time(NULL));
    // TODO: 여기에 코드를 입력합니다.

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WINDOWSPROJECT1, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 응용 프로그램 초기화를 수행합니다.
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINDOWSPROJECT1));

    MSG msg;

    // 기본 메시지 루프입니다.
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  함수: MyRegisterClass()
//
//  목적: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINDOWSPROJECT1));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
	//wcex.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WINDOWSPROJECT1);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   목적: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   설명:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

   HWND hWnd = CreateWindowW(szWindowClass, L"TEST", WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  목적:  주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 응용 프로그램 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//

//BOOL isDown = FALSE;
WORD x = 50;
WORD y = 50;
BOOL isRnder = FALSE;
int frame = 0;
BOOL isMove = FALSE;
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
	/*case WM_KEYDOWN:
	{
		HDC hdc = GetDC(hWnd);
		
		
		if (wCharLst.size() >= 10)
		{
			wCharLst.pop_front();
		}
		WCHAR input = L'a';
		wCharLst.push_back(input);
		auto iter = wCharLst.begin();
		int idx = 0;
		for (iter; iter != wCharLst.end(); ++iter)
		{
			TextOut(hdc, x + idx*10, y, &(*iter), 1);
			idx++;
		}
		
		ReleaseDC(hWnd, hdc);
	}
	case WM_LBUTTONDOWN:
	{
		isDown = TRUE;
	}
	break;
	case WM_LBUTTONUP:
	{
		isDown = FALSE;
	}
		break;
	case WM_MOUSEMOVE:
	{
		y = HIWORD(lParam);
		x = LOWORD(lParam);
		break;
	}*/
	case WM_KEYDOWN:
	{
		// 눌렸다는거 인지
		if (wParam == VK_LEFT || wParam == VK_UP || wParam == VK_DOWN || wParam == VK_RIGHT)
		{
			isMove = TRUE;
		}
	}
	break;
	case WM_KEYUP:
	{
		//풀렸다는 것 인지
		isMove = FALSE;
	}
	break;
	case WM_CREATE:
	{
		static HANDLE hTimer = (HANDLE)SetTimer(hWnd, 1, 100, NULL);
	}
		break;
	case WM_TIMER:
	{
		frame += 1;
		
		InvalidateRect(hWnd, NULL, true);
	}
	break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 메뉴 선택을 구문 분석합니다.
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
		// 펜
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다.
			/*HPEN pen = CreatePen(PS_SOLID, 5, RGB(255, 0, 0));
			HBRUSH bruch = CreateSolidBrush(RGB(0, 0, 255));

			SelectObject(hdc, pen);
			SelectObject(hdc, bruch);
			Rectangle(hdc, x, y, x+50, y+50);

			if (isRnder) {
				Rectangle(hdc, 0, 0, 10, 10);
			}
			
			DeleteObject(pen);
			DeleteObject(bruch);*/

			HBITMAP newBitmap, preBitmap;

			HDC DC = CreateCompatibleDC(hdc);

			newBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP2));// 비트맵을 실제로 읽어온다
			preBitmap = (HBITMAP)SelectObject(DC, (HGDIOBJ)newBitmap);  // 새로만든 dc에 읽어온 비트맵을 장착
			if (!isMove)
			{
				frame %= 3;
				if (frame == 0) {
					BitBlt(hdc, 0, 0, 34, 40, DC, 0, 0, SRCCOPY);
				}
				else if (frame == 1) {
					BitBlt(hdc, 3, 0, 30, 40, DC, 34, 0, SRCCOPY);
				}
				else if (frame == 2) {
					BitBlt(hdc, 4, 0, 32, 40, DC, 64, 0, SRCCOPY);
				}
			}
			else {
				frame %= 9;
				switch (frame)
				{
				case 0:BitBlt(hdc, 0, 0, 36, 43, DC, 102, 0, SRCCOPY); break;
				case 1:BitBlt(hdc, 0, 0, 31, 43, DC, 137, 0, SRCCOPY); break;
				case 2:BitBlt(hdc, 0, 0, 32, 43, DC, 168, 0, SRCCOPY); break;
				case 3:BitBlt(hdc, 0, 0, 34, 43, DC, 200, 0, SRCCOPY); break;
				case 4:BitBlt(hdc, 0, 0, 32, 43, DC, 234, 0, SRCCOPY); break;
				case 5:BitBlt(hdc, 0, 0, 32, 43, DC, 265, 0, SRCCOPY); break;
				case 6:BitBlt(hdc, 0, 0, 33, 43, DC, 297, 0, SRCCOPY); break;
				case 7:BitBlt(hdc, 0, 0, 36, 43, DC, 330, 0, SRCCOPY); break;
				case 8:BitBlt(hdc, 0, 0, 36, 43, DC, 366, 0, SRCCOPY); break;
				}
			}
			
			

			SelectObject(DC, preBitmap);
			DeleteObject(newBitmap);
			DeleteObject(DC);


            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// 정보 대화 상자의 메시지 처리기입니다.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
