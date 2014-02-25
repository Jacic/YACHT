/*
Yet Another CHip-8 emulaTor
Created by Jacob White
See LICENSE file for license information
*/

#include "chip8core.h"

void Chip8Core::init(uint32_t timerValue)
{
	opcode = 0;
	cyclespersecond = 0;
	opcodesthissecond = 0;
	mode = 0;
	screenwidth = 128;
	screenheight = 64;
	running = true;

	//store 8x5 pixel digit characters
	uint8_t digits[80] = {0xF0, 0x90, 0x90, 0x90, 0xF0, //0
						0x20, 0x60, 0x20, 0x20, 0x70,   //1
						0xF0, 0x10, 0xF0, 0x80, 0xF0,   //2
						0xF0, 0x10, 0xF0, 0x10, 0xF0,   //3
						0x90, 0x90, 0xF0, 0x10, 0x10,   //4
						0xF0, 0x80, 0xF0, 0x10, 0xF0,   //5
						0xF0, 0x80, 0xF0, 0x90, 0xF0,   //6
						0xF0, 0x10, 0x20, 0x40, 0x40,   //7
						0xF0, 0x90, 0xF0, 0x90, 0xF0,   //8
						0xF0, 0x90, 0xF0, 0x10, 0xF0,   //9
						0xF0, 0x90, 0xF0, 0x90, 0x90,   //A
						0xE0, 0x90, 0xE0, 0x90, 0xE0,   //B
						0xF0, 0x80, 0x80, 0x80, 0xF0,   //C
						0xE0, 0x90, 0x90, 0x90, 0xE0,   //D
						0xF0, 0x80, 0xF0, 0x80, 0xF0,   //E
						0xF0, 0x80, 0xF0, 0x80, 0x80};  //F

	//store 8x10 pixel digit characters used by SCHIP
	uint8_t bigdigits[160] = {
		0xFF, 0xFF, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xFF, 0xFF,  //0
		0x18, 0x78, 0x78, 0x18, 0x18, 0x18, 0x18, 0x18, 0xFF, 0xFF,  //1
		0xFF, 0xFF, 0x03, 0x03, 0xFF, 0xFF, 0xC0, 0xC0, 0xFF, 0xFF,  //2
		0xFF, 0xFF, 0x03, 0x03, 0xFF, 0xFF, 0x03, 0x03, 0xFF, 0xFF,  //3
		0xC3, 0xC3, 0xC3, 0xC3, 0xFF, 0xFF, 0x03, 0x03, 0x03, 0x03,  //4
		0xFF, 0xFF, 0xC0, 0xC0, 0xFF, 0xFF, 0x03, 0x03, 0xFF, 0xFF,  //5
		0xFF, 0xFF, 0xC0, 0xC0, 0xFF, 0xFF, 0xC3, 0xC3, 0xFF, 0xFF,  //6
		0xFF, 0xFF, 0x03, 0x03, 0x06, 0x0C, 0x18, 0x18, 0x18, 0x18,  //7
		0xFF, 0xFF, 0xC3, 0xC3, 0xFF, 0xFF, 0xC3, 0xC3, 0xFF, 0xFF,  //8
		0xFF, 0xFF, 0xC3, 0xC3, 0xFF, 0xFF, 0x03, 0x03, 0xFF, 0xFF,  //9
		0x7E, 0xFF, 0xC3, 0xC3, 0xC3, 0xFF, 0xFF, 0xC3, 0xC3, 0xC3,  //A
		0xFC, 0xFC, 0xC3, 0xC3, 0xFC, 0xFC, 0xC3, 0xC3, 0xFC, 0xFC,  //B
		0x3C, 0xFF, 0xC3, 0xC0, 0xC0, 0xC0, 0xC0, 0xC3, 0xFF, 0x3C,  //C
		0xFC, 0xFE, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xFE, 0xFC,  //D
		0xFF, 0xFF, 0xC0, 0xC0, 0xFF, 0xFF, 0xC0, 0xC0, 0xFF, 0xFF,  //E
		0xFF, 0xFF, 0xC0, 0xC0, 0xFF, 0xFF, 0xC0, 0xC0, 0xC0, 0xC0}; //F

	pc = 0x200;
	sp = 0;
	address = 0;

	delaytimer = 0;
	soundtimer = 0;

	timerStart = (timerValue >= 0) ? timerValue : 0;
	curTime = timerStart;

	for(int i = 0; i < 4096; i++)
	{
		if(i < 80)
		{
			data[i] = digits[i]; //put digit characters at beginning of memory
		}
		else if(i < 240)
		{
			data[i] = bigdigits[i]; //followed by the large digit characters
		}
		else
		{
			data[i] = 0; //initialize the rest of memeory to 0
		}
	}

	for(int i = 0; i < 16; i++)
	{
		keystates[i] = false;
		registers[i] = 0;
		stack[i] = 0;
	}

	for(int i = 0; i < 8; i++)
	{
		hp48flags[i] = 0;
	}

	for(int i = 0; i < screenwidth; i++)
	{
		for(int j = 0; j < screenheight; j++)
		{
			screendata[i][j] = 0;
		}
	}

	srand(time(0));
}

