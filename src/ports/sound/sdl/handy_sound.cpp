//
// Copyright (c) 2004 SDLemu Team
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from
// the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cctype>

//#include <alsa/asoundlib.h>
#include <SDL/SDL.h>

#include "handy_sdl_main.h"
#include "handy_sound.h"

//static snd_pcm_t *handle;





static volatile int audio_done;

	
static void audio_callback(void *blah, byte *stream, int len)
{
	memcpy(stream, gAudioBuffer, len);
	audio_done = 1;
}



int handy_audio_init(void)
{
    /* If we don't want sound, return 0 */
    if(gAudioEnabled == FALSE) return 0;

#ifdef HANDY_SDL_DEBUG
    printf("handy_audio_init - DEBUG\n");
#endif

    SDL_InitSubSystem(SDL_INIT_AUDIO);
	as.freq = HANDY_AUDIO_SAMPLE_FREQ;
	as.format = AUDIO_S16;
	as.channels = 2;
	as.samples = HANDY_AUDIO_SAMPLE_FREQ / 30;
	for (i = 1; i < as.samples; i<<=1);
	as.samples = i;
	as.callback = audio_callback;
	as.userdata = 0;
	if (SDL_OpenAudio(&as, 0) == -1)
		return 0;
	
	
	SDL_PauseAudio(0);
	
    return 1;
}

void handy_audio_close()
{
	SDL_PauseAudio(1);
	SDL_CloseAudio();
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

void handy_audio_loop()
{
	mpLynx->Update();
	if (gAudioBufferPointer >= HANDY_AUDIO_BUFFER_SIZE/2 && gAudioEnabled)
	{
		
		gAudioBufferPointer=0;
		//if (!pcm.buf) return;
		//if (pcm.pos < pcm.len) return;
		while (!audio_done)
			SDL_Delay(4);
		audio_done = 0;
		
		
	}
}
