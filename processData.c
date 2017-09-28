// E155 Final Project
// Hill Balliet - wballiet@g.hmc.edu - Dec 2, 2016

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SAMPERSEC 2000
#define DATALENGTH (int)(SAMPERSEC*1.2)
#define NUMTOAS 25
#define OFFSET 380

typedef struct node {
    double val;
    struct node *next;
    struct node *prev;
} node_t;

typedef struct linkedList {
    node_t *first;
    node_t *last;
    int numSamples;
} linkedList_t;

// Keep track of time of arrivals
linkedList_t Toas;

// Keep track of mic data, filtered mic data, and ordered filtered mic data
linkedList_t DataShiftReg;
linkedList_t BassData;
linkedList_t SquarAvData;
double *OrderedMicData[DATALENGTH];

// Keeps track of the current time in sample number since the beginning
long CurTime = 0;

/// Initialize a new node with the given value
node_t* initNode(double value) {
    node_t *newData = (node_t *) malloc(sizeof(node_t));
    newData->val = value;
    newData->next = NULL;
    newData->prev = NULL;

    return newData;
}

/// Deletes the first node from the global data list and returns that node
// The popped node needs to be deallocated because destroy list will no longer
// deallocate it now that it's not in the list
node_t* popFirst(linkedList_t *list) {
    node_t *oldNode = list->first;

    list->first = oldNode->next;
    if (list->first != NULL)
        list->first->prev = NULL;

    list->numSamples--;

    return oldNode;
}

/// Adds the given element to the end of the global data list
void pushBack(linkedList_t *list, node_t *newNode) {
    if (list->first == NULL) 
        list->numSamples = 0;

    if (list->numSamples > 0) {
        list->last->next = newNode;
        newNode->prev = list->last;
    }
    else {
        list->first = newNode;
    }
    
    list->last = newNode;


    list->numSamples++;
}

/// Adds the given element to the ordered data array in sorted order
void addOrdered(double *newData, double *oldData) {
    int newDataIndex = DATALENGTH - SquarAvData.numSamples;
    double *temp[DATALENGTH];
    char added = 0;

    for (int oldDataIndex = DATALENGTH - SquarAvData.numSamples; oldDataIndex < DATALENGTH; oldDataIndex++) {
        if (oldDataIndex == DATALENGTH - 1 && !added) {
            temp[newDataIndex] = newData;
        }
        // Skip the old data
        else if (oldData == OrderedMicData[oldDataIndex]) {
            newDataIndex--;
        }
        // Insert the new data in the middle and advance only in new array
        else if (*newData < *OrderedMicData[oldDataIndex] && !added) {
            temp[newDataIndex] = newData;
            oldDataIndex--;
            added = 1;
        }
        // Copy over
        else {
            temp[newDataIndex] = OrderedMicData[oldDataIndex];
        }
        newDataIndex++;
    }

    // Copy the ordered data back into the real array
    for (int i = DATALENGTH - SquarAvData.numSamples; i < DATALENGTH; i++) {
        OrderedMicData[i] = temp[i];
    }
}

