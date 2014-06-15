#include "core.h"
#include "openalaudiosystem.h"
#include "soundmanager.h"
#include "openalsound.h"
#include "openalsoundinstance.h"
#include "configmanager.h"
#include "mathfunc.h"
#include "interpolator.h"

OpenALAudioSystem::OpenALAudioSystem() : m_ReverbCategories() {
  DEBUGPRINTF("Initializing OpenAL audio system...\n");

  STATICHASH(AudioSystem);

  STATICHASH(MaxChannels);
  const int MaxChannels = ConfigManager::GetInt(sMaxChannels, 32, sAudioSystem);

  // Create OpenAL device and context
  device = alcOpenDevice(nullptr);
  context = alcCreateContext(device, nullptr);
  alcMakeContextCurrent(context);

  STATICHASH(DefaultReverb);
  const SimpleString DefaultReverb =
      ConfigManager::GetString(sDefaultReverb, "", sAudioSystem);
  SetReverbParams(DefaultReverb);

  STATICHASH(NumReverbCategories);
  const uint NumReverbCategories =
      ConfigManager::GetInt(sNumReverbCategories, 0, sAudioSystem);
  for (uint ReverbCategoryIndex = 0; ReverbCategoryIndex < NumReverbCategories;
       ++ReverbCategoryIndex) {
    const HashedString ReverbCategory = ConfigManager::GetSequenceHash(
        "ReverbCategory%d", ReverbCategoryIndex, "", sAudioSystem);
    m_ReverbCategories.PushBack(ReverbCategory);
  }
}

OpenALAudioSystem::~OpenALAudioSystem() {
  // Free sound instances before deleting sound manager, else
  // querying if sounds are streams when deleting instances will fail
  FreeSoundInstances();

  SafeDelete(m_SoundManager);

  alcMakeContextCurrent(nullptr);
  if (context) alcDestroyContext(context);
  if (device) alcCloseDevice(device);
}

void OpenALAudioSystem::Tick(float DeltaTime, bool GamePaused) {
  XTRACE_FUNCTION;

  AudioSystemCommon::Tick(DeltaTime, GamePaused);
}

ISound* OpenALAudioSystem::CreateSound(const SSoundInit& SoundInit) {
  auto  const pSound = new OpenALSound(this, SoundInit);
  pSound->Initialize(SoundInit);
  return pSound;
}

ISoundInstance* OpenALAudioSystem::Play(const SimpleString& DefinitionName,
                                        const Vector& Location) {
  ISoundInstance* const pInstance = CreateSoundInstance(DefinitionName);
  ASSERT(pInstance);

  // Set variables, then unpause
  pInstance->SetLocation(Location);

  // Apply reverb
  for (uint i = 0; i < m_ReverbCategories.Size(); ++i) {
    if (pInstance->GetCategory() == m_ReverbCategories[i]) {
      //	( ( FMODSoundInstance* )pInstance )->m_Channel->setChannelGroup(
      //m_ReverbGroup );
      break;
    }
  }

  pInstance->Tick();  // To make sure all changes are applied
  pInstance->SetPaused(false);

  return pInstance;
}

/*virtual*/ void OpenALAudioSystem::ConditionalApplyReverb(
    ISoundInstance* const pSoundInstance) const {
  ASSERT(pSoundInstance);

  const HashedString& SoundCategory = pSoundInstance->GetCategory();
  for (uint ReverbCategoryIndex = 0;
       ReverbCategoryIndex < m_ReverbCategories.Size(); ++ReverbCategoryIndex) {
    const HashedString& ReverbCategory =
        m_ReverbCategories[ReverbCategoryIndex];
    if (SoundCategory == ReverbCategory) {
      OpenALSoundInstance* const pOALInstance =
          static_cast<OpenALSoundInstance*>(pSoundInstance);
      // pOALInstance->m_Channel->setChannelGroup( m_ReverbGroup );
      break;
    }
  }
}

void OpenALAudioSystem::SetReverbParams(
    const SimpleString& DefinitionName) const {
  PRINTF("OpenALAudioSystem::SetReverbParams() STUB, doing nothing.\n");
  STATICHASH(DryLevel);
  STATICHASH(Room);
  STATICHASH(RoomHF);
  STATICHASH(DecayTime);
  STATICHASH(DecayHFRatio);
  STATICHASH(ReflectionsLevel);
  STATICHASH(ReflectionsDelay);
  STATICHASH(ReverbLevel);
  STATICHASH(ReverbDelay);
  STATICHASH(Diffusion);
  STATICHASH(Density);
  STATICHASH(HFReference);
  STATICHASH(RoomLF);
  STATICHASH(LFReference);
  MAKEHASH(DefinitionName);

  //~ m_ReverbDSP->setParameter( FMOD_DSP_SFXREVERB_DRYLEVEL,
  //ConfigManager::GetFloat( sDryLevel, 0.0f, sDefinitionName ) );
  //~ m_ReverbDSP->setParameter( FMOD_DSP_SFXREVERB_ROOM,
  //ConfigManager::GetFloat( sRoom, -10000.0f, sDefinitionName ) );
  //~ m_ReverbDSP->setParameter( FMOD_DSP_SFXREVERB_ROOMHF,
  //ConfigManager::GetFloat( sRoomHF, 0.0f, sDefinitionName ) );
  //~ m_ReverbDSP->setParameter( FMOD_DSP_SFXREVERB_DECAYTIME,
  //ConfigManager::GetFloat( sDecayTime, 1.0f, sDefinitionName ) );
  //~ m_ReverbDSP->setParameter( FMOD_DSP_SFXREVERB_DECAYHFRATIO,
  //ConfigManager::GetFloat( sDecayHFRatio, 0.5f, sDefinitionName ) );
  //~ m_ReverbDSP->setParameter( FMOD_DSP_SFXREVERB_REFLECTIONSLEVEL,
  //ConfigManager::GetFloat( sReflectionsLevel, -10000.0f, sDefinitionName ) );
  //~ m_ReverbDSP->setParameter( FMOD_DSP_SFXREVERB_REFLECTIONSDELAY,
  //ConfigManager::GetFloat( sReflectionsDelay, 0.02f, sDefinitionName ) );
  //~ m_ReverbDSP->setParameter( FMOD_DSP_SFXREVERB_REVERBLEVEL,
  //ConfigManager::GetFloat( sReverbLevel, 0.0f, sDefinitionName ) );
  //~ m_ReverbDSP->setParameter( FMOD_DSP_SFXREVERB_REVERBDELAY,
  //ConfigManager::GetFloat( sReverbDelay, 0.04f, sDefinitionName ) );
  //~ m_ReverbDSP->setParameter( FMOD_DSP_SFXREVERB_DIFFUSION,
  //ConfigManager::GetFloat( sDiffusion, 100.0f, sDefinitionName ) );
  //~ m_ReverbDSP->setParameter( FMOD_DSP_SFXREVERB_DENSITY,
  //ConfigManager::GetFloat( sDensity, 100.0f, sDefinitionName ) );
  //~ m_ReverbDSP->setParameter( FMOD_DSP_SFXREVERB_HFREFERENCE,
  //ConfigManager::GetFloat( sHFReference, 5000.0f, sDefinitionName ) );
  //~ m_ReverbDSP->setParameter( FMOD_DSP_SFXREVERB_ROOMLF,
  //ConfigManager::GetFloat( sRoomLF, 0.0f, sDefinitionName ) );
  //~ m_ReverbDSP->setParameter( FMOD_DSP_SFXREVERB_LFREFERENCE,
  //ConfigManager::GetFloat( sLFReference, 250.0f, sDefinitionName ) );
}