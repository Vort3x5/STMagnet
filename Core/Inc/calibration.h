#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <stdint.h>

#define CALIBRATION_OFFSET_X  1092.5f
#define CALIBRATION_OFFSET_Y   478.5f
#define CALIBRATION_OFFSET_Z  -543.0f
#define CALIBRATION_SCALE_X    0.905f
#define CALIBRATION_SCALE_Y    0.768f
#define CALIBRATION_SCALE_Z    1.684f

typedef struct {
    float offset_x;
    float offset_y;
    float offset_z;
    float scale_x;
    float scale_y;
    float scale_z;
    uint8_t is_calibrated;
} MagCalibration_t;

typedef struct {
    int16_t min_x, min_y, min_z;
    int16_t max_x, max_y, max_z;
    uint16_t sample_count;
    uint8_t in_progress;
} CalibrationData_t;

void Calibration_Init(MagCalibration_t *cal);
void Calibration_Start(CalibrationData_t *data);
void Calibration_Update(CalibrationData_t *data, int16_t x, int16_t y, int16_t z);
void Calibration_Finish(CalibrationData_t *data, MagCalibration_t *cal);
void Calibration_Apply(MagCalibration_t *cal, int16_t raw_x, int16_t raw_y, int16_t raw_z, 
                       float *cal_x, float *cal_y, float *cal_z);
void Calibration_LoadDefaults(MagCalibration_t *cal);

float Calibration_GetFieldStrength(float x, float y, float z);
float Calibration_GetEarthFieldStrength(MagCalibration_t *cal);

#endif
