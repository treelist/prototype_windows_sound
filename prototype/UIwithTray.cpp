
#include <windows.h>
#include <shellapi.h>
#include <stdio.h>
#include <Tchar.h>

// >> 17May21
#include "GrpScrl.h"
#include "GrpBtn.h"
#include "MyIconManager.h"
// <<
// >> 17May22
#include "CoreAudioInterfaceHelper.h"
#include "MyEndpointManager.h"
#include "MySessionManager.h"
// <<
// >> 17May23
#include "resource.h"
#include "MyBtnWnd.h"
//
#include "UIwithTray.h"
#include "MyWaveIn.h"

#ifdef UNICODE
#define stringcopy wcscpy_s
#else
#define stringcopy strcpy_s
#endif

UINT WM_TASKBARCREATED = 0;

HWND hWnd;
HWND hWndMain;
HMENU hMenu;

NOTIFYICONDATA notifyIconData;

int screenWidth = 0;
int screenHeight = 0;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
// >> 17May21
HINSTANCE g_hInst;

LRESULT CALLBACK BoxProc(HWND, UINT, WPARAM, LPARAM);
void DrawBitmap(HDC hdc, int x, int y, HBITMAP hBit);
// <<
// >> 
static BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
static void DrawPeakMeter(HWND, float);
// <<

// >> 17May24
struct content
{
	int ctrlNum;
	HICON hIcon;
	HWND hVert;
	HWND hBtn1;

	IAudioMeterInformation* pMeter;
	IAudioEndpointVolume* epVolume;
	ISimpleAudioVolume* saVolume;
	float limitVolume;
	float tempVolume;
	int count;
	float variable;
};
// <<
LRESULT CALLBACK WavProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL bRecording, bEnding, bTerminating;
PBYTE pBuffer;

DWORD dwDataLength;

#pragma region helper funcs
// Initialize the NOTIFYICONDATA structure.
// See MSDN docs http://msdn.microsoft.com/en-us/library/bb773352(VS.85).aspx
// for details on the NOTIFYICONDATA structure.
void InitNotifyIconData()
{
	memset(&notifyIconData, 0, sizeof(NOTIFYICONDATA));

	notifyIconData.cbSize = sizeof(NOTIFYICONDATA);

	notifyIconData.hWnd = hWnd;
	// Now GIVE the NOTIFYICON.. the thing that
	// will sit in the system tray, an ID.
	notifyIconData.uID = ID_TRAY_APP_ICON;
	// The COMBINATION of HWND and uID form
	// a UNIQUE identifier for EACH ITEM in the
	// system tray.  Windows knows which application
	// each icon in the system tray belongs to
	// by the HWND parameter.
	/////

	/////
	// Set up flags.
	notifyIconData.uFlags = NIF_ICON |		// promise that the hIcon member WILL BE A VALID ICON!!
		NIF_MESSAGE |	// when someone clicks on the system tray icon,
						// we want a WM_ type message to be sent to our WNDPROC
		NIF_TIP;		// we're gonna provide a tooltip as well, son.

	notifyIconData.uCallbackMessage = WM_TRAYICON;	//this message must be handled in hwnd's window procedure. more info below.

													// Load icon.  Be sure to include an icon "green_man.ico" .. get one
													// from the internet if you don't have an icon
	notifyIconData.hIcon = (HICON)LoadImage(NULL, TEXT("green_man.ico"), IMAGE_ICON, 0, 0, LR_LOADFROMFILE);

	stringcopy(notifyIconData.szTip, TOOLTIP_TEXT_LENGTH, TEXT("Alpha Sound"));

	Shell_NotifyIcon(NIM_ADD, &notifyIconData);
}
#pragma endregion

