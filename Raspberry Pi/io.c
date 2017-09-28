
// E155 Lab 6
// James Palmer

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

/////////////////////////////////////////////////////////////////////
// From lab6.c
/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
// Constants
/////////////////////////////////////////////////////////////////////

// GPIO FSEL Types
#define INPUT  0
#define OUTPUT 1
#define ALT0   4
#define ALT1   5
#define ALT2   6
#define ALT3   7
#define ALT4   3
#define ALT5   2

#define GPFSEL   ((volatile unsigned int *) (gpio + 0))
#define GPSET    ((volatile unsigned int *) (gpio + 7))
#define GPCLR    ((volatile unsigned int *) (gpio + 10))
#define GPLEV    ((volatile unsigned int *) (gpio + 13))
#define INPUT  0
#define OUTPUT 1

/////////////////////////////////////////////////////////////////////
// System Timer Registers
/////////////////////////////////////////////////////////////////////

typedef struct
{
	unsigned M0		:1;
	unsigned M1 	:1;
	unsigned M2 	:1;
	unsigned M3 	:1;
	unsigned 		:28;
}sys_timer_csbits;
#define SYS_TIMER_CSbits (*(volatile sys_timer_csbits*) (sys_timer + 0))
#define SYS_TIMER_CS 	(* (volatile unsigned int*)(sys_timer + 0))

#define SYS_TIMER_CLO   (* (volatile unsigned int*)(sys_timer + 1))
#define SYS_TIMER_CHI   (* (volatile unsigned int*)(sys_timer + 2))
#define SYS_TIMER_C0	(* (volatile unsigned int*)(sys_timer + 3))
#define SYS_TIMER_C1	(* (volatile unsigned int*)(sys_timer + 4))
#define SYS_TIMER_C2	(* (volatile unsigned int*)(sys_timer + 5))
#define SYS_TIMER_C3	(* (volatile unsigned int*)(sys_timer + 6))

// Physical addresses
#define BCM2836_PERI_BASE        0x3F000000
#define GPIO_BASE               (BCM2836_PERI_BASE + 0x200000)
#define SPI0_BASE		(BCM2836_PERI_BASE + 0x204000)
#define TIMER_BASE		(BCM2836_PERI_BASE + 0x3000)
#define BLOCK_SIZE (4*1024)
volatile unsigned int * sys_timer; //????
// Pointers that will be memory mapped when pioInit() is called
volatile unsigned int *gpio; //pointer to base of gpio
//volatile unsigned int *sys_timer; //pointer to base of sys_timer
volatile unsigned int *spi;

void pioInit() {
	int  mem_fd;
	void *reg_map;

	// /dev/mem is a psuedo-driver for accessing memory in the Linux filesystem
	if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
	      printf("can't open /dev/mem \n");
	      exit(-1);
	}

	reg_map = mmap(
	  NULL,             //Address at which to start local mapping (null means don't-care)
      BLOCK_SIZE,       //Size of mapped memory block
      PROT_READ|PROT_WRITE,// Enable both reading and writing to the mapped memory
      MAP_SHARED,       // This program does not have exclusive access to this memory
      mem_fd,           // Map to /dev/mem
      GPIO_BASE);       // Offset to GPIO peripheral

	if (reg_map == MAP_FAILED) {
      printf("gpio mmap error %d\n", (int)reg_map);
      close(mem_fd);
      exit(-1);
    }

	gpio = (volatile unsigned *)reg_map;
}


void spioInit() {
	int  mem_fd;
	void *reg_map;

	// /dev/mem is a psuedo-driver for accessing memory in the Linux filesystem
	if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
	      printf("can't open /dev/mem \n");
	      exit(-1);
	}

	reg_map = mmap(
	  NULL,      //Address at which to start local mapping (null=don't-care)
      5*4,       //Size of mapped memory block
      PROT_READ|PROT_WRITE,// Enable both read and write to the mapped memory
      MAP_SHARED,       // Program doesn't have exclusive access to this memory
      mem_fd,           // Map to /dev/mem
      SPI0_BASE);       // Offset to GPIO peripheral

	if (reg_map == MAP_FAILED) {
      printf("spio mmap error %d\n", (int)reg_map);
      close(mem_fd);
      exit(-1);
    }

	spi = (volatile unsigned *)reg_map;
}

