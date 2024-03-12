#ifndef HIGHPASSFILTER_H
#define HIGHPASSFILTER_H
#include "FilterBase.h"

namespace BackyardBrains{
    class HighPassFilter : public FilterBase
    {
        public:
            HighPassFilter();
            void calculateCoefficients();
            void setCornerFrequency(double newCornerFrequency);
            void setQ(double newQ);
        protected:
            double cornerFrequency;
            double Q;
        private:
    };
}

#endif // HIGHPASSFILTER_H
