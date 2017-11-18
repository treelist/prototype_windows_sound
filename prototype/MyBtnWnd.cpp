
#include <Windows.h>
#include "MyBtnWnd.h"

// Control data
struct MyBtnData {
	HBITMAP Bitmap[2];
	btnState State;
	BOOL bMasterVolumeOn;
};

LRESULT CALLBACK GrpBtnProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
static void DrawBitmap(HDC hdc, int x, int y, HBITMAP hBit);
static void ChangeState(HWND hWnd, btnState State);
static void DeleteAllBitmap(MyBtnData *pData);
static BOOL IsPtOnMe(HWND hWnd, LPARAM lParam);

class GrpBtnRegister {
public:
	GrpBtnRegister() {
		WNDCLASS WndClass;

		WndClass.cbClsExtra = 0;
		WndClass.cbWndExtra = sizeof(LONG_PTR);
		WndClass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
		WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		WndClass.hIcon = NULL;
		WndClass.hInstance = GetModuleHandle(NULL);
		WndClass.lpfnWndProc = GrpBtnProc;
		WndClass.lpszClassName = TEXT("MyBtn");
		WndClass.lpszMenuName = NULL;
		WndClass.style = 0;

		RegisterClass(&WndClass);
	}
} GrpBtnRegisterObject;

LRESULT CALLBACK GrpBtnProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	MyBtnData* pData;
	RECT crt;
	POINT pt;
	static btnState CheckState;
	HWND hParent;

	pData = (MyBtnData*)GetWindowLongPtr(hWnd, 0);

	switch (iMessage) {
	case WM_CREATE:
		pData = (MyBtnData*)calloc(1, sizeof(MyBtnData));
		SetWindowLongPtr(hWnd, 0, (LONG_PTR)pData);
		return 0;
	case MBW_SETIMAGE:
		DeleteAllBitmap(pData);
		pData->Bitmap[0] = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(wParam));
		pData->Bitmap[1] = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(lParam));
		InvalidateRect(hWnd, NULL, FALSE);
		return 0;
	case MBW_GETSTATE:
		return pData->State;
	case MBW_SETSTATE:
		ChangeState(hWnd, (btnState)wParam);
		return 0;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		if (pData->Bitmap[pData->State]) {
			DrawBitmap(hdc, 0, 0, pData->Bitmap[pData->State]);
		}
		EndPaint(hWnd, &ps);
		return 0;
	case WM_LBUTTONDOWN:
		if (pData->bMasterVolumeOn == TRUE)
			return 0;
		CheckState = pData->State;
		ChangeState(hWnd, CheckState == MBW_ON ? MBW_OFF : MBW_ON);
		SetCapture(hWnd);
		return 0;
	case WM_MOUSEMOVE:
		if (pData->bMasterVolumeOn == TRUE)
			return 0;
		if (GetCapture() == hWnd) {
			if (IsPtOnMe(hWnd, lParam)) {
				ChangeState(hWnd, CheckState == MBW_ON ? MBW_OFF : MBW_ON);
			}
			else {
				ChangeState(hWnd, CheckState);
			}
		}
		return 0;
	case WM_LBUTTONUP:
		if (pData->bMasterVolumeOn == TRUE)
			return 0;
		if (GetCapture() == hWnd) {
			ReleaseCapture();
			if (IsPtOnMe(hWnd, lParam)) {
				//SendMessage(GetParent(hWnd), WM_COMMAND, MAKEWPARAM(GetWindowLongPtr(hWnd, GWLP_ID), MBW_CLICKED), (LPARAM)hWnd);
				SendMessage(GetParent(hWnd), WM_COMMAND, MAKEWPARAM(GetWindowLongPtr(hWnd, GWLP_ID), MBW_CLICKED), (LPARAM)pData->State);
			}
		}
		return 0;

	case MBW_SETMVSTATE:
		pData->bMasterVolumeOn = (BOOL)wParam;

		return 0;

	case WM_DESTROY:
		DeleteAllBitmap(pData);
		free(pData);
		return 0;
	}
	return(DefWindowProc(hWnd, iMessage, wParam, lParam));
}
static void DrawBitmap(HDC hdc, int x, int y, HBITMAP hBit)
{
	HDC MemDC;
	HBITMAP OldBitmap;
	int bx, by;
	BITMAP bit;

	if (hBit == NULL) return;

	MemDC = CreateCompatibleDC(hdc);
	OldBitmap = (HBITMAP)SelectObject(MemDC, hBit);

	GetObject(hBit, sizeof(BITMAP), &bit);
	bx = bit.bmWidth;
	by = bit.bmHeight;

	BitBlt(hdc, x, y, bx, by, MemDC, 0, 0, SRCCOPY);

	SelectObject(MemDC, OldBitmap);
	DeleteDC(MemDC);
}
static void ChangeState(HWND hWnd, btnState State)
{
	MyBtnData* pData;

	pData = (MyBtnData*)GetWindowLongPtr(hWnd, 0);
	if (pData->State == State) return;

	pData->State = State;
	InvalidateRect(hWnd, NULL, FALSE);
}
static void DeleteAllBitmap(MyBtnData *pData)
{
	int i;

	for (i = 0; i < 2; i++) {
		if (pData->Bitmap[i]) {
			DeleteObject(pData->Bitmap[i]);
		}
	}
}
static BOOL IsPtOnMe(HWND hWnd, LPARAM lParam)
{
	POINT pt;
	RECT wrt;

	pt.x = (int)(short)LOWORD(lParam);
	pt.y = (int)(short)HIWORD(lParam);
	ClientToScreen(hWnd, &pt);
	GetWindowRect(hWnd, &wrt);
	return PtInRect(&wrt, pt);
}