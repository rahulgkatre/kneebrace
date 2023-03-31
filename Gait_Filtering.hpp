#ifndef GaitFiltering
#define GaitFiltering

#include <Arduino.h>
#include <Adafruit_BNO08x.h>
#include "Euler.hpp"

void quaternionToEuler(float qr, float qi, float qj, float qk, struct euler_t* ypr, bool degrees);
void quaternionToEulerRV(sh2_RotationVectorWAcc_t* rotational_vector, struct euler_t* ypr, bool degrees);
void quaternionToEulerGI(sh2_GyroIntegratedRV_t* rotational_vector, struct euler_t* ypr, bool degrees);

#endif
