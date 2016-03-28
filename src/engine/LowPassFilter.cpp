#include "LowPassFilter.h"

namespace BackyardBrains {
    LowPassFilter::LowPassFilter()
    {

    }

    void LowPassFilter::calculateCoefficients()
    {
        if ((cornerFrequency != 0.0f) && (Q != 0.0f))
        {
            intermediateVariables(cornerFrequency, Q);


            a0 = 1 + alpha;
            b0 = ((1 - omegaC)/2)      / a0;
            b1 = ((1 - omegaC))        / a0;
            b2 = ((1 - omegaC)/2)      / a0;
            a1 = (-2 * omegaC)         / a0;
            a2 = (1 - alpha)           / a0;

            setCoefficients();
        }
    }

    void LowPassFilter::setCornerFrequency(float newCornerFrequency)
    {
            cornerFrequency = newCornerFrequency;
            calculateCoefficients();
    }

    void LowPassFilter::setQ(float newQ)
    {
        Q = newQ;
        calculateCoefficients();
    }
}
