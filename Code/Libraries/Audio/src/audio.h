#ifndef AUDIO_H
#define AUDIO_H

class IAudioSystem;

IAudioSystem* CreateFMODAudioSystem();
IAudioSystem* CreateOpenALAudioSystem();

#endif // AUDIO_H