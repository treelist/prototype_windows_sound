
#include <Windows.h>
#include <Psapi.h>
#include "MyIconManager.h"

void MyIconManager::setPID(DWORD processID)
{
	this->processID = processID;
	hWnd = getWinHandle(processID);
	tryGetIcon();
}

ULONG MyIconManager::procIDFromWnd(HWND hwnd)
{
	ULONG idProc;
	GetWindowThreadProcessId(hwnd, &idProc);
	return idProc;
}

HWND MyIconManager::getWinHandle(DWORD pid)
{
	HWND tempHwnd = FindWindow(NULL, NULL); // 최상위 윈도우 핸들 찾기   

	while (tempHwnd != NULL) {
		if (GetParent(tempHwnd) == NULL) // 최상위 핸들인지 체크, 버튼 등도 핸들을 가질 수 있으므로 무시하기 위해   
			if (pid == procIDFromWnd(tempHwnd))
				return tempHwnd;
		tempHwnd = GetWindow(tempHwnd, GW_HWNDNEXT); // 다음 윈도우 핸들 찾기   
	}
	return NULL;
}

HICON MyIconManager::getAppIcon1(HWND hwnd) // 아이콘을 얻을 수 있는 방법들을 차례대로 시도한다. 하나라도 있으면 가지고 리턴
{
	HICON hIcon = (HICON)NULL;

	hIcon = (HICON)SendMessage(hwnd, WM_GETICON, ICON_BIG, 0);
	if (hIcon != (HICON)NULL) return hIcon;

	hIcon = (HICON)SendMessage(hwnd, WM_GETICON, ICON_SMALL, 0);
	if (hIcon != (HICON)NULL) return hIcon;

	hIcon = (HICON)SendMessage(hwnd, WM_GETICON, ICON_SMALL2, 0);
	if (hIcon != (HICON)NULL) return hIcon;

	hIcon = (HICON)GetClassLongPtr(hwnd, GCLP_HICON);
	if (hIcon != (HICON)NULL) return hIcon;

	hIcon = (HICON)GetClassLongPtr(hwnd, GCLP_HICONSM);
	if (hIcon != (HICON)NULL) return hIcon;

	return hIcon;
}

HICON MyIconManager::getAppIcon2(HINSTANCE hInstance, DWORD pid)
{
	HANDLE hProcess;
	HICON hIcon;
	TCHAR strPath[MAX_PATH];
	int numOfIcon;

	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid); // 프로세스 아이디로 모듈 핸들을 얻는다.
	if (hProcess == NULL) return NULL;
	
	GetModuleFileNameEx((HMODULE)hProcess, NULL, strPath, sizeof(strPath) / sizeof(TCHAR)); // 모듈 핸들로 실행 파일 이름(경로)을 얻는다.

	numOfIcon = (int)ExtractIcon(hInstance, strPath, -1); // 아이콘의 개수를 구한다.
	if (numOfIcon < 1) return NULL; // 아이콘이 없는 경우

	hIcon = ExtractIcon(hInstance, strPath, 0); // 대부분의 경우 첫 번째(index: 0) 아이콘이 메인 아이콘이다.

	return hIcon;
}

void MyIconManager::tryGetIcon()
{
	hIcon[0] = getAppIcon1(hWnd);
	hIcon[1] = getAppIcon2(hInstance, processID);
}

HICON MyIconManager::getIcon(int i)
{
	if (i == 0) return hIcon[0];
	if (i == 1) return hIcon[1];
	if (hIcon[1] != NULL) return hIcon[1];

	return hIcon[0];
}