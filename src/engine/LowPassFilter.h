#ifndef LOWPASSFILTER_H
#define LOWPASSFILTER_H
#include "FilterBase.h"

namespace BackyardBrains{
    class LowPassFilter : public FilterBase
    {
        public:
            LowPassFilter();
            void calculateCoefficients();
            void setCornerFrequency(float newCornerFrequency);
            void setQ(float newQ);
        protected:
            float cornerFrequency;
            float Q;
        private:
    };
}


#endif // LOWPASSFILTER_H
