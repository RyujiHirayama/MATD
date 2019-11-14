#pragma once
#include "../MATDAnimation.h"

class Pyramid : public MATDAnimation {
public:
	Pyramid(MATDHandle& mHandle) : MATDAnimation(mHandle) { ; }

	void createAnimation() {
		DisplayContent dContent;

		float pyramidWidthHalf = 0.01f;
		float pyramidHeight = sqrt(2.0) * pyramidWidthHalf;
		glm::vec3 vA(0.0f, 0.0f, pyramidHeight / 2.0f);
		glm::vec3 vB(-pyramidWidthHalf, -pyramidWidthHalf, -pyramidHeight / 2.0f);
		glm::vec3 vC(pyramidWidthHalf, -pyramidWidthHalf, -pyramidHeight / 2.0f);
		glm::vec3 vD(-pyramidWidthHalf, pyramidWidthHalf, -pyramidHeight / 2.0f);
		glm::vec3 vE(pyramidWidthHalf, pyramidWidthHalf, -pyramidHeight / 2.0f);

		int numLines = 4000;
		dContent.positions.resize(numLines * 10);
		dContent.colours.resize(numLines * 10);
		unsigned int address = 0;

		for (int j = 0; j < 10; j++) {
			for (int i = 0; i < numLines; i++) {
				glm::vec3 p0, p1, c;
				if (j == 0) { p0 = vA; p1 = vC; c = glm::vec3(255, 0, 0); }
				if (j == 1) { p0 = vC; p1 = vB; c = glm::vec3(0, 255, 255); }
				if (j == 2) { p0 = vB; p1 = vD; c = glm::vec3(0, 255, 255); }
				if (j == 3) { p0 = vD; p1 = vA; c = glm::vec3(255, 0, 0); }
				if (j == 4) { p0 = vA; p1 = vE; c = glm::vec3(255, 0, 0); }
				if (j == 5) { p0 = vE; p1 = vD; c = glm::vec3(0, 255, 255); }
				if (j == 6) { p0 = vD; p1 = vC; c = glm::vec3(0, 0, 0); }
				if (j == 7) { p0 = vC; p1 = vE; c = glm::vec3(0, 255, 255); }
				if (j == 8) { p0 = vE; p1 = vB; c = glm::vec3(0, 0, 0); }
				if (j == 9) { p0 = vB; p1 = vA; c = glm::vec3(255, 0, 0); }

				float path = sinf(i * M_PI / (float)(2 * numLines));
				glm::vec3 p = p0 + (p1 - p0) * path;
				dContent.positions[address] = p; 
				dContent.colours[address] = c;
				address++;
			}
		}

		std::cout << "Required exposure time is: ";
		std::cout << 25.0 * dContent.positions.size() * 0.000001 << " s" << std::endl;

		dContent.amplitudes.push_back(1.0f);
		dContent.phases.push_back(0.0f);
		dContent.twinFlags.push_back(true);
		dContents.push_back(dContent);
	};
};