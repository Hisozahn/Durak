#include <SDL.h>
#include <stdio.h>

const int SCREEN_WIDTH = 1600;
const int SCREEN_HEIGHT = 900;
const char* BACKGROUND_IMAGE = "C:/Users/123/Desktop/background.bmp";
const char* TEST_IMAGE = "C:/Users/123/Desktop/test.bmp";

void logSDLError(const char* msg) {
	printf("%s SDL_Error: %s\n", msg, SDL_GetError());
}

SDL_Texture* loadTexture(const char* file, SDL_Renderer* renderer){
	SDL_Surface *loadedImage = NULL;
	SDL_Texture *texture = NULL;
	loadedImage = SDL_LoadBMP(file);
	if (loadedImage != NULL){
		texture = SDL_CreateTextureFromSurface(renderer, loadedImage);
		SDL_FreeSurface(loadedImage);
	}
	else
		logSDLError("Image could not be loaded!");
	return texture;
}

void renderTexture(SDL_Texture *tex, SDL_Renderer *rend, int x, int y){
	SDL_Rect pos;
	pos.x = x;
	pos.y = y;
	SDL_QueryTexture(tex, NULL, NULL, &pos.w, &pos.h);
	SDL_RenderCopy(rend, tex, NULL, &pos);
}

void tile_renderTexture(SDL_Texture *tex, SDL_Renderer *rend) {
	SDL_Rect pos;
	int i, j, xt, yt;
	
	SDL_QueryTexture(tex, NULL, NULL, &pos.w, &pos.h);
	xt = SCREEN_WIDTH / pos.w + 1;
	yt = SCREEN_HEIGHT / pos.h + 1;
	for (i = 0; i < xt; i++) {
		int x = pos.w * i;
		for (j = 0; j < yt; j++) {
			int y = pos.h * j;
			renderTexture(tex, rend, x, y);
		}
	}
}

void cleanup(const char* format, ...) {
	va_list valist;
	size_t i, num = SDL_strlen(format);
	va_start(valist, format);
	for (i = 0; i < num; i++) {
		if (format[i] == 'w')
			SDL_DestroyWindow(va_arg(valist, SDL_Window*));
		if (format[i] == 'r')
			SDL_DestroyRenderer(va_arg(valist, SDL_Renderer*));
		if (format[i] == 't')
			SDL_DestroyTexture(va_arg(valist, SDL_Texture*));
		if (format[i] == 's')
			SDL_FreeSurface(va_arg(valist, SDL_Surface*));
	}
	va_end(valist);
}

int init_musthave(SDL_Window ** window, SDL_Renderer **renderer) {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0){
		logSDLError("SDL could not initialize!");
		return 1;
	}
	*window = SDL_CreateWindow("Lesson 2", 0,
		30, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (window == NULL){
		SDL_Quit();
		logSDLError("Window could not be created!");
		return 1;
	}
	*renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED
		| SDL_RENDERER_PRESENTVSYNC);
	if (renderer == NULL){
		logSDLError("Render could not be initialize");
		SDL_Quit();
		return 1;
	}
	return 0;
}


int main(int argc, char* args[])
{
	SDL_Window* window = NULL;
	SDL_Renderer *renderer = NULL;
	SDL_Texture *background = NULL, *image = NULL;
	int iW, iH, x, y;
	size_t i;

	if (init_musthave(&window, &renderer) != 0)
		return 1;
	
	background = loadTexture(BACKGROUND_IMAGE, renderer);
	image = loadTexture(TEST_IMAGE, renderer);
	if (background == NULL || image == NULL){
		cleanup("ttrw", background, image, renderer, window);
		SDL_Quit();
		return 1;
	}

	SDL_RenderClear(renderer);
	renderTexture(background, renderer, 0, 0);
	SDL_QueryTexture(image, NULL, NULL, &iW, &iH);
	x = SCREEN_WIDTH / 2 - iW / 2;
	y = SCREEN_HEIGHT / 2 - iH / 2;
	SDL_Event e;
	bool quit = false;
	while (!quit){
		while (SDL_PollEvent(&e)){
			if (e.type == SDL_QUIT){
				quit = true;
			}
			if (e.type == SDL_KEYDOWN){
				quit = true;
			}
			if (e.type == SDL_MOUSEBUTTONDOWN){
				x = e.button.x - iW / 2;
				y = e.button.y - iH / 2;
			}
			if (e.type == SDL_MOUSEMOTION) {
				x = e.motion.x - iW / 2;
				y = e.motion.y - iH / 2;
			}
		}
		SDL_RenderClear(renderer);
		tile_renderTexture(background, renderer);
		renderTexture(image, renderer, x, y);
		SDL_RenderPresent(renderer);
	}

	cleanup("ttrw", background, image, renderer, window);
	SDL_Quit();
	return 0;
}