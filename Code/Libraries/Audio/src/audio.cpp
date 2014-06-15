#include "core.h"
#include "audio.h"
#include "openalaudiosystem.h"

IAudioSystem* CreateOpenALAudioSystem() { return new OpenALAudioSystem; }
