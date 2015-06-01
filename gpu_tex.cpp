// g++ -o gpu_tex gpu_tex.cpp -Wall -lGL -lGLEW `sdl2-config --cflags --libs`

#include <iostream>
#include <SDL2/SDL.h>
#define   GLEW_STATIC
#include <GL/glew.h>
#include <SDL_opengl.h>
using namespace std;

//SDL_Texture   *texture  = NULL;
int main(int argc, char *argv[]) {
	SDL_Init(SDL_INIT_VIDEO);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	
	const float    videoWidth  = 1280;
	const float    videoHeight =  720;
	const uint32_t videoSize   = videoWidth * videoHeight;
	SDL_Window    *window    = NULL;
	SDL_GLContext  GLcontext = NULL;
	window = SDL_CreateWindow(
		__FILE__,                  //const char* title,
		SDL_WINDOWPOS_UNDEFINED,   //int         x,
		SDL_WINDOWPOS_UNDEFINED,   //int         y,
		videoWidth,                //int         w,
		videoHeight,               //int         h,
		SDL_WINDOW_OPENGL          //Uint32      flags
	);
	GLcontext = SDL_GL_CreateContext(window);
	
	glewExperimental = GL_TRUE;
	glewInit();
	
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	GLuint vbo;
	glGenBuffers(1, &vbo);
	float vertices[] = {
		 0.0f,  0.5f, // Vertex 1 (X, Y)
		 0.5f, -0.5f, // Vertex 2 (X, Y)
		-0.5f, -0.5f  // Vertex 3 (X, Y)
	};
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	
	int  shaderLogSize = 1024;
	char shaderLogBuffer[shaderLogSize];
	
	const char *vertexSource = 
		"#version 150\n"
		"in vec2 position;\n"
		"void main() { gl_Position = vec4(position, 0.0, 1.0); }";
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);
	glGetShaderInfoLog(vertexShader, shaderLogSize, NULL, shaderLogBuffer);
	cout << "compiling vertex shader...\n" << shaderLogBuffer << endl;
	
	const char *fragmentSource = 
		"#version 150\n"
		"out vec4 outColor;\n"
		"void main() { outColor = vec4(1.0, 1.0, 1.0, 1.0); }";
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);
	glGetShaderInfoLog(fragmentShader, shaderLogSize, NULL, shaderLogBuffer);
	cout << "compiling fragment shader...\n" << shaderLogBuffer << endl;
	
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glBindFragDataLocation(shaderProgram, 0, "outColor");
	glLinkProgram(shaderProgram);
	glUseProgram(shaderProgram);
	
	GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(posAttrib);
	
	
	
	bool  running  = true;
	float curFrame = 1;
	int   runTime  = 3000;//ms
	Uint32 timeout = SDL_GetTicks() + runTime;
	while (!SDL_TICKS_PASSED(SDL_GetTicks(), timeout) && running) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (
				event.type == SDL_QUIT ||
				(event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE)
			) {
				running = false;
			}
		}
		
		glDrawArrays(GL_TRIANGLES, 0, 3);
		
		SDL_GL_SwapWindow(window);
		curFrame++;
		SDL_Delay(10);
	}
	cout << "ended on frame " << curFrame << ", about " 
	<< curFrame/(runTime/1000) << " FPS" << endl;
	
	
	SDL_GL_DeleteContext(GLcontext);
	SDL_Quit();
	return 0;
}





