#pragma once
#include "../MATDAnimation.h"

class OscillationDrop : public MATDAnimation {
public:
	OscillationDrop(MATDHandle& mHandle) : MATDAnimation(mHandle) { ; }

	void createAnimation() {
		DisplayContent dContent;

		float frequency = 400.0;
		int numSamples = roundf(40000.0f / frequency);
		float m = 0.2;
		std::vector<float> memAmplitudes;
		for (int i = 0; i < numSamples; i++) {
			float amplitude = (1.0f + m * sin(i * 2.0 * M_PI / numSamples)) / (1.0f + m);
			dContent.amplitudes.push_back(amplitude);
		}

		dContent.positions.push_back(glm::vec3(0, 0, 0));
		dContent.phases.push_back(0.0f);
		dContent.colours.push_back(glm::vec3(0, 0, 0));
		dContent.twinFlags.push_back(true);

		dContents.push_back(dContent);
	};
};