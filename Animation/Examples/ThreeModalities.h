#pragma once
#include "../MATDAnimation.h"

class ThreeModalities : public MATDAnimation {
public:
	ThreeModalities(MATDHandle& mHandle) : MATDAnimation(mHandle) { ; }

	DisplayContent createNumber(int number) {
		DisplayContent dNumber;

		int numSamples = 4000;
		float radius = 0.003f;

		dNumber.positions.resize(numSamples);

		int numSamplesHalf = numSamples / 2;
		int numSamplesQuater = numSamples / 4;
		unsigned int address = 0;
		for (int s = 0; s < numSamplesQuater; s++) {
			dNumber.positions[address].x = 0.0f;
			dNumber.positions[address].y = radius * cos(s * 2.0 * M_PI / numSamplesHalf + M_PI * 0.5);
			dNumber.positions[address].z = radius * (1.0f + sin(s * 2.0 * M_PI / numSamplesHalf + M_PI * 0.5));
			address++;
		}
		for (int s = 0; s < numSamplesHalf; s++) {
			dNumber.positions[address].x = 0.0f;
			dNumber.positions[address].y = radius * sin(s * 2.0 * M_PI / numSamplesHalf);
			dNumber.positions[address].z = radius * (-1.0f + cos(s * 2.0 * M_PI / numSamplesHalf));
			address++;
		}
		for (int s = 0; s < numSamplesQuater; s++) {
			dNumber.positions[address].x = 0.0f;
			dNumber.positions[address].y = radius * cos(s * 2.0 * M_PI / numSamplesHalf + M_PI * 1.5);
			dNumber.positions[address].z = radius * (1.0f + sin(s * 2.0 * M_PI / numSamplesHalf + M_PI * 1.5));
			address++;
		}


		dNumber.colours.resize(numSamples);

		std::vector<int> segmentEnable;
		glm::vec3 colourOfNumber;
		switch (number) {
		case 0: segmentEnable = std::vector<int>{ 0, 1, 1, 1, 1, 1, 1 }; colourOfNumber = glm::vec3(255, 0, 0); break;
		case 1: segmentEnable = std::vector<int>{ 0, 0, 0, 1, 0, 0, 1 }; colourOfNumber = glm::vec3(255, 255, 0); break;
		case 2: segmentEnable = std::vector<int>{ 1, 0, 1, 1, 1, 1, 0 }; colourOfNumber = glm::vec3(0, 255, 0); break;
		case 3: segmentEnable = std::vector<int>{ 1, 0, 1, 1, 0, 1, 1 }; colourOfNumber = glm::vec3(0, 255, 255); break;
		case 4: segmentEnable = std::vector<int>{ 1, 1, 0, 1, 0, 0, 1 }; colourOfNumber = glm::vec3(0, 0, 255); break;
		case 5: segmentEnable = std::vector<int>{ 1, 1, 1, 0, 0, 1, 1 }; colourOfNumber = glm::vec3(255, 0, 255); break;
		case 6: segmentEnable = std::vector<int>{ 1, 1, 1, 0, 1, 1, 1 }; colourOfNumber = glm::vec3(255, 0, 0); break;
		case 7: segmentEnable = std::vector<int>{ 0, 0, 1, 1, 0, 0, 1 }; colourOfNumber = glm::vec3(255, 255, 0); break;
		case 8: segmentEnable = std::vector<int>{ 1, 1, 1, 1, 1, 1, 1 }; colourOfNumber = glm::vec3(0, 255, 0); break;
		case 9: segmentEnable = std::vector<int>{ 1, 1, 1, 1, 0, 1, 1 }; colourOfNumber = glm::vec3(0, 255, 255); break;
		default: segmentEnable = std::vector<int>{ 0, 1, 0, 1, 1, 0, 1 }; colourOfNumber = glm::vec3(255, 255, 255); break;	// pause mark like ||
		}

		int numSamplesOfSegment = numSamples / 16;
		for (int s = 0; s < numSamples; s++) {
			int segment;
			int place = s / numSamplesOfSegment;
			if (place == 3 || place == 4 || place == 11 || place == 12) segment = 0;
			else if (place == 13 || place == 14) segment = 3;
			else if (place == 0 || place == 15) segment = 2;
			else if (place == 1 || place == 2) segment = 1;
			else if (place == 5 || place == 6) segment = 6;
			else if (place == 7 || place == 8) segment = 5;
			else if (place == 9 || place == 10) segment = 4;

			int enSegment = 1;
			if (place % 2 == 0 && s % numSamplesOfSegment >= numSamplesOfSegment / 2 + 20) enSegment = 0;
			if (place % 2 == 1 && s  % numSamplesOfSegment < numSamplesOfSegment / 2 - 20) enSegment = 0;
			dNumber.colours[s] = (float)(segmentEnable[segment] * enSegment) * colourOfNumber;
		}

		float m = 0.2f;
		AudioProcessing audio;
		std::stringstream fileName;
		fileName << "media/num" << number << ".wav";
		audio.loadFile(fileName.str());
		audio.normalizeAudio(1.0);
		audio.modulateAmplitude(m, DSB_AM);
		dNumber.amplitudes.resize(audio.getNumSapmles());
		for (int s = 0; s < audio.getNumSapmles(); s++)
			dNumber.amplitudes[s] = audio.modulatedSignal[0][s];


		dNumber.phases.push_back(0.0f);
		dNumber.twinFlags.push_back(true);

		return dNumber;
	}

	void createAnimation() {
		for (int i = 0; i < 10; i++)
			dContents.push_back(createNumber(9 - i));

		glm::vec3 tactilePosition(-0.06f, 0.06f, 0.0f);
		int tactileFrequency = 200;
		for (int i = 0; i < dContents.size(); i++)
			multiplexMovingTactile(dContents[i], tactilePosition, tactileFrequency, 3, 1);
	};

	void runAnimation() {
		static glm::vec3 posBuffer[250], colourBuffer[250];
		static float ampBuffer[250], phaseBuffer[250];
		static bool twinBuffer[250];

		static glm::vec3 targetPosition, realPosition, currentPosition;
		static float stepSize = 0.0001f;

		int numFramesPerUpdate = 200;
		StateControl s(25 * numFramesPerUpdate);
		while (s.isWorking()) {
			if (s.isReady()) {
				dContents[contentNumber].fillBuffers(numFramesPerUpdate, posBuffer, ampBuffer, phaseBuffer, colourBuffer, twinBuffer);
				mHandle.updateMultipleFrames(numFramesPerUpdate, posBuffer, ampBuffer, phaseBuffer, colourBuffer, twinBuffer);

				if (dContents[contentNumber].isChangeTiming() && dContents[contentNumber].updateFinished()) {
					dContents[contentNumber].resetAddressAndFlag();
					if (contentNumber < dContents.size() - 1) contentNumber++;
				}

				s.countTimer();
			}
		}
	}
};