int WINAPI WinMain(HINSTANCE _hInstance, HINSTANCE _hPrevInstance, LPSTR _args, int _iCmdShow)
{
	TCHAR className[] = TEXT("Alpha Sound");
	// >> 17May21
	TCHAR* BoxClsName = TEXT("ctrlbox");
	g_hInst = _hInstance;
	// <<

	TCHAR* WaveInCtrl = TEXT("wavctrl");

	// I want to be notified when windows explorer
	// crashes and re-launches the taskbar.  the WM_TASKBARCREATED
	// event will be sent to my WndProc() AUTOMATICALLY whenever
	// explorer.exe starts up and fires up the taskbar again.
	// So its great, because now, even if explorer crashes,
	// I have a way to re-add my system tray icon in case
	// the app is already in the "minimized" (hidden) state.
	// if we did not do this an explorer crashed, the application
	// would remain inaccessible!!
	WM_TASKBARCREATED = RegisterWindowMessageA("TaskbarCreated");

	AllocConsole();
	AttachConsole(GetCurrentProcessId());
	freopen("CON", "w", stdout);


	WNDCLASSEX wnd = { 0 };

	wnd.hInstance = _hInstance;
	wnd.lpszClassName = className;
	wnd.lpfnWndProc = WndProc;
	wnd.style = CS_HREDRAW | CS_VREDRAW;
	wnd.cbSize = sizeof(WNDCLASSEX);
	wnd.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wnd.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wnd.hCursor = LoadCursor(NULL, IDC_ARROW);
	wnd.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);//COLOR_APPWORKSPACE;

	if (!RegisterClassEx(&wnd))
	{
		FatalAppExit(0, TEXT("Couldn't register window class!"));
	}
	// >> 17May21
	wnd.lpszClassName = BoxClsName;
	wnd.lpfnWndProc = BoxProc;
	wnd.cbWndExtra = sizeof(LONG_PTR);
	if (!RegisterClassEx(&wnd))
	{
		FatalAppExit(0, TEXT("Couldn't register window class!"));
	}
	// <<

	wnd.lpszClassName = WaveInCtrl;
	wnd.lpfnWndProc = WavProc;
	if (!RegisterClassEx(&wnd))
	{
		FatalAppExit(0, TEXT("Couldn't register window class!"));
	}

	HDC hDC = GetDC(NULL);

	screenWidth = GetDeviceCaps(hDC, HORZRES);
	screenHeight = GetDeviceCaps(hDC, VERTRES);

	HWND hTrayWnd = FindWindow(_T("Shell_TrayWnd"), NULL);
	RECT rc;

	if (hTrayWnd) {
		GetWindowRect(hTrayWnd, &rc);
		screenHeight -= rc.bottom - rc.top;
	}
	
	hWnd = CreateWindowEx(0, className, TEXT("Alpha Sound"), WS_SYSMENU | WS_CAPTION | WS_BORDER, screenWidth - WINDOW_WIDTH, screenHeight - WINDOW_HEIGHT, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, NULL, _hInstance, NULL);
	// >> 17May21
	// CreateWindow(TEXT("static"), TEXT("Here should be list of some volume controllers."), WS_CHILD | WS_VISIBLE | SS_CENTER | WS_HSCROLL, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, hWnd, 0, _hInstance, NULL);
	// <<
	InitNotifyIconData();

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (!IsWindowVisible(hWnd))
	{
		Shell_NotifyIcon(NIM_DELETE, &notifyIconData);
	}

	return msg.wParam;
}

const int BOX_WIDTH = 600;
const int BOX_HEIGHT = 400;
const int ENDPOINTBOX = 2000;
const int EQTESTBTN = 2001;
BOOL bMasterVolumeOn = FALSE;

PWAVEHDR pWaveHdr;
#define EQCHECK_TIMER  1212
#define EQCHECK_TIMER_PERIOD  50

