
#include "CoreAudioInterfaceHelper.h"

BOOL CoreAudioInterfaceHelper::tryGetDevice(IMMDevice** ppDevice)
{
	HRESULT hr = NULL;

	IMMDeviceEnumerator* pDeviceEnumerator = NULL;
	IMMDevice* pDevice = NULL;

	hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (LPVOID*)&pDeviceEnumerator);
	if (FAILED(hr)) { return FALSE; }

	hr = pDeviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
	if (FAILED(hr)) { return FALSE; }

	*ppDevice = pDevice;

	SafeRelease(&pDeviceEnumerator);

	return TRUE;
}

BOOL CoreAudioInterfaceHelper::tryGetEndpointVolume(IAudioEndpointVolume** ppEndpointVolume)
{
	HRESULT hr = NULL;
	IMMDevice* pDevice = NULL;
	IAudioEndpointVolume* pEndpointVolume = NULL;

	pDevice = getDevice();

	hr = pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (LPVOID*)&pEndpointVolume);
	if (FAILED(hr)) { return FALSE; }

	*ppEndpointVolume = pEndpointVolume;
	
	return TRUE;
}
BOOL CoreAudioInterfaceHelper::tryGetMeterInformation(IAudioMeterInformation** ppMeterInformation)
{
	HRESULT hr = NULL;
	IMMDevice* pDevice = NULL;
	IAudioMeterInformation* pMeterInformation = NULL;

	pDevice = getDevice();

	hr = pDevice->Activate(__uuidof(IAudioMeterInformation), CLSCTX_ALL, NULL, (LPVOID*)&pMeterInformation);
	if (FAILED(hr)) { return FALSE; }

	*ppMeterInformation = pMeterInformation;

	return TRUE;
}
BOOL CoreAudioInterfaceHelper::tryGetSessionEnumerator(IAudioSessionEnumerator** ppSessionEnumerator)
{
	HRESULT hr = NULL;
	IMMDevice* pDevice = NULL;
	IAudioSessionManager2* pSessionManager = NULL;
	IAudioSessionEnumerator* pSessionEnumerator = NULL;
	
	pDevice = getDevice();

	hr = pDevice->Activate(__uuidof(IAudioSessionManager2), CLSCTX_INPROC_SERVER, NULL, (void**)&pSessionManager);
	if (FAILED(hr)) { return FALSE; }

	hr = pSessionManager->GetSessionEnumerator(&pSessionEnumerator);
	if (FAILED(hr)) { return FALSE; }

	*ppSessionEnumerator = pSessionEnumerator;

	SafeRelease(&pSessionManager);
	
	return TRUE;
}
