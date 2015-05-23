

#include <CL/cl.h>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include "shared_all.hpp"
#include "shared_gpu.hpp"

const char *programTitle = "gpu_buf";

int main(int argc, char* argv[]) {
	cl_int status = CL_SUCCESS;
	const cl_uint maxDevices = 8;
	cl_device_id     computeDevices[maxDevices];
	cl_context       context = NULL;
	cl_command_queue commandQueue = NULL;
	initOpenCL(computeDevices, maxDevices, context, commandQueue, status);
	if (status != CL_SUCCESS) {
		cout << "failed: initOpenCL" << endl;
		exit(__LINE__);
	}
	cl_program program;
	{
		const char *filename = "gpu_buf.cl";
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
	
	cl_mem outputBuffer = clCreateBuffer(
		context, CL_MEM_WRITE_ONLY, videoSize*sizeof(uint32_t), NULL, NULL
	);
	cl_kernel kernel = clCreateKernel(program, "helloPixel", &status);
	if (status != CL_SUCCESS) {
		cout << "failed: clCreateKernel" << endl;
		exit(__LINE__);
	}
	
	status = clSetKernelArg(kernel, 0, sizeof(float),  (void*)&videoWidth);
	if (status != CL_SUCCESS) {
		cout << "failed to set kernel argument 0" << endl;
		exit(__LINE__);
	}
	status = clSetKernelArg(kernel, 1, sizeof(float),  (void*)&videoHeight);
	if (status != CL_SUCCESS) {
		cout << "failed to set kernel argument 1" << endl;
		exit(__LINE__);
	}
	status = clSetKernelArg(kernel, 3, sizeof(cl_mem), (void*)&outputBuffer);
	if (status != CL_SUCCESS) {
		cout << "failed to set kernel argument 3" << endl;
		exit(__LINE__);
	}
	
	videoOut = new uint32_t[videoSize];
	initVideo();
	SDL_SetWindowTitle(window, __FILE__);
	running = true;
	prevTime = getMicroseconds();
	while (running) {
		
		status = clSetKernelArg(kernel, 2, sizeof(float),  (void*)&curFrame);
		if (status != CL_SUCCESS) {
			cout << "failed to set kernel argument 2" << endl;
			exit(__LINE__);
		}
		
		//run the kernel
		size_t globalWorkSize[] = {videoSize};
		status = clEnqueueNDRangeKernel(
			commandQueue,       //cl_command_queue command_queue,
			kernel,             //cl_kernel        kernel,
			1,                  //cl_uint          work_dim,
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
		status = clEnqueueReadBuffer(
			commandQueue,                 //cl_command_queue command_queue,
			outputBuffer,                 //cl_mem           buffer,
			CL_TRUE,                      //cl_bool          blocking_read,
			0,                            //size_t           offset,
			videoSize*sizeof(uint32_t),   //size_t           cb,
			videoOut,                     //void            *ptr,
			0,                            //cl_uint          num_events_in_wait_list,
			NULL,                         //const            cl_event *event_wait_list,
			NULL                          //cl_event        *event
		);
		if (status != CL_SUCCESS) {
			cout << "failed: clEnqueueReadBuffer" << endl;
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
	status = clReleaseMemObject(outputBuffer);
	status = clReleaseCommandQueue(commandQueue);
	status = clReleaseContext(context);
	if (status != CL_SUCCESS) {
		cout << "failed to release some CL objects" << endl;
		exit(__LINE__);
	}
	delete[] videoOut;
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	
	exit(0);
}
