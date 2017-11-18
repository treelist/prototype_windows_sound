/*
아이콘 얻는 방법이 여러가지인데,
어플리케이션 마다 유효한 방법이 달라서,
모든 방법을 시도하여 아이콘을 얻어 놓고,
그 중 가장 적절한 것을 필요에 따라 불러올 수 있도록.
*/

#ifndef __MYICONMANAGER_H_
#define __MYICONMANAGER_H_

const int NUM_OF_ICON = 2;

class MyIconManager
{
private:
	HINSTANCE hInstance;
	DWORD processID;
	HWND hWnd;
	HICON hIcon[NUM_OF_ICON];

private: // 클래스 내부에서만 사용하는 함수
		 /*
		 윈도우 핸들로 프로세스 아이디 얻기.
		 */
	DWORD procIDFromWnd(HWND hwnd);
	/*
	프로세스 아이디로 윈도우 핸들 얻기.
	찾지 못한경우 NULL을 리턴.
	*/
	HWND getWinHandle(DWORD pid);
	/*
	HICON을 구하는 첫 번째 방법
	*/
	HICON getAppIcon1(HWND hWnd);
	/*
	HICON을 구하는 두 번째 방법
	*/
	HICON getAppIcon2(HINSTANCE hInstance, DWORD pid);
	/*
	아이콘을 구한다.
	getAppIcon1으로 얻은 아이콘은 hIcon[0]에
	getAppIcon2으로 얻은 아이콘은 hIcon[1]에
	*/
	void tryGetIcon();
public:
	MyIconManager(HINSTANCE hInstance) : hInstance(hInstance) {	}
	MyIconManager(HINSTANCE hInstance, DWORD processID) : hInstance(hInstance), processID(processID)
	{
		hWnd = getWinHandle(processID);
	}

	void setPID(DWORD processID);
	
	/*
	아이콘을 리턴한다.
	i = 0 이면 hIcon[0].
	i = 1 이면 hIcon[1].
	그 외의 경우 둘 중 NULL이 아닌 것. 둘 다 NULL이 아닐 경우 hIcon[0]이 우선적으로.
	*/
	HICON getIcon(int i);
};

#endif // !__MYICONMANAGER_H_
