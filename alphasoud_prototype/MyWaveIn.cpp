
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "libfftw3-3.lib")

#include <Windows.h>
//#include <mmsystem.h>
#include <math.h>

#include "fftw3.h"
#include "MyWaveIn.h"

const double FFTFREQ_N = INP_BUFFER_SIZE / 2; // SAMPLEREATE * (BPS / 8) * SEC
const double FFTFREQ_D = 1.0 / SAMPLERATE;
const double FFTFREQ_T = FFTFREQ_N * FFTFREQ_D;
const int RATIO = SAMPLERATE / 2;

//LPCTSTR PRESETNAME[10] = { TEXT("POP"),TEXT("LIVE"), TEXT("CLUB"), TEXT("ROCK"), TEXT("BASS"), TEXT("TREBLE"), TEXT("VOCAL"), TEXT("CLASS"), TEXT("COUNTRY"), TEXT("JAZZ") };
LPCTSTR PRESETNAME[10] = { TEXT("팝"),TEXT("라이브"), TEXT("클럽"), TEXT("락"), TEXT("저음강조"), TEXT("고음강조"), TEXT("보컬"), TEXT("클래식"), TEXT("컨트리"), TEXT("재즈") };
const int PRESET[10][10] = {
	{2,5,6,4,1,1,1,2,6,7}, // pop
	{2,3,4,2,2,2,2,3,4,5}, // live
	{6,6,5,2,1,1,1,2,5,6}, // club
	{2,3,4,5,2,2,5,5,3,5}, // rock
	{6,6,6,4,2,2,1,1,1,1}, // bass
	{1,1,1,1,1,2,4,6,6,6}, // treble
	{2,2,2,5,7,7,7,5,4,4}, // vocal
	{2,6,6,4,2,2,2,2,3,4}, // class
	{0,2,2,4,4,2,2,2,4,5}, // country
	{2,2,2,5,5,5,2,5,5,7}  // jazz
};
int SUMOFPRESET[10] = { 35, 29, 35, 36, 30, 29, 45, 33, 27, 40 };


double abs_complex(fftw_complex comp)
{
	return sqrt((comp[0] * comp[0]) + (comp[1] * comp[1]));
}

double getValue(const int* preset, double* freq ) {
	double sum = 0.0;
	double diff = 0.0;

	for (int i = 0; i < 10; i++) {
		diff = (double)preset[i] / SUMOFPRESET[i] - freq[i];
		sum += diff * diff;
	}

	return sum;
}
int getBestEQ(double* freq) {
	int best = -1;
	double temp = 1024.0;
	double score[10] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

	for (int i = 0; i < 10; i++) {
		score[i] = getValue(PRESET[i], freq);
	}

	for (int i = 0; i < 10; i++) {
		if (temp > score[i]) {
			temp = score[i];
			best = i;
		}
	}

	return best;
}

HRESULT WavToFreq(HWND hWnd, PBYTE Buffer, DWORD BufferLen)
{
	short* wav = (short*)Buffer;
	int length = BufferLen / 2;

	if (BufferLen != INP_BUFFER_SIZE) {
		printf("Error in WavToFreq\n");
		return -1;
	}

	fftw_complex* in;
	fftw_complex* out;
	fftw_plan p;

	int N = length;

	in = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * N);
	out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * N);
	p = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

	for (int i = 0; i < length; i++) {
		in[i][0] = (double)wav[i];
		in[i][1] = 0;
	}

	fftw_execute(p);

	int temp;
	double fftfreq;
	double freq[10] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

	double sum = 1.0;

	for (int i = 0; i < length / 2; i++) {
		fftfreq = i / FFTFREQ_T;
		temp = (int)(abs_complex(out[i]) / SAMPLERATE);

		if (fftfreq < 22) {
			continue;
		}
		else if (fftfreq < 44) {
			freq[0] += temp;
		}
		else if (fftfreq < 88) {
			freq[1] += temp;
		}
		else if (fftfreq < 177) {
			freq[2] += temp;
		}
		else if (fftfreq < 354) {
			freq[3] += temp;
		}
		else if (fftfreq < 707) {
			freq[4] += temp;
		}
		else if (fftfreq < 1414) {
			freq[5] += temp;
		}
		else if (fftfreq < 2828) {
			freq[6] += temp;
		}
		else if (fftfreq < 5657) {
			freq[7] += temp;
		}
		else if (fftfreq < 11314) {
			freq[8] += temp;
		}
		else if (fftfreq < 22627) {
			freq[9] += temp;
		}
		else {
			continue;
		}
	}

	for (int i = 0; i < 10; i++) {
		sum += freq[i];
	}
	for (int i = 0; i < 10; i++) {
		freq[i] = freq[i] / sum;
	}

	printf("\n");
	printf("EQ test: %s\n", PRESETNAME[getBestEQ(freq)]);

	SendMessage(hWnd, WM_SENDEQNAME, WPARAM(PRESETNAME[getBestEQ(freq)]), LPARAM(0));

	fftw_destroy_plan(p);
	fftw_free(in);
	fftw_free(out);

	return 0;
}

