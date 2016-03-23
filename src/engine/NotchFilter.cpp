#include "NotchFilter.h"
namespace BackyardBrains {
    NotchFilter::NotchFilter()
    {

    }

    void NotchFilter::calculateCoefficients()
    {
         if ((centerFrequency != 0.0f) && (Q != 0.0f)) {
                intermediateVariables(centerFrequency, Q);

                a0 = (1 + alpha);
                b0 = 1                      / a0;
                b1 = (-2 * omegaC)          / a0;
                b2 = 1                      / a0;
                a1 = (-2 * omegaC)          / a0;
                a2 = (1 - alpha)            / a0;
                setCoefficients();
        }
    }

    void NotchFilter::setCenterFrequency(float newCenterFrequency)
    {
            centerFrequency = newCenterFrequency;
            calculateCoefficients();
    }

    void NotchFilter::setQ(float newQ)
    {
        Q = newQ;
        calculateCoefficients();
    }
}