/////////////////////////////////////////////////////////////////////
// From lab7 Givens
/////////////////////////////////////////////////////////////////////

typedef struct
{
	unsigned CS 		:2;
	unsigned CPHA		:1;
	unsigned CPOL		:1;
	unsigned CLEAR 		:2;
	unsigned CSPOL		:1;
	unsigned TA 		:1;
	unsigned DMAEN		:1;
	unsigned INTD 		:1;
	unsigned INTR 		:1;
	unsigned ADCS		:1;
	unsigned REN 		:1;
	unsigned LEN 		:1;
	unsigned LMONO 		:1;
	unsigned TE_EN		:1;
	unsigned DONE		:1;
	unsigned RXD		:1;
	unsigned TXD		:1;
	unsigned RXR 		:1;
	unsigned RXF 		:1;
	unsigned CSPOL0 	:1;
	unsigned CSPOL1 	:1;
	unsigned CSPOL2 	:1;
	unsigned DMA_LEN	:1;
	unsigned LEN_LONG	:1;
	unsigned 			:6;
}spi0csbits;
#define SPI0CSbits (* (volatile spi0csbits*) (spi + 0))   
#define SPI0CS (* (volatile unsigned int *) (spi + 0))

#define SPI0FIFO (* (volatile unsigned int *) (spi + 1))
#define SPI0CLK (* (volatile unsigned int *) (spi + 2))
#define SPI0DLEN (* (volatile unsigned int *) (spi + 3))


void pinMode(int pin, int fcn) {
	int reg = pin/10;
	int depth = 3*(pin % 10);
	int mask = 0b111;
	GPFSEL[reg] = (GPFSEL[reg] & ~(mask << depth)) | ((mask & fcn) << depth);
}

void digitalWrite(int pin, int val) {
	int reg = pin/32;
	int depth = pin%32;
	if(val)
		GPSET[reg] = GPSET[reg] | (1 << depth);
	else
		GPCLR[reg] = GPCLR[reg] | (1 << depth);
}

int digitalRead(int pin) {
	int reg = pin/32;
	int depth = pin%32;
	return (GPLEV[reg]>>depth) & 1;
}

void sysTimerInit() {
	int  mem_fd;
	void *reg_map;

	// /dev/mem is a psuedo-driver for accessing memory in the Linux filesystem
	if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
	      printf("can't open /dev/mem \n");
	      exit(-1);
	}

	reg_map = mmap(
	  NULL,
	7*4,       //Size of mapp
	PROT_READ|PROT_WRITE,// Enable both reading and writing to the mapped memory
      MAP_SHARED,       // Program does not have exclusive access to this memory
      mem_fd,           // Map to /dev/mem
      TIMER_BASE);       // Offset to GPIO peripheral

	if (reg_map == MAP_FAILED) {
      printf("gpio mmap error %d\n", (int)reg_map);
      close(mem_fd);
      exit(-1);
    }

	sys_timer = (volatile unsigned *)reg_map;
}

//maybe should rewrite...
char spiSendReceive(char send){
    SPI0FIFO = send;            // send data to slave
    while(!SPI0CSbits.DONE);    // wait until SPI transmission complete
    return SPI0FIFO;            // return received data
}

short spiSendReceive16(short send) {
    short rec;
    SPI0CSbits.TA = 1;          // turn SPI on with the "transfer active" bit
    rec = spiSendReceive((send & 0xFF00) >> 8); // send data MSB first
    rec = (rec << 8) | spiSendReceive(send & 0xFF);
    SPI0CSbits.TA = 0;          // turn off SPI
    return rec;
}

/////////////////////////////////////////////////////////////////////
// Created for the MicroPs Final Project
/////////////////////////////////////////////////////////////////////

#define COLOR 6

short getAudioSample(){
	short returnval[1];
	fread(returnval, 2, 1, fp);
	return *returnval;
}

void sendColor(char col) {
	
	spiSendReceive(col);
}

void sendColorsPower(char red, char green, char blue, char pow) {

	SPI0CSbits.TA = 1;
	digitalWrite(COLOR,0);
	sendColor(red);  
	sendColor(green);
	sendColor(blue);
	sendColor(pow);  //can treat pow as a color
	digitalWrite(COLOR,1);
	SPI0CSbits.TA = 0; 

}
