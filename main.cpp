#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<string.h>

extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define SPAWN_X 0.0
#define SPAWN_Y 3.84
#define BORDER_X_OFFSET 0.0
#define BORDER_Y_OFFSET 0.0
#define PUSHBACK 0.08
#define MOVESPEED_Y 3.0
#define MOVESPEED_X 3.0
#define PLATFORMS 5
#define LADDERS 5
#define DEBUG_MODE 0
#define TIMER 120.0
#define PRINCESS_SPAWN_X -3.7
#define PRINCESS_SPAWN_Y -2.8
#define KING_SPAWN_X -1.65
#define KING_SPAWN_Y -2.8
#define STUDENT_MODE 0
#define FULLSCREEN 0



struct PlatformInfo {
	const int t[PLATFORMS][3]= { { 0, 0, 20 }, { 1, 3, 18 }, { 11, 6, 6 }, { 2, 9, 11 }, { 2, 9, 11 } };
	// {przesuniecie x, przesuniecie y, dlugosc}
};

struct LadderInfo {
	
	const int t[LADDERS][3] = { { 5, 1, 3 }, { 14, 1, 6 }, { 12, 7, 5 }, { 2, 4, 6 }, { 7, 7, 4 } };
	// {przesuniecie x, przesuniecie y, wysokosc}
};


// narysowanie napisu txt na powierzchni screen, zaczynaj�c od punktu (x, y)
// charset to bitmapa 128x128 zawieraj�ca znaki
// draw a text txt on surface screen, starting from the point (x, y)
// charset is a 128x128 bitmap containing character images
void DrawString(SDL_Surface *screen, int x, int y, const char *text,
                SDL_Surface *charset) {
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while(*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
		};
	};

void ThrowError(SDL_Surface* screen, SDL_Texture* scrtex, SDL_Window* window, SDL_Renderer* renderer)
{
	printf("SDL_LoadBMP(images/missingfile) error: %s\n", SDL_GetError());
	SDL_FreeSurface(screen);
	SDL_DestroyTexture(scrtex);
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_Quit();
}

// narysowanie na ekranie screen powierzchni sprite w punkcie (x, y)
// (x, y) to punkt �rodka obrazka sprite na ekranie
// draw a surface sprite on a surface screen in point (x, y)
// (x, y) is the center of sprite on screen
void DrawSurface(SDL_Surface *screen, SDL_Surface *sprite, int x, int y) {
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
	};


// rysowanie pojedynczego pixela
// draw a single pixel
void DrawPixel(SDL_Surface *surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32 *)p = color;
	};


// rysowanie linii o d�ugo�ci l w pionie (gdy dx = 0, dy = 1) 
// b�d� poziomie (gdy dx = 1, dy = 0)
// draw a vertical (when dx = 0, dy = 1) or horizontal (when dx = 1, dy = 0) line
void DrawLine(SDL_Surface *screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for(int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
		};
	};


// rysowanie prostok�ta o d�ugo�ci bok�w l i k
// draw a rectangle of size l by k
void DrawRectangle(SDL_Surface *screen, int x, int y, int l, int k,
                   Uint32 outlineColor, Uint32 fillColor) {
	int i;
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for(i = y + 1; i < y + k - 1; i++)
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
	};

// rysowanie prostok�ta o d�ugo�ci bok�w l i k
// draw a rectangle of size l by k
void DrawPlatform(SDL_Surface* screen, SDL_Surface* sprite, int x, int y, int h, int n, int orientation) {
	if (orientation) {
		for (int i = 0;i < n;i++)
		{
			DrawSurface(screen, sprite, h/2 + h * (i+x), SCREEN_HEIGHT - h/2 - y*h);
		}
	}
	else {
		for (int i = 0;i < n;i++)
		{
			DrawSurface(screen, sprite, h / 2 + x * h, SCREEN_HEIGHT - h / 2 - h * (i + y));
		}
	}
};


