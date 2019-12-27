#include <fstream>
#include <iostream>
#include <string>
#include <ios>
#include <string.h>
#include <stdio.h>
#include <vector>
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

typedef struct
{
    uint8_t R;
    uint8_t G;
    uint8_t B;
    uint8_t align;
} RGB;

typedef struct
{
    bool type;
    uint32_t size;
    uint32_t height;
    uint32_t weight;
    RGB *data;
} Image;

Image *readbmp(const char *filename)
{
    std::ifstream bmp(filename, std::ios::binary);
    char header[54];
    bmp.read(header, 54);
    uint32_t size = *(int *)&header[2];
    uint32_t offset = *(int *)&header[10];
    uint32_t w = *(int *)&header[18];
    uint32_t h = *(int *)&header[22];
    uint16_t depth = *(uint16_t *)&header[28];
    if (depth != 24 && depth != 32)
    {
        printf("we don't suppot depth with %d\n", depth);
        exit(0);
    }
    bmp.seekg(offset, bmp.beg);

    Image *ret = new Image();
    ret->type = 1;
    ret->height = h;
    ret->weight = w;
    ret->size = w * h;
    ret->data = new RGB[w * h]{};
    for (int i = 0; i < ret->size; i++)
    {
        bmp.read((char *)&ret->data[i], depth / 8);
    }
    return ret;
}

int writebmp(const char *filename, Image *img)
{

    uint8_t header[54] = {
        0x42,        // identity : B
        0x4d,        // identity : M
        0, 0, 0, 0,  // file size
        0, 0,        // reserved1
        0, 0,        // reserved2
        54, 0, 0, 0, // RGB data offset
        40, 0, 0, 0, // struct BITMAPINFOHEADER size
        0, 0, 0, 0,  // bmp width
        0, 0, 0, 0,  // bmp height
        1, 0,        // planes
        32, 0,       // bit per pixel
        0, 0, 0, 0,  // compression
        0, 0, 0, 0,  // data size
        0, 0, 0, 0,  // h resolution
        0, 0, 0, 0,  // v resolution
        0, 0, 0, 0,  // used colors
        0, 0, 0, 0   // important colors
    };

    // file size
    uint32_t file_size = img->size * 4 + 54;
    header[2] = (unsigned char)(file_size & 0x000000ff);
    header[3] = (file_size >> 8) & 0x000000ff;
    header[4] = (file_size >> 16) & 0x000000ff;
    header[5] = (file_size >> 24) & 0x000000ff;

    // width
    uint32_t width = img->weight;
    header[18] = width & 0x000000ff;
    header[19] = (width >> 8) & 0x000000ff;
    header[20] = (width >> 16) & 0x000000ff;
    header[21] = (width >> 24) & 0x000000ff;

    // height
    uint32_t height = img->height;
    header[22] = height & 0x000000ff;
    header[23] = (height >> 8) & 0x000000ff;
    header[24] = (height >> 16) & 0x000000ff;
    header[25] = (height >> 24) & 0x000000ff;

    std::ofstream fout;
    fout.open(filename, std::ios::binary);
    fout.write((char *)header, 54);
    fout.write((char *)img->data, img->size * 4);
    fout.close();
    return 0;
}

cl_int err;
cl_device_id device_id;
cl_platform_id platform_id;
cl_context myctx;
cl_command_queue myqueue;
cl_program myprogram;
cl_mem d_img, d_r, d_g, d_b;
cl_kernel mykernel;
Image *img;
uint32_t R[256];
uint32_t G[256];
uint32_t B[256];

/*
void histogram(Image *img,uint32_t R[256],uint32_t G[256],uint32_t B[256]){
    std::fill(R, R+256, 0);
    std::fill(G, G+256, 0);
    std::fill(B, B+256, 0);

    for (int i = 0; i < img->size; i++){
        RGB &pixel = img->data[i];
        R[pixel.R]++;
        G[pixel.G]++;
        B[pixel.B]++;
    }
}
*/

cl_program load_program(cl_context context, const char* filename){
	std::ifstream in(filename, std::ios_base::binary);
	if(!in.good()){
		return 0;
	}

	// get file length
	in.seekg(0, std::ios_base::end);
	size_t length = in.tellg();
	in.seekg(0, std::ios_base::beg);

	// read program source
	std::vector<char> data(length + 1);
	in.read(&data[0], length);
	data[length] = 0;

	// create and build program 
	const char* source = &data[0];
	cl_program program = clCreateProgramWithSource(context, 1, &source, 0, 0);

	if(program == 0){
		return 0;
	}

	err = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);

	if(err != CL_SUCCESS){
		/*
        size_t len = 0;
		cl_int ret = CL_SUCCESS;
		ret = clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &len);
		char *buffer = new char[len]();//calloc(len, sizeof(char));
		err = clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, len, buffer, NULL);
		//err = clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 1024, buildlog, NULL);
		printf("buildlog: %s\n", buffer);
		delete[] buffer;
        */
		return 0;
	}

	return program;
}

