#include "filters.h"
#include <string.h>

void LowPassFilter_Init(LowPassFilter_t *filter, float alpha)
{
    filter->alpha = alpha;
    filter->filtered.x = 0.0f;
    filter->filtered.y = 0.0f;
    filter->filtered.z = 0.0f;
    filter->initialized = 0;
}

void LowPassFilter_Update(LowPassFilter_t *filter, int16_t x, int16_t y, int16_t z)
{
    if (!filter->initialized) 
	{
        filter->filtered.x = (float)x;
        filter->filtered.y = (float)y;
        filter->filtered.z = (float)z;
        filter->initialized = 1;
    } 
	else 
	{
        filter->filtered.x = filter->alpha * x + (1.0f - filter->alpha) * filter->filtered.x;
        filter->filtered.y = filter->alpha * y + (1.0f - filter->alpha) * filter->filtered.y;
        filter->filtered.z = filter->alpha * z + (1.0f - filter->alpha) * filter->filtered.z;
    }
}
