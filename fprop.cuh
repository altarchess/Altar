#include "Tinn.h"
Tinn xtloadgpu(const char* const path);
float* xtpredictgpu(const Tinn t, const float* const in);
void xtfreegpu(Tinn t);