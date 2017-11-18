#include <windows.h>
#include "GrpScrl.h"

#define IsHorz (GetWindowLong(hWnd,GWL_STYLE) & GSS_HORZ)
struct GrpScrlData {
	int Max, Min;
	int Pos;
	int TSize;
	int TGap;
	COLORREF BackColor;
	COLORREF ThumbColor;
};

LRESULT CALLBACK GrpScrlProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
void GetThumbRect(HWND hWnd, RECT *trt);
void PosFromPixel(HWND hWnd, int Pixel);

class GrpScrlRegister {
public:
	GrpScrlRegister() {
		WNDCLASS WndClass;

		WndClass.cbClsExtra = 0;
		WndClass.cbWndExtra = 4;
		WndClass.hbrBackground = NULL;
		WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		WndClass.hIcon = NULL;
		WndClass.hInstance = GetModuleHandle(NULL);
		WndClass.lpfnWndProc = GrpScrlProc;
		WndClass.lpszClassName = TEXT("GrpScrl");
		WndClass.lpszMenuName = NULL;
		WndClass.style = 0;

		RegisterClass(&WndClass);
	}
} GrpScrlRegisterObject;

LRESULT CALLBACK GrpScrlProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	HDC hdc, MemDC;
	HBITMAP MemBit, hOldBmp;
	PAINTSTRUCT ps;
	GrpScrlData *pData;
	RECT crt, trt;
	POINT pt;
	static int offset;
	static HPEN hPen[5];
	int i;
	HBRUSH BackBrush, ThumbBrush, OldBrush;

	pData = (GrpScrlData *)GetWindowLong(hWnd, 0);
	switch (iMessage) {
	case WM_CREATE:
		pData = (GrpScrlData *)calloc(sizeof(GrpScrlData), 1);
		SetWindowLong(hWnd, 0, (LONG)pData);
		pData->TSize = 18;
		pData->TGap = 3;
		// hPen 배열이 정적이므로 두 인스턴스 생성시 따로 만들 필요가 없다.
		if (hPen[0] == NULL) {
			for (i = 0; i<5; i++) {
				hPen[i] = CreatePen(PS_SOLID, 1, RGB(i * 32 + 32, i * 32 + 32, i * 32 + 32));
			}
		}
		pData->BackColor = GetSysColor(COLOR_BTNFACE);
		pData->ThumbColor = RGB(255, 255, 255);
		return 0;
	case GSM_SETRANGE:
		pData->Min = wParam;
		pData->Max = lParam;
		InvalidateRect(hWnd, NULL, TRUE);
		return 0;
	case GSM_GETRANGEHI:
		return pData->Max;
	case GSM_GETRANGELOW:
		return pData->Min;
	case GSM_SETPOS:
		pData->Pos = wParam;
		pData->Pos = max(pData->Min, min(pData->Max, pData->Pos));
		InvalidateRect(hWnd, NULL, TRUE);
		return 0;
	case GSM_GETPOS:
		return pData->Pos;
	case GSM_SETTSIZE:
		GetClientRect(hWnd, &crt);
		pData->TSize = wParam;
		if (IsHorz) {
			pData->TSize = max(4, min(crt.right - 10, pData->TSize));
		}
		else {
			pData->TSize = max(4, min(crt.bottom - 10, pData->TSize));
		}
		InvalidateRect(hWnd, NULL, TRUE);
		return 0;
	case GSM_GETTSIZE:
		return pData->TSize;
	case GSM_SETTGAP:
		GetClientRect(hWnd, &crt);
		pData->TGap = wParam;
		if (IsHorz) {
			pData->TGap = max(0, min(crt.bottom / 2 - 2, pData->TGap));
		}
		else {
			pData->TGap = max(0, min(crt.right / 2 - 2, pData->TGap));
		}
		InvalidateRect(hWnd, NULL, TRUE);
		return 0;
	case GSM_GETTGAP:
		return pData->TGap;
	case GSM_SETBACKCOLOR:
		pData->BackColor = (COLORREF)wParam;
		InvalidateRect(hWnd, NULL, TRUE);
		return 0;
	case GSM_SETTHUMBCOLOR:
		pData->ThumbColor = (COLORREF)wParam;
		InvalidateRect(hWnd, NULL, TRUE);
		return 0;
		// 크기가 바뀌면 썸의 모양도 범위 점검을 다시 해야 한다.
	case WM_SIZE:
		SendMessage(hWnd, GSM_SETTSIZE, pData->TSize, 0);
		SendMessage(hWnd, GSM_SETTGAP, pData->TGap, 0);
		return 0;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		GetClientRect(hWnd, &crt);
		MemDC = CreateCompatibleDC(hdc);
		MemBit = CreateCompatibleBitmap(hdc, crt.right, crt.bottom);
		hOldBmp = (HBITMAP)SelectObject(MemDC, MemBit);

		BackBrush = CreateSolidBrush(pData->BackColor);
		FillRect(MemDC, &crt, BackBrush);
		DeleteObject(BackBrush);
		if (IsHorz) {
			for (i = 0; i<5; i++) {
				SelectObject(MemDC, hPen[i]);
				MoveToEx(MemDC, crt.left, crt.bottom / 2 - 2 + i, NULL);
				LineTo(MemDC, crt.right, crt.bottom / 2 - 2 + i);
			}
		}
		else {
			for (i = 0; i<5; i++) {
				SelectObject(MemDC, hPen[i]);
				MoveToEx(MemDC, crt.right / 2 - 2 + i, crt.top, NULL);
				LineTo(MemDC, crt.right / 2 - 2 + i, crt.bottom);
			}
		}
		GetThumbRect(hWnd, &trt);
		SelectObject(MemDC, GetStockObject(BLACK_PEN));
		ThumbBrush = CreateSolidBrush(pData->ThumbColor);
		OldBrush = (HBRUSH)SelectObject(MemDC, ThumbBrush);
		RoundRect(MemDC, trt.left, trt.top, trt.right, trt.bottom, 10, 10);
		DeleteObject(SelectObject(MemDC, OldBrush));

		BitBlt(hdc, 0, 0, crt.right, crt.bottom, MemDC, 0, 0, SRCCOPY);
		SelectObject(MemDC, hOldBmp);
		DeleteObject(MemBit);
		DeleteDC(MemDC);
		EndPaint(hWnd, &ps);
		return 0;
	case WM_LBUTTONDOWN:
		GetThumbRect(hWnd, &trt);
		pt.x = LOWORD(lParam);
		pt.y = HIWORD(lParam);
		if (PtInRect(&trt, pt)) {
			if (IsHorz) {
				offset = trt.left - LOWORD(lParam);
			}
			else {
				offset = trt.top - HIWORD(lParam);
			}
			SetCapture(hWnd);
		}
		else {
			if (IsHorz) {
				PosFromPixel(hWnd, LOWORD(lParam) - pData->TSize / 2);
			}
			else {
				PosFromPixel(hWnd, HIWORD(lParam) - pData->TSize / 2);
			}
			SendMessage(GetParent(hWnd), IsHorz ? WM_HSCROLL : WM_VSCROLL,
				MAKEWPARAM(SB_THUMBPOSITION, 0), (LPARAM)hWnd);
		}
		return 0;
	case WM_MOUSEMOVE:
		if (GetCapture() == hWnd) {
			GetClientRect(hWnd, &crt);
			if (IsHorz) {
				PosFromPixel(hWnd, (int)(short)LOWORD(lParam) + offset);
			}
			else {
				PosFromPixel(hWnd, (int)(short)HIWORD(lParam) + offset);
			}
			// 위치는 보내지 않음. 32비트이므로 직접 조사할 것
			SendMessage(GetParent(hWnd), IsHorz ? WM_HSCROLL : WM_VSCROLL,
				MAKEWPARAM(SB_THUMBTRACK, 0), (LPARAM)hWnd);
		}
		return 0;
	case WM_LBUTTONUP:
		if (GetCapture() == hWnd) {
			ReleaseCapture();
			SendMessage(GetParent(hWnd), IsHorz ? WM_HSCROLL : WM_VSCROLL,
				MAKEWPARAM(SB_THUMBPOSITION, 0), (LPARAM)hWnd);
		}
		return 0;
	case WM_DESTROY:
		if (hPen[0]) {
			for (i = 0; i<5; i++) {
				DeleteObject(hPen[i]);
				hPen[0] = NULL;
			}
		}
		free(pData);
		return 0;
	}
	return(DefWindowProc(hWnd, iMessage, wParam, lParam));
}