void FreeTheMemory(SDL_Surface* screen, SDL_Texture* scrtex, SDL_Window* window, SDL_Renderer* renderer, SDL_Surface* charset, SDL_Surface* princess, SDL_Surface* ladder, SDL_Surface* box, SDL_Surface* king, SDL_Surface* player)
{
	SDL_FreeSurface(charset);
	SDL_FreeSurface(princess);
	SDL_FreeSurface(ladder);
	SDL_FreeSurface(box);
	SDL_FreeSurface(king);
	SDL_FreeSurface(player);
	SDL_FreeSurface(screen);
	SDL_DestroyTexture(scrtex);
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_Quit();
}



double* CalculateBorder(int first, int h, int n) {
	static double tab[2];

	double start = -(double)SCREEN_WIDTH / 100;

	double x1 = start + (double)first * 2*h/100;
	double x2 = start + (double)(first+n) * 2 * h / 100;

	tab[0] = x1;
	tab[1] = x2;
	
	return tab;
}

int GetFloor(double distanceY)
{
	int dist = distanceY * 100 - SCREEN_HEIGHT;

	dist/=64;

	return -dist;
}

int GetTile(double distance)
{
	int dist =  distance * 100 + SCREEN_WIDTH;

	dist /= 64;

	return dist;
}

double GetDistanceY(int floor)
{
	double dist = (double)floor * 0.64 - (double)SCREEN_HEIGHT/100;
	return -dist;
}

double GetDistanceX(int tile)
{
	double dist = (double)tile * 0.64 - (double)SCREEN_WIDTH/100;
	return dist;
}

void DrawImages(SDL_Surface *screen, SDL_Surface *player, SDL_Surface *princess, SDL_Surface *king, double distance, double distanceY)
{
	int offset = (STUDENT_MODE ? 0 : 20);
	int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);


	SDL_FillRect(screen, NULL, czarny);

	DrawSurface(screen, player, SCREEN_WIDTH / 2 + distance * SCREEN_HEIGHT / 10, SCREEN_HEIGHT / 2 + distanceY * SCREEN_HEIGHT / 10);

	DrawSurface(screen, princess, SCREEN_WIDTH / 2 + PRINCESS_SPAWN_X * SCREEN_HEIGHT / 10, SCREEN_HEIGHT / 2 + PRINCESS_SPAWN_Y * SCREEN_HEIGHT / 10 + offset);

	DrawSurface(screen, king, SCREEN_WIDTH / 2 + KING_SPAWN_X * SCREEN_HEIGHT / 10, SCREEN_HEIGHT / 2 + KING_SPAWN_Y * SCREEN_HEIGHT / 10);
}

void DrawInfoText(SDL_Surface* screen, SDL_Surface* charset, double worldTime, double fps)
{
	char text[128];
	int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	int czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);

	DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, 36, czerwony, niebieski);
	//            "template for the second project, elapsed time = %.1lf s  %.0lf frames / s"
	sprintf(text, "King Donkey by Jan Bancerewicz, TIME = %.1lf s  %.0lf fps", worldTime, fps);
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);
	//	      "Esc - exit, \030 - faster, \031 - slower"
	sprintf(text, "Esc - exit, arrows - move, n - new game / requirements met: 12345");
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text, charset);


	//draw the floor
	DrawRectangle(screen, 0, SCREEN_HEIGHT - 32, SCREEN_WIDTH, 32, niebieski, niebieski);
}

SDL_Surface loadBMP(const char* path)
{
	SDL_Surface* loadedSurface = SDL_LoadBMP(path);
	if (loadedSurface == NULL)
	{
		printf("Unable to load image %s! SDL Error: %s\n", path, SDL_GetError());
	}
	return *loadedSurface;
}

