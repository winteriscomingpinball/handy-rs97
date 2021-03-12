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

//////////////////////////////////////////////////////////////////////////////
//                       Handy/SDL - An Atari Lynx Emulator                 //
//                             Copyright (c) 2005                           //
//                                SDLemu Team                               //
//                                                                          //
//                          Based upon Handy v0.90 WIN32                    // 
//                            Copyright (c) 1996,1997                       //
//                                  K. Wilkins                              //
//////////////////////////////////////////////////////////////////////////////
// handy_sdl_graphics.cpp                                                   //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This is the Handy/SDL handling. It manages the handling functions        //
// of the keyboard and/or joypad for emulating the Atari Lynx emulator      //
// using the SDL Library.                                                     //
//                                                                          //
//    N. Wagenaar                                                           //
// December 2005                                                            //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
// Revision History:                                                        //
// -----------------                                                        //
//                                                                          //
// December 2005 :                                                          //
//  Since the 14th of April, the WIN32 of Handy (written by Keith Wilkins)  //
//  Handy has become OpenSource. Handy/SDL v0.82 R1 was based upon the old  //
//  v0.82 sources and was released closed source.                           //
//                                                                          //
//  Because of this event, the new Handy/SDL will be released as OpenSource //
//  but is rewritten from scratch because of lost sources (tm). The SDLemu  //
//  team has tried to bring Handy/SDL v0.1 with al the functions from the   //
//  closed source version.                                                  //
//////////////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cctype>
#include <SDL/SDL.h>
#include <SDL/SDL_main.h>
#include <SDL/SDL_timer.h>

#include "handy_sdl_main.h"
#include "handy_sdl_handling.h"
#include "handy_sdl_graphics.h"

extern int Invert;
extern int gui_SwapAB;
extern void gui_Run();
#ifndef NOJOYSTICK
extern SDL_Joystick* joystick;
#define AXIS_JOYSTICK 8192
int16_t x_move = 0, y_move = 0;
uint32_t dpad_pressed[4] = {0, 0, 0, 0};
#endif


int exitCheck=0;
int menuCheck=0;
int opt1Check=0;
int opt2Check=0;

//	SDL_JoystickUpdate();

// map keys differently for zipit
#define BT_LEFT     SDLK_LEFT
#define BT_RIGHT    SDLK_RIGHT
#define BT_UP       SDLK_UP
#define BT_DOWN     SDLK_DOWN
#define BT_PAUSE    SDLK_F1      // START
#define BT_A        SDLK_RSHIFT       // A
#define BT_B        SDLK_KP_MULTIPLY        // B
#define BT_OPT1     SDLK_LSHIFT      // Y
#define BT_OPT2     SDLK_SPACE       // X
#define BT_OFF      SDLK_ESCAPE      // SELECT