void GetThumbRect(HWND hWnd, RECT *trt)
{
	GrpScrlData *pData;
	RECT crt;
	int x, y;

	pData = (GrpScrlData *)GetWindowLong(hWnd, 0);
	GetClientRect(hWnd, &crt);
	if (IsHorz) {
		x = MulDiv(pData->Pos, crt.right - pData->TSize, pData->Max - pData->Min);
		SetRect(trt, x, crt.top + pData->TGap, x + pData->TSize, crt.bottom - pData->TGap);
	}
	else {
		y = MulDiv(pData->Pos, crt.bottom - pData->TSize, pData->Max - pData->Min);
		SetRect(trt, crt.left + pData->TGap, y, crt.right - pData->TGap, y + pData->TSize);
	}
}

// 화면상의 ScrPos에 해당하는 범위값으로 변경한다.
void PosFromPixel(HWND hWnd, int Pixel)
{
	GrpScrlData *pData;
	RECT crt;
	int width;

	pData = (GrpScrlData *)GetWindowLong(hWnd, 0);
	GetClientRect(hWnd, &crt);
	if (IsHorz) {
		width = crt.right - pData->TSize;
	}
	else {
		width = crt.bottom - pData->TSize;
	}

	pData->Pos = MulDiv(Pixel, pData->Max - pData->Min, width);
	pData->Pos = max(pData->Min, min(pData->Max, pData->Pos));
	InvalidateRect(hWnd, NULL, TRUE);
}