LRESULT CALLBACK WndProc(HWND _hwnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam)
{
	// >> 17May21
	HDC hdc;
	PAINTSTRUCT ps;
	static HICON* hIcon;
	static MyIconManager* iconManager = new MyIconManager(g_hInst);
	// <<
	// >> 17May 22
	static CoreAudioInterfaceHelper* helper = new CoreAudioInterfaceHelper(_hwnd);
	static MyEndpointManager* epMan = new MyEndpointManager(_hwnd, helper);
	static MySessionManager* ssMan = new MySessionManager(_hwnd, helper);
	TCHAR mvStr[100];
	float mvLev;
	// <<
	// >> 17May 23
	static workingPair* wPair = NULL;
	static int ssNum = 0;
	HICON* tempHIcon = NULL;
	DWORD boxInfo = 0;
	static HWND* hBoxes;
	int volume;
	int btn1State;
	int btn2State;

	// <<
	static HWND hEndpointBox;
	static HWND testBtn;

	static HWAVEIN hWaveIn;
	static HWAVEOUT hWaveOut;
	static WAVEFORMATEX waveform;

	static int sampleRate = SAMPLERATE;
	static HWND hWavCtrl;
	static LPCTSTR eqName;
	static TCHAR explain[MAX_PATH];

	if (_uMsg == WM_TASKBARCREATED && !IsWindowVisible(hWnd))
	{
		ShowWindow(hWnd, SW_HIDE);
		return 0;
	}
	
	switch (_uMsg)
	{
	case WM_CREATE:
		hMenu = CreatePopupMenu();

		AppendMenu(hMenu, MF_STRING, ID_TRAY_CONTROLLER_CONTEXT_MENU_ITEM, TEXT("Sound Controller"));
		AppendMenu(hMenu, MF_STRING, ID_TRAY_EXIT_CONTEXT_MENU_ITEM, TEXT("Exit"));
		// >> 17May21
		AppendMenu(hMenu, MF_STRING, ID_TRAY_PEAK_CONTEXT_MENU_ITEM, TEXT("Show peak meter"));
		
		// <<
		// >> 17May 23
		wPair = ssMan->getPairs();
		ssNum = ssMan->getWorkingCount();
		hIcon = new HICON[ssNum];
		hBoxes = new HWND[ssNum];
		for (int i = 0; i < ssNum; i++) {
			iconManager->setPID(wPair[i].ProcessID);
			hIcon[i] = iconManager->getIcon(-1);
		}

		swprintf(mvStr, TEXT("<%d>"), ssNum);
		MessageBox(_hwnd, mvStr, TEXT("dd"), MB_OK);
		
		hEndpointBox = CreateWindow(TEXT("ctrlbox"), NULL, WS_CHILD | WS_VISIBLE, 0, 0, 100, 250, _hwnd, (HMENU)ENDPOINTBOX, g_hInst, LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON1)));
		SendMessage(hEndpointBox, WM_SETCTRLID, (WPARAM)ENDPOINTBOX, 0);
		SendMessage(hEndpointBox, WM_SETABOUTVOLUME, (WPARAM)epMan->getMeterInfo(), (LPARAM)epMan->getEndpointVolume());
		
		

		for (int i = 0; i < ssNum; i++) {
			tempHIcon = &hIcon[i];
			hBoxes[i] = CreateWindow(TEXT("ctrlbox"), NULL, WS_CHILD | WS_VISIBLE, 100 + i * 100, 0, 100, 250, _hwnd, (HMENU)i, g_hInst, (LPVOID)hIcon[i]);
			SendMessage(hBoxes[i], WM_SETCTRLID, (WPARAM)i, 0);
			SendMessage(hBoxes[i], WM_SETABOUTVOLUME, (WPARAM)wPair[i].pMeterInformation, (LPARAM)wPair[i].pAudioVolume);
		}
		// <<
		//hMainScrl = CreateWindow(TEXT("GrpScrl"), NULL, WS_CHILD | WS_VISIBLE | GSS_VERT, 42, 60, GetSystemMetrics(SM_CXVSCROLL), 120, _hwnd, (HMENU)MAINSCRL, g_hInst, NULL);
		//SendMessage(hMainScrl, GSM_SETRANGE, 0, 100);
		printf("HI, This is test.\n");
		hWndMain = _hwnd;
		CreateWindow(TEXT("button"), TEXT("TEST"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 20, 260, 100, 30, _hwnd, (HMENU)EQTESTBTN, g_hInst, NULL);
		hWavCtrl = CreateWindow(TEXT("wavctrl"), NULL, WS_CHILD, 0, 0, 0, 0, _hwnd, (HMENU)WAVCTRLWIN, g_hInst, NULL);
		pWaveHdr = (PWAVEHDR)(malloc(sizeof(WAVEHDR)));
		
		lstrcpy(explain, TEXT("Welcome"));

		break;

	case WM_COMMAND:
		switch (LOWORD(_wParam)) {
		case EQTESTBTN:
			pBuffer = (PBYTE)malloc(INP_BUFFER_SIZE);

			if (!pBuffer) {
				free(pBuffer);
				MessageBox(hWnd, TEXT("Error in WM_COMMAND"), TEXT("Error"), MB_ICONEXCLAMATION | MB_OK);
				return TRUE;
			}
			// Open waveform audio for input
			waveform.wFormatTag = WAVE_FORMAT_PCM;
			waveform.nChannels = 1;
			waveform.nSamplesPerSec = SAMPLERATE;
			waveform.nAvgBytesPerSec = SAMPLERATE * (BPS / 8);
			waveform.nBlockAlign = BPS / 8;
			waveform.wBitsPerSample = BPS;
			waveform.cbSize = 0;

			int result = waveInOpen(&hWaveIn, WAVE_MAPPER, &waveform, (DWORD)hWavCtrl, 0, CALLBACK_WINDOW);


			if (result != MMSYSERR_NOERROR) {
				printf("Error: waveInOpen<%d> \n", result);
				free(pBuffer);
			}

			// Set up headers and prepare them
			pWaveHdr->lpData = (LPSTR)pBuffer;
			pWaveHdr->dwBufferLength = INP_BUFFER_SIZE;
			pWaveHdr->dwBytesRecorded = 0;
			pWaveHdr->dwUser = 0;
			pWaveHdr->dwFlags = 0;
			pWaveHdr->dwLoops = 1;
			pWaveHdr->lpNext = NULL;
			pWaveHdr->reserved = 0;

			waveInPrepareHeader(hWaveIn, pWaveHdr, sizeof(WAVEHDR));
			lstrcpy(explain, TEXT("현재 출력 중인 소리를 분석 중 입니다."));
			InvalidateRect(_hwnd, NULL, TRUE);
			break;
		}
		return 0;

	case WM_SYSCOMMAND:
		switch (_wParam & 0xfff0)  // (filter out reserved lower 4 bits:  see msdn remarks http://msdn.microsoft.com/en-us/library/ms646360(VS.85).aspx)
		{
		case SC_CLOSE:  // redundant to WM_CLOSE, it appears
			printf("Minimize or Close Button is pushed.\n");
			ShowWindow(hWnd, SW_HIDE);
			return 0;
		}
	case WM_TRAYICON:
		printf("Tray icon notification, from %d\n", _wParam);

		if (_lParam == WM_LBUTTONDBLCLK)
		{
			printf("Window is restored by double-clicking.\n");

			if (!IsWindowVisible(hWnd))
				SetWindowPos(hWnd, HWND_TOP, screenWidth - WINDOW_WIDTH, screenHeight - WINDOW_HEIGHT, WINDOW_WIDTH, WINDOW_HEIGHT, SWP_NOSIZE);

			ShowWindow(hWnd, SW_SHOW);
		}
		else if (_lParam == WM_RBUTTONUP)
		{
			printf("Right click is done. Show menu.\n");

			POINT curPoint;
			GetCursorPos(&curPoint);

			UINT clicked = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_NONOTIFY, // don't send me WM_COMMAND messages about this window, instead return the identifier of the clicked menu item
				curPoint.x, curPoint.y, 0, _hwnd, NULL);

			if (clicked == ID_TRAY_CONTROLLER_CONTEXT_MENU_ITEM)
			{
				printf("Show Sound Controller Window\n");

				if (!IsWindowVisible(hWnd))
					SetWindowPos(hWnd, HWND_TOP, screenWidth - WINDOW_WIDTH, screenHeight - WINDOW_HEIGHT, WINDOW_WIDTH, WINDOW_HEIGHT, SWP_NOSIZE);

				ShowWindow(hWnd, SW_SHOW);
			}
			if (clicked == ID_TRAY_EXIT_CONTEXT_MENU_ITEM)
			{
				printf("Exit program\n");
				PostQuitMessage(0);
			}
			// >> 17May21
			if (clicked == ID_TRAY_PEAK_CONTEXT_MENU_ITEM) {

			}
			// <<
		}

		break;
	// >> 17May 22
	case WM_LBUTTONDOWN:
		int temp;
		epMan->getVolume(&mvLev);
		temp = ssMan->getWorkingCount();
		swprintf(mvStr, TEXT("%f %d"), mvLev, temp);
		MessageBox(_hwnd, mvStr, TEXT("dd"), MB_OK);

		for (int i = 0; i < temp; i++) {
			volume = SendMessage(hBoxes[i], WM_GETLIMITVOLUME, 0, 0);
			btn1State = SendMessage(hBoxes[i], WM_GETBTN1STATE, 0, 0);
			//btn2State = SendMessage(hBoxes[i], WM_GETBTN2STATE, 0, 0);
			printf("Session %d => volume_limit: %d, ", i, volume);
			//printf("limit button: %s, EQ button: %s\n", btn1State == 1 ? "ON" : "OFF", btn2State == 1 ? "ON" : "OFF");
		}
		InvalidateRect(_hwnd, NULL, TRUE);

		return 0;
	
	case WM_BOXVSCROLL:
		volume = (int)_lParam;
		printf("Session %d => ", (int)_wParam);
		printf("Scroll chnaged: %d\n", volume);
		return 0;
	// <<
	case WM_SENDEQNAME:
		eqName = (LPCTSTR)_wParam;
		wsprintf(explain, TEXT("추천하는 EQ 세팅: %s"), eqName);
		InvalidateRect(_hwnd, NULL, TRUE);

		return 0;

	case WM_PAINT:
		hdc = BeginPaint(_hwnd, &ps);
		TextOut(hdc, 140, 270, explain, lstrlen(explain));


		EndPaint(_hwnd, &ps);
		return 0;

	case WM_CLOSE:
		printf("Window Message CLOSE.\n");
		ShowWindow(hWnd, SW_HIDE);
		return 0;

	case WM_MVSETCHANGED:
		for (int i = 0; i < ssNum; i++) {
			SendMessage(hBoxes[i], WM_MVSETCHANGED, 0, 0);
		}
		return 0;

	case WM_DESTROY:
		printf("Window Message DESTROY.\n");
		// >> 17May21
		delete iconManager;
		// <<
		// >> 17May 22
		delete ssMan;
		delete epMan; 
		delete helper;
		delete[] hIcon;
		delete[] hBoxes;
		// <<
		PostQuitMessage(0);
		break;
	}

	return DefWindowProc(_hwnd, _uMsg, _wParam, _lParam);
}