int Chip8Core::coreLoop()
{
	bool extraTime = false;
	bool invalidOpcode = false;

	if(mode == 0)
	{
		cyclespersecond = 10; //600 instructions/second
	}
	else
	{
		cyclespersecond = 30; //1800 instructions/second
	}

	if(opcodesthissecond < cyclespersecond)
	{
		if(pc < 4096)
		{
			opcode = data[pc];
			opcode <<= 8;
			opcode |= data[pc + 1];
			pc += 2;
			if(decodeOpcode(opcode) == 1)
			{
				//if set to quit on error, quit
				printf("invalid opcode discovered: 0x%x%x\n", data[pc - 2], data[pc - 1]);
				invalidOpcode = true;
			}
		}
		else
		{
			printf("end of memory reached");
			return 1;
		}

		opcodesthissecond++;
	}
	else
	{
		extraTime = true;
	}

	if(curTime - timerStart >= (1000 / 60))
	{
		opcodesthissecond = 0;

		if(delaytimer > 0)
		{
			delaytimer -= 1;
		}
		if(soundtimer > 0)
		{
			soundtimer -= 1;
		}

		timerStart = curTime;
	}

	if(invalidOpcode)
	{
		return 1;
	}
	if(extraTime)
	{
		return 2;
	}

	return 0;
}

int Chip8Core::loadProgram(std::string filename)
{
	std::ifstream file(filename, std::ios::binary | std::ios::ate);
	if(!file.is_open())
	{
		printf("%s could not be opened\n", filename.c_str());
		return 1;
	}

	int size = file.tellg();
	if(size > 0xFFF - 0x200)
	{
		printf("%s is too large\n", filename.c_str());
		return 1;
	}

	file.seekg(0, std::ios::beg);
	file.read(reinterpret_cast<char*>(&data[0x200]), size);
	file.close();

	return 0;
}

void Chip8Core::setKeyState(uint8_t keynum, bool ispressed)
{
	if(keynum < 16)
	{
		keystates[keynum] = ispressed;
	}
}

void Chip8Core::setTimer(uint32_t curTimeValue)
{
	curTime = (curTimeValue >= 0) ? curTimeValue : 0;
}

uint8_t Chip8Core::getSoundTimer()
{
	return soundtimer;
}

uint8_t Chip8Core::getMode()
{
	return mode;
}

inline void Chip8Core::pushStack(uint16_t callingaddress)
{
	stack[sp] = callingaddress;
	sp++;
}

inline uint16_t Chip8Core::popStack()
{
	sp--;
	return stack[sp];
}

