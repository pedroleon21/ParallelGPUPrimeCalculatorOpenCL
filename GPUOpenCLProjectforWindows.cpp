#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include<time.h>

#include <CL/cl.h>


const char* ProgramSource ="__kernel void prime_calculator(__global uint *primes,const uint start) \n"\
"{\n"\
"   int i, g = get_global_id(0) + start;\n"\
"   for (i = 2; i * i <= g; i++) {\n"\
"       if (g % i == 0) {\n"\
"           primes[g] = 1;\n"\
"       }\n"\
"   }\n"\
"}\n"\
"\n";


int eprimo(int n) {
    for (int i = 2; i * i <= n; i++) {
        if (n % i == 0) {
            return 0;
        }
    }
    return 1;
}

int distance(int start, int final) {
    return final - start;
}

int main(void)
{
    cl_context context;
    cl_context_properties properties[3];
    cl_kernel kernel;
    cl_command_queue command_queue;
    cl_program program;
    cl_int err;
    cl_uint num_of_platforms = 0;
    cl_platform_id platform_id;
    cl_device_id device_id;
    cl_uint num_of_devices = 0;
    cl_mem primes;
    size_t global[2];
    size_t local[2];
    int start,final,range;
    int block_size = 4,*host_primes=NULL;
    scanf("%d %d", &start, &final); // sugest divid by 100 number
    range = distance(start, final);
    if (clGetPlatformIDs(1, &platform_id, &num_of_platforms) != CL_SUCCESS)
    {
        printf("Unable to get platform id\n");
        return 1;
    }

    if (clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_id, &num_of_devices) != CL_SUCCESS)
    {
        printf("Unable to get device_id\n");
        return 2;
    }

    properties[0] = CL_CONTEXT_PLATFORM;
    properties[1] = (cl_context_properties)platform_id;
    properties[2] = 0;

    context = clCreateContext(properties, 1, &device_id, NULL, NULL, &err);

    command_queue = clCreateCommandQueue(context, device_id, 0, &err);

    program = clCreateProgramWithSource(context, 1, (const char**)&ProgramSource, NULL, &err);

    if (clBuildProgram(program, 0, NULL, NULL, NULL, NULL) != CL_SUCCESS)
    {
        printf("Error building program\n");
        return 3;
    }

    kernel = clCreateKernel(program, "prime_calculator", &err);

    host_primes = (int*)malloc(sizeof(int) * range);

    for (int i = 0; i < range; i++) host_primes[i] = 0;

    primes = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int) * range, NULL, NULL);

    clEnqueueWriteBuffer(command_queue, primes , CL_TRUE, 0, sizeof(int) , host_primes , 0, NULL, NULL);

    clSetKernelArg(kernel, 0, sizeof(cl_mem), &primes);
    clSetKernelArg(kernel, 1, sizeof(int), &start);
    global[0] = range;
    local[0] = block_size;
    local[1] = block_size;

    clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, global, local, 0, NULL, NULL);
    clFlush(command_queue);
    clFinish(command_queue);

    //reading from GPU
    clEnqueueReadBuffer(command_queue, primes, CL_TRUE, 0, sizeof(int) * range, host_primes, 0, NULL, NULL);
    
    //logica para coletar os primos
    for (int i = 0; i < range; i++) {
        if (!host_primes[i])
            printf("\n%d %d %d", host_primes[i], i + start, eprimo(i + start));
    }
    
    clReleaseMemObject(primes);
    clReleaseProgram(program);
    clReleaseKernel(kernel);
    clReleaseCommandQueue(command_queue);
    clReleaseContext(context);
    free(host_primes);
    system("pause");
    return 0;
}