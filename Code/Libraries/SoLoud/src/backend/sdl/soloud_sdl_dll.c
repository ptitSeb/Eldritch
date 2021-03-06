/*
SoLoud audio engine
Copyright (c) 2013-2014 Jari Komppa

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

   3. This notice may not be removed or altered from any source
   distribution.
*/
#include <stdlib.h>
#if defined(_MSC_VER)
#define WINDOWS_VERSION
#include "SDL.h"
#else
#ifdef WITH_SDL
#ifdef WITH_SDL2
#error Cannot use both SDL and SDL2 backends as dynamic library
#else
#include <SDL/SDL.h>
#endif
#else
#include <SDL2/SDL.h>
#endif
#endif
#include <math.h>

typedef int (*SDLOpenAudio)(SDL_AudioSpec *desired, SDL_AudioSpec *obtained);
typedef void (*SDLCloseAudio)();
typedef void (*SDLPauseAudio)(int pause_on);

static SDLOpenAudio dSDL_OpenAudio = NULL;
static SDLCloseAudio dSDL_CloseAudio = NULL;
static SDLPauseAudio dSDL_PauseAudio = NULL;

#ifdef WINDOWS_VERSION
#include <windows.h>

static HMODULE openDll()
{
	HMODULE res = LoadLibraryA("SDL2.dll");
	if (!res) res = LoadLibraryA("SDL.dll");
    return res;
}

static void* getDllProc(HMODULE aDllHandle, const char *aProcName)
{
    return GetProcAddress(aDllHandle, aProcName);
}

#else
#include <dlfcn.h> // dll functions

static void * openDll()
{
	void * res;
	res = dlopen("/Library/Frameworks/SDL2.framework/SDL2", RTLD_LAZY);
	if (!res) res = dlopen("/Library/Frameworks/SDL.framework/SDL", RTLD_LAZY);
	if (!res) res = dlopen("libSDL2.so", RTLD_LAZY);
	if (!res) res = dlopen("libSDL.so", RTLD_LAZY);
    return res;
}

static void* getDllProc(void * aLibrary, const char *aProcName)
{
    return dlsym(aLibrary, aProcName);
}

#endif

static int load_dll()
{
#ifdef WINDOWS_VERSION
	HMODULE dll = NULL;
#else
	void * dll = NULL;
#endif

	if (dSDL_OpenAudio != NULL)
	{
		return 1;
	}

    dll = openDll();

    if (dll)
    {
	    dSDL_OpenAudio = (SDLOpenAudio)getDllProc(dll, "SDL_OpenAudio");
	    dSDL_CloseAudio = (SDLCloseAudio)getDllProc(dll, "SDL_CloseAudio");
	    dSDL_PauseAudio = (SDLPauseAudio)getDllProc(dll, "SDL_PauseAudio");


        if (dSDL_OpenAudio && 
        	dSDL_CloseAudio &&
        	dSDL_PauseAudio)
        {
        	return 1;
        }
	}
	dSDL_OpenAudio = NULL;
    return 0;
}

int dll_SDL_found()
{
	return load_dll();
}

int dll_SDL_OpenAudio(SDL_AudioSpec *desired, SDL_AudioSpec *obtained)
{
	if (load_dll())
		return dSDL_OpenAudio(desired, obtained);
	return 0;
}

void dll_SDL_CloseAudio()
{
	if (load_dll())
		dSDL_CloseAudio();
}

void dll_SDL_PauseAudio(int pause_on)
{
	if (load_dll())
		dSDL_PauseAudio(pause_on);
}
