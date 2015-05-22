
#include <iostream>
#include <SDL2/SDL.h>

#ifdef __linux__
#include <time.h>
#define microSleep usleep
long getMicroseconds() {
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return  ts.tv_sec * 1e6  +  ts.tv_nsec / 1000;
}
#endif

using namespace std;

const float videoWidth  = 1280;
const float videoHeight =  720;
const uint32_t videoSize = videoWidth * videoHeight;
uint32_t     *videoOut = NULL;
SDL_Window   *window   = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture  *texture  = NULL;
void initVideo() {
	if ( SDL_Init(SDL_INIT_VIDEO) ) {
		cout << "failed to initialize SDL:\n" << SDL_GetError() << endl;
		exit(__LINE__);
	}
	window = SDL_CreateWindow(
		"window title",            //const char* title,
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
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
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

bool running = false;
long prevTime;
float curFrame = 1;
