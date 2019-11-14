#pragma once
#include "../MATDAnimation.h"

class SpeedTest : public MATDAnimation {
public:
	SpeedTest(MATDHandle& mHandle) : MATDAnimation(mHandle) { ; }

	DisplayContent createCurvatureTest() {
		DisplayContent dContent;

		bool initPosition = false;
		int testMode = 0; // 0: horizontal, 1: vertical (upward), 2: vertical (downward)
		int mode = 0; //0: OSTm (A=1.0), 1: PSTm (A=0.8), 2: PDTm (A=0.8,80%duty)

		int numLev = 3;
		int numTac = 1;

		float m = 0.2;
		float audioAmplitude = 1.0f / (1.0f + m);
		float radius = 0.05f;

		int sameFrame = 1;	// 40,000Hz -> 1
							// 20,000Hz -> 2
							// 10,000Hz -> 4
							//  5,000Hz -> 8
							//  2,500Hz -> 16
							//  1,250Hz -> 32
							//    625Hz -> 64
							//    312.5hz -> 128

		float vmax = 3.0f;


		float aconstant = 0.000025 * vmax * vmax / (radius * 2.0);
		float vset = 1.0f * vmax; // 1.0 Vmax, 0.8 Vmax or 0.5 Vmax
		float amax = 1.0f * aconstant; // from 1.0 amax to 2.0 amax
		float T = 2.0 * radius / vmax;

		glm::vec3 tacPosition(-0.06, -0.06, 0.0f);
		std::vector<float> memVelocities, memAccerelation;
		int sAddr, mAddr, eAddr;

		float v = 0.0f; // m / s
		float pos = -radius;
		// stay at the home position for a while
		int stopTime = 20000;
		for (int i = 0; i < stopTime; i++) {
			glm::vec3 position;
			if (testMode == 0) position.y += pos;
			if (testMode == 1) position.z += pos;
			if (testMode == 2) position.z -= pos;
			dContent.positions.push_back(position);
			memVelocities.push_back(0);
			memAccerelation.push_back(0);
		}
		sAddr = dContent.positions.size();

		// acceration part
		while (v < vset) {
			glm::vec3 position;
			if (testMode == 0) position.y += pos;
			if (testMode == 1) position.z += pos;
			if (testMode == 2) position.z -= pos;
			dContent.positions.push_back(position);
			memVelocities.push_back(v);
			memAccerelation.push_back(amax);
			pos += 0.000025 * v;
			v += amax;
		}
		v = vset;
		float constantPosition = pos;
		// constant part
		while (pos < -constantPosition) {
			glm::vec3 position;
			if (testMode == 0) position.y += pos;
			if (testMode == 1) position.z += pos;
			if (testMode == 2) position.z -= pos;
			dContent.positions.push_back(position);
			memVelocities.push_back(v);
			memAccerelation.push_back(0);
			pos += 0.000025 * v;
		}
		// deceration part
		while (v > 0) {
			glm::vec3 position;
			if (testMode == 0) position.y += pos;
			if (testMode == 1) position.z += pos;
			if (testMode == 2) position.z -= pos;
			dContent.positions.push_back(position);
			memVelocities.push_back(v);
			memAccerelation.push_back(-amax);
			pos += 0.000025 * v;
			v -= amax;
		}
		eAddr = dContent.positions.size() - 1;
		mAddr = (eAddr - stopTime) / 2 + stopTime;

		// return to home position part
		for (int i = 0; i < stopTime; i++) {
			glm::vec3 position;
			if (testMode == 0) position.y += pos;
			if (testMode == 1) position.z += pos;
			if (testMode == 2) position.z -= pos;
			dContent.positions.push_back(position);
		}
		v = 0.1;
		while (pos > -radius) {
			glm::vec3 position;
			if (testMode == 0) position.y += pos;
			if (testMode == 1) position.z += pos;
			if (testMode == 2) position.z -= pos;
			dContent.positions.push_back(position);
			pos += -0.000025 * v;
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
			dContent.colours.push_back(glm::vec3(0, 0, 0));
		}
		int numFramesPerUpdate = 20;
		int rest = numFramesPerUpdate - dContent.positions.size() % numFramesPerUpdate;
		for (int i = 0; i < rest; i++) {
			dContent.positions.push_back(dContent.positions[0]);
			dContent.amplitudes.push_back(levAmplitude);
			dContent.colours.push_back(glm::vec3(0, 0, 0));
		}

		int lPeriod = 4;
		int midPeriod = 3;
		int diff = 40;
		int tMinimum = sAddr + diff * 0;

		for (int j = sAddr; j < eAddr; j += diff) {
			for (int i = 0; i < lPeriod; i++) {
				int address = j + i;
				if (address <= eAddr && address >= tMinimum)
					dContent.colours[address] = glm::vec3(255, 255, 255);
			}
		}
		dContent.colours[mAddr] = glm::vec3(0, 255, 0);
		for (int i = 0; i < midPeriod; i++) {
			dContent.colours[mAddr + i] = glm::vec3(0, 255, 255);
			dContent.colours[mAddr - i] = glm::vec3(0, 255, 255);
		}

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