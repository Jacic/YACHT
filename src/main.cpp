/*
Yet Another CHip-8 emulaTor
Created by Jacob White
See LICENSE file for license information
*/

#include <fstream>
#include <sstream>
#include "chip8core.h"
#include "sdlhelper.h"

using namespace std;

Chip8Core core;

Uint8 drawframerate; //the frame rate at which to draw the screen (not cycles per second)
string filename;     //the name of the CHIP-8 program to run
string soundpath;    //the location of the sound to use with the sound timer
bool quitonerror;    //whether or not we will quit on an invalid opcode
Uint8 screenscale;   //the amount the screen is scaled by
string restartkey;   //the key used to reset the emulation
string keys[16];     //the keys mapped to Chip-8 keys
Uint8 *data;         //program data to pass to the core

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Event e;
Mix_Chunk *sound;

int init()
{
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
	{
		logSDLError(cout, "SDL Init");
		return 1;
	}

	window = SDL_CreateWindow("YACHT", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, core.screenwidth * 2 * screenscale, core.screenheight * 2 * screenscale, 0);
	if(window == nullptr)
	{
		logSDLError(cout, "SDL Create Window");
		return 1;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if(renderer == nullptr)
	{
		logSDLError(cout, "SDL Create Renderer");
		return 1;
	}

	if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) != 0)
	{
		logMIXError(cout, "SDL_mixer Init");
		return 1;
	}

	if(soundpath != "")
	{
		sound = Mix_LoadWAV(soundpath.c_str());
		if(sound == nullptr)
		{
			logMIXError(cout, "SDL_mixer Load WAV");
			return 1;
		}
	}

	return 0;
}

void cleanup()
{
	Mix_FreeChunk(sound);
	Mix_Quit();
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

int loadConfig()
{
	bool success = true;
	char* config;
	string keyvalues[22];
	string values[22];
	string defaultkeys[16];
	string names[22] = {"defaultprogram", "scale", "framerate", "quitonerror", "sound", "keyrestart",
	"key0", "key1", "key2", "key3", "key4", "key5", "key6", "key7", "key8",
	 "key9", "keya", "keyb", "keyc", "keyd", "keye", "keyf"};

	defaultkeys[0] = "X";
	defaultkeys[1] = "1";
	defaultkeys[2] = "2";
	defaultkeys[3] = "3";
	defaultkeys[4] = "Q";
	defaultkeys[5] = "W";
	defaultkeys[6] = "E";
	defaultkeys[7] = "A";
	defaultkeys[8] = "S";
	defaultkeys[9] = "D";
	defaultkeys[0xA] = "Z";
	defaultkeys[0xB] = "C";
	defaultkeys[0xC] = "4";
	defaultkeys[0xD] = "R";
	defaultkeys[0xE] = "F";
	defaultkeys[0XF] = "V";

	ifstream conf("yacht.cfg", ios::ate);
	if(!conf.is_open())
	{
		success = false;

		screenscale = 1;
		drawframerate = 60;
		quitonerror = true;
		soundpath = "";
		restartkey = "ESCAPE";

		for(int i = 0; i < 16; i++)
		{
			keys[i] = defaultkeys[i];
		}
	}
	else
	{
		int size = conf.tellg();
		conf.seekg(0, ios::beg);

		config = new char[size];
		conf.read(config, size);

		conf.close();

		istringstream reader(config);
		string line;
		int num = 0;
		while(getline(reader, line))
		{
			istringstream curline(line);
			string key;

			if(line.compare(0, 1, "#", 0, 1))
			{
				if(getline(curline, key, '='))
				{
					for(int i = 0; i < 19; i++)
					{
						if(key == names[i])
						{
							keyvalues[num] = key;
							getline(curline, values[num]);

							num++;
							break;
						}
					}
				}
			}
		}

		for(int i = 0; i < 21; i++)
		{
			if(keyvalues[i] == names[0])
			{
				filename = values[i];
			}
			else if(keyvalues[i] == names[1])
			{
				screenscale = (atoi(values[i].c_str()) > 0) ? atoi(values[i].c_str()) : 2;
			}
			else if(keyvalues[i] == names[2])
			{
				drawframerate = (atoi(values[i].c_str()) > 0) ? atoi(values[i].c_str()) : 2;
			}
			else if(keyvalues[i] == names[3])
			{
				quitonerror = values[i].compare(0, 5, "false");
			}
			else if(keyvalues[i] == names[4])
			{
				soundpath = values[i];
			}
			else if(keyvalues[i] == names[5])
			{
				restartkey = (SDL_GetScancodeFromName(values[i].c_str()) == SDL_SCANCODE_UNKNOWN) ? "ESCAPE" : values[i];
			}
			else
			{
				keys[i - 6] = (SDL_GetScancodeFromName(values[i].c_str()) == SDL_SCANCODE_UNKNOWN) ? defaultkeys[i - 6] : values[i];
			}
		}

		delete[] config;
	}

	if(success)
	{
		return 0;
	}

	return 1;
}

//draws pixels on the screen
void drawScreen()
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	for(int i = 0; i < core.screenwidth; i++)
	{
		for(int j = 0; j < core.screenheight; j++)
		{
			if(core.screendata[i][j])
			{
				setRendererScaledPixel(renderer, screenscale * 2, i, j, 255, 255, 255);
			}
		}
	}
	SDL_RenderPresent(renderer);
}

