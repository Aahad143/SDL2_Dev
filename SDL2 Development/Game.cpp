#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include <stdio.h>
#include <string>

const int SCREEN_WIDTH = 1366;
const int SCREEN_HEIGHT = 768;
float scale = (float)1 / 6;

class LTexture {
public:
	LTexture();
	~LTexture();

	bool loadFromFile(std::string path);

	void free();

	void colorKey(SDL_Surface* surface, uint8_t r, uint8_t g, uint8_t b);

	void render(int x, int y, SDL_Rect* clip = NULL);

	int getWidth();
	int getHeight();

private:
	SDL_Texture* mTexture;

	int mWidth;
	int mHeight;
};

bool init();

bool loadMedia();

void spriteSheetMaker(int i, int x, int y, int w, int h);

void close();

SDL_Window* window = NULL;

SDL_Renderer* renderer = NULL;

LTexture background;
LTexture player;
SDL_Rect spriteSheets[16];

LTexture::LTexture() {
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
}

LTexture::~LTexture() {
	free();
}

void LTexture::free() {
	if (mTexture != NULL)
	{
		SDL_DestroyTexture(mTexture);
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}

bool LTexture::loadFromFile(std::string path) {
	free();

	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	SDL_Surface* resizedSurface = NULL;
	SDL_Texture* loadedTexture = NULL;

	if (loadedSurface == NULL) {
		printf("Failed to load surface from %s. SDL Error: %s\n", path.c_str(), SDL_GetError());
	}
	else {
		if (path == "Bob.png") {
			colorKey(loadedSurface, 0xFE, 0xFE, 0xFE);
			int newWidth = loadedSurface->w * scale;
			int newHeight = loadedSurface->h * scale;
			resizedSurface = SDL_CreateRGBSurface(0, newWidth, newHeight, loadedSurface->format->BitsPerPixel, loadedSurface->format->Rmask, loadedSurface->format->Gmask, loadedSurface->format->Bmask, loadedSurface->format->Amask);
			if (SDL_BlitScaled(loadedSurface, NULL, resizedSurface, NULL) != 0) {
				printf("Failed to resize player! SDL Error: %s\n", SDL_GetError());
			}
			else {
				SDL_FreeSurface(loadedSurface);
				loadedSurface = resizedSurface;
			}
		}
		loadedTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
		if (loadedTexture == NULL) {
			printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
		}
		else {
			mWidth = loadedSurface->w;
			mHeight = loadedSurface->h;
		}
		SDL_FreeSurface(loadedSurface);
	}
	mTexture = loadedTexture;
	return mTexture != NULL;
}

void LTexture::colorKey(SDL_Surface* surface, uint8_t r, uint8_t g, uint8_t b) {
	SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(surface->format, r, g, b));
}

void spriteSheetMaker(int i, int x, int y, int w, int h) {
	spriteSheets[i].x = x;
	spriteSheets[i].y = y;
	spriteSheets[i].w = w;
	spriteSheets[i].h = h;
}

void LTexture::render(int x, int y, SDL_Rect* clip) {
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };
	if (clip != NULL)
	{
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}
	SDL_RenderCopy(renderer, mTexture, clip, &renderQuad);
}

int LTexture::getWidth() {
	return mWidth;
}

int LTexture::getHeight() {
	return mHeight;
}

bool init() {
	bool success = true;
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("Unable to initialize SDL! SDL Error: %s\n", SDL_GetError());
		success = !success;
	}
	else {
		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
		{
			printf("Warning: Linear texture filtering not enabled!");
		}

		window = SDL_CreateWindow("Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (window == NULL) {
			printf("Unable to create window! SDL Error: %s\n", SDL_GetError());
			success = !success;
		}
		else {
			renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
			if (renderer == NULL) {
				printf("Unable to create renderer! SDL Error: %s\n", SDL_GetError());
				success = !success;
			}
			else {
				SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);

				int imgFlags = IMG_INIT_PNG;
				if (!(IMG_Init(imgFlags) & imgFlags)) {
					printf("Unable to initialize SDL_image! SDL_image Error: %s\n", IMG_GetError());
					success = !success;
				}
			}
		}
	}
	return success;
}

bool loadMedia() {
	bool success = true;
	if (!player.loadFromFile("Bob.png")) {
		printf("Failed to load Bob!\n");
		success = false;
	}
	else {
		int i = 0;
		int w = 409;
		int h = 599;
		int newW = w * scale;
		int newH = h * scale;
		int x_increment = newW;
		int y_increment = newH;
		bool iterate = true;
		for (int y = 0; y < player.getHeight(); y += y_increment) {
			for (int x = 0; x < player.getWidth(); x += x_increment) {
				spriteSheetMaker(i, x, y, newW, newH);
				i++;
				if (i == 16) {
					break;
				}
			}
			if (i == 16) {
				break;
			}
		}
	}
	
	//Load texture
	if (!background.loadFromFile("2d landscape 2.png"))
	{
		printf("Failed to load background texture!\n");
		success = false;
	}

	return success;
}

void close() {
	player.free();
	background.free();

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	window = NULL;
	renderer = NULL;

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}

int main(int argc, char* argv[]) {
	if (!init()) {
		printf("Unable to initialize SDL!");
	}
	else {
		if (!loadMedia()) {
			printf("Unable to load media!");
		}
		else {
			bool quit = false;
			SDL_Event e;
			int i = 0;
			int down = 0;
			int up = 4;
			int left = 8;
			int right = 12;
			while (!quit) {
				while (SDL_PollEvent(&e)) {
					if (e.type == SDL_QUIT) {
						quit = !quit;
					}
					else if (e.type == SDL_KEYDOWN) {
						switch (e.key.keysym.sym) {
						case SDLK_w:
							if (up >= 4 && up <= 7) {
								if (up == 7) {
									up = 3;
								}
								up++;
								i = up;
							}
							break;
						case SDLK_s:
							if (down >= 0 && down <= 3) {
								if (down == 3) {
									down = -1;
								}
								down++;
								i = down;
							}
							break;
						case SDLK_d:
							if (right >= 12 && right <= 15) {
								if (right == 15) {
									right = 11;
								}
								right++;
								i = right;
							}
							break;

						case SDLK_a:
							if (left >= 8 && left <= 11) {
								if (left == 11) {
									left = 7;
								}
								left++;
								i = left;
							}
							break;
						}
					}
				}
				SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
				SDL_RenderClear(renderer);
				background.render(0, 0);
				player.render(0, 0, &spriteSheets[i]);

				SDL_RenderPresent(renderer);
			}
		}
	}
	close();
	return 0;
}