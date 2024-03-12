#ifndef FILTERBASE_H
#define FILTERBASE_H
 #include <stdint.h>

//
// Base class that is inherited by all filters
//
namespace BackyardBrains {
    class FilterBase {
        public:
            FilterBase();
            void initWithSamplingRate(double sr);
            void setCoefficients();
            void filterIntData(int16_t * data, int32_t numFrames,bool flush = false);
            void filterContiguousData( double * data, uint32_t numFrames, bool flush =false);
        protected:
        
            void intermediateVariables(double Fc, double Q);
            double zero, one;
            double samplingRate;
            double gInputKeepBuffer[2];
            double gOutputKeepBuffer[2];
            double omega, omegaS, omegaC, alpha;
            double coefficients[5];
            double a0, a1, a2, b0, b1, b2;
            int flushFilterValues;
        private:
    };

}
#endif // FILTERBASE_H
