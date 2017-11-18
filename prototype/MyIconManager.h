/*
������ ��� ����� ���������ε�,
���ø����̼� ���� ��ȿ�� ����� �޶�,
��� ����� �õ��Ͽ� �������� ��� ����,
�� �� ���� ������ ���� �ʿ信 ���� �ҷ��� �� �ֵ���.
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

private: // Ŭ���� ���ο����� ����ϴ� �Լ�
		 /*
		 ������ �ڵ�� ���μ��� ���̵� ���.
		 */
	DWORD procIDFromWnd(HWND hwnd);
	/*
	���μ��� ���̵�� ������ �ڵ� ���.
	ã�� ���Ѱ�� NULL�� ����.
	*/
	HWND getWinHandle(DWORD pid);
	/*
	HICON�� ���ϴ� ù ��° ���
	*/
	HICON getAppIcon1(HWND hWnd);
	/*
	HICON�� ���ϴ� �� ��° ���
	*/
	HICON getAppIcon2(HINSTANCE hInstance, DWORD pid);
	/*
	�������� ���Ѵ�.
	getAppIcon1���� ���� �������� hIcon[0]��
	getAppIcon2���� ���� �������� hIcon[1]��
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
	�������� �����Ѵ�.
	i = 0 �̸� hIcon[0].
	i = 1 �̸� hIcon[1].
	�� ���� ��� �� �� NULL�� �ƴ� ��. �� �� NULL�� �ƴ� ��� hIcon[0]�� �켱������.
	*/
	HICON getIcon(int i);
};

#endif // !__MYICONMANAGER_H_
