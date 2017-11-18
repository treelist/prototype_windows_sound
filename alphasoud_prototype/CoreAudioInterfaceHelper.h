
#ifndef __COREAUDIOINTERFACEHELPER_H_
#define __COREAUDIOINTERFACEHELPER_H_

#include <Windows.h>
#include <Mmdeviceapi.h> // IMMDeviceEnumerator, IMMDevice
#include <endpointvolume.h> // IAudioEndpointVolume
#include <audiopolicy.h> // IAudioSessionManager, IAudioSessionEnumerator

template <class T>
void SafeRelease(T **ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = NULL;
	}
}

class CoreAudioInterfaceHelper
{
private:
	HWND hWnd;
	IMMDevice* pDevice;
	IAudioEndpointVolume* pEndpointVolume;
	IAudioMeterInformation* pMeterInformation;
	IAudioSessionEnumerator* pSessionEnumerator;

private: // 클래스 내부에서만 사용하는 함수
	BOOL tryGetDevice(IMMDevice** ppDevice);
	IMMDevice* getDevice() { return pDevice; }
	BOOL tryGetEndpointVolume(IAudioEndpointVolume** ppEndpointVolume);
	BOOL tryGetMeterInformation(IAudioMeterInformation** ppMeterInformation); 
	BOOL tryGetSessionEnumerator(IAudioSessionEnumerator** ppSessionEnumerator);

public:
	CoreAudioInterfaceHelper(HWND hWnd) : hWnd(hWnd)
	{
		CoInitializeEx(NULL, COINIT_MULTITHREADED);
		
		tryGetDevice(&pDevice);
		tryGetEndpointVolume(&pEndpointVolume);
		tryGetMeterInformation(&pMeterInformation);
		tryGetSessionEnumerator(&pSessionEnumerator);
		
		CoUninitialize();
	}
	~CoreAudioInterfaceHelper()
	{
		SafeRelease(&pDevice);
		SafeRelease(&pEndpointVolume);
		SafeRelease(&pMeterInformation);
		SafeRelease(&pSessionEnumerator);
	}

	IAudioEndpointVolume* getEndpointVolume() { return pEndpointVolume; }
	IAudioMeterInformation* getMeterInformation() { return pMeterInformation; }
	IAudioSessionEnumerator* getSessionEnumerator() { return pSessionEnumerator; }

	void reset() {
		CoInitializeEx(NULL, COINIT_MULTITHREADED);

		tryGetDevice(&pDevice);
		tryGetEndpointVolume(&pEndpointVolume);
		tryGetMeterInformation(&pMeterInformation);
		tryGetSessionEnumerator(&pSessionEnumerator);

		CoUninitialize();
	}
};

#endif // !__COREAUDIOINTERFACEHELPER_H_