// >> 17May21
void DrawBitmap(HDC hdc, int x, int y, HBITMAP hBit)
{
	HDC MemDC;
	HBITMAP OldBitmap;
	int bx, by;
	BITMAP bit;

	MemDC = CreateCompatibleDC(hdc);
	OldBitmap = (HBITMAP)SelectObject(MemDC, hBit);

	GetObject(hBit, sizeof(BITMAP), &bit);
	bx = bit.bmWidth;
	by = bit.bmHeight;

	BitBlt(hdc, x, y, bx, by, MemDC, 0, 0, SRCCOPY);

	SelectObject(MemDC, OldBitmap);
	DeleteObject(MemDC);
}

#define BOX_BTN1 1
#define BOX_BTN2 2
#define BOX_TIMER  10
#define BOX_TIMER_PERIOD  50

LRESULT CALLBACK BoxProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	CREATESTRUCT cs;
	TCHAR str[128];
	float volume;
	float tempVolume;
	int num;
	int limit;
	float limitf;

	HRESULT hr = NULL;
	HWND hPeakMeter = NULL;
	float peak = 0;

	content* tempContent = NULL;
	tempContent = (content*)GetWindowLongPtr(hWnd, 0);

	switch (iMessage) {
	case WM_CREATE:
		cs = *(LPCREATESTRUCT)lParam;

		tempContent = (content*)calloc(1, sizeof(content));
		tempContent->hIcon = (HICON)calloc(1, sizeof(HICON));
		tempContent->hVert = (HWND)calloc(1, sizeof(HWND));
		tempContent->hBtn1 = (HWND)calloc(1, sizeof(HWND));

		tempContent->hIcon = (HICON)(cs.lpCreateParams);
		tempContent->hVert = CreateWindow(TEXT("GrpScrl"), NULL, WS_CHILD | WS_VISIBLE | GSS_VERT, 42, 60, GetSystemMetrics(SM_CXVSCROLL), 120, hWnd, (HMENU)0, g_hInst, NULL);
		SendMessage(tempContent->hVert, GSM_SETRANGE, 0, 100);
		tempContent->hBtn1 = CreateWindow(TEXT("MyBtn"), NULL, WS_CHILD | WS_VISIBLE, 30, 195, 40, 40, hWnd, (HMENU)1, g_hInst, NULL);
		SendMessage(tempContent->hBtn1, MBW_SETIMAGE, (WPARAM)IDB_LIMITON, (LPARAM)IDB_LIMITOFF);
		SendMessage(tempContent->hBtn1, MBW_SETSTATE, (WPARAM)MBW_OFF, (LPARAM)0);

		tempContent->count = 0;
		tempContent->variable = 0;

		SetWindowLong(hWnd, 0, (LONG_PTR)tempContent);
		return 0;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case BOX_BTN1:
			num = tempContent->ctrlNum;
			printf("Limit button clicked\n");
			if ((btnState)lParam == MBW_ON) {
				printf("On\n");
				SetTimer(hWnd, BOX_TIMER, BOX_TIMER_PERIOD, NULL);
				if (num == ENDPOINTBOX) {
					hr = (tempContent->epVolume)->GetMasterVolumeLevelScalar(&tempContent->limitVolume);
					bMasterVolumeOn = TRUE;
					SendMessage(GetParent(hWnd), WM_MVSETCHANGED, 0, 0);
				}
				else {
					hr = (tempContent->saVolume)->GetMasterVolume(&tempContent->limitVolume);
				}
				tempContent->tempVolume = tempContent->limitVolume;
			}
			else {
				printf("Off\n");
				KillTimer(hWnd, BOX_TIMER);
				if (num == ENDPOINTBOX) {
					(tempContent->epVolume)->SetMasterVolumeLevelScalar(tempContent->limitVolume, NULL);
					bMasterVolumeOn = FALSE;
					SendMessage(GetParent(hWnd), WM_MVSETCHANGED, 0, 0);
				}
				else {
					hr = (tempContent->saVolume)->SetMasterVolume(tempContent->limitVolume, NULL);
				}
			}
			break;
		}
		return 0;
	case WM_TIMER:
		printf("We got into WM_TIMER!\n");
		switch ((int)wParam) {
		case BOX_TIMER:
			printf("We got into BOX_TIMER!\n");
			num = tempContent->ctrlNum;
			hr = (tempContent->pMeter)->GetPeakValue(&peak);
			if (FAILED(hr)) {
				MessageBox(NULL, TEXT("The program will exit."), TEXT("Fatal error"), MB_OK);
				KillTimer(hWnd, BOX_TIMER);
				return 0;
			}
			limit = 100 - SendMessage(tempContent->hVert, GSM_GETPOS, 0, 0);
			limitf = (float)limit / 100;
			tempVolume = tempContent->tempVolume;

			if (num == ENDPOINTBOX) {
				if (peak * tempVolume > limitf) {
					tempVolume = limitf / peak;
					tempContent->count = 0;
					tempContent->variable = (tempContent->limitVolume - tempVolume) / 20;
				}
				else {
					if (tempContent->count < 20) {
						tempVolume += tempContent->variable;
						tempContent->count++;
					}
				}
				(tempContent->epVolume)->SetMasterVolumeLevelScalar(tempVolume, NULL);
				tempContent->tempVolume = tempVolume;
			}
			else {
				if (peak * tempVolume > limitf) {
					tempVolume = limitf / peak;
					tempContent->count = 0;
					tempContent->variable = (tempContent->limitVolume - tempVolume) / 20;
				}
				else {
					if (tempContent->count < 20) {
						tempVolume += tempContent->variable;
						tempContent->count++;
					}
				}
				(tempContent->saVolume)->SetMasterVolume(tempVolume, NULL);
				tempContent->tempVolume = tempVolume;
			}
			return 0;
		}
		break;
	case WM_SETCTRLID:
		tempContent->ctrlNum = (int)wParam;
		return 0;
	case WM_SETABOUTVOLUME:
		if (tempContent->ctrlNum == ENDPOINTBOX) {
			tempContent->pMeter = (IAudioMeterInformation*)wParam;
			tempContent->epVolume = (IAudioEndpointVolume*)lParam;
			tempContent->saVolume = NULL;
		}
		else {
			tempContent->pMeter = (IAudioMeterInformation*)wParam;
			tempContent->epVolume = NULL;
			tempContent->saVolume = (ISimpleAudioVolume*)lParam;
		}
		return 0;
	case WM_GETLIMITVOLUME:
		limit = SendMessage(tempContent->hVert, GSM_GETPOS, 0, 0);
		return limit;
	case WM_GETBTN1STATE:
		if (SendMessage(tempContent->hBtn1, MBW_GETSTATE, 0, 0) == MBW_ON) return 1;
		return 0;
	case WM_VSCROLL:
		if (lParam == (LPARAM)tempContent->hVert) {
			InvalidateRect(hWnd, NULL, TRUE);
		}
		SendMessage(GetParent(hWnd), WM_BOXVSCROLL, (WPARAM)(tempContent->ctrlNum), (LPARAM)(100 - SendMessage(tempContent->hVert, GSM_GETPOS, 0, 0)));
		return 0;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		DrawIcon(hdc, 34, 14, tempContent->hIcon);

		EndPaint(hWnd, &ps);
		return 0;
	case WM_LBUTTONDOWN:
		num = tempContent->ctrlNum;
		hr = (tempContent->pMeter)->GetPeakValue(&peak);
		if (num == ENDPOINTBOX)
			hr = (tempContent->epVolume)->GetMasterVolumeLevelScalar(&volume);
		else
			hr = (tempContent->saVolume)->GetMasterVolume(&volume);
		swprintf(str, TEXT("peak: %f, volume: %f"), peak, volume);
		MessageBox(hWnd, str, TEXT("TEST"), MB_OK);
		return 0;
	case WM_MOUSEMOVE:

		return 0;
	case WM_LBUTTONUP:

		return 0;
	case WM_RBUTTONDOWN:
		SendMessage(hWnd, WM_SHOWPEAKMETER, 0, 0);
		return 0;

	case WM_SHOWPEAKMETER:
		DialogBoxParam(g_hInst, TEXT("PEAKMETER"), hWnd, (DLGPROC)DlgProc, (LPARAM)tempContent->pMeter);
		return 0;

	case WM_MVSETCHANGED:
		num = tempContent->ctrlNum;

		if (num != ENDPOINTBOX) {
			if (SendMessage(tempContent->hBtn1, MBW_GETSTATE, 0, 0) == MBW_ON) {
				printf("Off\n");
				KillTimer(hWnd, BOX_TIMER);
				SendMessage(tempContent->hBtn1, MBW_SETSTATE, (WPARAM)MBW_OFF, 0);
				hr = (tempContent->saVolume)->SetMasterVolume(tempContent->limitVolume, NULL);
			}
			SendMessage(tempContent->hBtn1, MBW_SETMVSTATE, (WPARAM)bMasterVolumeOn, 0);
		}

		return 0;

	case WM_DESTROY:
		KillTimer(hWnd, BOX_TIMER);
		free(tempContent->hIcon);
		free(tempContent->hVert);
		free(tempContent->hBtn1);
		free(tempContent);
		return 0;
	default:
		return DefWindowProc(hWnd, iMessage, wParam, lParam);
	}
	return 0;
}

