/*
Yet Another CHip-8 emulaTor
Created by Jacob White
See LICENSE file for license information
*/

#ifndef CHIP8_CORE_H
#define CHIP8_CORE_H

#include <time.h>
#include <cstdint>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <string>

class Chip8Core
{
private:
	uint32_t timerStart;
	uint32_t curTime;
	uint16_t opcode;
	uint16_t cyclespersecond;
	uint16_t opcodesthissecond;

	bool keystates[16];         //true if the respective key is pressed
	uint16_t pc;                //points to the current instruction
	uint8_t data[4096];         //holds a CHIP-8 program
	uint8_t registers[16];      //holds 16 4-bit registers
	uint8_t hp48flags[8];       //additional flags used by SCHIP
	uint16_t stack[16];         //call stack
	uint8_t sp;                 //stack pointer
	uint16_t address;           //address register (also known as I)
	uint8_t delaytimer;         //used for timing, can be set and read
	uint8_t soundtimer;         //used for sound effects, makes beeping sound when nonzero
    	                        //both timers count down at 60 hertz
	uint8_t mode;               //0 for CHIP8, 1 for SCHIP

	int decodeOpcode(uint16_t opcode);
	inline void pushStack(uint16_t callingaddress);
	inline uint16_t popStack();
	void drawSprite(uint8_t vx, uint8_t vy, uint8_t n);
	void storeRegisterAsBCD(int value);
	bool keyIsDown(uint8_t keynum);

public:
	int screenwidth;
	int screenheight;
	uint8_t screendata[128][64]; //holds the pixels of the screen;
	bool running;                //if the emulation should end, used by SCHIP

	void init(uint32_t timerValue);
	int coreLoop();
	int loadProgram(std::string filename);
	void setKeyState(uint8_t keynum, bool ispressed);
	void setTimer(uint32_t curTimeValue);
	uint8_t getSoundTimer();
	uint8_t getMode();
};

#endif