// E155 Final Project
// Hill Balliet - wballiet@g.hmc.edu - Dec 4, 2016
//
// TOAS stands for Time Of Arrivals, referring to pulses detected by beatFinding

#include "bpmestimation.c"
#include "micData.c"
#include "beatFinding.c"

#define ISBEATPIN 25

int main(void) {
    // The time between each sample in microseconds
    int sampPeriod = 1000000/SAMPERSEC;

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

    // There won't be beats closer together than WAITTIME in typical music
    int wait = 0;
    
    while(true) {
        int beat = 0;

        // Measure current time
        unsigned int startTime = sys_timer[1];

        // Get a new sample from the ADC and store it in the data structures
        sample();

        // get current signal power
        char power = (char)round(SquarAvData.last->val);
        
        // get color data from the web interface to the FPGA
        char colors[3];
        FILE *colorFile = fopen("~/.webcolors.txt", "w");
        fgets(colors, 3, colorFile);
        fclose(colorFile);
        
        // Send current signal power and color data
        sendColorsPower(colors[0], colors[1], colors[2], power);

        // find toas
        beatFinder();

        // predict number of samples until the next beat
        float sampTilBeat = 0;
        float uncertainty = 10000000;

        bpmestimation(&sampTilBeat, &uncertainty);
        //float bpm = bpmestimation(&sampTilBeat, &uncertainty); // If want bpm


        // check if we are currently at a beat
        if (wait == 0)
            beat = isBeat(sampTilBeat, uncertainty, wait);

            if (beat)
                wait = WAITTIME;
        else
            wait--;

        // let the FPGA know if we found a beat
        digitalWrite(ISBEATPIN, beat);

        // Delay until the next sample should be taken
        unsigned int elapsed = sys_timer[1] - startTime;
        delayMicroseconds(sampPeriod - elapsed);
    }
}

// Collects data from the ADC and stores it in the data structures
void sample(void) {
    // Collect data
    int newAudio = getAudio();

    // store data
    storeSample(newAudio);
}