//draws a chip-8 sprite to the screen array
void Chip8Core::drawSprite(uint8_t vx, uint8_t vy, uint8_t n)
{
	uint8_t byte;

	registers[15] = 0;

	if(mode == 0) //CHIP8 mode
	{
		if(n == 0)
		{
			n = 16;
		}

		for(int y = 0; y < n; y++)
		{
			byte = data[address + y];
			for(int x = 0; x < 8; x++)
			{
				if(vx + x < screenwidth * .5 && vx + x >= 0 && vy + y < screenheight * .5 && vy + y >= 0)
				{
					if(byte & (0x80 >> x))
					{
						if(screendata[(vx + x) * 2][(vy + y) * 2])
						{
							registers[15] = 1;
						}
						screendata[(vx + x) * 2][(vy + y) * 2] ^= 1;
						screendata[((vx + x) * 2) + 1][(vy + y) * 2] ^= 1;
						screendata[(vx + x) * 2][((vy + y) * 2) + 1] ^= 1;
						screendata[((vx + x) * 2) + 1][((vy + y) * 2) + 1] ^= 1;
					}
				}
			}
		}
	}
	else //SCHIP mode
	{
		if(n == 0)
		{
			for(int y = 0; y < 16; y++)
			{
				byte = data[address + (y * 2)];
				for(int x = 0; x < 8; x++)
				{
					if(byte & (0x80 >> x))
					{
						if(vx + x < screenwidth && vx + x >= 0 && vy + y < screenheight && vy + y >= 0)
						{
							if(screendata[vx + x][vy + y])
							{
								registers[15] = 1;
							}
							screendata[vx + x][vy + y] ^= 1;
						}
					}
				}
				byte = data[address + 1 + (y * 2)];
				for(int x = 0; x < 8; x++)
				{
					if(byte & (0x80 >> x))
					{
						if(vx + x + 8 < screenwidth && vx + x + 8 >= 0 && vy + y < screenheight && vy + y >= 0)
						{
							if(screendata[vx + x + 8][vy + y])
							{
								registers[15] = 1;
							}
							screendata[vx + x + 8][vy + y] ^= 1;
						}
					}
				}
			}
		}
		else
		{
			for(int y = 0; y < n; y++)
			{
				byte = data[address + y];
				for(int x = 0; x < 8; x++)
				{
					if(byte & (0x80 >> x))
					{
						if(vx + x < screenwidth && vx + x >= 0 && vy + y < screenheight && vy + y >= 0)
						{
							if(screendata[vx + x][vy + y])
							{
								registers[15] = 1;
							}
							screendata[vx + x][vy + y] ^= 1;
						}
					}
				}
			}
		}
	}
}

//convert a register to BCD and store it beginning at I
inline void Chip8Core::storeRegisterAsBCD(int value)
{
	for(int i = 2; i >= 0; i--)
	{
		data[address + i] = value % 10;
		value *= .1;
	}
}

bool Chip8Core::keyIsDown(uint8_t keynum)
{
	return keystates[keynum];
}

