
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
	HWND tempHwnd = FindWindow(NULL, NULL); // �ֻ��� ������ �ڵ� ã��   

	while (tempHwnd != NULL) {
		if (GetParent(tempHwnd) == NULL) // �ֻ��� �ڵ����� üũ, ��ư � �ڵ��� ���� �� �����Ƿ� �����ϱ� ����   
			if (pid == procIDFromWnd(tempHwnd))
				return tempHwnd;
		tempHwnd = GetWindow(tempHwnd, GW_HWNDNEXT); // ���� ������ �ڵ� ã��   
	}
	return NULL;
}

HICON MyIconManager::getAppIcon1(HWND hwnd) // �������� ���� �� �ִ� ������� ���ʴ�� �õ��Ѵ�. �ϳ��� ������ ������ ����
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

	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid); // ���μ��� ���̵�� ��� �ڵ��� ��´�.
	if (hProcess == NULL) return NULL;
	
	GetModuleFileNameEx((HMODULE)hProcess, NULL, strPath, sizeof(strPath) / sizeof(TCHAR)); // ��� �ڵ�� ���� ���� �̸�(���)�� ��´�.

	numOfIcon = (int)ExtractIcon(hInstance, strPath, -1); // �������� ������ ���Ѵ�.
	if (numOfIcon < 1) return NULL; // �������� ���� ���

	hIcon = ExtractIcon(hInstance, strPath, 0); // ��κ��� ��� ù ��°(index: 0) �������� ���� �������̴�.

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