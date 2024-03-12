#ifndef NOTCHFILTER_H
#define NOTCHFILTER_H
#include "FilterBase.h"
namespace BackyardBrains{
    class NotchFilter : public FilterBase
    {
        public:
            NotchFilter();
            void calculateCoefficients();
            void setCenterFrequency(double newCenterFrequency);
            void setQ(double newQ);
        protected:
            double centerFrequency;
            double Q;
        private:
    };
}
#endif // NOTCHFILTER_H
