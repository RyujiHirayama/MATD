#pragma once
#include "../MATDAnimation.h"

class CurvatureTest : public MATDAnimation {
public:
	CurvatureTest(MATDHandle& mHandle) : MATDAnimation(mHandle) { ; }

	DisplayContent createCurvatureTest() {
		DisplayContent dContent;

		int mode = 0; // 0: OSTm (A=1.0), 1: PSTm (A=0.8), 2: PDTm (A=0.8,80%duty), 3: ODTm (A=1.0,50%duty), 4: Other (A=0.5,100%duty)
		bool vertical = false;
		int numLev = 3;
		int numTac = 1; // 1 : 1 => 50 %, 4 : 1 => 80 %
		float audioAmplitude = 1.0f / (1.0f + 0.2f);
		float radius = 0.03;
		float vmax = 1.0f; // m/s
		float acc = 0.0001f; // m/s increase per 40 kHz update

		float wmax = 0.000025 * vmax / radius; // maximum angular frequency per 40 kHz update 
		float v = 0.0f; // m / s
		float radian = 0.0;
		float startRadian = 0.0;
		int rotationCount = 0;
		int numRotations = 5;
		glm::vec3 tacPosition;
		if (radius < 0.03f) tacPosition = glm::vec3(0.06, 0.0f, 0.0f);
		else tacPosition = glm::vec3(0.0f, 0.0f, 0.0f);

		// acceration part
		while (v < vmax) {
			if (!vertical)
				dContent.positions.push_back(glm::vec3(radius * sin(radian), radius * cos(radian), 0.0f));
			else
				dContent.positions.push_back(glm::vec3(0.0f, radius * cos(radian), radius * sin(radian)));
			float w = 0.000025 * v / radius;
			radian += w;
			if (radian > 2.0 * M_PI)
				radian -= 2.0 * M_PI;
			v += acc;
			startRadian = radian;
		}

		// constant part
		while (rotationCount < numRotations) {
			if (!vertical)
				dContent.positions.push_back(glm::vec3(radius * sin(radian), radius * cos(radian), 0.0f));
			else
				dContent.positions.push_back(glm::vec3(0.0f, radius * cos(radian), radius * sin(radian)));
			radian += wmax;
			if (radian > 2.0 * M_PI) {
				radian -= 2.0 * M_PI;
				rotationCount++;
			}
		}

		// deceration part
		while (v > 0) {
			if (!vertical)
				dContent.positions.push_back(glm::vec3(radius * sin(radian), radius * cos(radian), 0.0f));
			else
				dContent.positions.push_back(glm::vec3(0.0f, radius * cos(radian), radius * sin(radian)));
			float w = 0.000025 * v / radius;
			radian += w;
			if (radian > 2.0 * M_PI)
				radian -= 2.0 * M_PI;
			v -= acc;
		}

		// go back to the first position part
		for (int i = 0; i < 500; i++) {
			if (!vertical)
				dContent.positions.push_back(glm::vec3(radius * sin(radian), radius * cos(radian), 0.0f));
			else
				dContent.positions.push_back(glm::vec3(0.0f, radius * cos(radian), radius * sin(radian)));
		}
		bool atStart = false;
		v = 0.2;
		while (!atStart) {
			if (!vertical)
				dContent.positions.push_back(glm::vec3(radius * sin(radian), radius * cos(radian), 0.0f));
			else
				dContent.positions.push_back(glm::vec3(0.0f, radius * cos(radian), radius * sin(radian)));
			float w = 0.000025 * v / radius;
			radian += w;
			if (radian > 2.0 * M_PI) {
				radian = 0.0;
				atStart = true;
			}
		}

		float levAmplitude = (mode == 0 ? 1.0f : audioAmplitude);
		float tacAmplitude = levAmplitude;
		for (int i = 0; i < dContent.positions.size(); i++) {
			if (i % (numLev + numTac) < numLev) {
				dContent.amplitudes.push_back(levAmplitude);
			}
			else {
				if (mode == 2) {
					dContent.positions[i] = tacPosition;
					dContent.amplitudes.push_back(tacAmplitude);
				}
				else {
					dContent.amplitudes.push_back(levAmplitude);
				}
			}
		}
		
		int numFramesPerUpdate = 20;
		int rest = numFramesPerUpdate - dContent.positions.size() % numFramesPerUpdate;
		for (int i = 0; i < rest; i++) {
			dContent.positions.push_back(glm::vec3(0.0f, radius, 0.0f));
			dContent.amplitudes.push_back(levAmplitude);
		}
		
		dContent.colours.push_back(glm::vec3(0, 0, 0));
		dContent.phases.push_back(0.0f);
		dContent.twinFlags.push_back(true);

		return dContent;
	}

	DisplayContent createStop(glm::vec3 stopPosition) {
		DisplayContent dContent;

		dContent.positions.push_back(stopPosition);
		dContent.amplitudes.push_back(1.0f);
		dContent.phases.push_back(0.0f);
		dContent.colours.push_back(glm::vec3(0, 0, 0));
		dContent.twinFlags.push_back(true);

		return dContent;
	}

	void createAnimation() {
		dContents.push_back(createCurvatureTest());
		dContents.push_back(createStop(dContents[0].positions[0]));
	};

	void runAnimation() {
		static glm::vec3 posBuffer[250], colourBuffer[250];
		static float ampBuffer[250], phaseBuffer[250];
		static bool twinBuffer[250];

		static glm::vec3 targetPosition, realPosition, currentPosition;
		static float stepSize = 0.0001f;

		int numSuccesses = 0;
		int numTotalTrials = 0;

		int numFramesPerUpdate = 20;
		StateControl s(25 * numFramesPerUpdate);
		while (s.isWorking()) {
			if (s.isReady()) {
				dContents[contentNumber].fillBuffers(numFramesPerUpdate, posBuffer, ampBuffer, phaseBuffer, colourBuffer, twinBuffer);
				mHandle.updateMultipleFrames(numFramesPerUpdate, posBuffer, ampBuffer, phaseBuffer, colourBuffer, twinBuffer);

				if (dContents[contentNumber].isChangeTiming() && dContents[contentNumber].updateFinished()) {
					dContents[contentNumber].resetAddressAndFlag();
					if (contentNumber == dContents.size() - 1) {
						if (s.anyInput()) {
							if (s.checkSuccess()) numSuccesses++;
							numTotalTrials++;
							std::cout << numSuccesses << " / " << numTotalTrials << std::endl;
							contentNumber = 0;
						}
					}
					else contentNumber++;
				}

				s.countTimer();
			}
		}
	};
};