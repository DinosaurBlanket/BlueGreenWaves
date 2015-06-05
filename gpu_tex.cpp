// g++ -o gpu_tex gpu_tex.cpp -Wall -lGL -lGLEW `sdl2-config --cflags --libs`

#include <iostream>
#include <SDL2/SDL.h>
#define   GLEW_STATIC
#include <GL/glew.h>
#include <SDL_opengl.h>
#include <CL/cl.h>
#include <CL/cl_gl.h>
#include <string>
#include <sstream>
#include <fstream>
using namespace std;

//SDL_Texture   *texture  = NULL;
int main(int argc, char *argv[]) {
	cout << "\n" << __FILE__ << endl;
	
	SDL_Init(SDL_INIT_VIDEO);
	SDL_GL_SetAttribute(
		SDL_GL_CONTEXT_PROFILE_MASK, 
		SDL_GL_CONTEXT_PROFILE_CORE
	);
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
		//position      texcoords
		 1.0f, -1.0f,   1.0, 0.0, //top left
		 1.0f,  1.0f,   1.0, 1.0, //top right
		-1.0f, -1.0f,   0.0, 0.0, //bottom left
		-1.0f,  1.0f,   0.0, 1.0  //bottom right
	};
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	
	int  shaderLogSize = 1024;
	char shaderLogBuffer[shaderLogSize];
	
	const char *vertexSource = 
		"#version 150\n"
		"in  vec2 position;"
		"in  vec2 texcoordIntoVS;"
		"out vec2 TexcoordOutOfVS;"
		"void main() {"
			"TexcoordOutOfVS = texcoordIntoVS;"
			"gl_Position = vec4(position, 0.0, 1.0);"
		"}";
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);
	glGetShaderInfoLog(vertexShader, shaderLogSize, NULL, shaderLogBuffer);
	cout << "compiling vertex shader...\n" << shaderLogBuffer << endl;
	
	const char *fragmentSource = 
		"#version 150\n"
		"out vec4 outColor;"
		"in  vec2 TexcoordOutOfVS;"
		"uniform sampler2D GLtex;"
		"void main() { outColor = texture(GLtex, TexcoordOutOfVS); }";
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
	GLint texAttrib = glGetAttribLocation(shaderProgram, "texcoordIntoVS");
	glVertexAttribPointer(
		posAttrib, 
		2, 
		GL_FLOAT, 
		GL_FALSE, 
		4*sizeof(float),
		0
	);
	glVertexAttribPointer(
		texAttrib, 
		2, 
		GL_FLOAT, 
		GL_FALSE, 
		4*sizeof(float), 
		(void*)(2*sizeof(float))
	);
	glEnableVertexAttribArray(posAttrib);
	glEnableVertexAttribArray(texAttrib);
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	cl_int status = CL_SUCCESS;
	const cl_uint    maxDevices = 8;
	cl_device_id     CLdevices[maxDevices];
	cl_context       CLcontext = NULL;
	cl_command_queue CLqueue = NULL;
	{
		cl_uint platformCount;
		cl_platform_id platform = NULL;
		const int maxPlatforms = 8;
		cl_platform_id platforms[maxPlatforms];
		status = clGetPlatformIDs(maxPlatforms, platforms, &platformCount);
		if (status != CL_SUCCESS) {
			cout << "failed: clGetPlatformIDs" << endl;
			return  __LINE__;
		}
		if (platformCount < 1) {
			cout << "failed to find any OpenCL platforms" << endl;
			return  __LINE__;
		}
		platform = platforms[0];
		cl_uint deviceCount = 0;
		status = clGetDeviceIDs(
			platform, CL_DEVICE_TYPE_GPU, maxDevices, CLdevices, &deviceCount
		);
		if (status != CL_SUCCESS) {
			cout << "failed: clGetDeviceIDs" << endl;
			return  __LINE__;
		}
		if (!deviceCount) {
			status = clGetDeviceIDs(
				platform, CL_DEVICE_TYPE_CPU, maxDevices, CLdevices, &deviceCount
			);
			if (!deviceCount) {
				cout << "failed to find any GPU or CPU devices" << endl;
				return  __LINE__;
			}
			cout << "no GPU devices found, using CPU instead" << endl;
		}
		
		cl_context_properties props[] = {
			CL_CONTEXT_PLATFORM, (cl_context_properties)platform,
			CL_GL_CONTEXT_KHR,   (cl_context_properties)GLcontext,
			0
		};
		CLcontext = clCreateContext(props,1, CLdevices, NULL,NULL, &status);
		if (status != CL_SUCCESS) {
			cout << "failed: clCreateContext" << endl;
			return  __LINE__;
		}
		CLqueue = clCreateCommandQueueWithProperties(
			CLcontext, CLdevices[0], 0, &status
		);
		if (status != CL_SUCCESS) {
			cout << "failed: clCreateCommandQueueWithProperties" << endl;
			return  __LINE__;
		}
	}
	
	cl_program program;
	{
		ifstream sourceFile("gpu_img.cl");
		stringstream sourceStream;
		sourceStream << sourceFile.rdbuf();
		string source = sourceStream.str();
		const char *sources[] = {source.c_str()};
		size_t  sourceSizes[] = {strlen(sources[0])};
		program = clCreateProgramWithSource(
			CLcontext, 1, sources, sourceSizes, &status
		);
		if (status != CL_SUCCESS) {
			cout << "failed: clCreateProgramWithSource" << endl;
			return  __LINE__;
		}
		status = clBuildProgram(program, 1, CLdevices, NULL,NULL,NULL);
		if (status != CL_SUCCESS) {
			cout << "failed: clBuildProgram" << endl;
			return  __LINE__;
		}
	}
	cl_kernel kernel = clCreateKernel(program, "helloPixel", &status);
	if (status != CL_SUCCESS) {
		cout << "failed to create kernel" << endl;
		exit(__LINE__);
	}
	
	
	
	
	
	
	
	
	
	
	GLuint GLtex;
	glGenTextures(1, &GLtex);
	glBindTexture(GL_TEXTURE_2D, GLtex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
	uint32_t *pixels = new uint32_t[videoSize];
	glTexImage2D(
		GL_TEXTURE_2D, 
		0, 
		GL_RGBA8, 
		videoWidth, 
		videoHeight, 
		0, 
		GL_RGBA, 
		GL_UNSIGNED_BYTE, 
		pixels
	);
	
	
	
	
	
	
	
	
	
	
	
	cl_mem CLtex = clCreateFromGLTexture(
		CLcontext, 
		CL_MEM_WRITE_ONLY, 
		GL_TEXTURE_2D, 
		0,
		GLtex,
		NULL
	);
	
	
	
	
	
	
	
	
	
	
	
	bool  running  = true;
	float curFrame = 1;
	//int   runTime  = 3000;//ms
	//Uint32 timeout = SDL_GetTicks() + runTime;
	while (/*!SDL_TICKS_PASSED(SDL_GetTicks(), timeout) && */running) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (
				event.type == SDL_QUIT ||
				(event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE)
			) {
				running = false;
			}
		}
		
		
		
		
		
		glFinish();
		clEnqueueAcquireGLObjects(CLqueue, 1, &CLtex, 0, 0, NULL);
		status = clSetKernelArg(kernel, 0, sizeof(float),  (void*)&curFrame);
		if (status != CL_SUCCESS) {
			cout << "failed to set kernel arg 0" << endl;
			return __LINE__;
		}
		status = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void*)&CLtex);
		if (status != CL_SUCCESS) {
			cout << "failed to set kernel arg 1" << endl;
			return __LINE__;
		}
		
		
		
		
		
		
		
		
		clFinish(CLqueue);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		
		SDL_GL_SwapWindow(window);
		curFrame++;
		SDL_Delay(10);
	}
	//cout << "ended on frame " << curFrame << ", about " 
	//<< curFrame/(runTime/1000) << " FPS" << endl;
	
	delete[] pixels;
	SDL_GL_DeleteContext(GLcontext);
	SDL_Quit();
	return 0;
}
