#include "AudioProcessing.h"

bool AudioProcessing::loadFile(std::string fileName) {

	bool error = audioFile.load(fileName);
	if (!error)
		std::cout << "file could not open" << std::endl;
	return error;
}

bool AudioProcessing::saveFile(std::string fileName) {

	bool error = audioFile.save(fileName);
	if (!error)
		std::cout << "file could not open" << std::endl;
	return error;
}

float* AudioProcessing::cloneSamples(int channel) {
	float *signal = new float[audioFile.getNumSamplesPerChannel()];
	for (int i = 0; i < audioFile.getNumSamplesPerChannel(); i++)
		signal[i] = audioFile.samples[channel][i];
	return signal;
}

void AudioProcessing::convertSampleRate(int newSampleRate) {
	int currentSampleRate = audioFile.getSampleRate();
	int currentNumSamples = audioFile.getNumSamplesPerChannel();
	int newNumChannels = audioFile.getNumChannels();
	int newNumSamples = currentNumSamples * ((float)newSampleRate / currentSampleRate);

	AudioFile<float>::AudioBuffer buffer;
	buffer.resize(newNumChannels);
	for (int j = 0; j < newNumChannels; j++)
		buffer[j].resize(newNumSamples);

	for (int j = 0; j < newNumChannels; j++) {
		for (int i = 0; i < newNumSamples; i++) {
			float t = (float)i / newSampleRate;
			float index = t * currentSampleRate;
			int intIndex = (int)index;
			float decIndex = index - intIndex;
			buffer[j][i] = (1 - decIndex) * audioFile.samples[j][intIndex] + decIndex * audioFile.samples[j][intIndex + 1];
		}
	}

	audioFile.setAudioBuffer(buffer);
	audioFile.setSampleRate(newSampleRate);
}

void AudioProcessing::resizeAudio(int newNumChannels, int newNumSamples) {
	AudioFile<float>::AudioBuffer buffer;
	buffer.resize(newNumChannels);
	for (int j = 0; j < newNumChannels; j++)
		buffer[j].resize(newNumSamples);

	for (int j = 0; j < newNumChannels; j++) {
		for (int i = 0; i < newNumSamples; i++) {
			if (i < audioFile.getNumSamplesPerChannel() && j < audioFile.getNumChannels())
				buffer[j][i] = audioFile.samples[j][i];
			else
				buffer[j][i] = 0;
		}
	}

	audioFile.setAudioBuffer(buffer);
}

void AudioProcessing::saveAsCSV(std::string fileName) {
	std::ofstream outFile(fileName);
	for (int j = 0; j < audioFile.getNumSamplesPerChannel(); j++) {
		outFile << j << ", ";
		for (int i = 0; i < audioFile.getNumChannels(); i++) {
			outFile << audioFile.samples[i][j] << ",";
		}
		outFile << std::endl;
	}
	outFile.close();
}

void AudioProcessing::equalizeAudio(int fftSize, int windowFlag) {
	AudioFile<float>::AudioBuffer buffer;
	buffer.resize(audioFile.getNumChannels());
	for (int j = 0; j < audioFile.getNumChannels(); j++)
		buffer[j].resize(audioFile.getNumSamplesPerChannel());
	for (int j = 0; j < audioFile.getNumChannels(); j++)
		for (int i = 0; i < audioFile.getNumSamplesPerChannel(); i++)
			buffer[j][i] = 0.0;

	int frameShift = fftSize / 2;
	this->createWindowFunction(fftSize, windowFlag);
	int numFFTTry = ceil(audioFile.getNumSamplesPerChannel() / (float)frameShift) + 1;
	fftw_complex *fftSignal = new fftw_complex[fftSize];

	for (int k = 0; k < numFFTTry; k++) {
		for (int j = 0; j < audioFile.getNumChannels(); j++) {
			for (int i = 0; i < fftSize; i++) {
				int address = i + (k - 1) * frameShift;
				if (address >= 0 && address < audioFile.getNumSamplesPerChannel())
					fftSignal[i][0] = audioFile.samples[j][address] * windowFunction[i];
				else
					fftSignal[i][0] = 0.0;
				fftSignal[i][1] = 0.0;
			}
			audio1DFFT(fftSignal, fftSize, FFTW_FORWARD);

			for (int i = 0; i < fftSize / 2 + 1; i++) {
				if (i == 0) {
					fftSignal[i][0] = 0;
					fftSignal[i][1] = 0;
				}
				else if (i == fftSize / 2) {
					float f = audioFile.getSampleRate() * i / (float)fftSize;
					fftSignal[i][0] /= (f * f);
					fftSignal[i][1] /= (f * f);
				}
				else {
					float f = audioFile.getSampleRate() * i / (float)fftSize;
					fftSignal[i][0] /= (f * f);
					fftSignal[i][1] /= (f * f);
					fftSignal[fftSize - i][0] /= (f * f);
					fftSignal[fftSize - i][1] /= (f * f);
				}
			}

			audio1DFFT(fftSignal, fftSize, FFTW_BACKWARD);
			for (int i = 0; i < fftSize; i++) {
				int address = i + (k - 1) * frameShift;
				if (address >= 0 && address < audioFile.getNumSamplesPerChannel())
					buffer[j][address] += fftSignal[i][0];
			}
				
		}
	}
	audioFile.setAudioBuffer(buffer);
	delete[] fftSignal;
}