inline bool keyIsDown(SDL_Scancode key)
{
	const Uint8 *state = SDL_GetKeyboardState(NULL);
	if(state[key])
	{
		return true;
	}

	return false;
}

int main(int argc, char *argv[])
{
	filename = "";

	core.init(SDL_GetTicks());

	Uint32 drawTimeStart = SDL_GetTicks();

	int loadConfigReturn = loadConfig();

	if(argc > 1)
	{
		filename = argv[1];

		if(loadConfigReturn == 1)
		{
			printf("error reading from yacht.cfg config file\nusing supplied program name...");
		}
	}
	else
	{
		if(loadConfigReturn == 1)
		{
			printf("error reading from yacht.cfg config file\nexiting...");
			return 1;
		}
	}

	if(init() != 0)
	{
		return 1;
	}

	if(core.loadProgram(filename) != 0)
	{
		return 1;
	}

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	SDL_RenderPresent(renderer);

	while(core.running)
	{
		while(SDL_PollEvent(&e))
		{
			switch(e.type)
			{
				case SDL_QUIT:
					core.running = false;
					break;
			}
		}

		const Uint8 *keystate = SDL_GetKeyboardState(NULL);
		for(int i = 0; i < 16; i++)
		{
			if(keystate[SDL_GetScancodeFromName(keys[i].c_str())])
			{
				core.setKeyState(i, true);
			}
			else
			{
				core.setKeyState(i, false);
			}
		}
		if(keystate[SDL_GetScancodeFromName(restartkey.c_str())])
		{
			core.init(SDL_GetTicks());
			core.loadProgram(filename);
		}

		core.setTimer(SDL_GetTicks());
		int coreReturn = core.coreLoop();
		if(coreReturn == 1)
		{
			if(quitonerror)
			{
				core.running = false;
			}
		}
		else if(coreReturn == 2)
		{
			SDL_Delay(3);
		}

		if(core.getSoundTimer() > 0)
		{
			//play sound
			if(soundpath != "" && !Mix_Playing(-1))
			{
				if(Mix_PlayChannel(-1, sound, 0) == -1)
				{
					logMIXError(cout, "SDL_mixer Play Channel");
				}
			}
		}

		if(SDL_GetTicks() - drawTimeStart >= (1000 / drawframerate))
		{
			drawScreen();
			drawTimeStart = SDL_GetTicks();
		}
	}

	cleanup();
}