/**
* Applies a 10th order elliptic filter with 1 dB of passband ripple, 60dB
* to stopband, and wc = 200 Hz to the raw mic data.
*
* From 
* https://www.mathworks.com/help/signal/ug/filter-implementation-and-analysis.html
* "This is the standard time-domain representation of a digital filter, computed 
* starting with y(1) and assuming a causal system with zero initial conditions. 
* This representation's progression is
*
* y(1)=b(1)x(1)
* y(2)=b(1)x(2)+b(2)x(1)−a(2)y(1)
* y(3)=b(1)x(3)+b(2)x(2)+b(3)x(1)−a(2)y(2)−a(3)y(1)
* ⋮"                 
* 
* **Note that Matlab indexes from 1 as opposed to C, which indexes from 0**
*
*
* As calculated by Matlab, using: [b, a] = ellip(10, 1, 60, 0.1*2);
* b = [0.0024, -0.0131, 0.0372, -0.0700, 0.0984, -0.1093, 0.0984, -0.0700,
*       0.0372, -0.0131, 0.0024]
* 
* a = [1.0000, -8.1884, 31.3052, -73.3202, 116.2455, -130.1808, 104.2046, 
*       -58.8533, 22.4489, -5.2256, 0.5644]
* 
*/
double bassFilter() {
    // Initialize constants that define the filter.
    const int numFilterPoints = 11;
    double y = 0;

    double b[numFilterPoints] = {0.0023828536056377,-0.0130958334492431,0.0372141533382333,-0.0700301623207206,0.0983815662084311,-0.1093276527751890,0.0983815662084310,-0.0700301623207205,0.0372141533382332,-0.0130958334492431,0.0023828536056377};
 
    double a[numFilterPoints] = {1.0000000000000000,-8.1883658956990786,31.3051598439003200,-73.3201552438257380,116.2455336759196882,-130.1807939032020158,104.2045701322850846,-58.8532949438235136,22.4489181172146317,-5.2255736205161751,0.5644254019455265};

    
    // Apply the filter to the current data as described above
    node_t *curX = DataShiftReg.last;
    node_t *curY = BassData.last;
    
    for (int i = 0; i < fmin(DataShiftReg.numSamples, numFilterPoints); i++) {
        if (i == 0) {
            y += (double)curX->val * b[i];
            curX = curX->prev;
        }
        else {
            y += ((double)curX->val * b[i]) - ((double)curY->val * a[i]);
            curX = curX->prev;
            curY = curY->prev;
        }
    }
    return y;
}

/**
* Squares the signal and takes a numPoints point moving average.
*/
double squarAvFilter() {
    double sum = 0;
    int numPoints = SAMPERSEC/20;

    node_t *curNode = BassData.last;

    for (int i = 0; i < fmin(BassData.numSamples, numPoints); i++) {
        sum += curNode->val * curNode->val;
        curNode = curNode->prev;
    }

    return sum/(double)fmin(BassData.numSamples, numPoints);
}

void storeSample(int micData) {
    // subtract out the offset from the ADC
    micData -= OFFSET;

    // Init the elements in ordered array to 0 so that sorting works
    if (SquarAvData.numSamples == 0) {
        for (int i = 0; i < DATALENGTH; i++) {
            OrderedMicData[i] = 0;
        }
    }

    // Initialize the new data point and shift it in
    node_t *newData = initNode(micData);
    pushBack(&DataShiftReg, newData);
    
    // Initialize the new bass data point and shift it in
    node_t *newBassData = initNode(bassFilter());
    pushBack(&BassData, newBassData);
    
    // Initialize the new squarAv data point and shift it in
    node_t *newSquarAvData = initNode(squarAvFilter());
    pushBack(&SquarAvData, newSquarAvData);
    
    // Remove old data and put the new data into the ordered array
    if (DataShiftReg.numSamples > DATALENGTH) {

        node_t *oldData = popFirst(&DataShiftReg);
        node_t *oldBassData = popFirst(&BassData);
        node_t *oldSquarAvData = popFirst(&SquarAvData);
        
        addOrdered(&(newSquarAvData->val), &(oldSquarAvData->val));
    
        // Deallocate the memory for the old data.
        free(oldData);
        free(oldBassData);
        free(oldSquarAvData);
    }
    else {
        node_t *oldData = NULL;

        addOrdered(&(newSquarAvData->val), &(oldData->val));
    }

    CurTime++;
}

/// Deallocates all of the memory associated with the list
void destroyLists() {
    while (DataShiftReg.numSamples != 0) {
        node_t *oldData = popFirst(&DataShiftReg);
        free(oldData);
    }
    while (BassData.numSamples != 0) {
        node_t *oldBassData = popFirst(&BassData);
        free(oldBassData);
    }
    while (SquarAvData.numSamples != 0) {
        node_t *oldSquarAvData = popFirst(&SquarAvData);
        free(oldSquarAvData);
    }
}