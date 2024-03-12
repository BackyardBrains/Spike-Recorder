#ifndef LOWPASSFILTER_H
#define LOWPASSFILTER_H
#include "FilterBase.h"

namespace BackyardBrains{
    class LowPassFilter : public FilterBase
    {
        public:
            LowPassFilter();
            void calculateCoefficients();
            void setCornerFrequency(double newCornerFrequency);
            void setQ(double newQ);
        protected:
            double cornerFrequency;
            double Q;
        private:
    };
}


#endif // LOWPASSFILTER_H
