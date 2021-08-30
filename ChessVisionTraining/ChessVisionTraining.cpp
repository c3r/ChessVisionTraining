#include <SDL.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <map>

struct ChessField
{
	SDL_Rect rect;
	SDL_Color original_color;
	SDL_Color color;
};

const int CHESSBOARD_WIDTH = 8;
const int CHESSFIELD_PIXEL_SIZE = 75;
const int SCREEN_WIDTH = CHESSFIELD_PIXEL_SIZE * CHESSBOARD_WIDTH;

SDL_Window* WINDOW;
SDL_Renderer* RENDERER;

SDL_Surface* gText_surface;
SDL_Texture* gText_texture;

std::map<std::string, ChessField> gChess_board;
std::string gCurrent_field_mouseover;
std::string gCurrent_field_to_find;

constexpr int SCREEN_X = 5;
constexpr int SCREEN_Y = 25;

SDL_Color WHITE_COLOR = { 0xDD, 0xDD, 0xDD, 0xFF };
SDL_Color BLACK_COLOR = { 0x22, 0x55, 0x44, 0xFF };

bool DrawInit(int width, int height)
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		return false;

	if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2"))
		printf("Warning: Linear texture filtering not enabled!");

	WINDOW = SDL_CreateWindow("ChessVisionTraining v0.0.1", SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);

	if (WINDOW == NULL)
		return false;

	RENDERER = SDL_CreateRenderer(WINDOW, -1, SDL_RENDERER_ACCELERATED);
	SDL_SetRenderDrawBlendMode(RENDERER, SDL_BLENDMODE_BLEND);

	if (RENDERER == NULL)
		return false;

	TTF_Init();

	return true;
}

void SetDrawColor(const SDL_Color& c) { SDL_SetRenderDrawColor(RENDERER, c.r, c.g, c.b, c.a); }

void DrawChessField(const ChessField& field, const SDL_Color color)
{
	SetDrawColor(color);
	SDL_RenderFillRect(RENDERER, &field.rect);
}

void DrawChessField(const std::string symbol)
{
	ChessField cf = gChess_board[symbol];
	DrawChessField(cf, cf.color);
}

void ClearScreen() { SDL_RenderClear(RENDERER); }
void DrawScreen() { SDL_RenderPresent(RENDERER); }

std::string PixelCoordsToChessFieldSymbol(int x, int y)
{
	char x_to_char = static_cast<char>('A' - 1 + (x / CHESSFIELD_PIXEL_SIZE + 1));
	std::string key = x_to_char + std::to_string(8 - y / CHESSFIELD_PIXEL_SIZE);
	return key;
}

std::string CoordsToFieldSymbol(int x, int y)
{
	return PixelCoordsToChessFieldSymbol(x * CHESSFIELD_PIXEL_SIZE, y * CHESSFIELD_PIXEL_SIZE);
}

void InitChessBoard()
{
	Uint8 alt_cnt = 1;
	for (Uint8 i = 0; i < 8; i++)
	{
		for (Uint8 j = 0; j < 8; j++)
		{
			SDL_Color color = alt_cnt % 2 ? WHITE_COLOR : BLACK_COLOR;
			SDL_Rect rect = { i * CHESSFIELD_PIXEL_SIZE, j * CHESSFIELD_PIXEL_SIZE, CHESSFIELD_PIXEL_SIZE, CHESSFIELD_PIXEL_SIZE };
			std::string symbol = PixelCoordsToChessFieldSymbol(i * CHESSFIELD_PIXEL_SIZE, j * CHESSFIELD_PIXEL_SIZE);
			ChessField cf = { rect, color, color };
			gChess_board[symbol] = cf;
			alt_cnt++;
		}
		alt_cnt = i % 2 ? 1 : 0;
	}
}

void DrawChessBoard(const std::map<std::string, ChessField> chess_board)
{
	auto iter = chess_board.begin();
	while (iter != chess_board.end())
	{
		DrawChessField(iter->first);
		++iter;
	}
}

void FocusCurrentlyMouseOverField(int x, int y)
{
	std::string field = PixelCoordsToChessFieldSymbol(x, y);
	if (field != gCurrent_field_mouseover)
	{		
		gChess_board[gCurrent_field_mouseover].color = gChess_board[gCurrent_field_mouseover].original_color;
		gChess_board[field].color = { 0xFF, 0xFF, 0x00, 0x66 };
		gCurrent_field_mouseover = field;
	}
}

bool CheckForWin(int x, int y)
{
	std::string field = PixelCoordsToChessFieldSymbol(x, y);
	return field == gCurrent_field_to_find;
}

bool MouseClicked(Uint32 mouseButtons)
{
	return (mouseButtons & SDL_BUTTON_LMASK) != 0;
}

void ClearConsoleScreen()
{
	int n;
	for (n = 0; n < 10; n++)
		printf("\n\n\n\n\n\n\n\n\n\n");
}

std::string GetSymbolToFind()
{
	Uint8 rand_x = rand() % CHESSBOARD_WIDTH;
	Uint8 rand_y = rand() % CHESSBOARD_WIDTH;
	return CoordsToFieldSymbol(rand_x, rand_y);
}

void DrawText(std::string str, TTF_Font* font)
{	
	SDL_Color yellow = { 0xDD, 0xCC, 0x00, 0xFF };
	SDL_Color bgcolor = { 0x00, 0x00, 0x00, 0x66 };
	gText_surface = TTF_RenderText_Shaded(font, str.c_str(), yellow, bgcolor);
	gText_texture = SDL_CreateTextureFromSurface(RENDERER, gText_surface);
		
	Uint16 center_x = SCREEN_WIDTH / 2 - (gText_surface->w / 2);
	Uint16 center_y = SCREEN_WIDTH / 2 - (gText_surface->h / 2);
	SDL_Rect rect = { center_x, center_y, gText_surface->w, gText_surface->h };
	//SDL_UpdateTexture(gText_texture, &rect, gText_surface->pixels, gText_surface->pitch);

	SDL_RenderCopy(RENDERER, gText_texture, NULL, &rect);
	SDL_RenderPresent(RENDERER);
	SDL_DestroyTexture(gText_texture);
	SDL_FreeSurface(gText_surface);
}

int main(int argc, char* args[])
{
	if (!DrawInit(SCREEN_WIDTH, SCREEN_WIDTH))
	{
		printf("Initialization failed! Exiting...");
		return 1;
	}
	
	TTF_Font* font = TTF_OpenFont("arial.ttf", 200);
	if (font == NULL) {
		fprintf(stderr, "error: font not found\n");
		exit(EXIT_FAILURE);
	}

	srand(time(NULL));

	InitChessBoard();
	
	gCurrent_field_to_find = GetSymbolToFind();	

	int x, y;
	Uint32 mouse_btns;
	std::string new_field_mouseover;
	
	while (1) 
	{
		ClearScreen();
		DrawChessBoard(gChess_board);
		DrawText(gCurrent_field_to_find, font);
		SDL_PumpEvents();  
		mouse_btns = SDL_GetMouseState(&x, &y);								
		FocusCurrentlyMouseOverField(x, y);
		if (MouseClicked(mouse_btns) && CheckForWin(x, y))
		{									
			gCurrent_field_to_find = GetSymbolToFind();						
		}		
		DrawScreen();
		SDL_Delay(50);
	}

	SDL_DestroyWindow(WINDOW);
	TTF_Quit();
	SDL_Quit();
	return 0;
}