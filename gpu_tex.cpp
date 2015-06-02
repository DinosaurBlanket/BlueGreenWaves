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
	SDL_GL_SetAttribute(
		SDL_GL_CONTEXT_PROFILE_MASK, 
		SDL_GL_CONTEXT_PROFILE_CORE
	);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	
	const float    videoWidth  = 1280;
	const float    videoHeight =  720;
	//const uint32_t videoSize   = videoWidth * videoHeight;
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
		 0.5f, -0.5f,   1.0, 0.0, //top left
		 0.5f,  0.5f,   1.0, 1.0, //top right
		-0.5f, -0.5f,   0.0, 0.0, //bottom left
		-0.5f,  0.5f,   0.0, 1.0  //bottom right
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
		"uniform sampler2D tex;"
		"void main() { outColor = texture(tex, TexcoordOutOfVS); }";
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
	
	
	
	
	float pixels[] = {
		0.0f, 1.0f, 0.0f,   0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,   0.0f, 1.0f, 0.0f
	};
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_FLOAT, pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
	
	
	
	
	bool  running  = true;
	float curFrame = 1;
	int   runTime  = 3000;//ms
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
		
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		
		SDL_GL_SwapWindow(window);
		curFrame++;
		SDL_Delay(10);
	}
	//cout << "ended on frame " << curFrame << ", about " 
	//<< curFrame/(runTime/1000) << " FPS" << endl;
	
	
	SDL_GL_DeleteContext(GLcontext);
	SDL_Quit();
	return 0;
}
