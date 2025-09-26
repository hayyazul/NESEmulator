#include "frameCounter.h"
#include <exception>

FrameCounter::FrameCounter() : frame_lookback(60) {}
FrameCounter::FrameCounter(const unsigned int frame_lookback) : frame_lookback(frame_lookback) {
	if (frame_lookback > MAX_FRAME_LOOKBACK_ALLOWED) {
		throw std::exception("frame_lookback given is too big; max size of 1000 is allowed.");
	}
}
FrameCounter::~FrameCounter() {}

double FrameCounter::getFrameRate() const {
	if (this->recorded_times.size() == 0) return 0;
	return (1000.0 / this->getMSPerFrame().count());
}

std::chrono::milliseconds FrameCounter::getMSPerFrame() const {
	if (this->recorded_times.size() == 0) return std::chrono::milliseconds(0);
	// Get the time_point of the first recorded frame in the queue
	std::chrono::steady_clock::time_point first_frame_timepoint = this->recorded_times.front();
	std::chrono::steady_clock::time_point current_timepoint = std::chrono::steady_clock::now();
	
	// Getting the difference in time in milliseconds.
	std::chrono::nanoseconds avg_time_elapsed = (current_timepoint - first_frame_timepoint) / this->recorded_times.size();
	return std::chrono::duration_cast<std::chrono::milliseconds>(avg_time_elapsed);
}

void FrameCounter::countFrame() {
	if (this->recorded_times.size() >= this->frame_lookback) {
		this->recorded_times.pop();  // Remove the first element added.
	}
	// Record the current time.
	this->recorded_times.push(std::chrono::steady_clock::now());
}