void AudioProcessing::audio1DFFT(fftw_complex *signal, int fftSize, int flag) {
	fftw_complex *tmp = new fftw_complex[fftSize];
	fftw_plan p;
	p = fftw_plan_dft_1d(fftSize, signal, tmp, flag, FFTW_ESTIMATE);
	fftw_execute(p);

	for (int i = 0; i < fftSize; i++) {
		signal[i][0] = tmp[i][0];
		signal[i][1] = tmp[i][1];
	}
	fftw_destroy_plan(p);
	delete[] tmp;
}

void AudioProcessing::createWindowFunction(int fftSize, int windowFlag) {
	windowFunction.resize(fftSize);
	switch (windowFlag) {
	case HAMMING_WINDOW:
		for (int i = 0; i < fftSize; i++)
			windowFunction[i] = 0.54 - 0.46 * cos(2 * M_PI * i / fftSize);
		break;
	case HANN_WINDOW:
		for (int i = 0; i < fftSize; i++)
			windowFunction[i] = 0.5 - 0.5 * cos(2 * M_PI * i / fftSize);
		break;
	case BLACKMAN_WINDOW:
		for (int i = 0; i < fftSize; i++)
			windowFunction[i] = 0.42 - 0.5 * cos(2 * M_PI * i / fftSize) + 0.08 * cos(4 * M_PI * i / fftSize);
		break;
	default:
		std::cout << "this window flag is not valid" << std::endl;
		break;
	}
}

void AudioProcessing::normalizeAudio(float amplitudeMax) {
	float absMax = 0.0;
	for (int j = 0; j < audioFile.getNumChannels(); j++) {
		for (int i = 0; i < audioFile.getNumSamplesPerChannel(); i++) {
			float absSignal = abs(audioFile.samples[j][i]);
			if (absMax < absSignal) absMax = absSignal;
		}
	}
	float normValue = (absMax != 0 ? amplitudeMax / absMax : 0);
	for (int j = 0; j < audioFile.getNumChannels(); j++)
		for (int i = 0; i < audioFile.getNumSamplesPerChannel(); i++)
			audioFile.samples[j][i] *= normValue;
}

void AudioProcessing::modulateAmplitude(float m, int modulationFlag) {
	modulatedSignal.resize(audioFile.getNumChannels());
	modulatedPhase.resize(audioFile.getNumChannels());
	for (int i = 0; i < audioFile.getNumChannels(); i++) {
		modulatedSignal[i].resize(audioFile.getNumSamplesPerChannel());
		modulatedPhase[i].resize(audioFile.getNumSamplesPerChannel());
	}

	switch (modulationFlag) {
	case DSB_AM:
		for (int j = 0; j < audioFile.getNumChannels(); j++) {
			for (int i = 0; i < audioFile.getNumSamplesPerChannel(); i++) {
				modulatedSignal[j][i] = (1.0 + m * audioFile.samples[j][i]);
				modulatedPhase[j][i] = 0.0;
			}
		}
		break;
	case SR_AM:
		for (int j = 0; j < audioFile.getNumChannels(); j++) {
			for (int i = 0; i < audioFile.getNumSamplesPerChannel(); i++) {
				modulatedSignal[j][i] = sqrt(1.0 + m * audioFile.samples[j][i]);
				modulatedPhase[j][i] = 0.0;
			}
		}
		break;
	case SSB_AM:
		calculateHilbert();
		for (int j = 0; j < audioFile.getNumChannels(); j++) {
			for (int i = 0; i < audioFile.getNumSamplesPerChannel(); i++) {
				//float a = 1.0 - m * hilbert[j][i];
				//float b = m * audioFile.samples[j][i];
				float a = m * hilbert[j][i];
				float b = 1.0 + m * audioFile.samples[j][i];
				modulatedSignal[j][i] = sqrt(a * a + b * b);
				modulatedPhase[j][i] = atan2(b, a);
			}
		}
		break;
	case RSSB_AM:
		break;
	case M_AM:
		break;
	default:
		std::cout << "this modulation flag is not valid..." << std::endl;
		break;
	}

	float max = 0.0f;
	for (int j = 0; j < audioFile.getNumChannels(); j++)
		for (int i = 0; i < audioFile.getNumSamplesPerChannel(); i++)
			if (max < modulatedSignal[j][i])
				max = modulatedSignal[j][i];

	for (int j = 0; j < audioFile.getNumChannels(); j++)
		for (int i = 0; i < audioFile.getNumSamplesPerChannel(); i++)
			modulatedSignal[j][i] /= max;
}

