#pragma once
#include "../MATDAnimation.h"

class Butterfly : public MATDAnimation {
public:
	Butterfly(MATDHandle& mHandle) : MATDAnimation(mHandle) { ; }

	void createAnimation() {
		DisplayContent dContent;

		int numSamples = 4000;
		float radius = 0.007f;
		float radiusXYMove = 0.015f;
		float radiusZMove = 0.003f;
		int numXYMoves = 400;
		int numZMoves = 100;
		float wingAngle = 0.6;
		int numWing = 10;

		dContent.positions.resize(numSamples * numXYMoves);
		dContent.colours.resize(numSamples);

		// first just create an animation of character of 8 and colour of it
		std::vector<glm::vec3> eightAnimation;
		eightAnimation.resize(numSamples);
		int numSamplesHalf = numSamples / 2;
		unsigned int address = 0;
		for (int s = 0; s < numSamplesHalf; s++) {
			eightAnimation[address].x = radius * (1.0f - cos(s * 2.0 * M_PI / numSamplesHalf));
			eightAnimation[address].y = radius * sin(s * 2.0 * M_PI / numSamplesHalf);
			eightAnimation[address].z = 0.0f;

			dContent.colours[address] = HSV2RGB(s * 360.0f / numSamplesHalf);
			address++;
		}
		for (int s = 0; s < numSamplesHalf; s++) {
			eightAnimation[address].x = -radius * (1.0f - cos(s * 2.0 * M_PI / numSamplesHalf));
			eightAnimation[address].y = radius * sin(s * 2.0 * M_PI / numSamplesHalf);
			eightAnimation[address].z = 0.0f;

			dContent.colours[numSamplesHalf + s] = HSV2RGB(s * 360.0f / numSamplesHalf);
			address++;
		}

		// then by moving and rotating 8, butterfly can be created
		glm::vec3 butterfly;
		for (int m = 0; m < numXYMoves; m++) {
			// two circles of 8 represent wings of butterfly by flapping
			float pR, pL;
			if ((m / numWing) % 4 == 0) pR = (m % numWing) / (float)numWing;
			else if ((m / numWing) % 4 == 1) pR = 1.0 - (m % numWing) / (float)numWing;
			else if ((m / numWing) % 4 == 2) pR = -(m % numWing) / (float)numWing;
			else pR = -1.0 + (m % numWing) / (float)numWing;
			pR *= wingAngle * 0.5 * M_PI;
			pL = -pR;
			glm::mat3 wingR(cos(pR), 0, -sin(pR), 0, 1, 0, sin(pR), 0, cos(pR));
			glm::mat3 wingL(cos(pL), 0, -sin(pL), 0, 1, 0, sin(pL), 0, cos(pL));

			// the butterfly move in a circle, and up and down
			glm::vec3 movement;
			movement.x = radiusXYMove * cos((m % numXYMoves) * 2.0 * M_PI / numXYMoves);
			movement.y = radiusXYMove * sin((m % numXYMoves) * 2.0 * M_PI / numXYMoves);
			movement.z = radiusZMove * sin(m * 2.0 * M_PI / numZMoves);

			// the butterfly needs to be rotated to toward where it goes
			float p = -(m % numXYMoves) * 2.0 * M_PI / numXYMoves;
			glm::mat3 rotation(cos(p), -sin(p), 0, sin(p), cos(p), 0, 0, 0, 1);

			for (int s = 0; s < numSamples; s++) {
				if (s < numSamplesHalf) butterfly = wingR * eightAnimation[s];
				else butterfly = wingL * eightAnimation[s];
				dContent.positions[m * numSamples + s] = rotation * butterfly + movement;
			}
		}

		dContent.amplitudes.push_back(1.0f);
		dContent.phases.push_back(0.0f);
		dContent.twinFlags.push_back(true);

		dContents.push_back(dContent);
	};
};