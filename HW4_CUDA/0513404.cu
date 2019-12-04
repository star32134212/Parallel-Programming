/* *********************************************************************
* DESCRIPTION: 
* Serial Concurrent Wave Equation - C Version 
* This program implements the concurrent wave equation 
*********************************************************************/ 
#include <stdio.h> 
#include <stdlib.h> 
#include <math.h> 
#include <time.h> 
#define MAXPOINTS 1000000 
#define MAXSTEPS 1000000 
#define MINPOINTS 20 
#define PI 3.14159265 

void check_param(void); 
void printfinal (void); 


int nsteps ,/* number of time steps */ 
    tpoints ,/* total points along string */ 
    rcode;/* generic return code */ 




/* *********************************************************************
* Checks input values from parameters 
*********************************************************************/ 
void check_param(void) 
{ 
    char tchar[20]; 
    /* check number of points , number of iterations */ 
    while (( tpoints < MINPOINTS) || (tpoints > MAXPOINTS)) { 
        printf("Enter number of points along vibrating string [%d-%d]: " ,MINPOINTS, MAXPOINTS); 
        scanf("%s", tchar); 
        tpoints = atoi(tchar); 
        if (( tpoints < MINPOINTS) || (tpoints > MAXPOINTS)) 
            printf("Invalid. Please enter value between %d and %d\n", MINPOINTS , MAXPOINTS); 
    } 
    while (( nsteps < 1) || (nsteps > MAXSTEPS)) { 
            printf("Enter number of time steps [1-%d]: ", MAXSTEPS); 
            scanf("%s", tchar); 
            nsteps = atoi(tchar); 
            if ((nsteps < 1) || (nsteps > MAXSTEPS)) 
                printf("Invalid. Please enter value between 1 and %d\n", MAXSTEPS); 
    } 
    printf("Using points = %d, steps = %d\n", tpoints , nsteps); 
    
} 
/* ********************************************************************** 
Initialize points on line 
*********************************************************************/ 
__global__ void init_line_kernel(float* oldval, float* values, int tpoints) { 
    int j; 
    float x, fac , k, tmp; 
    int thread_id = blockIdx.x * blockDim.x + threadIdx.x;
    int thread_num = blockDim.x * gridDim.x;
    
    /* Calculate initial values based on sine curve */ 
    fac = 2.0 * PI; 
    tmp = (float)(tpoints - 1); 
    for (j = thread_id; j <= tpoints; j += thread_num) { 
        if (j >= 0){
            k = (float)(j - 1);
            x = k/tmp; 
            values[j] = __sinf(fac * x); //GPU內建  __sinf
            oldval[j] = values[j]; 
        }

    } 
}

/* ********************************************************************** 
Update all values along line a specified number of times 
*********************************************************************/ 
__global__ void update_kernel(float* oldval, float* values, float* newval, int nsteps, int tpoints){
    int i, j; 
    int thread_id = blockIdx.x * blockDim.x + threadIdx.x;
    int thread_num = blockDim.x * gridDim.x;
    /* Update values for each time step */ 
    for (i = 1; i <= nsteps; i++) { 
    /* Update points along line for this time step */ 
        for (j = thread_id; j <= tpoints; j += thread_num) { 
            /* global endpoints */ 
            if ((j == 1) || (j == tpoints)){
                newval[j] = 0.0; 
            }else {
                /*********************************************************************** 
                Calculate new values using wave equation 
                *********************************************************************/ 
                float dtime , c, dx, tau , sqtau; 
                dtime = 0.3; 
                c = 1.0; 
                dx = 1.0; 
                tau = (c * dtime / dx); 
                sqtau = tau * tau; 
                newval[j] = (2.0 * values[j]) - oldval[j] + (sqtau * ( -2.0)*values[j]); 
            }
            oldval[j] = values[j]; 
            values[j] = newval[j]; 
        } 
    }
} 
/* ********************************************************************** 
Print final results 
*********************************************************************/ 
void printfinal (float* values, int tpoints) { 
    int i; 
    for (i = 1; i <= tpoints; i++) { 
        printf("%6.4f ", values[i]); 
        if (i%10 == 0) 
            printf("\n"); 
    } 
} 
/* ********************************************************************** 
Main program 
*********************************************************************/ 
int main(int argc , char *argv []) { 
    sscanf(argv[1],"%d" ,&tpoints); 
    sscanf(argv[2],"%d" ,&nsteps); 
    check_param(); 
    int threadinblock = 512;
    int blocknum = (tpoints + threadinblock - 1)/threadinblock; // 無條件補上
    //Host
    float *oldval, *values, *newval;
    oldval = (float*)malloc((tpoints + 2) * sizeof(float)); /* values at time (t-dt) */
    values = (float*)malloc((tpoints + 2) * sizeof(float)); /* values at time t */
    newval = (float*)malloc((tpoints + 2) * sizeof(float)); /* values at time (t+dt) */
    
    //Device
    float *gpu_oldval, *gpu_values, *gpu_newval;
    cudaMalloc(&gpu_oldval, (tpoints + 2) * sizeof(float));
    cudaMalloc(&gpu_values, (tpoints + 2) * sizeof(float));
    cudaMalloc(&gpu_newval, (tpoints + 2) * sizeof(float));



    printf("Initializing points on the line ...\n"); 
    init_line_kernel <<<blocknum, threadinblock>>> (gpu_oldval, gpu_values, tpoints);

    printf("Updating all points for all time steps ...\n"); 
    update_kernel <<<blocknum, threadinblock>>> (gpu_oldval, gpu_values, gpu_newval, nsteps, tpoints);

    //把資料送回Host
    cudaMemcpy(oldval, gpu_oldval, (tpoints + 2) * sizeof(float), cudaMemcpyDeviceToHost);
    cudaMemcpy(values, gpu_values, (tpoints + 2) * sizeof(float), cudaMemcpyDeviceToHost);
    cudaMemcpy(newval, gpu_newval, (tpoints + 2) * sizeof(float), cudaMemcpyDeviceToHost);

    printf("Printing final results ...\n"); 
    printfinal(values, tpoints); 
    printf("\nDone .\n\n"); 
    return 0; 
} 