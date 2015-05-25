
#include "shared_all.hpp"
using namespace std;

double sintao(double n) {
	return sin(n*6.283185307179586476925286766559005768394338798750);
}

SDL_atomic_t threadPunch;
int threadCount;

#ifdef __linux__
#include <unistd.h>
#include <time.h>
void threadWait() {usleep(threadCount*5);}
#endif

Uint32 timeout = 0;
void betweenFrames() {
	SDL_UpdateTexture(
		texture,
		NULL, 
		videoOut,
		videoWidth * sizeof(uint32_t)
	);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);
	handleEvents();
	if (SDL_TICKS_PASSED(SDL_GetTicks(), timeout)) running = false;
	curFrame++;
}

int sharedWork(void *ti) {
	const int threadIndex = *(int*)ti;
	uint32_t my_row_start =  threadIndex * (videoHeight/threadCount);
	uint32_t my_row_end;
	if (threadIndex == threadCount-1) my_row_end = videoHeight;
	else my_row_end = (threadIndex+1) * (videoHeight/threadCount);
	int64_t punchWait = threadCount;
	while (running) {
		for (uint32_t row = my_row_start; row < my_row_end; row++) {
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
				videoOut[(row*(int)videoWidth)+col] = color;
			}
		}
		if (threadIndex) {
			SDL_AtomicIncRef(&threadPunch);
			while (SDL_AtomicGet(&threadPunch) < punchWait) threadWait();
		}
		else {
			while (SDL_AtomicGet(&threadPunch) < punchWait-1) threadWait();
			betweenFrames();
			SDL_AtomicIncRef(&threadPunch);
		}
		punchWait += threadCount;
	}
	return 0;
}


int main(int argc, char **argv) {
	cout << "\n" << __FILE__ << endl;
	
	videoOut = new uint32_t[videoSize];
	initVideo(__FILE__);
	
	SDL_AtomicSet(&threadPunch, 0);
	threadCount = SDL_GetCPUCount();
	cout << "using " << threadCount << " threads" << endl;
	
	running = true;
	timeout = SDL_GetTicks() + runTime;
	
	SDL_Thread *threads[threadCount];
	int threadIndices[threadCount];
	threadIndices[0] = 0;
	for (int i = 1; i < threadCount; i++) {
		threadIndices[i] = i;
		threads[i] = SDL_CreateThread(
			sharedWork, 
			"workerThread", 
			(void*)&threadIndices[i]
		);
		if (!threads[i]) {
			cout << "failed to create thread " << i << endl;
			exit(1);
		}
	}
	
	sharedWork((void*)&threadIndices[0]);
	
	for (int i = 1; i < threadCount; i++) SDL_WaitThread(threads[i], NULL);
	printFrameCount();
	
	delete[] videoOut;
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	exit(0);
}
