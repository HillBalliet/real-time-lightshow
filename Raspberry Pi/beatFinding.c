// E155 Final Project
// beatFinding.c
// Hill Balliet - wballiet@g.hmc.edu - Dec 3, 2016
//
// Finds the beat of the song using the processed song data

// Since songs will not go above 250 BPM we can be sure that there will
// not be more than one beat per 0.15 seconds
#define WAITTIME (int)(SAMPERSEC*0.15)

// Determines whether there is currently a beat or not
int isBeat() {
    float quantile = 0.95;

    int offset = (int)roundf((1.0-quantile)*(SquarAvData.numSamples)) - 1;
    double threshold = *OrderedMicData[DATALENGTH - offset];

    // We only want to check the current state of beat or not beat
    node_t *curData = SquarAvData.last;

    // If the signal is crossing the quantile
    if (curData->val > threshold && curData->prev->val < threshold)
        return(1);
    else
        return(0);
}