void init_opencl(){ // 準備環境
	err = clGetPlatformIDs(1, &platform_id, NULL); // only 1 platform 
	
	err = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_id, NULL); // only 1 device
	
	myctx = clCreateContext(NULL, 1, &device_id, NULL, NULL, &err); // create context
	
	myqueue = clCreateCommandQueue(myctx, device_id, 0, &err); // create command queue
	
	myprogram = load_program(myctx, "histogram.cl"); // load program
	
	mykernel = clCreateKernel(myprogram, "histogram", &err); // create kernel

}


void setup_and_run_opencl(){
    // buffer memory
	d_img = clCreateBuffer(myctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(RGB) * img->size, img->data, &err); // create buffer for input image (global memory)
	
	d_r = clCreateBuffer(myctx, CL_MEM_WRITE_ONLY, sizeof(uint32_t) * 256, NULL, &err); // create buffer for output R (global memory)

	d_g = clCreateBuffer(myctx, CL_MEM_WRITE_ONLY, sizeof(uint32_t) * 256, NULL, &err); // create buffer for output G (global memory)

	d_b = clCreateBuffer(myctx, CL_MEM_WRITE_ONLY, sizeof(uint32_t) * 256, NULL, &err); // create buffer for output B (global memory)

	//err = clEnqueueWriteBuffer(myqueue, d_img, CL_TRUE, 0, sizeof(Image), img, 0, NULL, NULL); // copy data to buffer

	// 先在buffer塞滿0
	uint32_t zero = 0;
	err = clEnqueueFillBuffer(myqueue, d_r, &zero, sizeof(uint32_t), 0, sizeof(uint32_t) * 256, 0, NULL, NULL); 

	err = clEnqueueFillBuffer(myqueue, d_g, &zero, sizeof(uint32_t), 0, sizeof(uint32_t) * 256, 0, NULL, NULL);

	err = clEnqueueFillBuffer(myqueue, d_b, &zero, sizeof(uint32_t), 0, sizeof(uint32_t) * 256, 0, NULL, NULL);
	
    // 設定 kernel 變數
	err = clSetKernelArg(mykernel, 0, sizeof(cl_mem), &d_img); 

	err = clSetKernelArg(mykernel, 1, sizeof(cl_mem), &d_r);

	err = clSetKernelArg(mykernel, 2, sizeof(cl_mem), &d_g);

	err = clSetKernelArg(mykernel, 3, sizeof(cl_mem), &d_b);

    // 設定 command queue
	size_t work_size = img->size; //setup img size
	err = clEnqueueNDRangeKernel(myqueue, mykernel, 1, NULL, &work_size, NULL, 0, NULL, NULL);


	err = clEnqueueReadBuffer(myqueue, d_r, CL_TRUE, 0, sizeof(uint32_t) * 256, R, 0, NULL, NULL);

	err = clEnqueueReadBuffer(myqueue, d_g, CL_TRUE, 0, sizeof(uint32_t) * 256, G, 0, NULL, NULL);

	err = clEnqueueReadBuffer(myqueue, d_b, CL_TRUE, 0, sizeof(uint32_t) * 256, B, 0, NULL, NULL);

	
	// release resources
	clReleaseMemObject(d_img);
	clReleaseMemObject(d_r);
	clReleaseMemObject(d_g);
	clReleaseMemObject(d_b);
}

// release resources
void release_opencl(){ 
	clReleaseKernel(mykernel);
	clReleaseProgram(myprogram);
	clReleaseCommandQueue(myqueue);
	clReleaseContext(myctx);
}

int main(int argc, char *argv[])
{
    char *filename;
    if (argc >= 2)
    {
        init_opencl();
        int many_img = argc - 1; //處理input多張圖片的狀況
        for (int i = 0; i < many_img; i++)
        {
            filename = argv[i + 1];
            img = readbmp(filename);

            std::cout << img->weight << ":" << img->height << "\n";
            /*
            uint32_t R[256];
            uint32_t G[256];
            uint32_t B[256];
            histogram(img,R,G,B);
            */
			setup_and_run_opencl();

            //限定範圍
            int max = 0;
            for(int i=0;i<256;i++){
                max = R[i] > max ? R[i] : max;
                max = G[i] > max ? G[i] : max;
                max = B[i] > max ? B[i] : max;
            }

            Image *ret = new Image();
            ret->type = 1;
            ret->height = 256;
            ret->weight = 256;
            ret->size = 256 * 256;
            ret->data = new RGB[256 * 256]();

            for(int i=0;i<ret->height;i++){
                for(int j=0;j<256;j++){
                    if(R[j]*256/max > i)
                        ret->data[256*i+j].R = 255;
                    if(G[j]*256/max > i)
                        ret->data[256*i+j].G = 255;
                    if(B[j]*256/max > i)
                        ret->data[256*i+j].B = 255;
                }
            }

            //輸出bmp
            std::string newfile = "hist_" + std::string(filename); 
            writebmp(newfile.c_str(), ret);
        }

        release_opencl();
    }else{
        printf("Usage: ./hist <img.bmp> [img2.bmp ...]\n");
    }
    return 0;
}