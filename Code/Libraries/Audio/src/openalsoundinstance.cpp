#include "core.h"
#include "openalsoundinstance.h"
#include "isound.h"
#include "iaudiosystem.h"
#include "configmanager.h"
#include "mathcore.h"
#include "sound3dlistener.h"

OpenALSoundInstance::OpenALSoundInstance(ISound* const pSound) {
  SetSound(pSound);
  alGenSources(1, &source);
}

OpenALSoundInstance::~OpenALSoundInstance() {
  if (!IsFinished()) {
    Stop();
  }

  // Delete the corresponding sound iff it's a stream (incidentally, stream
  // sounds
  // only get automatically deleted if they're ever actually played).
  if (GetSound()->GetIsStream()) {
    SafeDelete(m_Sound);
  }

  alDeleteSources(1, &source);
}

void OpenALSoundInstance::Stop() { alSourceStop(source); }

void OpenALSoundInstance::SetPriority(ESoundPriority Priority) const {
  PRINTF("OpenALSoundInstance::SetPriority() STUB, doing nothing.\n");
  if (Priority == ESP_Default) {
    // m_Channel->setPriority( 128 );
  } else if (Priority == ESP_High) {
    // m_Channel->setPriority( 0 );
  }
}

void OpenALSoundInstance::SetPaused(bool Paused) {
  if (Paused) {
    alSourcePause(source);
  } else {
    int status;
    alGetSourcei(source, AL_SOURCE_STATE, &status);
    if (status == AL_PAUSED) {
      alSourcePlay(source);
    }
  }
}

void OpenALSoundInstance::SetVolume(float Volume) {
  alSourcef(source, AL_GAIN, Volume);
}

void OpenALSoundInstance::SetPan(float Pan) {
  // PRINTF("OpenALSoundInstance::SetPan() STUB, doing nothing.\n");
}

/*virtual*/ bool OpenALSoundInstance::IsPlaying() const {
  int status;
  alGetSourcei(source, AL_SOURCE_STATE, &status);
  return status == AL_PLAYING;
}

bool OpenALSoundInstance::IsFinished() const {
  int status;
  alGetSourcei(source, AL_SOURCE_STATE, &status);
  return !(status == AL_PLAYING || status == AL_PAUSED);
}

float OpenALSoundInstance::GetTimeElapsed() const {
  float elapsed;
  alGetSourcef(source, AL_SEC_OFFSET, &elapsed);
  return elapsed;
}