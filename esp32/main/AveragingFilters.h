#ifndef AveragingFilters_h
#define AveragingFilters_h
#include "Arduino.h"

class IIR {
    public:
        float alpha;
        float y_prev;
        IIR(float a, float y0=0);
        float step(float input);
        void set(float new_val);
        void reset();
};

#endif