void AudioProcessing::createAudioSignal(int numWaves, float *amplitudes, float *frequencies, int numSamples, int numChannels, int sampleRate, int bitDepth) {
	AudioFile<float>::AudioBuffer buffer;
	buffer.resize(numChannels);
	for (int i = 0; i < numChannels; i++)
		buffer[i].resize(numSamples);
	
	for (int k = 0; k < numChannels; k++) {
		for (int j = 0; j < numSamples; j++) {
			float sample = 0.0f;
			for (int i = 0; i < numWaves; i++)
				sample += amplitudes[i] * sin(2.0 * M_PI * ((float)j / sampleRate) * frequencies[i]);

			buffer[k][j] = sample;
		}
	}
	audioFile.setAudioBuffer(buffer);
	audioFile.setSampleRate(sampleRate);
	audioFile.setBitDepth(bitDepth);
}

void AudioProcessing::generateMonauralAudio() {
	int sampleRate = 40000;
	int bitDepth = 16;
	int numChannels = 1;

	float signalFrequency = 261.626;
	int numTotalSamples = 4000;

	AudioFile<float>::AudioBuffer buffer;
	buffer.resize(numChannels);
	for (int i = 0; i < numChannels; i++)
		buffer[i].resize(numTotalSamples);

	for (int j = 0; j < numChannels; j++) {
		for (int i = 0; i < numTotalSamples; i++) {
			buffer[j][i] = sin(2.0 * M_PI * ((float)i / sampleRate) * signalFrequency);
		}
	}

	audioFile.setAudioBuffer(buffer);
	audioFile.setSampleRate(sampleRate);
	audioFile.setBitDepth(bitDepth);
}

void AudioProcessing::calculateHilbert() {
	hilbert.resize(audioFile.getNumChannels());
	for (int i = 0; i < audioFile.getNumChannels(); i++)
		hilbert[i].resize(audioFile.getNumSamplesPerChannel());

	int N = audioFile.getNumSamplesPerChannel();
	fftw_complex *y;
	y = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * N);

	int windowLength = 200;
	for (int j = 0; j < audioFile.getNumChannels(); j++) {
		for (int i = 0; i < N; i++) {
			float w = 1.0;
			if (i < windowLength) w = 0.5 - 0.5 * cos(i * M_PI / windowLength);
			if (i >= N - windowLength) w = 0.5 - 0.5 * cos((i - N + 2*windowLength) * M_PI / windowLength);
			y[i][0] = audioFile.samples[j][i] * w;
			y[i][1] = 0;
		}

		fftw_plan plan = fftw_plan_dft_1d(N, y, y, FFTW_FORWARD, FFTW_ESTIMATE);
		fftw_execute(plan);
		fftw_destroy_plan(plan);
		int hN = N >> 1;
		int numRem = hN;

		for (int i = 1; i < hN; i++) {
			y[i][0] *= 2;
			y[i][1] *= 2;
		}

		if (N % 2 == 0)
			numRem--;
		else if (N > 1) {
			y[hN][0] *= 2;
			y[hN][1] *= 2;
		}

		memset(&y[hN + 1][0], 0, numRem * sizeof(fftw_complex));
		plan = fftw_plan_dft_1d(N, y, y, FFTW_BACKWARD, FFTW_ESTIMATE);
		fftw_execute(plan);
		fftw_destroy_plan(plan);
		fftw_cleanup();

		for (int i = 0; i < N; i++) {
			hilbert[j][i] = y[i][1] / N;
		}
	}
	fftw_free(y);
}
