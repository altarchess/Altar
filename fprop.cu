#include <iostream>
#include <math.h>




#include "Tinn.h"
#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <cuda_runtime_api.h>




__global__ void proph(Tinn t, const float* const in) {
    int i = threadIdx.x;
    float sum = 0.0f;
        for (int j = 0; j < t.nips; j++) {
            sum += in[j] * t.w[i * t.nips + j];
        }
        t.h[i] = 1.0f / (1.0f + expf(-(sum + t.b[0])));
}

__global__ 
void propo(Tinn t) {
    int i = threadIdx.x;
    float sum = 0.0f;
       for (int j = 0; j < t.nhid; j++)
           sum += t.h[j] * t.x[i * t.nhid + j];
       t.o[i] = 1.0f / (1.0f + expf(-(sum + t.b[1])));
 
}

Tinn xtbuildgpu (const int nips, const int nhid, const int nops)
{
    Tinn t;
    t.nb = 2;
    t.nw = nhid * (nips + nops);
    cudaMallocManaged(&t.w, t.nw * sizeof(*t.w));
    t.x = t.w + nhid * nips;
    cudaMallocManaged(&t.b, t.nb * sizeof(*t.b));
    cudaMallocManaged(&t.h, nhid * sizeof(*t.h));
    cudaMallocManaged(&t.o, nops * sizeof(*t.o));


    t.nips = nips;
    t.nhid = nhid;
    t.nops = nops;
    return t;
}


Tinn xtloadgpu (const char* const path)
{
    FILE* const file = fopen(path, "r");
    int nips = 0;
    int nhid = 0;
    int nops = 0;
    fscanf(file, "%d %d %d\n", &nips, &nhid, &nops);
    const Tinn t = xtbuildgpu(nips, nhid, nops);
    for (int i = 0; i < t.nb; i++) fscanf(file, "%f\n", &t.b[i]);
    for (int i = 0; i < t.nw; i++) fscanf(file, "%f\n", &t.w[i]);
    fclose(file);
    return t;
}
void xtfpropgpu(const Tinn t, const float* const in)
{



    proph<<<1, 64 >>> (t, in);
   // cudaDeviceSynchronize();
    propo<<<1, 1>>> (t);
    cudaDeviceSynchronize();
}
float* xtpredictgpu(const Tinn t, const float* const in)
{
    xtfpropgpu(t, in);
    return t.o;
}

void xtfreegpu(Tinn t) {
    cudaFree(t.w);
    cudaFree(t.b);
    cudaFree(t.h);
    cudaFree(t.o);
}
