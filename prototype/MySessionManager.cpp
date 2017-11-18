
#include "CoreAudioInterfaceHelper.h"

#include "MySessionManager.h"

#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = 0; } }

BOOL MySessionManager::tryGetCount(int* pNum)
{
	HRESULT hr;
	hr = pSessionEnumerator->GetCount(pNum);
	if (FAILED(hr)) { return FALSE; }

	return TRUE;
}
BOOL MySessionManager::tryGetControllers(controller* controllers)
{
	HRESULT hr;

	for (int i = 0; i < SessionCount; i++) {

		hr = pSessionEnumerator->GetSession(i, &(controllers[i].pSessionControl));
		if (FAILED(hr)) { return FALSE; }

		hr = controllers[i].pSessionControl->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&(controllers[i].pSessionControl2));
		if (FAILED(hr)) { return FALSE; }
		
		hr = controllers[i].pSessionControl->QueryInterface(__uuidof(IAudioMeterInformation), (void**)&(controllers[i].pMeterInformation));
		if (FAILED(hr)) { return FALSE; }

		hr = controllers[i].pSessionControl2->QueryInterface(__uuidof(ISimpleAudioVolume), (void**)&(controllers[i].pAudioVolume));
		if (FAILED(hr)) { return FALSE; }

		hr = controllers[i].pSessionControl->GetGroupingParam(&(controllers[i].groupingParam));
		if (FAILED(hr)) { return FALSE; }

		hr = controllers[i].pSessionControl2->GetState(&(controllers[i].state));
		if (FAILED(hr)) { return FALSE; }

		hr = controllers[i].pSessionControl2->GetProcessId(&(controllers[i].ProcessID));
		if (hr != S_OK) { controllers[i].ProcessID = 0; }
	}

	return TRUE;
}
BOOL MySessionManager::tryGetWorkingCount(int* pWorkingSessionCount)
{
	HRESULT hr;
	int counter = 0;

	if (SessionCount == 0) { *pWorkingSessionCount = counter; return FALSE; }

	/*
	for (int i = 0; i < SessionCount; i++) {
		if (controllers[i].ProcessID != 0) {
			counter++;
		}
	}
	*/
	for (int i = 0; i < SessionCount; i++) {
		if (controllers[i].state == AudioSessionStateActive) {
			counter++;
		}
	}

	*pWorkingSessionCount = counter;
	return TRUE;
}
BOOL MySessionManager::tryGetPairs(workingPair* pairs)
{
	HRESULT hr;
	TCHAR str[100];
	GUID tempGroupParam = { 0,0,0,0 };
	int prCounter = 0;

	for (int i = 0; i < SessionCount; i++) {
		if (controllers[i].groupingParam == tempGroupParam) {
			continue;
		}
		tempGroupParam = controllers[i].groupingParam;

		if ((controllers[i].state == AudioSessionStateActive) && controllers[i].ProcessID != 0) {
			pairs[prCounter].pMeterInformation = controllers[i].pMeterInformation;
			pairs[prCounter].ProcessID = controllers[i].ProcessID;
			pairs[prCounter].pAudioVolume = controllers[i].pAudioVolume;
			prCounter++;
			continue;
		}
		if ((controllers[i].state == AudioSessionStateActive) && controllers[i].ProcessID == 0) {
			pairs[prCounter].pMeterInformation = controllers[i].pMeterInformation;
			for (int j = i + 1; j < SessionCount; j++) {
				if ((controllers[j].groupingParam == tempGroupParam) && controllers[j].ProcessID != 0) {
					pairs[prCounter].ProcessID = controllers[j].ProcessID;
					pairs[prCounter].pAudioVolume = controllers[i].pAudioVolume;
					prCounter++;
					continue;
				}
			}
		}
		if ((controllers[i].state == AudioSessionStateInactive) && controllers[i].ProcessID != 0) {
			pairs[prCounter].ProcessID = controllers[i].ProcessID;
			pairs[prCounter].pAudioVolume = controllers[i].pAudioVolume;
			for (int j = i + 1; j < SessionCount; j++) {
				if ((controllers[j].groupingParam == tempGroupParam) && controllers[j].state == AudioSessionStateActive) {
					pairs[prCounter].pMeterInformation = controllers[j].pMeterInformation;
					prCounter++;
					continue;
				}
			}
		}
	}
	if (prCounter != WorkingSessionCount) {
		wsprintf(str, TEXT("prCounter != WorkingSessionCount <%d %d>"), prCounter, WorkingSessionCount);
		MessageBox(NULL, str, TEXT("Error"), MB_OK);
		return FALSE;
	}

	return TRUE;
}