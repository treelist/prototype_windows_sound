#include <windows.h>
#include "GrpBtn.h"

void GrpBtn::SetImage(WORD Normal, WORD Hot, WORD Down, WORD Disable)
{
	DeleteAllBitmap();
	Bitmap[0] = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(Normal));
	Bitmap[1] = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(Hot));
	Bitmap[2] = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(Down));
	Bitmap[3] = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(Disable));
	DrawBitmap(NULL);
}

void GrpBtn::SetState(eState State)
{
	if ((Style & GBS_CHECK) != 0) {
		ChangeState(State);
	}
}

BOOL GrpBtn::IsPtOnMe(POINT pt)
{
	RECT rt;

	SetRect(&rt, x, y, x + w, y + h);
	return (PtInRect(&rt, pt));
}

BOOL GrpBtn::IsPtOnMe(int x, int y)
{
	POINT pt = { x,y };

	return IsPtOnMe(pt);
}

void GrpBtn::ChangeState(eState State)
{
	if (this->State == State) return;

	this->State = State;
	DrawBitmap(NULL);
}

void GrpBtn::DeleteAllBitmap()
{
	int i;

	for (i = 0; i<4; i++) {
		if (Bitmap[i]) {
			DeleteObject(Bitmap[i]);
		}
	}
}

void GrpBtn::DrawBitmap(HDC hdc)
{
	HDC MemDC;
	HBITMAP OldBitmap;
	HBITMAP hBit;
	BOOL NullDc = FALSE;

	if (hdc == NULL) {
		NullDc = TRUE;
		hdc = GetDC(hParent);
	}

	if (State == GB_HIDDEN) return;
	hBit = Bitmap[State];

	MemDC = CreateCompatibleDC(hdc);
	OldBitmap = (HBITMAP)SelectObject(MemDC, hBit);

	BitBlt(hdc, x, y, w, h, MemDC, 0, 0, SRCCOPY);

	SelectObject(MemDC, OldBitmap);
	DeleteDC(MemDC);
	if (NullDc) {
		ReleaseDC(hParent, hdc);
	}
}

void GrpBtn::OnDown(LPARAM lParam)
{
	if (State == GB_HIDDEN || State == GB_DISABLE) return;
	if (IsPtOnMe(LOWORD(lParam), HIWORD(lParam))) {
		if ((Style & GBS_CHECK) == 0) {
			ChangeState(GB_DOWN);
		}
		else {
			CheckState = State;
			ChangeState(CheckState == GB_NORMAL ? GB_DOWN : GB_NORMAL);
		}
		SetCapture(hParent);
		bCapture = TRUE;
	}
}

void GrpBtn::OnMove(LPARAM lParam)
{
	HWND hParent;
	int x = (int)(short)LOWORD(lParam);
	int y = (int)(short)HIWORD(lParam);

	if (State == GB_HIDDEN || State == GB_DISABLE) return;
	if (bCapture) {
		if ((Style & GBS_CHECK) == 0) {
			if (IsPtOnMe(x, y)) {
				ChangeState(GB_DOWN);
			}
			else {
				ChangeState(GB_NORMAL);
			}
		}
		else {
			if (IsPtOnMe(x, y)) {
				ChangeState(CheckState == GB_NORMAL ? GB_DOWN : GB_NORMAL);
			}
			else {
				ChangeState(CheckState);
			}
		}
	}
	else {
		for (hParent = this->hParent; GetParent(hParent); hParent = GetParent(hParent)) { ; }
		if (GetForegroundWindow() != hParent) return;
		// 누군가가 캡처를 하고 있는 상황에서는 핫 상태로 가지 말아야 한다.
		if (GetCapture() == NULL && (Style & GBS_CHECK) == 0 && IsPtOnMe(x, y)) {
			SetTimer(hParent, 1234, 50, NULL);
			TimerByMe = TRUE;
			ChangeState(GB_HOT);
		}
	}
}

void GrpBtn::OnUp(LPARAM lParam)
{
	POINT pt;

	if (bCapture) {
		ReleaseCapture();
		bCapture = FALSE;
		if ((Style & GBS_CHECK) == 0) {
			ChangeState(GB_NORMAL);
		}
		GetCursorPos(&pt);
		ScreenToClient(hParent, &pt);
		if (IsPtOnMe(pt)) {
			SendMessage(hParent, WM_COMMAND, MAKEWPARAM(ID, GBN_CLICKED), 0);
		}
	}
}

void GrpBtn::OnPaint(HDC hdc)
{
	DrawBitmap(hdc);
}

void GrpBtn::OnTimer()
{
	POINT pt;

	if (TimerByMe == FALSE) return;
	GetCursorPos(&pt);
	ScreenToClient(hParent, &pt);
	if (IsPtOnMe(pt) == FALSE) {
		KillTimer(hParent, 1234);
		TimerByMe = FALSE;
		ChangeState(GB_NORMAL);
	}
}

void GrpBtn::Enable(BOOL bEnable)
{
	if (bEnable) {
		State = GB_NORMAL;
	}
	else {
		State = GB_DISABLE;
		KillTimer(hParent, 1234);
		TimerByMe = FALSE;
	}
	DrawBitmap(NULL);
}

void GrpBtn::Show(BOOL bShow)
{
	if (bShow) {
		State = GB_NORMAL;
		DrawBitmap(NULL);
	}
	else {
		State = GB_HIDDEN;
		InvalidateRect(hParent, NULL, TRUE);
	}
}

void GrpBtn::Move(int nx, int ny)
{
	x = nx;
	y = ny;
	InvalidateRect(hParent, NULL, TRUE);
}



