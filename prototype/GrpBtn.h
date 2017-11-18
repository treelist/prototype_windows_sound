#include <Windows.h>

#define GBS_PUSH 0
#define GBS_CHECK 1
#define GBN_CLICKED 0
enum eState { GB_NORMAL, GB_HOT, GB_DOWN, GB_DISABLE, GB_HIDDEN };
class GrpBtn
{
private:
	int x, y, w, h;
	DWORD Style;
	UINT ID;
	HWND hParent;
	HBITMAP Bitmap[4];
	eState State;
	eState CheckState;
	BOOL bCapture;
	BOOL TimerByMe;

	void DeleteAllBitmap();
	void ChangeState(eState State);
	void DrawBitmap(HDC hdc);
	BOOL IsPtOnMe(POINT pt);
	BOOL IsPtOnMe(int x, int y);

public:
	GrpBtn() {}
	GrpBtn(int ax, int ay, int aw, int ah, DWORD aStyle, UINT aID, HWND aParent) :
		x(ax), y(ay), w(aw), h(ah), Style(aStyle), ID(aID), hParent(aParent) {
		State = GB_NORMAL;
		bCapture = FALSE;
		TimerByMe = FALSE;
		for (int i = 0; i<4; i++) {
			Bitmap[i] = NULL;
		}
	}
	~GrpBtn() { DeleteAllBitmap(); }
	void ChangeParent(HWND hParent) { this->hParent = hParent; }
	void SetImage(WORD Normal, WORD Hot, WORD Down, WORD Disable);
	eState GetState() { return State; }
	void SetState(eState State);
	void OnDown(LPARAM lParam);
	void OnMove(LPARAM lParam);
	void OnUp(LPARAM lParam);
	void OnPaint(HDC hdc);
	void OnTimer();
	void Enable(BOOL bEnable);
	BOOL IsEnabled() { return (State != GB_DISABLE); }
	void Show(BOOL bShow);
	BOOL IsShow() { return (State != GB_HIDDEN); }
	int GetX() { return x; }
	int GetY() { return y; }
	int GetWidth() { return w; }
	int GetHeight() { return h; }
	void Move(int nx, int ny);
};
