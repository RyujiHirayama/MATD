#pragma once
#include "../MATDAnimation.h"

class TactileVisualization : public MATDAnimation {
public:
	TactileVisualization(MATDHandle& mHandle) : MATDAnimation(mHandle) { ; }

	DisplayContent create8(glm::vec3 levitationPosition) {
		DisplayContent dNumber;

		int numSamples = 4000;
		float radius = 0.004f;

		dNumber.positions.resize(numSamples);
		dNumber.colours.resize(numSamples);

		int numSamplesHalf = numSamples / 2;
		unsigned int address = 0;
		for (int s = 0; s < numSamplesHalf; s++) {
			dNumber.positions[address].x = 0.f;
			dNumber.positions[address].y = radius * sin(s * 2.0 * M_PI / numSamplesHalf);
			dNumber.positions[address].z = radius * (1.0f - cos(s * 2.0 * M_PI / numSamplesHalf));
			dNumber.positions[address] += levitationPosition;

			dNumber.colours[address] = HSV2RGB(s * 360.0f / numSamplesHalf);
			address++;
		}
		for (int s = 0; s < numSamplesHalf; s++) {
			dNumber.positions[address].x = 0.f;
			dNumber.positions[address].y = radius * sin(s * 2.0 * M_PI / numSamplesHalf);
			dNumber.positions[address].z = -radius * (1.0f - cos(s * 2.0 * M_PI / numSamplesHalf));
			dNumber.positions[address] += levitationPosition;

			dNumber.colours[numSamplesHalf + s] = HSV2RGB(s * 360.0f / numSamplesHalf);
			address++;
		}


		dNumber.amplitudes.push_back(1.0f);
		dNumber.phases.push_back(0.0f);
		dNumber.twinFlags.push_back(true);

		return dNumber;
	}

	void createAnimation() {
		dContents.push_back(create8(glm::vec3(0,-0.0f,0)));

		glm::vec3 tactilePosition(-0.05f, -0.05f, 0);
		int tactileFrequency = 10;
		multiplexMovingTactile(dContents[0], tactilePosition, tactileFrequency, 1, 1);

		dContents.push_back(create8(glm::vec3(0, 0.0f, 0)));
		tactilePosition = glm::vec3(0.05, 0.05f, 0);
		multiplexMovingTactile(dContents[1], tactilePosition, tactileFrequency, 1, 1);
	};


	void runAnimation() {
		static glm::vec3 posBuffer[250], colourBuffer[250];
		static float ampBuffer[250], phaseBuffer[250];
		static bool twinBuffer[250];

		static glm::vec3 targetPosition, realPosition, currentPosition;
		static float stepSize = 0.0001f;

		int count = 0;
		int numFramesPerUpdate = 200;
		StateControl s(25 * numFramesPerUpdate);
		while (s.isWorking()) {
			if (s.isReady()) {
				dContents[contentNumber].fillBuffers(numFramesPerUpdate, posBuffer, ampBuffer, phaseBuffer, colourBuffer, twinBuffer);
				mHandle.updateMultipleFrames(numFramesPerUpdate, posBuffer, ampBuffer, phaseBuffer, colourBuffer, twinBuffer);

				if (dContents[contentNumber].isChangeTiming() && dContents[contentNumber].updateFinished()) {
					dContents[contentNumber].resetAddressAndFlag();
					if (count == 9) {
						count = 0;
						if (contentNumber == dContents.size() - 1) contentNumber = 0;
						else contentNumber++;
					}
					else count++;
				}

				s.countTimer();
			}
		}
	}
};