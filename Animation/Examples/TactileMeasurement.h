#pragma once
#include "../MATDAnimation.h"

class TactileMeasurement : public MATDAnimation {
public:
	TactileMeasurement(MATDHandle& mHandle) : MATDAnimation(mHandle) { ; }

	void createAnimation() {
		DisplayContent dContent;

		int measurementMode = 0; // 0: tactile only, 1: tactile + visual, 2: tactile + visual + audio

		float radius = 0.003f;
		int numSamples = 2000;
		glm::vec3 levitationPoint(-0.045f, 0.045f,0.0f);

		dContent.positions.resize(numSamples * 20);
		dContent.colours.resize(numSamples * 20);
		unsigned int address = 0;
		for (int j = 0; j < 10; j++) {
			for (int i = 0; i < numSamples; i++) {
				dContent.positions[address].x = levitationPoint.x + radius * sin(i * 2.0 * M_PI / numSamples);
				dContent.positions[address].y = levitationPoint.y + radius * (1.0f - cos(i * 2.0 * M_PI / numSamples));
				dContent.positions[address].z = levitationPoint.z;
				if (j < 5) dContent.colours[address] = glm::vec3(255, 0, 0);
				else dContent.colours[address] = glm::vec3(0, 255, 0);
				address++;
			}
			for (int i = 0; i < numSamples; i++) {
				dContent.positions[address].x = levitationPoint.x + radius * sin(i * 2.0 * M_PI / numSamples);
				dContent.positions[address].y = levitationPoint.y - radius * (1.0f - cos(i * 2.0 * M_PI / numSamples));
				dContent.positions[address].z = levitationPoint.z;
				if (j < 5) dContent.colours[address] = glm::vec3(255, 0, 0);
				else dContent.colours[address] = glm::vec3(0, 255, 0);
				address++;
			}
		}

		glm::vec3 tactilePoint(0.0f, 0.0f, 0);
		int numTotalSec = 40000; // 1 sec
		int numTactileSamples = numTotalSec / 250; // 250 Hz tactile
		int numAudioSamples = numTotalSec / 2000; // 2000 Hz audio
		int numLevitation = 3;
		int numTactile = 1;
		float amplitudeMax = 1.0f;
		float m = 0.2;

		dContent.amplitudes.resize(dContent.positions.size());
		for (int i = 0; i < dContent.positions.size(); i++) {
			float gt;
			if (measurementMode == 0 || measurementMode == 1)
				gt = sin(i * 2.0 * M_PI / numTactileSamples);
			else
				gt = 0.5f * (sin(i * 2.0 * M_PI / numTactileSamples) + sin(i * 2.0 * M_PI / numAudioSamples));
			dContent.amplitudes[i] = amplitudeMax * (1.0 + m * gt) / (1.0 + m);

			if (i % (numLevitation + numTactile) >= numLevitation)
				dContent.positions[i] = tactilePoint;
			else if (measurementMode == 0)
				dContent.amplitudes[i] = 0.0f;	// for first tactile only test
		}

		dContent.phases.push_back(0.0f);
		dContent.twinFlags.push_back(true);

		dContents.push_back(dContent);
	};
};