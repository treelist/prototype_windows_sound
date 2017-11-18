
#include <endpointvolume.h> // IAudioEndpointVolume
#include "CoreAudioInterfaceHelper.h"
#include "MyEndpointManager.h"

BOOL MyEndpointManager::getVolume(float* valOut)
{
	HRESULT hr;
	hr = pEndpointVolume->GetMasterVolumeLevelScalar(valOut);
	if (FAILED(hr)) { return FALSE; }

	return TRUE;
}
BOOL MyEndpointManager::setVolume(float newVal)
{
	HRESULT hr;
	hr = pEndpointVolume->SetMasterVolumeLevelScalar(newVal, NULL);
	if (FAILED(hr)) { return FALSE; }
	
	return TRUE;
}
BOOL MyEndpointManager::getPeakMeter(float* peakOut)
{
	HRESULT hr;
	hr = pMeterInformation->GetPeakValue(peakOut);
	if (FAILED(hr)) { return FALSE; }

	return TRUE;
}