/*
#include <iostream>
#include <SDL2/SDL.h>
#include <CL/cl.h>
using namespace std;

const float    videoWidth  = 1280;
const float    videoHeight =  720;
const uint32_t videoSize   = videoWidth * videoHeight;
uint32_t      *videoOut = NULL;
SDL_Window    *window   = NULL;
SDL_Renderer  *renderer = NULL;
SDL_Texture   *texture  = NULL;
void initVideo(const char* windowTitle) {
	if ( SDL_Init(SDL_INIT_VIDEO) ) {
		cout << "failed to initialize SDL:\n" << SDL_GetError() << endl;
		exit(__LINE__);
	}
	window = SDL_CreateWindow(
		windowTitle,               //const char* title,
		SDL_WINDOWPOS_UNDEFINED,   //int         x,
		SDL_WINDOWPOS_UNDEFINED,   //int         y,
		videoWidth,                //int         w,
		videoHeight,               //int         h,
		SDL_WINDOW_OPENGL          //Uint32      flags
	);
	if (window == NULL) {
		cout << "failed to create window:\n" << SDL_GetError() << endl;
		exit(__LINE__);
	}
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_TARGETTEXTURE);
	if (renderer == NULL) {
		cout << "failed to create renderer:\n" << SDL_GetError() << endl;
		exit(__LINE__);
	}
	texture = SDL_CreateTexture(
		renderer,                     //SDL_Renderer* renderer,
		SDL_PIXELFORMAT_RGBA8888,     //Uint32        format,
		SDL_TEXTUREACCESS_STREAMING,  //int           access, 
		videoWidth,                   //int           w,
		videoHeight                   //int           h
	);
	if (texture == NULL) {
		cout << "failed to create texture:\n" << SDL_GetError() << endl;
		exit(__LINE__);
	}
}

bool  running  = false;
float curFrame = 1;
int   runTime  = 3000;//ms

void handleEvents() {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT) running = false;
	}
}

void printFrameCount() {
	cout << "ended on frame " << curFrame << ", about " 
	<< curFrame/(runTime/1000) << " FPS" << endl;
}

void initOpenCL(
	cl_device_id     *devices, 
	const cl_uint     maxDevices, 
	cl_context       &context, 
	cl_command_queue &commandQueue, 
	cl_int           &status
) {
	cl_uint platformCount;
	cl_platform_id platform = NULL;
	const int maxPlatforms = 8;
	cl_platform_id platforms[maxPlatforms];
	status = clGetPlatformIDs(maxPlatforms, platforms, &platformCount);
	if (status != CL_SUCCESS) {
		cout << "failed: clGetPlatformIDs" << endl;
		return;
	}
	if (platformCount < 1) {
		cout << "failed to find any OpenCL platforms" << endl;
		return;
	}
	platform = platforms[0];
	cl_uint deviceCount = 0;
	status = clGetDeviceIDs(
		platform, CL_DEVICE_TYPE_GPU, maxDevices, devices, &deviceCount
	);
	if (status != CL_SUCCESS) {
		cout << "failed: clGetDeviceIDs" << endl;
		return;
	}
	if (!deviceCount) {
		status = clGetDeviceIDs(
			platform, CL_DEVICE_TYPE_CPU, maxDevices, devices, &deviceCount
		);
		if (!deviceCount) {
			cout << "failed to find any GPU or CPU devices" << endl;
			return;
		}
		cout << "no GPU devices found, using CPU instead" << endl;
	}	
	context = clCreateContext(NULL,1, devices, NULL,NULL, &status);
	if (status != CL_SUCCESS) {
		cout << "failed: clCreateContext" << endl;
		return;
	}
	commandQueue = clCreateCommandQueueWithProperties(
		context, devices[0], 0, &status
	);
	if (status != CL_SUCCESS) {
		cout << "failed: clCreateCommandQueueWithProperties" << endl;
		return;
	}
}


#include <string>
#include <sstream>
#include <fstream>
void initClProgram(
	const char   *filename, 
	cl_program   &program, 
	cl_context   &context, 
	cl_device_id *devices, 
	cl_int       &status
) {
	ifstream sourceFile(filename);
	stringstream sourceStream;
	sourceStream << sourceFile.rdbuf();
	string source = sourceStream.str();
	const char *sources[] = {source.c_str()};
	size_t  sourceSizes[] = {strlen(sources[0])};
	program = clCreateProgramWithSource(
		context, 1, sources, sourceSizes, &status
	);
	if (status != CL_SUCCESS) {
		cout << "failed: clCreateProgramWithSource" << endl;
		return;
	}
	status = clBuildProgram(program, 1, devices, NULL,NULL,NULL);
	if (status != CL_SUCCESS) {
		cout << "failed: clBuildProgram" << endl;
		return;
	}
}

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
*/
