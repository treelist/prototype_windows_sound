/*
����� ���ǿ� ���� ������ ���ϰ� ���� �����ϴ� Ŭ�����Դϴ�.
*/

#ifndef __MYSESSIONMANAGER_H_
#define __MYSESSIONMANAGER_H_

struct controller
{
	IAudioSessionControl* pSessionControl;
	IAudioSessionControl2* pSessionControl2;
	IAudioMeterInformation* pMeterInformation;
	ISimpleAudioVolume* pAudioVolume;
	AudioSessionState state;
	GUID groupingParam;
	DWORD ProcessID;
};
struct workingPair
{
	IAudioMeterInformation* pMeterInformation;
	ISimpleAudioVolume* pAudioVolume;
	DWORD ProcessID;
};

class MySessionManager
{
private:
	HWND hWnd;
	int SessionCount; // ��ȿ�� ���� �����Դϴ�.
	int WorkingSessionCount;
	controller* controllers;
	workingPair* pairs;
	IAudioSessionEnumerator* pSessionEnumerator;
	
private: // Ŭ���� ���ο����� ����ϴ� �Լ�
	BOOL tryGetCount(int* pSessionCount);
	BOOL tryGetControllers(controller* controllers);
	BOOL tryGetWorkingCount(int* pWorkingSessionCount);
	BOOL tryGetPairs(workingPair* pairs);
public:
	MySessionManager(HWND hWnd, CoreAudioInterfaceHelper* helper) : hWnd(hWnd), SessionCount(0), WorkingSessionCount(0)
	{
		reset(helper);
	}
	~MySessionManager()
	{
		release();
		SafeRelease(&pSessionEnumerator);
	}

	int getCount() { return SessionCount; }
	int getWorkingCount() { return WorkingSessionCount; }
	workingPair* getPairs() { return pairs; }

	void release()
	{
		for (int i = 0; i < SessionCount; i++) {
			SafeRelease(&(controllers[i].pMeterInformation));
			SafeRelease(&(controllers[i].pAudioVolume));
			SafeRelease(&(controllers[i].pSessionControl2));
			SafeRelease(&(controllers[i].pSessionControl));
			delete[] controllers;
			delete[] pairs;
		}
	}
	void reset(CoreAudioInterfaceHelper* helper)
	{
		CoInitializeEx(NULL, COINIT_MULTITHREADED);
		
		release();
		pSessionEnumerator = helper->getSessionEnumerator();
		
		tryGetCount(&SessionCount);
		controllers = new controller[SessionCount];
		tryGetControllers(controllers);

		tryGetWorkingCount(&WorkingSessionCount);
		pairs = new workingPair[WorkingSessionCount];
		if (tryGetPairs(pairs) == FALSE) {
			MessageBox(NULL, TEXT("tryGetPairs"), TEXT("Error"), MB_OK);
		}
		
		CoUninitialize();
	}
};
#endif // !__MYSESSIONMANAGER_H_