int Chip8Core::decodeOpcode(uint16_t opcode)
{
	bool invalid = false;

	switch(opcode & 0xF000)
	{
		case 0x0000:
			if((opcode & 0xFFF0) == 0x00C0)
			{
				//00CN - scroll screen N lines down - SCHIP
				uint8_t n = (opcode & 0x000F);
				for(int i = screenheight - 1; i >= n; i--)
				{
					for(int j = 0; j < screenwidth; j++)
					{
						screendata[j][i] = screendata[j][i - n];
					}
				}
				for(int i = 0; i < n; i++)
				{
					for(int j = 0; j < screenwidth; j++)
					{
						screendata[j][i] = 0;
					}
				}
				break;
			}
			switch(opcode & 0x00FF)
			{
				case 0x00E0:
					//00E0 - clear screen
					for(int i = 0; i < screenwidth; i++)
					{
						for(int j = 0; j < screenheight; j++)
						{
							screendata[i][j] = 0;
						}
					}
					break;
				case 0x00EE:
					//00EE - return from subroutine
					pc = popStack();
					break;
				case 0x00FB:
					//00FB - scroll screen 4 lines right - SCHIP
					for(int i = 0; i < screenheight; i++)
					{
						for(int j = screenwidth - 1; j > 3; j--)
						{
							screendata[j][i] = screendata[j - 4][i];
						}
						screendata[0][i] = 0;
						screendata[1][i] = 0;
						screendata[2][i] = 0;
						screendata[3][i] = 0;
					}
					break;
				case 0x00FC:
					//00FC - scroll screen 4 lines left - SCHIP
					for(int i = 0; i < screenheight; i++)
					{
						for(int j = 0; j < screenwidth - 4; j++)
						{
							screendata[j][i] = screendata[j + 4][i];
						}
						screendata[screenwidth - 4][i] = 0;
						screendata[screenwidth - 3][i] = 0;
						screendata[screenwidth - 2][i] = 0;
						screendata[screenwidth - 1][i] = 0;
					}
					break;
				case 0x00FD:
					//00FD - stop emulator - SCHIP
					running = false;
					break;
				case 0x00FE:
					//00FE - disable extended screen mode - SCHIP
					mode = 0;
					break;
				case 0x00FF:
					//00FF - enable extended screen mode - SCHIP
					mode = 1;
					break;
				default:
					//0NNN - no longer used
					break;
			}
			break;
		case 0x1000:
			//1NNN - jump to address NNN
			pc = opcode & 0x0FFF;
			break;
		case 0x2000:
			//2NNN - call subroutine at NNN
			pushStack(pc);
			pc = opcode & 0x0FFF;
			break;
		case 0x3000:
			//3XNN - skip next instruction if VX == NN
			if(registers[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
			{
				pc += 2;
			}
			break;
		case 0x4000:
			//4XNN - skip next instruction if VX != NN
			if(registers[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
			{
				pc += 2;
			}
			break;
		case 0x5000:
			//5XY0 - skip next instruction if VX == VY
			if(registers[(opcode & 0x0F00) >> 8] == registers[(opcode & 0x00F0) >> 4])
			{
				pc += 2;
			}
			break;
		case 0x6000:
			//6XNN - set VX to NN
			registers[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);
			break;
		case 0x7000:
			//7XNN - add NN to VX
			registers[(opcode & 0x0F00) >> 8] += (opcode & 0x00FF);
			break;
		case 0x8000:
			switch(opcode & 0x000F)
			{
				case 0x0000:
					//8XY0 - set VX to the value of VY
					registers[(opcode & 0x0F00) >> 8] = registers[(opcode & 0x00F0) >> 4];
					break;
				case 0x0001:
					//8XY1 - set VX to VX OR VY
					registers[(opcode & 0x0F00) >> 8] |= registers[(opcode & 0x00F0) >> 4];
					break;
				case 0x0002:
					//8XY2 - set VX to VX AND VY
					registers[(opcode & 0x0F00) >> 8] &= registers[(opcode & 0x00F0) >> 4];
					break;
				case 0x0003:
					//8XY3 - set VX to VX XOR VY
					registers[(opcode & 0x0F00) >> 8] ^= registers[(opcode & 0x00F0) >> 4];
					break;
				case 0x0004:
					//8XY4 - add VY to VX, VF = 1 if carry, otherwise VF = 0
					if(registers[(opcode & 0x0F00) >> 8] + registers[(opcode & 0x00F0) >> 4] > 255)
					{
						registers[15] = 1;
					}
					else
					{
						registers[15] = 0;
					}
					registers[(opcode & 0x0F00) >> 8] += registers[(opcode & 0x00F0) >> 4];
					break;
				case 0x0005:
					//8XY5 - subtract VY from VX, VF = 0 if borrow, otherwise VF = 1
					if(registers[(opcode & 0x0F00) >> 8] < registers[(opcode & 0x00F0) >> 4])
					{
						registers[15] = 0;
					}
					else
					{
						registers[15] = 1;
					}
					registers[(opcode & 0x0F00) >> 8] -= registers[(opcode & 0x00F0) >> 4];
					break;
				case 0x0006:
					//8XY6 - shift VX right by 1, VF = LSB of VX before shift
					registers[15] = registers[(opcode & 0x0F00) >> 8] & 0x01;
					registers[(opcode & 0x0F00) >> 8] >>= 1;
					break;
				case 0x0007:
					//8XY7 - VX = VY - VX, VF = 0 if borrow, otherwise VF = 1
					if(registers[(opcode & 0x00F0) >> 4] < registers[(opcode & 0x0F00) >> 8])
					{
						registers[15] = 0;
					}
					else
					{
						registers[15] = 1;
					}
					registers[(opcode & 0x0F00) >> 8] = registers[(opcode & 0x00F0) >> 4] - registers[(opcode & 0x0F00) >> 8];
					break;
				case 0x000E:
					//8XYE - shift VX left by 1, VF = MSB of VX before shift
					registers[15] = (registers[(opcode & 0x0F00) >> 8] >> 7) & 0x01;
					registers[(opcode & 0x0F00) >> 8] <<= 1;
					break;
				default:
					invalid = true;
					break;
			}
			break;
		case 0x9000:
			//9XY0 - skip next instruction if VX != VY
			if(registers[(opcode & 0x0F00) >> 8] != registers[(opcode & 0x00F0) >> 4])
			{
				pc += 2;
			}
			break;
		case 0xA000:
			//ANNN - set I to address NNN
			address = opcode & 0x0FFF;
			break;
		case 0xB000:
			//BNNN - jump to address NNN + V0
			pc = (opcode & 0x0FFF) + registers[0];
			break;
		case 0xC000:
			//CXNN - set VX to random number AND NN
			registers[(opcode & 0x0F00) >> 8] = rand() & (opcode & 0x00FF);
			break;
		case 0xD000:
			//DXYN - draw sprite at VX, VY with width of 8 and height of N
			drawSprite(registers[(opcode & 0x0F00) >> 8], registers[(opcode & 0x00F0) >> 4], opcode & 0x000F);
			break;
		case 0xE000:
			switch(opcode & 0x00FF)
			{
				case 0x009E:
					//EX9E - skip next instruction if key stored in VX is pressed
					if(keyIsDown(registers[(opcode & 0x0F00) >> 8]))
					{
						pc += 2;
					}
					break;
				case 0x00A1:
					//EXA1 - skip next instruction if key stored in VX isn't pressed
					if(!keyIsDown(registers[(opcode & 0x0F00) >> 8]))
					{
						pc += 2;
					}
					break;
				default:
					invalid = true;
					break;
			}
			break;
		case 0xF000:
			switch(opcode & 0x00FF)
			{
				case 0x0007:
					//FX07 - set VX to value of delay timer
					registers[(opcode & 0x0F00) >> 8] = delaytimer;
					break;
				case 0x000A:
					//FX0A - wait for keypress, then store it in VX
					{
						bool waitingforkeypress = true;
						for(int i = 0; i < 16; i++)
						{
							if(keyIsDown(i))
							{
								registers[(opcode & 0x0F00) >> 8] = i;
								waitingforkeypress = false;
								break;
							}
						}
						if(waitingforkeypress)
						{
							pc -= 2;
						}
					}
					break;
				case 0x0015:
					//FX15 - set delay timer to VX
					delaytimer = registers[(opcode & 0x0F00) >> 8];
					break;
				case 0x0018:
					//FX18 - set sound timer to VX
					soundtimer = registers[(opcode & 0x0F00) >> 8];
					break;
				case 0x001E:
					//FX1E - add VX to I
					if(address + registers[(opcode & 0x0F00) >> 8] > 0xfff)
					{
						registers[15] = 1;
					}
					else
					{
						registers[15] = 0;
					}
					address += registers[(opcode & 0x0F00) >> 8];
					break;
				case 0x0029:
					//FX29 - set I to the location of the sprite for the character in VX
					address = registers[(opcode & 0x0F00) >> 8] * 5; //stored at beginning of memory
					break;
				case 0x0030:
					//FX30 - set I to the location of the 10 byte sprite for the character in VX
					address = (registers[(opcode & 0x0F00) >> 8] * 10) + 80;
					break;
				case 0x0033:
					//FX33 - store BCD representation of VX in I, I + 1, and I + 2
					storeRegisterAsBCD(registers[(opcode & 0x0F00) >> 8]);
					break;
				case 0x0055:
					//FX55 - store V0 to VX in memory starting at I
					for(int i = 0; i <= ((opcode & 0x0F00) >> 8); i++)
					{
						data[address + i] = registers[i];
					}
					break;
				case 0x0065:
					//FX65 - fill V0 to VX with values from memory starting at I
					for(int i = 0; i <= ((opcode & 0x0F00) >> 8); i++)
					{
						registers[i] = data[address + i];
					}
					break;
				case 0x0075:
					//FX75 - store V0 to VX in hp48flags - SCHIP
					for(int i = 0; i <= ((opcode & 0x0F00) >> 8); i++)
					{
						hp48flags[i] = registers[i];
					}
					break;
				case 0x0085:
					//FX85 - load V0 to VX from hp48flags - SCHIP
					for(int i = 0; i <= ((opcode & 0x0F00) >> 8); i++)
					{
						registers[i] = hp48flags[i];
					}
					break;
				default:
					invalid = true;
					break;
			}
			break;
		default:
			invalid = true;
			break;
	}

	if(invalid)
	{
		return 1;
	}

	return 0;
}