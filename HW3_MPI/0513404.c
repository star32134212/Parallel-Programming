#include <stdio.h> 
#include <stdlib.h> 
#include <mpi.h>
#ifndef W 
#define W 20 // Width 
#endif 
int main(int argc, char **argv) {
     int rank,size;
     int L = atoi(argv[1]);// Length 
     int iteration = atoi(argv[2]);// Iteration 
     srand(atoi(argv[3]));// Seed 
     float d = (float) random() / RAND_MAX * 0.2; // Diffusivity 擴散性
     int *temp = malloc(L*W*sizeof(int)); // Current temperature 
     int *next = malloc(L*W*sizeof(int)); // Next time step 
     MPI_Init(&argc, &argv);
     MPI_Comm_rank(MPI_COMM_WORLD, &rank);
     MPI_Comm_size(MPI_COMM_WORLD, &size);
     MPI_Status status[2];
     MPI_Request request[2];


    
     int count = 0, balance = 0 , test = 0; 
     int local_start = ( L / size ) * rank;
     int local_end = ( L / size ) * (rank + 1);
    
     MPI_Bcast(&d, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
     if(rank != 0){
        MPI_Recv( &temp[ local_start * W ], L / size * W, MPI_INT,  0, 0, MPI_COMM_WORLD, &status[0]);   

     }else{
        for (int i = 0; i < L; i++) { 
            for (int j = 0; j < W; j++) { 
                temp[i*W+j] = random()>>3; //往右推3個bits
            } 
        }
        for (int r = 1; r < size; r++){
            MPI_Send( &temp[ r * L * W / size], L / size * W, MPI_INT,  r, 0, MPI_COMM_WORLD);   
        }
     }


     while (iteration--) { // Compute with up, left, right, down points 
        if(rank > 0){ 
            MPI_Irecv( &temp[ ( local_start - 1 ) * W], W , MPI_INT, rank -1 , 0, MPI_COMM_WORLD, &request[0]);
        }
        if(rank < size - 1 ){ //知道source有-1 所以這裏補+1
            MPI_Irecv( &temp[ local_end * W ], W , MPI_INT, rank + 1, 0, MPI_COMM_WORLD, &request[1]);
        }
        if(rank > 0){ //0~7 送出1~7 原本第一個(rank=0)不送 接收方會有7個 是0~7 所以rank-1
            MPI_Send( &temp[ local_start * W ], W , MPI_INT, rank -1 , 0, MPI_COMM_WORLD);
        }
        if(rank < size - 1 ){ 
            MPI_Send( &temp[ ( local_end - 1 ) * W], W , MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
        }
        if(rank > 0){
            MPI_Wait(&request[0], &status[0]);
        }
        if(rank < size - 1 ){
            MPI_Wait(&request[1], &status[1]);
        }        
        balance = 1; 
        count++; 
        for (int i = local_start; i < local_end; i++) { 
            for (int j = 0; j < W; j++) { 
                float t = temp[i*W+j] / d; 
                t += temp[i*W+j] * -4; 
                t += temp[(i - 1 < 0 ? 0 : i - 1) * W + j]; 
                t += temp[(i + 1 >= L ? i : i + 1)*W+j]; 
                t += temp[i*W+(j - 1 < 0 ? 0 : j - 1)]; 
                t += temp[i*W+(j + 1 >= W ? j : j + 1)]; 
                t *= d; 
                next[i*W+j] = t ; 
                if (next[i*W+j] != temp[i*W+j]) { 
                    balance = 0; 
                } 
            } 
        }

        MPI_Allreduce( &balance , &test, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
        if (test != 0) { 
            break; 
        }
        int *tmp = temp; 
        temp = next; 
        next = tmp; 

    } 
    int local_min = temp[L / size * rank * W]; 
    int total_min;
    for (int i = L / size * rank; i < L / size * (rank + 1); i++) { 
        for (int j = 0; j < W; j++) { 
            if (temp[i*W+j] < local_min) { 
                local_min = temp[i*W+j]; 
            } 
        } 
    } 
    MPI_Reduce(&local_min, &total_min, 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);
    MPI_Finalize();
    if( rank == 0 ){
        printf("Size: %d*%d, Iteration: %d, Min Temp: %d\n", L, W, count, total_min); 
    }
    return 0; 
} 