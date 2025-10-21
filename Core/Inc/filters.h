#ifndef FILTERS_H
#define FILTERS_H

#include <stdint.h>

typedef struct {
    float x, y, z;
} Vector3f_t;

typedef struct {
    float alpha;
    Vector3f_t filtered;
    uint8_t initialized;
} LowPassFilter_t;

void LowPassFilter_Init(LowPassFilter_t *filter, float alpha);
void LowPassFilter_Update(LowPassFilter_t *filter, int16_t x, int16_t y, int16_t z);
Vector3f_t LowPassFilter_Get(LowPassFilter_t *filter);

#endif
