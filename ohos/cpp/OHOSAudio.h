#pragma once

#include <string>
#include <mutex>

typedef int (*OHOSAudioCallback)(short *buffer, int numSamples, int sampleRateHz);

class AudioContext {
public:
	AudioContext(OHOSAudioCallback cb, int _FramesPerBuffer, int _SampleRate);
	virtual bool Init() { return false; }
	virtual bool AudioRecord_Start(int sampleRate) { return false; };
	virtual bool AudioRecord_Stop() { return false; };

	int SampleRate() const { return sampleRate; }

	virtual ~AudioContext() {}

protected:
	void SetErrorString(const std::string &error);
	OHOSAudioCallback audioCallback;

	int framesPerBuffer;
	int sampleRate;
	std::mutex errorMutex_;
};

struct OHOSAudioState;


// It's okay for optimalFramesPerBuffer and optimalSampleRate to be 0. Defaults will be used.
OHOSAudioState *OHOSAudio_Init(OHOSAudioCallback cb, int optimalFramesPerBuffer, int optimalSampleRate);
bool OHOSAudio_Recording_SetSampleRate(OHOSAudioState *state, int sampleRate);
bool OHOSAudio_Recording_Start(OHOSAudioState *state);
bool OHOSAudio_Recording_Stop(OHOSAudioState *state);
bool OHOSAudio_Recording_State(OHOSAudioState *state);
bool OHOSAudio_Pause(OHOSAudioState *state);
bool OHOSAudio_Resume(OHOSAudioState *state);
bool OHOSAudio_Shutdown(OHOSAudioState *state);
const std::string OHOSAudio_GetErrorString(OHOSAudioState *state);
