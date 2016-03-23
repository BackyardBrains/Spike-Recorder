#ifndef NOTCHFILTER_H
#define NOTCHFILTER_H
#include "FilterBase.h"
namespace BackyardBrains{
    class NotchFilter : public FilterBase
    {
        public:
            NotchFilter();
            void calculateCoefficients();
            void setCenterFrequency(float newCenterFrequency);
            void setQ(float newQ);
        protected:
            float centerFrequency;
            float Q;
        private:
    };
}
#endif // NOTCHFILTER_H
