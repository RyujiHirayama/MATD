#pragma once
#include "AudioFile.h" // this method needs the external files @copyright Copyright (C) 2017  Adam Stark
#define _USE_MATH_DEFINES
#include <math.h>
#include <fstream>
#include <fftw3.h> // this method also need the external library for Fast Fourier Transform, FFTW 

#define HAMMING_WINDOW		1
#define HANN_WINDOW			2
#define BLACKMAN_WINDOW		3
#define DSB_AM				1
#define SR_AM				2
#define SSB_AM				3
#define RSSB_AM				4
#define M_AM				5

class AudioProcessing {
	std::vector<float> windowFunction;			// window function used when computing FFT
	AudioFile<float>::AudioBuffer hilbert;		// hilbert function of the audio file

public:
	AudioFile<float> audioFile;
	AudioFile<float>::AudioBuffer modulatedSignal;
	AudioFile<float>::AudioBuffer modulatedPhase;

	bool loadFile(std::string fileName);
	bool saveFile(std::string fileName);
	void printSummary() { audioFile.printSummary(); };
	int getNumSapmles() { return audioFile.getNumSamplesPerChannel(); };
	int getNumChannels() { return audioFile.getNumChannels(); };
	float* cloneSamples(int channel);
	void convertSampleRate(int newSampleRate);
	void resizeAudio(int newNumChannels, int newNumSamples);
	void saveAsCSV(std::string fileName);
	void audio1DFFT(fftw_complex *signal, int fftSize, int flag = FFTW_FORWARD);
	void equalizeAudio(int fftSize, int windowFlag = HAMMING_WINDOW);
	void createWindowFunction(int fftSize, int windowFlag);
	void normalizeAudio(float amplitudeMax = 1.0);
	void modulateAmplitude(float m, int modulationFlag = DSB_AM);
	void createAudioSignal(int numWaves, float *amplitudes, float *frequencies, int numSamples, int numChannels = 1, int sampleRate = 40000, int bitDepth = 16);
	void generateMonauralAudio();
	void calculateHilbert();

private:
};
