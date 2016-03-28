#ifndef HIGHPASSFILTER_H
#define HIGHPASSFILTER_H
#include "FilterBase.h"

namespace BackyardBrains{
    class HighPassFilter : public FilterBase
    {
        public:
            HighPassFilter();
            void calculateCoefficients();
            void setCornerFrequency(float newCornerFrequency);
            void setQ(float newQ);
        protected:
            float cornerFrequency;
            float Q;
        private:
    };
}

#endif // HIGHPASSFILTER_H
