
#include "shared_all.hpp"
using namespace std;

double sintao(double n) {
	return sin(n*6.283185307179586476925286766559005768394338798750);
}

int main(int argc, char **argv) {
	cout << "\n" << __FILE__ << endl;
	
	uint32_t *videoOut = new uint32_t[videoSize];
	initVideo(__FILE__);
	running = true;
	Uint32 timeout = SDL_GetTicks() + runTime;
	while (!SDL_TICKS_PASSED(SDL_GetTicks(), timeout) && running) {
		uint32_t i = 0;
		for (uint32_t row = 0; row < videoHeight; row++) {
			for (uint32_t col = 0; col < videoWidth; col++) {
				uint8_t r = 0;
				uint8_t g = 255 * (
					sintao((row+fmod(curFrame,  videoHeight))/videoHeight)/2.0 + 0.5
				);
				uint8_t b = 255 * (
					sintao((col+fmod(curFrame*8, videoWidth))/videoWidth )/2.0 + 0.5
				);
				uint8_t a = 255;
				uint32_t color = 0;
				color |= r << 24;
				color |= g << 16;
				color |= b <<  8;
				color |= a;
				videoOut[i++] = color;
			}
		}
		
		//render
		SDL_UpdateTexture(
			texture,
			NULL, 
			videoOut,
			videoWidth*sizeof(uint32_t)
		);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);
		
		handleEvents();
		curFrame++;
	}
	printFrameCount();
	
	delete[] videoOut;
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	exit(0);
}