//-----------------------------------------------------------
// DlgProc -- Dialog box procedure
//-----------------------------------------------------------
#define ID_TIMER  1
#define TIMER_PERIOD  50

BOOL CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static IAudioMeterInformation *pMeterInfo = NULL;
	static HWND hPeakMeter = NULL;
	static float peak = 0;
	HRESULT hr;

	switch (message) {
	case WM_INITDIALOG:
		pMeterInfo = (IAudioMeterInformation*)lParam;
		SetTimer(hDlg, ID_TIMER, TIMER_PERIOD, NULL);
		hPeakMeter = GetDlgItem(hDlg, IDC_PEAK_METER);
		return TRUE;
	case WM_COMMAND:
		switch ((int)LOWORD(wParam)) {
		case IDCANCEL:
			KillTimer(hDlg, ID_TIMER);
			EndDialog(hDlg, TRUE);
			return TRUE;
		}
		break;
	case WM_TIMER:
		switch ((int)wParam) {
		case ID_TIMER:
			// Update the peak meter in the dialog box.
			hr = pMeterInfo->GetPeakValue(&peak);
			if (FAILED(hr)) {
				MessageBox(hDlg, TEXT("The program will exit."), TEXT("Fatal error"), MB_OK);
				KillTimer(hDlg, ID_TIMER);
				EndDialog(hDlg, TRUE);
				return TRUE;
			}
			DrawPeakMeter(hPeakMeter, peak);
			return TRUE;
		}
		break;

	case WM_PAINT:
		// Redraw the peak meter in the dialog box.
		ValidateRect(hPeakMeter, NULL);
		DrawPeakMeter(hPeakMeter, peak);
		break;
	}
	return FALSE;
}

