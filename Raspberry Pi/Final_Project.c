// E155 Final Project
// Final_Project.c
// Hill Balliet - wballiet@g.hmc.edu - Dec 4, 2016
//
// Wraps the functionality of storing data, processing data, finding beats, and
// communicating with the FPGA.
//
// Compile with: 
// gcc -std=gnu11 -lm -lrt -o witnessMe -g Final_Project.c -Wall -Wextra

#include "micData.c"
#include "io.c"
#include "beatFinding.c"

#include <unistd.h>
#include <time.h>
#include <sched.h>

#define ISBEATPIN 19

struct timespec ts;

static void sleep_until(struct timespec *ts, int delay);
void sample(void);
void playSong(void);

// Initializes all of the necessary IO and executes all of the core
// functionality in the while loop in the proper order
int main(void) {

    // Init the I/O functionality
    pioInit();
    spioInit();
    sysTimerInit();
    pinMode(COLOR,OUTPUT);

    digitalWrite(COLOR,1);

    pinMode(8,ALT0);
    pinMode(9,ALT0);
    pinMode(10,ALT0);
    pinMode(11,ALT0);
    unsigned int freq = 500000;
    SPI0CLK = 250000000/freq;   // set SPI clock to 250MHz / freq
    pinMode(ISBEATPIN, OUTPUT); 
    fp = fopen("piMusic.wav","r");

    // There won't be beats closer together than WAITTIME in typical music
    int wait = 0;
	playSong();

    // System timing functionality taken from:
    // http://www.isy.liu.se/edu/kurs/TSEA81/lecture_linux_realtime.html
	// Set our thread to real time priority
	struct sched_param sp;
	sp.sched_priority = 1; // Must be > 0. Threads with a higher
	// value in the priority field will be schedueld before
	// threads with a lower number.  Linux supports up to 99, but
        // the POSIX standard only guarantees up to 32 different
	// priority levels.

	// Note: If you are not using pthreads, use the
	// sched_setscheduler call instead.
        if (sched_setscheduler(0, SCHED_FIFO, & sp) != 0) {
              perror("sched_setscheduler");
              exit(EXIT_FAILURE);  
        }	

    clock_gettime(CLOCK_MONOTONIC, &ts);

    while(1) {
        int beat = 0;

        // Get a new sample from the ADC and store it in the data structures
        sample();
        
        // get current signal power
        char power = (char)round(SquarAvData.last->val);
        
        // get color data from the web interface to the FPGA
        char colors[3];
        FILE *colorFile = fopen("/home/pi/.webcolors.txt", "r");
	    fscanf(colorFile, "%6x", (unsigned int*)colors);
        fclose(colorFile);

        // Send current signal power and color data
        sendColorsPower(colors[2], colors[1], colors[0], power);

        // check if we are currently at a beat
        if (wait == 0) {
            beat = isBeat();

            if (beat) {
                wait = WAITTIME;
	        }
	    }
        else {
            wait--;
	    }
        // let the FPGA know if we found a beat
        digitalWrite(ISBEATPIN, beat);

        // Delay until the next sample should be taken
	    sleep_until(&ts, 500000);	
    }
}

// Collects data from the ADC and stores it in the data structures
void sample(void) {
    // Collect data
    int newAudio = 380 + getAudioSample();

    // store data
    storeSample(newAudio);
}


// System timing functionality taken from:
// http://www.isy.liu.se/edu/kurs/TSEA81/lecture_linux_realtime.html
// Adds "delay" nanoseconds to timespecs and sleeps until that time
static void sleep_until(struct timespec *ts, int delay)
{
	
	ts->tv_nsec += delay;
	if(ts->tv_nsec >= 1000*1000*1000){
		ts->tv_nsec -= 1000*1000*1000;
		ts->tv_sec++;
	}
	clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, ts,  NULL);
}

// Author: James Palmer
void playSong(void) {
    system("aplay actualMusic.wav &");
}
