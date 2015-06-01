

#include <CL/cl.h>
#include "shared_all.hpp"
#include "shared_gpu.hpp"
using namespace std;

int main(int argc, char* argv[]) {
	cout << "\n" << __FILE__ << endl;
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
	initClProgram("gpu_img.cl", program, context, computeDevices, status);
	if (status != CL_SUCCESS) {
		cout << "failed: initClProgram" << endl;
		exit(__LINE__);
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
	initVideo(__FILE__);
	SDL_SetRenderTarget(renderer, texture);
	SDL_RenderClear(renderer);
	
	running = true;
	Uint32 timeout = SDL_GetTicks() + runTime;
	while (!SDL_TICKS_PASSED(SDL_GetTicks(), timeout) && running) {
		
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
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);
		
		handleEvents();
		curFrame++;
	}
	printFrameCount();
	
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
