#include "HighPassFilter.h"
namespace BackyardBrains {
    HighPassFilter::HighPassFilter()
    {

    }

    void HighPassFilter::calculateCoefficients()
    {
        if ((cornerFrequency != 0.0f) && (Q != 0.0f))
        {
            intermediateVariables(cornerFrequency, Q);


            a0 = 1 + alpha;
            b0 = ((1 + omegaC)/2)      / a0;
            b1 = (-1*(1 + omegaC))     / a0;
            b2 = ((1 + omegaC)/2)      / a0;
            a1 = (-2 * omegaC)         / a0;
            a2 = (1 - alpha)           / a0;

            setCoefficients();
        }
    }

    void HighPassFilter::setCornerFrequency(double newCornerFrequency)
    {
            cornerFrequency = newCornerFrequency;
            calculateCoefficients();
    }

    void HighPassFilter::setQ(double newQ)
    {
        Q = newQ;
        calculateCoefficients();
    }
}
