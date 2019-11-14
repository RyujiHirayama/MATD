#pragma once
#include "../MATDAnimation.h"

class TorusKnot : public MATDAnimation {
public:
	TorusKnot(MATDHandle& mHandle) : MATDAnimation(mHandle) { ; }

	void createAnimation() {
		DisplayContent dContent;

		int numSamples = 4000;
		float radius = 0.005f;
		float zscale = 1.0f;

		dContent.positions.resize(numSamples);
		dContent.colours.resize(numSamples);

		for (int s = 0; s < numSamples; s++) {
			float t = s * 2.0 * M_PI / numSamples;
			dContent.positions[s].x = -radius * zscale * sin(3 * t);
			dContent.positions[s].y = radius * (sin(t) - 2 * sin(2 * t));
			dContent.positions[s].z = radius * (cos(t) + 2 * cos(2 * t));

			dContent.colours[s] = HSV2RGB(s * 360.0f / numSamples);
		}

		dContent.amplitudes.push_back(1.0f);
		dContent.phases.push_back(0.0f);
		dContent.twinFlags.push_back(true);

		dContents.push_back(dContent);
	};
};