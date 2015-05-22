

#include <CL/cl.h>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include "shared_all.hpp"


using namespace std;

const uint32_t clPlatformSelection = 0;

int main(int argc, char* argv[]) {
	cl_int status;
	cl_uint cpuDeviceCount;
	cl_uint gpuDeviceCount;
	const int maxDevices = 8;
	cl_device_id cpuDevices[maxDevices];
	cl_device_id gpuDevices[maxDevices];
	cl_device_id *computeDevices;
	{
		cl_uint platformCount;
		cl_int status = clGetPlatformIDs(0, NULL, &platformCount);
		if (platformCount < 1) {
			cout << "no OpenCL platforms were found" << endl;
			exit(__LINE__);
		}
		cl_platform_id platform = NULL;
		cl_platform_id platforms[platformCount];
		status = clGetPlatformIDs(platformCount, platforms, NULL);
		if (status != CL_SUCCESS) {
			cout << "failed: clGetPlatformIDs" << endl;
			exit(__LINE__);
		}
		platform = platforms[0]; //we'll just use the first one for simplicity
		status = clGetDeviceIDs(
			platform, CL_DEVICE_TYPE_CPU, maxDevices, cpuDevices, &cpuDeviceCount
		);
		status = clGetDeviceIDs(
			platform, CL_DEVICE_TYPE_GPU, maxDevices, gpuDevices, &gpuDeviceCount
		);
		if (gpuDeviceCount) computeDevices = gpuDevices;
		else {
			cout << "no GPU devices found, using CPU instead" << endl;
			computeDevices = cpuDevices;
		}
	}
	cl_context context = clCreateContext(NULL,1, computeDevices,NULL,NULL,NULL);
	cl_command_queue commandQueue = clCreateCommandQueueWithProperties(
		context, computeDevices[0], 0, NULL
	);
	cl_program program;
	{
		const char *filename = "gpu_img.cl";
		ifstream sourceFile(filename);
		stringstream sourceStream;
		sourceStream << sourceFile.rdbuf();
		string source = sourceStream.str();
		const char *sources[] = {source.c_str()};
		size_t  sourceSizes[] = {strlen(sources[0])};
		program = clCreateProgramWithSource(
			context, 1, sources, sourceSizes, NULL
		);
		status = clBuildProgram(program, 1, computeDevices, NULL,NULL,NULL);
		if (status != CL_SUCCESS) {
			cout << "failed: clBuildProgram" << endl;
			exit(__LINE__);
		}
	}
	
	cl_image_format imageFormat = {CL_RGBA, CL_UNORM_INT8};
	cl_image_desc imageDesc;
	memset(&imageDesc, '\0', sizeof(cl_image_desc));
	imageDesc.image_type   = CL_MEM_OBJECT_IMAGE2D;
	imageDesc.image_width  = videoWidth;
	imageDesc.image_height = videoHeight;
	cl_mem outputImage = clCreateImage(
		context,               //cl_context             context,
		CL_MEM_WRITE_ONLY,     //cl_mem_flags           flags,
		&imageFormat,          //const cl_image_format *image_format,
		&imageDesc,            //const cl_image_desc   *image_desc,
		NULL,                  //void                  *host_ptr,
		&status                //cl_int                *errcode_ret
	);
	if (status != CL_SUCCESS) {
		cout << "failed: clCreateImage2D, from: " << endl;
		exit(__LINE__);
	}
	
	cl_kernel kernel = clCreateKernel(program, "helloPixel", &status);
	if (status != CL_SUCCESS) {
		cout << "failed to create kernel" << endl;
		exit(__LINE__);
	}
	
	status = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void*)&outputImage);
	if (status != CL_SUCCESS) {
		cout << "failed to set kernel arg 1" << endl;
		return __LINE__;
	}
	
	videoOut = new uint32_t[videoSize];
	initVideo();
	SDL_SetWindowTitle(window, __FILE__);
	running = true;
	prevTime = getMicroseconds();
	while (running) {
		
		status = clSetKernelArg(kernel, 0, sizeof(float),  (void*)&curFrame);
		if (status != CL_SUCCESS) {
			cout << "failed to set kernel arg 0" << endl;
			return __LINE__;
		}
		
		//run the kernel
		size_t globalWorkSize[] = {(uint32_t)videoWidth, (uint32_t)videoHeight};
		status = clEnqueueNDRangeKernel(
			commandQueue,       //cl_command_queue command_queue,
			kernel,             //cl_kernel        kernel,
			2,                  //cl_uint          work_dim,
			NULL,               //const size_t    *global_work_offset,
			globalWorkSize,     //const size_t    *global_work_size,
			NULL,               //const size_t    *local_work_size,
			0,                  //cl_uint          num_events_in_wait_list,
			NULL,               //const cl_event  *event_wait_list,
			NULL                //cl_event        *event
		);
		if (status != CL_SUCCESS) {
			cout << "failed: clEnqueueNDRangeKernel" << endl;
			exit(__LINE__);
		}
		
		//read the outputBuffer modified by our kernel back to host memory
		size_t origin[] = {0, 0, 0};
		size_t region[] = {(uint32_t)videoWidth, (uint32_t)videoHeight, 1};
		status = clEnqueueReadImage(
			commandQueue,      //cl_command_queue command_queue,
			outputImage,       //cl_mem           image,
			CL_TRUE,           //cl_bool          blocking_read,
			origin,            //const            size_t origin[3],
			region,            //const            size_t region[3],
			0,                 //size_t           row_pitch,
			0,                 //size_t           slice_pitch,
			videoOut,          //void            *ptr,
			0,                 //cl_uint          num_events_in_wait_list,
			NULL,              //const cl_event  *event_wait_list,
			NULL               //cl_event        *event
		);
		if (status != CL_SUCCESS) {
			cout << "failed: clEnqueueReadImage" << endl;
			exit(__LINE__);
		}
		
		//render image on screen
		SDL_UpdateTexture(texture, NULL, videoOut, videoWidth*sizeof(uint32_t));
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);
		
		//print framerate
		{
			long now = getMicroseconds();
			float fps = 1e6/(now-prevTime);
			prevTime = now;
			printf("FPS: %4.4f\n", fps);
		}
		
		//handle events
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) running = false;
		}
		curFrame++;
	}
	
	status = clReleaseKernel(kernel);
	status = clReleaseProgram(program);
	status = clReleaseMemObject(outputImage);
	status = clReleaseCommandQueue(commandQueue);
	status = clReleaseContext(context);
	if (status != CL_SUCCESS) {
		cout << "failed to release some CL objects" << endl;
		return __LINE__;
	}
	delete[] videoOut;
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	
	exit(0);
}
