#define uint8_t unsigned char
#define uint32_t unsigned int

typedef struct __attribute__ ((packed))
{
    uint8_t R;
    uint8_t G;
    uint8_t B;
    uint8_t align;
} RGB;


__kernel void histogram(__global const RGB *data, __global uint32_t *R, __global uint32_t *G, __global uint32_t *B){
    // Get the index of the current element to be processed
    int i = get_global_id(0);

    RGB pixel = data[i];

	atomic_inc(&R[pixel.R]);
	atomic_inc(&G[pixel.G]);
	atomic_inc(&B[pixel.B]);
}