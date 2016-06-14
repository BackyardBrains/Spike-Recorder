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
            void initWithSamplingRate(float sr);
            void setCoefficients();
            void filterIntData(int16_t * data, int32_t numFrames);
            void filterContiguousData( float * data, uint32_t numFrames);
        protected:
        
            void intermediateVariables(float Fc, float Q);
            float zero, one;
            float samplingRate;
            float gInputKeepBuffer[2];
            float gOutputKeepBuffer[2];
            float omega, omegaS, omegaC, alpha;
            float coefficients[5];
            float a0, a1, a2, b0, b1, b2;
        private:
    };

}
#endif // FILTERBASE_H
