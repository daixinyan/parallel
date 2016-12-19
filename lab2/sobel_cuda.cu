
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MASK_N 2
#define MASK_X 5
#define MASK_Y 5
#define SCALE  8

unsigned char *image_s = NULL;     // source image array
unsigned char *image_t = NULL;     // target image array
FILE *fp_s = NULL;                 // source file handler
FILE *fp_t = NULL;                 // target file handler

unsigned int   width, height;      // image width, image height
unsigned int   rgb_raw_data_offset;// RGB raw data offset
unsigned char  bit_per_pixel;      // bit per pixel
unsigned short byte_per_pixel;     // byte per pixel

// bitmap header
unsigned char header[54] = {
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
		24, 0,       // bit per pixel
		0, 0, 0, 0,  // compression
		0, 0, 0, 0,  // data size
		0, 0, 0, 0,  // h resolution
		0, 0, 0, 0,  // v resolution
		0, 0, 0, 0,  // used colors
		0, 0, 0, 0   // important colors
};

// sobel mask (5x5 version)
int
		mask[MASK_N][MASK_X][MASK_Y] = {
		{{ -1, -4, -6, -4, -1},
				{ -2, -8,-12, -8, -2},
				{  0,  0,  0,  0,  0},
				{  2,  8, 12,  8,  2},
				{  1,  4,  6,  4,  1}}
		,
		{{ -1, -2,  0,  2,  1},
				{ -4, -8,  0,  8,  4},
				{ -6,-12,  0, 12,  6},
				{ -4, -8,  0,  8,  4},
				{ -1, -2,  0,  2,  1}}
};


int
read_bmp (const char *fname_s) {
	fp_s = fopen(fname_s, "rb");
	if (fp_s == NULL) {
		printf("fopen fp_s error\n");
		return -1;
	}

	// move offset to 10 to find rgb raw data offset
	fseek(fp_s, 10, SEEK_SET);
	fread(&rgb_raw_data_offset, sizeof(unsigned int), 1, fp_s);

	// move offset to 18 to get width & height;
	fseek(fp_s, 18, SEEK_SET);
	fread(&width,  sizeof(unsigned int), 1, fp_s);
	fread(&height, sizeof(unsigned int), 1, fp_s);

	// get bit per pixel
	fseek(fp_s, 28, SEEK_SET);
	fread(&bit_per_pixel, sizeof(unsigned short), 1, fp_s);
	byte_per_pixel = bit_per_pixel / 8;

	// move offset to rgb_raw_data_offset to get RGB raw data
	fseek(fp_s, rgb_raw_data_offset, SEEK_SET);

	// image_s = (unsigned char *) malloc((size_t)width * height * byte_per_pixel);
	cudaMallocHost( (void**)&image_s, (size_t)width * height * byte_per_pixel);

	fread(image_s, sizeof(unsigned char), (size_t)(long) width * height * byte_per_pixel, fp_s);

	return 0;
}

__global__ void
sobel_Kernel (
		unsigned char* cuda_image_t,
		const unsigned char* cuda_image_s,
		int* global_cuda_mask,
		unsigned int* cuda_width,
		unsigned int *cuda_height,
		short *cuda_byte_per_pixel
) {
	int  x, y, i, v, u;            // for loop counter
	int  R, G, B;                  // color of R, G, B
	double val[MASK_N*3] = {0.0};
	int adjustX, adjustY, xBound, yBound;
	unsigned int width = *cuda_width;
	unsigned int height = *cuda_height;
	short byte_per_pixel = *cuda_byte_per_pixel;


	__shared__ int share_cuda_mask[MASK_N][MASK_X][MASK_Y];
	// int id = (threadIdx.z * (blockDim.x * blockDim.y)) + (threadIdx.y * blockDim.x) + threadIdx.x;
	if(threadIdx.x<MASK_N*MASK_X*MASK_Y)
	{
		int dim_1, dim_2, dim_3;
		dim_1 = threadIdx.x % MASK_Y;
		dim_2 = threadIdx.x / MASK_Y;
		dim_3 = dim_2 / MASK_X;
		dim_2 = dim_2 % MASK_X;
		share_cuda_mask[dim_3][dim_2][dim_1] = global_cuda_mask[threadIdx.x];
	}
	__syncthreads();


	y = threadIdx.x+blockIdx.x*blockDim.x;
	if(y<height)
		for (x = 0; x < width; ++x) {

			for (i = 0; i < MASK_N; ++i) {
				adjustX = (MASK_X % 2) ? 1 : 0;
				adjustY = (MASK_Y % 2) ? 1 : 0;
				xBound = MASK_X /2;
				yBound = MASK_Y /2;

				val[i*3+2] = 0.0;
				val[i*3+1] = 0.0;
				val[i*3] = 0.0;

				for (v = -yBound; v < yBound + adjustY; ++v) {
					for (u = -xBound; u < xBound + adjustX; ++u) {
						if ((x + u) >= 0 && (x + u) < width && y + v >= 0 && y + v < height) {
							R = cuda_image_s[byte_per_pixel * (width * (y+v) + (x+u)) + 2];
							G = cuda_image_s[byte_per_pixel * (width * (y+v) + (x+u)) + 1];
							B = cuda_image_s[byte_per_pixel * (width * (y+v) + (x+u)) + 0];
							val[i*3+2] += R * share_cuda_mask[i][u + xBound][v + yBound];
							val[i*3+1] += G * share_cuda_mask[i][u + xBound][v + yBound];
							val[i*3+0] += B * share_cuda_mask[i][u + xBound][v + yBound];
						}
					}
				}

			}

			double totalR = 0.0;
			double totalG = 0.0;
			double totalB = 0.0;
			for (i = 0; i < MASK_N; ++i) {
				totalR += val[i*3+2] * val[i*3+2];
				totalG += val[i*3+1] * val[i*3+1];
				totalB += val[i*3+0] * val[i*3+0];
			}

			totalR = sqrt(totalR) / SCALE;
			totalG = sqrt(totalG) / SCALE;
			totalB = sqrt(totalB) / SCALE;
			const unsigned char cR = (totalR > 255.0) ? 255 : totalR;
			const unsigned char cG = (totalG > 255.0) ? 255 : totalG;
			const unsigned char cB = (totalB > 255.0) ? 255 : totalB;
			cuda_image_t[ byte_per_pixel * (width * y + x) + 2 ] = cR;
			cuda_image_t[ byte_per_pixel * (width * y + x) + 1 ] = cG;
			cuda_image_t[ byte_per_pixel * (width * y + x) + 0 ] = cB;
		}

}

