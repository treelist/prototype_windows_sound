#ifndef __MYWAVEIN_H_
#define __MYWAVEIN_H_

// 44100(Sample rate) * 2(Bytes for sampling) * 0.2(sec)
#define INP_BUFFER_SIZE (44100 * 2 * 5)
#define IDC_START 1
#define IDC_STOP 2

//defines for menu
#define APP_SAVE 5201
#define APP_EXIT  5202

#define SAMPLERATE 44100
#define	BPS 16

#define WAVCTRLWIN 7200

HRESULT WavToFreq(HWND hWnd, PBYTE Buffer, DWORD BufferLen);

#define WM_SENDEQNAME WM_USER + 3701

#endif // !__MYWAVEIN_H_
