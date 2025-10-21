#include "calibration.h"
#include <math.h>
#include <stdio.h>

#define LSB_TO_MICROTESLA (100.0f / 3000.0f)

void Calibration_Init(MagCalibration_t *cal)
{
    cal->offset_x = 0.0f;
    cal->offset_y = 0.0f;
    cal->offset_z = 0.0f;
    cal->scale_x = 1.0f;
    cal->scale_y = 1.0f;
    cal->scale_z = 1.0f;
    cal->is_calibrated = 0;
}

void Calibration_Start(CalibrationData_t *data)
{
    data->min_x = 32767;
    data->min_y = 32767;
    data->min_z = 32767;
    data->max_x = -32768;
    data->max_y = -32768;
    data->max_z = -32768;
    data->sample_count = 0;
    data->in_progress = 1;
    
    printf("Calibration started! Rotate sensor in figure-8 pattern...\r\n");
}

void Calibration_Update(CalibrationData_t *data, int16_t x, int16_t y, int16_t z)
{
    if (!data->in_progress) 
		return;
    
    if (x < data->min_x)
		data->min_x = x;
    if (y < data->min_y)
		data->min_y = y;
    if (z < data->min_z)
		data->min_z = z;
    
    if (x > data->max_x) 
		data->max_x = x;
    if (y > data->max_y) 
		data->max_y = y;
    if (z > data->max_z) 
		data->max_z = z;
    
    ++(data->sample_count);
}

void Calibration_Finish(CalibrationData_t *data, MagCalibration_t *cal)
{
    if (!data->in_progress) 
		return;
    
    data->in_progress = 0;
    
    printf("\r\nCalibration data collected: %d samples\r\n", data->sample_count);
    printf("X: min=%d, max=%d\r\n", data->min_x, data->max_x);
    printf("Y: min=%d, max=%d\r\n", data->min_y, data->max_y);
    printf("Z: min=%d, max=%d\r\n", data->min_z, data->max_z);
    
    cal->offset_x = (float)(data->max_x + data->min_x) / 2.0f;
    cal->offset_y = (float)(data->max_y + data->min_y) / 2.0f;
    cal->offset_z = (float)(data->max_z + data->min_z) / 2.0f;
    
    float radius_x = (float)(data->max_x - data->min_x) / 2.0f;
    float radius_y = (float)(data->max_y - data->min_y) / 2.0f;
    float radius_z = (float)(data->max_z - data->min_z) / 2.0f;
    
    float avg_radius = (radius_x + radius_y + radius_z) / 3.0f;
    
    cal->scale_x = avg_radius / radius_x;
    cal->scale_y = avg_radius / radius_y;
    cal->scale_z = avg_radius / radius_z;
    
    cal->is_calibrated = 1;
    
    printf("\r\nCalibration complete!\r\n");
    printf("Offsets: X=%.1f, Y=%.1f, Z=%.1f\r\n", 
           cal->offset_x, cal->offset_y, cal->offset_z);
    printf("Scales: X=%.3f, Y=%.3f, Z=%.3f\r\n", 
           cal->scale_x, cal->scale_y, cal->scale_z);
}

void Calibration_Apply(MagCalibration_t *cal, int16_t raw_x, int16_t raw_y, int16_t raw_z,
                       float *cal_x, float *cal_y, float *cal_z)
{
    if (cal->is_calibrated) 
	{
        *cal_x = ((float)raw_x - cal->offset_x) * cal->scale_x;
        *cal_y = ((float)raw_y - cal->offset_y) * cal->scale_y;
        *cal_z = ((float)raw_z - cal->offset_z) * cal->scale_z;
    } 
	else 
	{
        *cal_x = (float)raw_x;
        *cal_y = (float)raw_y;
        *cal_z = (float)raw_z;
    }
}

void Calibration_LoadDefaults(MagCalibration_t *cal)
{
    cal->offset_x = CALIBRATION_OFFSET_X;
    cal->offset_y = CALIBRATION_OFFSET_Y;
    cal->offset_z = CALIBRATION_OFFSET_Z;
    cal->scale_x = CALIBRATION_SCALE_X;
    cal->scale_y = CALIBRATION_SCALE_Y;
    cal->scale_z = CALIBRATION_SCALE_Z;
    cal->is_calibrated = 1;
    
    printf("Loaded default calibration values\r\n");
}

float Calibration_GetFieldStrength(float x, float y, float z)
{
    float magnitude_lsb = sqrtf(x*x + y*y + z*z);
    float magnitude_ut = magnitude_lsb * LSB_TO_MICROTESLA;
    return magnitude_ut;
}

float Calibration_GetEarthFieldStrength(MagCalibration_t *cal)
{
    float avg_radius = (cal->offset_x + cal->offset_y + cal->offset_z) / 3.0f;
    return avg_radius * LSB_TO_MICROTESLA;
}