int
write_bmp (const char *fname_t) {
	unsigned int file_size; // file size

	fp_t = fopen(fname_t, "wb");
	if (fp_t == NULL) {
		printf("fopen fname_t error\n");
		return -1;
	}

	// file size
	file_size = width * height * byte_per_pixel + rgb_raw_data_offset;
	header[2] = (unsigned char)(file_size & 0x000000ff);
	header[3] = (file_size >> 8)  & 0x000000ff;
	header[4] = (file_size >> 16) & 0x000000ff;
	header[5] = (file_size >> 24) & 0x000000ff;

	// width
	header[18] = width & 0x000000ff;
	header[19] = (width >> 8)  & 0x000000ff;
	header[20] = (width >> 16) & 0x000000ff;
	header[21] = (width >> 24) & 0x000000ff;

	// height
	header[22] = height &0x000000ff;
	header[23] = (height >> 8)  & 0x000000ff;
	header[24] = (height >> 16) & 0x000000ff;
	header[25] = (height >> 24) & 0x000000ff;

	// bit per pixel
	header[28] = bit_per_pixel;

	// write header
	fwrite(header, sizeof(unsigned char), rgb_raw_data_offset, fp_t);

	// write image
	fwrite(image_t, sizeof(unsigned char), (size_t)(long)width * height * byte_per_pixel, fp_t);

	fclose(fp_s);
	fclose(fp_t);

	return 0;
}

int
init_device ()
{
	cudaSetDevice(0);
	return 0;
}

int
main(int argc, char **argv) {
	init_device();

	const char *input = "candy.bmp";
	if (argc > 1) input = argv[1];
	read_bmp(input); // 24 bit gray level image

	unsigned char * cuda_image_t;
	unsigned char * cuda_image_s;
	int  * global_cuda_mask ;
	unsigned int * cuda_width;
	unsigned int * cuda_height;
	short * cuda_byte_per_pixel;


	size_t mask_size = sizeof(int)*MASK_N*MASK_X*MASK_Y;
	size_t image_size = (size_t)width * height * byte_per_pixel;

	cudaMalloc((void**)&cuda_image_s, image_size);
	cudaMalloc((void**)&cuda_image_t, image_size);
	cudaMalloc((void**)&global_cuda_mask, mask_size);
	cudaMalloc((void**)&cuda_width, 1*sizeof(unsigned int));
	cudaMalloc((void**)&cuda_height, 1*sizeof(unsigned int));
	cudaMalloc((void**)&cuda_byte_per_pixel, 1*sizeof(short));

	cudaMemcpy(cuda_image_s, image_s, image_size, cudaMemcpyHostToDevice);
	// free (image_s);
	// cudaFreeHost(image_s);
	cudaMemcpy(global_cuda_mask,(int*)mask, mask_size, cudaMemcpyHostToDevice);
	cudaMemcpy(cuda_width, &width, 1*sizeof(unsigned int), cudaMemcpyHostToDevice);
	cudaMemcpy(cuda_height, &height, 1*sizeof(unsigned int), cudaMemcpyHostToDevice);
	cudaMemcpy(cuda_byte_per_pixel, &byte_per_pixel, 1*sizeof(short), cudaMemcpyHostToDevice);



	int thread_number = 64;
	sobel_Kernel<<<height/thread_number+1, thread_number>>>(cuda_image_t, cuda_image_s, global_cuda_mask, cuda_width, cuda_height, cuda_byte_per_pixel);

	// image_t = (unsigned char *) malloc((size_t) width * height * byte_per_pixel);
	cudaMallocHost( (void**)&image_t, image_size);

	cudaMemcpy(image_t, cuda_image_t, image_size, cudaMemcpyDeviceToHost);

	write_bmp("result.bmp");

	// free (image_t);
	// cudaFreeHost(image_t);
	// cudaFree(cuda_image_t);
	// cudaFree(cuda_image_s);
	// cudaFree(global_cuda_mask);
	// cudaFree(cuda_width);
	// cudaFree(cuda_width);
	// cudaFree(cuda_byte_per_pixel);

}
