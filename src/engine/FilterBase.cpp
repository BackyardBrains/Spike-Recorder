#include "FilterBase.h"
#include <math.h>
#include <cstdlib>
#include <cstring>

namespace BackyardBrains {
FilterBase::FilterBase() {

}

void FilterBase::initWithSamplingRate(double sr)
{
        samplingRate = sr;

        for (int i = 0; i < 5; i++) {
            coefficients[i] = 0.0f;
        }

        gInputKeepBuffer[0] = 0.0f;
        gInputKeepBuffer[1] = 0.0f;
        gOutputKeepBuffer[0] = 0.0f;
        gOutputKeepBuffer[1] = 0.0f;

        zero = 0.0f;
        one = 1.0f;
}

void FilterBase::setCoefficients()
{
    coefficients[0] = b0;
    coefficients[1] = b1;
    coefficients[2] = b2;
    coefficients[3] = a1;
    coefficients[4] = a2;
}

//
// Filter integer data buffer
//
void FilterBase::filterIntData(int16_t * data, int32_t numFrames, bool flush)
{
    double *tempdoubleBuffer = (double*) std::malloc(numFrames * sizeof(double));
    for(int32_t i=numFrames-1;i>=0;i--)
    {
        tempdoubleBuffer[i] = (double) data[i];
    }
    filterContiguousData(tempdoubleBuffer, numFrames, flush);
    if(flush)
    {
        
        for(int32_t i=numFrames-1;i>=0;i--)
        {
            data[i] = 0;
        }
    }
    else
    {
        for(int32_t i=numFrames-1;i>=0;i--)
        {
            data[i] = (int16_t) tempdoubleBuffer[i];
        }
    }
    free(tempdoubleBuffer);
}

//
// Filter single channel data
//
void FilterBase::filterContiguousData( double * data, uint32_t numFrames, bool flush)
{
    // Provide buffer for processing
    double *tInputBuffer = (double*) std::malloc((numFrames + 2) * sizeof(double));
    double *tOutputBuffer = (double*) std::malloc((numFrames + 2) * sizeof(double));

    // Copy the data
    memcpy(tInputBuffer, gInputKeepBuffer, 2 * sizeof(double));
    memcpy(tOutputBuffer, gOutputKeepBuffer, 2 * sizeof(double));
    memcpy(&(tInputBuffer[2]), data, numFrames * sizeof(double));

    // Do the processing
    // vDSP_deq22(tInputBuffer, 1, coefficients, tOutputBuffer, 1, numFrames);
    //https://developer.apple.com/library/ios/documentation/Accelerate/Reference/vDSPRef/index.html#//apple_ref/c/func/vDSP_deq22
    int n;
    for(n=2;n<numFrames+2;n++)
    {
        tOutputBuffer[n] = tInputBuffer[n]*coefficients[0]+ tInputBuffer[n-1]*coefficients[1]+tInputBuffer[n-2]*coefficients[2] - tOutputBuffer[n-1]*coefficients[3]- tOutputBuffer[n-2]*coefficients[4];
    }
    
    // Copy the data
    memcpy(data, tOutputBuffer, numFrames * sizeof(double));
    memcpy(gInputKeepBuffer, &(tInputBuffer[numFrames]), 2 * sizeof(double));
    memcpy(gOutputKeepBuffer, &(tOutputBuffer[numFrames]), 2 * sizeof(double));

    free(tInputBuffer);
    free(tOutputBuffer);
}

void FilterBase::intermediateVariables(double Fc, double Q)
{
    omega = 2*M_PI*Fc/samplingRate;
    omegaS = sin(omega);
    omegaC = cos(omega);
    alpha = omegaS / (2*Q);
}








}
