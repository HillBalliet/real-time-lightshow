// E155 Final Project
// Hill Balliet - wballiet@g.hmc.edu - Dec 3, 2016

#include "micData.c"

// Since songs will not go above 250 BPM we can be sure that there will
// not be more than one beat per 0.15 seconds
#define WAITTIME (int)(SAMPERSEC*0.15)

// This function returns all pulses found in a sampled signal based on a
// pre-set threshold.
// It takes in a batch of the last samples and the sampling rate and outputs
// the times that each beat in the batch of samples occurred
void beatFinder() {
        
    // Crossing the quantile mark means it's a pulse
    float quantile = 0.95;
    double threshold = *OrderedMicData[(int)round(quantile*DATALENGTH)];
    
    node_t *data = SquarAvData.last;

    // Loop through each sample in SquarAvData checking for pulses    
    for (int i = CurTime; i > fmax(Toas.last->val + WAITTIME, CurTime - SquarAvData.numSamples + 1); i--) {

        // If the signal crosses a threshold mark, it's a pulse
        if (data->prev->val < threshold && data->val > threshold) {
            // Shift in the time of the peak as the most recent toa
            node_t *newToa = initNode((double)i);
            pushBack(&Toas, newToa);

            // Shift out the old toa if we already have NUMTOAS toas
            if (Toas.numSamples > NUMTOAS) {
                node_t *oldToa = popFirst(&Toas);
                free(oldToa);
            }

            break;
        }

        data = data->prev;      
    }
}

int isBeat(float sampTilBeat, float uncertainty) {
    float stdevs = fabsf(sampTilBeat/uncertainty);

    // If the linearized rate of change of the signal exceeds threshold it
    // counts as a pulse.  threshold will be quantile(0.9) when nextBeat = 0 and 
    // go up by stdevs^2 so that 1 uncertainty/stdev away, the threshold is
    // 0.97 and two uncertainties/stdevs away the threshold is capped at the max
    float quantile = fmin(0.07*stdevs*stdevs+ 0.9, 1.0F-1.0F/(float)DATALENGTH);
    double threshold = *OrderedMicData[(int)round(quantile*DATALENGTH)];

    // We only want to check the current state of beat or not beat
    node_t *curData = SquarAvData.last;

    // If the signal is crossing the quantile
    if (curData->val > threshold && curData->prev->val < threshold)
        return(1);
    else
        return(0);
}