//-----------------------------------------------------------
// DrawPeakMeter -- Draws the peak meter in the dialog box.
//-----------------------------------------------------------

void DrawPeakMeter(HWND hPeakMeter, float peak)
{
	HDC hdc;
	RECT rect;

	GetClientRect(hPeakMeter, &rect);
	hdc = GetDC(hPeakMeter);
	FillRect(hdc, &rect, (HBRUSH)(COLOR_3DSHADOW + 1));
	rect.left++;
	rect.top++;
	rect.right = rect.left + max(0, (LONG)(peak*(rect.right - rect.left) - 1.5));
	rect.bottom--;
	FillRect(hdc, &rect, (HBRUSH)(COLOR_3DHIGHLIGHT + 1));
	ReleaseDC(hPeakMeter, hdc);
}

LRESULT CALLBACK WavProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static TCHAR szOpenError[] = TEXT("Error opening waveform audio!");
	static TCHAR szMemError[] = TEXT("Error allocating memory!");
	static HWAVEIN hwi;

	switch (message) {
	case WIM_OPEN:
		hwi = (HWAVEIN)wParam;

		waveInAddBuffer(hwi, pWaveHdr, sizeof(WAVEHDR));
		bRecording = TRUE;
		dwDataLength = 0;
		waveInStart(hwi);

		return 0;

	case WIM_DATA:
		WavToFreq(hWndMain, (PBYTE)((PWAVEHDR)lParam)->lpData, ((PWAVEHDR)lParam)->dwBytesRecorded);
		return 0;

	case WIM_CLOSE:
		waveInUnprepareHeader(hwi, pWaveHdr, sizeof(WAVEHDR));
		waveInClose(hwi);
		free(pBuffer);

		bRecording = FALSE;
		return 0;
	}
}
