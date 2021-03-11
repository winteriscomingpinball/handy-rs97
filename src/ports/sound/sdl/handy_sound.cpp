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


static uint32_t buf_read_pos = 0;
static uint32_t buf_write_pos = 0;
static int32_t buffered_bytes = 0;

//static snd_pcm_t *handle;



static int32_t sdl_read_buffer(uint8_t* data, int32_t len)
{
	if (buffered_bytes >= len) 
	{
		if(buf_read_pos + len <= HANDY_AUDIO_BUFFER_SIZE ) 
		{
			memcpy(data, gAudioBuffer + buf_read_pos, len);
		} 
		else 
		{
			int32_t tail = HANDY_AUDIO_BUFFER_SIZE - buf_read_pos;
			memcpy(data, gAudioBuffer + buf_read_pos, tail);
			memcpy(data + tail, gAudioBuffer, len - tail);
		}
		buf_read_pos = (buf_read_pos + len) % HANDY_AUDIO_BUFFER_SIZE;
		buffered_bytes -= len;
	}

	return len;
}

void sdl_callback(void *unused, uint8_t *stream, int32_t len)
{
	sdl_read_buffer((uint8_t *)stream, len);
}



static void sdl_write_buffer(uint8_t* data, int32_t len)
{
	for(uint32_t i = 0; i < len; i += 4) 
	{
		if(buffered_bytes == HANDY_AUDIO_BUFFER_SIZE) return; // just drop samples
		*(int32_t*)((char*)(gAudioBuffer + buf_write_pos)) = *(int32_t*)((char*)(data + i));
		//memcpy(buffer + buf_write_pos, data + i, 4);
		buf_write_pos = (buf_write_pos + 4) % HANDY_AUDIO_BUFFER_SIZE;
		buffered_bytes += 4;
	}
}




int handy_audio_init(void)
{
    /* If we don't want sound, return 0 */
    if(gAudioEnabled == FALSE) return 0;

    SDL_AudioSpec aspec, obtained;

	/* Add some silence to the buffer */
	//buffered_bytes = 0;
	//buf_read_pos = 0;
	//buf_write_pos = 0;

	aspec.format   = AUDIO_S16SYS;
	aspec.freq     = HANDY_AUDIO_SAMPLE_FREQ;
	aspec.channels = 2;
	aspec.samples  = HANDY_AUDIO_BUFFER_SIZE;
	aspec.callback = (sdl_callback);
	aspec.userdata = NULL;

	/* initialize the SDL Audio system */
	if (SDL_InitSubSystem (SDL_INIT_AUDIO | SDL_INIT_NOPARACHUTE)) 
	{
		printf("SDL: Initializing of SDL Audio failed: %s.\n", SDL_GetError());
		return 0;
	}

	/* Open the audio device and start playing sound! */
	if(SDL_OpenAudio(&aspec, &obtained) < 0) 
	{
		printf("SDL: Unable to open audio: %s\n", SDL_GetError());
		return 0;
	}
	
	SDL_PauseAudio(0);
	
	
	
	
	gAudioEnabled = 1;
    return 1;
}

void handy_audio_close()
{
	SDL_PauseAudio(1);
	SDL_CloseAudio();
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
	//if (buffer)
	//{
	//	free(buffer);	
	//	buffer = NULL;
	//}
}

void handy_audio_loop()
{
	mpLynx->Update();
	if (gAudioBufferPointer >= HANDY_AUDIO_BUFFER_SIZE/2 && gAudioEnabled)
	{
	SDL_LockAudio();
	sdl_write_buffer(gAudioBuffer, HANDY_AUDIO_BUFFER_SIZE * 4);
	SDL_UnlockAudio();
	}
	
}