uint32_t Joystick_Down(uint32_t mask, SDL_Event event)
{
	
	
	
	while (SDL_PollEvent(&event))
	{
		switch(event.type)
		{
			case SDL_KEYDOWN:
				switch(event.key.keysym.sym) 
				{
						case BT_LEFT: {  // Lynx LEFT
							#ifndef NOJOYSTICK
							dpad_pressed[0] = 1;
							#endif
							menuCheck+=1;
							opt1Check+=1;
							switch(Invert)
							{
								case 0:
									mask|=BUTTON_LEFT;
								break;
								case 1:
									mask|=BUTTON_UP;
								break;
								case 2:
									mask|=BUTTON_DOWN;
								break;
							}
							break;
						}
						case BT_RIGHT: { // Lynx RIGHT
							#ifndef NOJOYSTICK
							dpad_pressed[1] = 1;
							#endif
							opt2Check+=1;
							switch(Invert)
							{
								case 0:
									mask|=BUTTON_RIGHT;
								break;
								case 1:
									mask|=BUTTON_DOWN;
								break;
								case 2:
									mask|=BUTTON_UP;
								break;
							}
							break;
						}

						case BT_UP: { // Lynx UP
							#ifndef NOJOYSTICK
							dpad_pressed[2] = 1;
							#endif
							switch(Invert)
							{
								case 0:
									mask|=BUTTON_UP;
								break;
								case 1:
									mask|=BUTTON_RIGHT;
								break;
								case 2:
									mask|=BUTTON_LEFT;
								break;
							}
							break;
						}

						case BT_DOWN: { // Lynx DOWN
							#ifndef NOJOYSTICK
							dpad_pressed[3] = 1;
							#endif
							exitCheck+=1;
							switch(Invert)
							{
								case 0:
									mask|=BUTTON_DOWN;
								break;
								case 1:
									mask|=BUTTON_LEFT;
								break;
								case 2:
									mask|=BUTTON_RIGHT;
								break;
							}
							break;
						}

						case BT_PAUSE: { // Lynx PAUSE
							mask|=BUTTON_PAUSE;
							exitCheck+=1;
							menuCheck+=1;
							opt1Check+=1;
							opt2Check+=1;
							break;
						}
						
						case BT_A: { // Lynx A
							if (gui_SwapAB == 1) mask|=BUTTON_B;
							else mask|=BUTTON_A;
							exitCheck+=1;
							menuCheck+=1;
							break;
						}

						case BT_B: { // Lynx B
							if (gui_SwapAB == 1) mask|=BUTTON_A;
							else mask|=BUTTON_B;
							exitCheck+=1;
							menuCheck+=1;
							break;
						}

						case BT_OPT1: { // Lynx Option 1
							mask|=BUTTON_OPT1;
							break;
						}

						case BT_OPT2: { // Lynx Option 2
							mask|=BUTTON_OPT2;
							break;
						}

						default: {
							break;
						}

				}
			if (opt1Check==2){
				mask|=BUTTON_OPT1;
				mask&= ~BUTTON_LEFT;
				mask&= ~BUTTON_PAUSE;
			}
			if (opt2Check==2){
				mask|=BUTTON_OPT2;
				mask&= ~BUTTON_RIGHT;
				mask&= ~BUTTON_PAUSE;
			}
			break;
			case SDL_KEYUP:
				switch(event.key.keysym.sym) 
				{
						case SDLK_ESCAPE:
						case SDLK_END:
						case SDLK_RCTRL:
						case SDLK_HOME:
							#ifdef IPU_SCALE
							Handy_Change_Res(1);
							#endif
							gui_Run();
							#ifdef IPU_SCALE
							Handy_Change_Res(0);
							#endif
							mask = 0;
							#ifndef NOJOYSTICK
							x_move = 0;
							y_move = 0;
							dpad_pressed[0] = 0;
							dpad_pressed[1] = 0;
							dpad_pressed[2] = 0;
							dpad_pressed[3] = 0;
							#endif
						break;
						case BT_LEFT: {  // Lynx LEFT
							#ifndef NOJOYSTICK
							dpad_pressed[0] = 0;
							#endif
							menuCheck=0;
							opt1Check=0;
							switch(Invert)
							{
								case 0:
									mask&= ~BUTTON_LEFT;
								break;
								case 1:
									mask&= ~BUTTON_UP;
								break;
								case 2:
									mask&= ~BUTTON_DOWN;
								break;
							}
							break;
						}
						case BT_RIGHT: { // Lynx RIGHT
							#ifndef NOJOYSTICK
							dpad_pressed[1] = 0;
							#endif
							opt2Check=0;
							switch(Invert)
							{
								case 0:
									mask&= ~BUTTON_RIGHT;
								break;
								case 1:
									mask&= ~BUTTON_DOWN;
								break;
								case 2:
									mask&= ~BUTTON_UP;
								break;
							}
							break;
						}

						case BT_UP: { // Lynx UP
							#ifndef NOJOYSTICK
							dpad_pressed[2] = 0;
							#endif
							switch(Invert)
							{
								case 0:
									mask&= ~BUTTON_UP;
								break;
								case 1:
									mask&= ~BUTTON_RIGHT;
								break;
								case 2:
									mask&= ~BUTTON_LEFT;
								break;
							}
							break;
						}

						case BT_DOWN: { // Lynx DOWN
							#ifndef NOJOYSTICK
							dpad_pressed[3] = 0;
							#endif
							exitCheck=0;
							switch(Invert)
							{
								case 0:
									mask&= ~BUTTON_DOWN;
								break;
								case 1:
									mask&= ~BUTTON_LEFT;
								break;
								case 2:
									mask&= ~BUTTON_RIGHT;
								break;
							}
							break;
						}
						
						case BT_A: { // Lynx A
							if (gui_SwapAB == 1) mask&= ~BUTTON_B;
							else mask&= ~BUTTON_A;
							exitCheck=0;
							menuCheck=0;
							break;
						}

						case BT_B: { // Lynx B
							if (gui_SwapAB == 1) mask&= ~BUTTON_A;
							else mask&= ~BUTTON_B;
							exitCheck=0;
							menuCheck=0;
							break;
						}

						case BT_PAUSE: { // Lynx PAUSE
							mask&= ~BUTTON_PAUSE;
							
							mask&= ~BUTTON_OPT1;
							
							mask&= ~BUTTON_OPT2;
							exitCheck=0;
							menuCheck=0;
							opt2Check=0;
							opt1Check=0;
							break;
						}

						case BT_OPT1: { // Lynx Option 1
							mask&= ~BUTTON_OPT1;
							break;
						}

						case BT_OPT2: { // Lynx Option 2
							mask&= ~BUTTON_OPT2;
							break;
						}

						default: {
							break;
						}

				}
			break;
			#ifndef NOJOYSTICK
			case SDL_JOYAXISMOTION:
				if (event.jaxis.axis == 0) x_move = event.jaxis.value;
				else if (event.jaxis.axis == 1) y_move = event.jaxis.value;
			break;
			case SDL_JOYBUTTONDOWN:
			switch(event.jbutton.button)
			{
				case 0:
					mask |= BUTTON_OPT1;
				break;
				case 1:
					mask |= BUTTON_A;
				break;
				case 2:
					mask |= BUTTON_B;
				break;
				case 3:
					mask |= BUTTON_OPT2;
				break;
			}
			break;
			case SDL_JOYBUTTONUP:
			switch(event.jbutton.button)
			{
				case 0:
					mask&= ~BUTTON_OPT1;
				break;
				case 1:
					mask&= ~BUTTON_A;
				break;
				case 2:
					mask&= ~BUTTON_B;
				break;
				case 3:
					mask&= ~BUTTON_OPT2;
				break;
			}
			break;
			#endif
			default:
				mask = 0;
			break;
		}
	}
    
    #ifndef NOJOYSTICK
	switch (Invert)
	{
		case 0:
			if(x_move > AXIS_JOYSTICK)
				mask |= BUTTON_RIGHT;
			else if(x_move < -AXIS_JOYSTICK)
				mask |= BUTTON_LEFT;
			else if (dpad_pressed[0] == 0 && dpad_pressed[1] == 0)
			{
				mask &= ~BUTTON_RIGHT;
				mask &= ~BUTTON_LEFT;
			}

			if(y_move > AXIS_JOYSTICK)
				mask |= BUTTON_DOWN;
			else if(y_move < -AXIS_JOYSTICK)
				mask |= BUTTON_UP;
			else if (dpad_pressed[2] == 0 && dpad_pressed[3] == 0)
			{
				mask &= ~BUTTON_UP;
				mask &= ~BUTTON_DOWN;
			}
		break;
		case 1:
			if(x_move > AXIS_JOYSTICK)
				mask |= BUTTON_DOWN;
			else if(x_move < -AXIS_JOYSTICK)
				mask |= BUTTON_UP;
			else if (dpad_pressed[0] == 0 && dpad_pressed[1] == 0)
			{
				mask &= ~BUTTON_UP;
				mask &= ~BUTTON_DOWN;
			}

			if(y_move > AXIS_JOYSTICK)
				mask |= BUTTON_LEFT;
			else if(y_move < -AXIS_JOYSTICK)
				mask |= BUTTON_RIGHT;
			else if (dpad_pressed[2] == 0 && dpad_pressed[3] == 0)
			{
				mask &= ~BUTTON_RIGHT;
				mask &= ~BUTTON_LEFT;
			}
		break;
			
		/* KLAX */
		case 2:
			if (x_move > AXIS_JOYSTICK) mask |= BUTTON_UP;
			else if (x_move < -AXIS_JOYSTICK) mask |= BUTTON_DOWN;
			else if (dpad_pressed[0] == 0 && dpad_pressed[1] == 0)
			{
				mask &= ~BUTTON_UP;
				mask &= ~BUTTON_DOWN;
			}

			if (y_move > AXIS_JOYSTICK) mask |= BUTTON_RIGHT;
			else if (y_move < -AXIS_JOYSTICK) mask |= BUTTON_LEFT;
			else if (dpad_pressed[2] == 0 && dpad_pressed[3] == 0)
			{
				mask &= ~BUTTON_RIGHT;
				mask &= ~BUTTON_LEFT;
			}
		break;
	}
	#endif
    
	if (exitCheck==4){
		handy_sdl_quit();
	}
	
	if (menuCheck==4){
		#ifdef IPU_SCALE
		Handy_Change_Res(1);
		#endif
		gui_Run();
		#ifdef IPU_SCALE
		Handy_Change_Res(0);
		#endif
		mask = 0;
		#ifndef NOJOYSTICK
		x_move = 0;
		y_move = 0;
		dpad_pressed[0] = 0;
		dpad_pressed[1] = 0;
		dpad_pressed[2] = 0;
		dpad_pressed[3] = 0;
		#endif
	}
	
	return mask;
	
}

