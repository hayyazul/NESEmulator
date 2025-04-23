// frameCounter.h Basic frame counting class.
#pragma once

#include <chrono>
#include <queue>

constexpr unsigned int MAX_FRAME_LOOKBACK_ALLOWED = 1000;  // This is to prevent the vector of times_recorded from growing obscenely big.

class FrameCounter {
public:
	// The number of frames to look back to calculate the framerate. Default is 60.
	FrameCounter();
	FrameCounter(const unsigned int frame_lookback);
	~FrameCounter();

	// Calculates the framerate given the current time and the number of frames elapsed. If fewer than frame_lookback frames have elapsed, then
	// the number of frames which have elapsed is used. If no frames have elapsed, then this returns 0.
	double getFrameRate() const;
	
	std::chrono::milliseconds getMSPerFrame() const;  // Gets how many milliseconds on average (of the last frame_lookback frames) each frame lasts.

	void countFrame();  // Counts the current frame.	
private:
	const int frame_lookback;  // How many frames should be measured to determine the fps (also the number of frames ago the last time measurement was made).
	std::queue<std::chrono::time_point<std::chrono::steady_clock>> recorded_times;  // The times recorded at each frame; is equal in length to frame_look back.
};