void Move(double *distance, double *distanceY, double moveSpeed, double moveSpeedY, double delta, SDL_Window* window)
{

	//calculate the borders
	PlatformInfo p;
	LadderInfo l;

	//horizontal movement

	int playerY = GetFloor(*distanceY);
	int playerX = GetTile(*distance);
	int custX = 0, custY = 0;
	double newtabX[2] = { 0,0 };
	double newtabY[2] = { 0,0 };
	double limY[2] = { GetDistanceY(playerY) , GetDistanceY(playerY) - 0.64 };
	double limX[2] = { GetDistanceX(playerX) , GetDistanceX(playerX) + 0.64 };

	for (int i = 0;i < PLATFORMS;i++)
	{
		if (playerY == p.t[i][1] + 1 && (playerX >= p.t[i][0] - 1 && playerX <= p.t[i][0] + p.t[i][2]))
		{
			double limX[2] = { GetDistanceX(p.t[i][0]) , GetDistanceX(p.t[i][0] + p.t[i][2]) };
			newtabX[0] = limX[0];
			newtabX[1] = limX[1];

			custX = 1;
		}
	}

	//if custom platform
	if (custX)
	{
		if (*distance <newtabX[1] && *distance >newtabX[0]) {
			*distance += moveSpeed * delta;
		}
		else { *distance -= moveSpeed * PUSHBACK; }
	}
	else
	{
		if (newtabX[0] != 0 && newtabX[1] != 0)
		{
			if (*distance <(BORDER_X_OFFSET + SCREEN_WIDTH / 100) && *distance >-(BORDER_X_OFFSET + SCREEN_WIDTH / 100)) {
				*distance += moveSpeed * delta;
			}
			else { *distance -= moveSpeed * PUSHBACK; }
		}
	}


	//vertical movement

	for (int i = 0;i < LADDERS;i++)
	{
		if (playerX == l.t[i][0] && (playerY >= l.t[i][1] && playerY <= l.t[i][1] + l.t[i][2]))
		{
			double limY[2] = { GetDistanceY(l.t[i][1]) , GetDistanceY(l.t[i][1] + l.t[i][2]) - 0.64 };
			newtabY[0] = limY[0];
			newtabY[1] = limY[1];

			custY = 1;
		}
	}

	//if custom ladder
	if (custY)
	{
		if (*distanceY <newtabY[0] - 0.16 && *distanceY >newtabY[1]) {
			*distanceY += moveSpeedY * delta;
		}
		else { *distanceY -= moveSpeedY * PUSHBACK; }
	}
	else
	{
		if (newtabY[0] == 0 && newtabY[1] == 0)
		{
			if (newtabX[0] == 0 && newtabX[1] == 0)
			{
				*distanceY -= moveSpeedY * PUSHBACK;
			}
		}
		else
		{
			if (*distanceY <(BORDER_Y_OFFSET + SCREEN_HEIGHT / 100) && *distanceY >-(BORDER_Y_OFFSET + SCREEN_HEIGHT / 100)) {
				*distanceY += moveSpeedY * delta;
			}
			else { *distanceY -= moveSpeedY * PUSHBACK; }
		}

	}

	//logging and debugmode

	if (DEBUG_MODE) {
		char logtext[256];
		sprintf(logtext, "dist{ %.2lf , %.2lf }, floor: %.2d, tile: %.2d, X{ %.2lf , %.2lf }, Y{ %.2lf , %.2lf }", *distance, *distanceY, playerY, playerX, newtabX[0], newtabX[1], newtabY[0], newtabY[1]);
		SDL_SetWindowTitle(window, logtext);
	}
	else {
		SDL_SetWindowTitle(window, "Platformer template");
	}

}





