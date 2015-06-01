
#include <iostream>
#include <SDL2/SDL.h>
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
