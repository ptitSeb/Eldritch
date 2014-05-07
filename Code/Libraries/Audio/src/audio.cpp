#include "core.h"
#include "audio.h"
#include "fmodaudiosystem.h"
#include "openalaudiosystem.h"

IAudioSystem* CreateFMODAudioSystem()
{
	return new FMODAudioSystem;
}

IAudioSystem* CreateOpenALAudioSystem()
{
	return new OpenALAudioSystem;
}