// main
#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char **argv) {
	int t1, t2, quit, frames, rc;
	double delta, worldTime, fpsTimer, fps, distance, distanceY, moveSpeed, moveSpeedY;
	SDL_Event event;
	SDL_Surface *screen, *charset, *princess, *ladder, *box, *king, *player;
	SDL_Texture *scrtex;
	SDL_Window *window;
	SDL_Renderer *renderer;


	if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
		}

	rc =(FULLSCREEN ? SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP, &window, &renderer) :
		SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer));
	
	if(rc != 0) {
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return 1;
		};
	
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

	SDL_ShowCursor(SDL_DISABLE);

	screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

	scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

	

	// wczytanie obrazka cs8x8.bmp
	charset = SDL_LoadBMP("./images/cs8x8.bmp");
	
	if(charset == NULL) {
		ThrowError(screen,scrtex,window,renderer);
		return 1;
		};
	SDL_SetColorKey(charset, true, 0x000000);


	// load the images
	player = SDL_LoadBMP("./images/player.bmp");
	if(player == NULL) {
		ThrowError(screen, scrtex, window, renderer);
		return 1;
		};

	king = SDL_LoadBMP("./images/monkey.bmp");
	if (king == NULL) {
		ThrowError(screen, scrtex, window, renderer);
		return 1;
	};

	box = SDL_LoadBMP("./images/box.bmp");
	if (box == NULL) {
		ThrowError(screen, scrtex, window, renderer);
		return 1;
	};

	ladder = SDL_LoadBMP("./images/ladder.bmp");
	if (ladder == NULL) {
		ThrowError(screen, scrtex, window, renderer);
		return 1;
	};

	if (STUDENT_MODE){
		princess = SDL_LoadBMP("./images/princess.bmp");
	}else{
		princess = SDL_LoadBMP("./images/pixelart.bmp");
	}

	if (princess == NULL) {
		ThrowError(screen, scrtex, window, renderer);
		return 1;
	};


	t1 = SDL_GetTicks();

	frames = 0;
	fpsTimer = 0;
	fps = 0;
	quit = 0;
	worldTime = TIMER;
	distance = SPAWN_X;
	distanceY = SPAWN_Y;
	moveSpeed = 0;
	moveSpeedY = 0;


	//petla rysowania

	while(!quit) {
		t2 = SDL_GetTicks();

		// here t2-t1 is the time in milliseconds since
		// the last screen was drawn
		// delta is the same time in seconds
		delta = (t2 - t1) * 0.001;
		t1 = t2;

		if (worldTime > 0) {
			worldTime -= delta;
		}

		// move the player
		Move(&distance, &distanceY, moveSpeed, moveSpeedY, delta, window);

		PlatformInfo p;
		LadderInfo l;

		// draw all images here
		DrawImages(screen, player, princess, king, distance, distanceY);

		fpsTimer += delta;
		if(fpsTimer > 0.5) {
			fps = frames * 2;
			frames = 0;
			fpsTimer -= 0.5;
			};

		// tekst informacyjny / info text
		DrawInfoText(screen, charset, worldTime, fps);


		//draw the platforms

		for (int i = 0;i < PLATFORMS;i++)
		{
			//draw platform
			DrawPlatform(screen, box, p.t[i][0], p.t[i][1], 32, p.t[i][2], 1);
		}

		for (int i = 0;i < LADDERS;i++)
		{
			//draw ladder
			DrawPlatform(screen, ladder, l.t[i][0], l.t[i][1], 32, l.t[i][2], 0);
		}
		

		SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
		SDL_RenderCopy(renderer, scrtex, NULL, NULL);
		SDL_RenderPresent(renderer);

		// obsluga zdarzen (o ile jakies zaszly) / handling of events (if there were any)
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_KEYDOWN:
					if(event.key.keysym.sym == SDLK_ESCAPE) quit = 1;
					if (event.key.keysym.sym == SDLK_n) {
						fpsTimer = 0;fps = 0;
						worldTime = TIMER; distance = SPAWN_X; distanceY = SPAWN_Y; };
					if(event.key.keysym.sym == SDLK_UP ) { moveSpeedY = -MOVESPEED_Y;}
					if(event.key.keysym.sym == SDLK_DOWN) { moveSpeedY = MOVESPEED_X; }
					if(event.key.keysym.sym == SDLK_LEFT ) { moveSpeed = -MOVESPEED_X; }
					if(event.key.keysym.sym == SDLK_RIGHT) { moveSpeed = MOVESPEED_Y; }
					break;
				case SDL_KEYUP:
					moveSpeed = 0.0;
					moveSpeedY = 0.0;
					break;
				case SDL_QUIT:
					quit = 1;
					break;
				};
			};
		frames++;
		};

	// zwolnienie powierzchni / freeing all surfaces
	FreeTheMemory(screen, scrtex, window, renderer, charset, princess, ladder, box, king, player);
	return 0;
	};
