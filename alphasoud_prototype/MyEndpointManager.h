
#ifndef __MYENDPOINTMANAGER_H_
#define __MYENDPOINTMANAGER_H_

class MyEndpointManager
{
private:
	HWND hWnd; // �� Ŭ������ ����ϴ� �������� �ڵ�
	IAudioEndpointVolume* pEndpointVolume;
	IAudioMeterInformation* pMeterInformation;
	float val;

public:
	MyEndpointManager(HWND hWnd, CoreAudioInterfaceHelper* helper) : hWnd(hWnd)
	{
		reset(helper);
	}
	~MyEndpointManager()
	{
		SafeRelease(&pEndpointVolume);
		SafeRelease(&pMeterInformation);
	}

	BOOL getVolume(float* valOut);
	BOOL setVolume(float newVal);
	BOOL getPeakMeter(float* peakOut);
	IAudioEndpointVolume* getEndpointVolume() { return pEndpointVolume; }
	IAudioMeterInformation* getMeterInfo() { return pMeterInformation; }

	void reset(CoreAudioInterfaceHelper* helper)
	{
		pEndpointVolume = helper->getEndpointVolume();
		pMeterInformation = helper->getMeterInformation();
	}
};

#endif // !__MYENDPOINTMANAGER_H_
