#include "gfx.h"
#include <stdint.h>

#define NUM_CHARS      128
#define FONT_TEX_SIZE  512

typedef struct
{
	int X, Y, W, H;
	int Advance;
} GlyphInfo;

typedef struct
{
	uint8_t Char;
	SDL_Surface *Bitmap;
	GlyphInfo Info;
} Glyph;

SDL_Window *window;
SDL_Renderer *renderer;
static uint32_t user_event;
static SDL_Surface *font_surface;
static SDL_Texture *font;
static GlyphInfo glyphs[512];
static int offset_render;
static int offset_load;
static int ox, oy;

void gfx_destroy(void)
{
	SDL_FreeSurface(font_surface);
	SDL_DestroyTexture(font);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	TTF_Quit();
	SDL_Quit();
}

static int cmp_glyph(const void *a, const void *b)
{
	const Glyph *g = a;
	const Glyph *h = b;
	return g->Info.W - h->Info.W;
}

static void render_glyph(Glyph *g, int chr, TTF_Font *ttf, SDL_Color color)
{
	g->Char = chr;
	if(chr != ' ')
	{
		g->Bitmap = TTF_RenderGlyph32_Blended(ttf, chr, color);
		g->Info.W = g->Bitmap->w;
		g->Info.H = g->Bitmap->h;
	}

	int minx, maxx, miny, maxy, advance;
	TTF_GlyphMetrics32(ttf, chr, &minx, &maxx, &miny, &maxy, &advance);
	g->Info.Advance = advance;
}

static int load_glyphs_ascii(Glyph *chars, const char *file, int size)
{
	TTF_Font *ttf = TTF_OpenFont(file, size);
	if(!ttf)
	{
		fprintf(stderr, "Loading font failed: %s\n", SDL_GetError());
		return 1;
	}

	SDL_Color color = { 255, 255, 255, 0 };
	for(int i = 32; i <= 126; ++i)
	{
		render_glyph(chars + i, i, ttf, color);
	}

	TTF_CloseFont(ttf);
	return 0;
}

static void blit_glyph(Glyph *g)
{
	static int x, y, cur_line_h;

	GlyphInfo *gi = &g->Info;
	if(g->Bitmap)
	{
		if(cur_line_h == 0)
		{
			cur_line_h = gi->H;
		}

		if(x + gi->W > FONT_TEX_SIZE)
		{
			y += cur_line_h;
			x = 0;
			cur_line_h = gi->H;
		}

		SDL_Rect src = { 0, 0, gi->W, gi->H };
		gi->X = x;
		gi->Y = y;

		x += gi->W;

		SDL_Rect dst = { gi->X, gi->Y, gi->W, gi->H };
		SDL_BlitSurface(g->Bitmap, &src, font_surface, &dst);
		SDL_FreeSurface(g->Bitmap);
	}

	glyphs[offset_load + g->Char] = *gi;
}

int font_load(const char *file, int size)
{
	Glyph chars[NUM_CHARS];
	memset(chars, 0, sizeof(chars));
	if(load_glyphs_ascii(chars, file, size))
	{
		return 1;
	}

	qsort(chars, NUM_CHARS, sizeof(Glyph), cmp_glyph);
	for(int i = 0; i < NUM_CHARS; ++i)
	{
		blit_glyph(chars + i);
	}

	offset_load += NUM_CHARS;
	return 0;
}

static int load_default_fonts(void)
{
	font_surface = SDL_CreateRGBSurface(0,
		FONT_TEX_SIZE, FONT_TEX_SIZE, 32, 0xff, 0xff00, 0xff0000, 0xff000000);
	if(!font_surface)
	{
		return 1;
	}

	if(font_load("fonts/terminus.ttf", 32))
	{
		return 1;
	}

	if(font_load("fonts/arial.ttf", 20))
	{
		return 1;
	}

	font = SDL_CreateTextureFromSurface(renderer, font_surface);
	if(!font)
	{
		return 1;
	}

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	return 0;
}

int gfx_init(int width, int height, const char *title)
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		fprintf(stderr, "Error initializing SDL: %s\n", SDL_GetError());
		gfx_destroy();
		return 1;
	}

	if(TTF_Init())
	{
		printf("Loading initializing TTF: %s\n", TTF_GetError());
		gfx_destroy();
		return 1;
	}

	if(!(window = SDL_CreateWindow(title,
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		width, height, SDL_WINDOW_RESIZABLE)))
	{
		fprintf(stderr, "Error creating SDL window: %s\n", SDL_GetError());
		gfx_destroy();
		return 1;
	}

	if(!(renderer = SDL_CreateRenderer(window,
		-1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC)))
	{
		fprintf(stderr, "Error creating SDL renderer: %s\n", SDL_GetError());
		gfx_destroy();
		return 1;
	}

	if(load_default_fonts())
	{
		gfx_destroy();
		return 1;
	}

	user_event = SDL_RegisterEvents(1);
	return 0;
}

int render_char(int x, int y, int c)
{
	GlyphInfo *cur = glyphs + offset_render + c;
	if(c != ' ')
	{
		SDL_Rect src = { cur->X, cur->Y, cur->W, cur->H };
		SDL_Rect dst = { ox + x, oy + y, cur->W, cur->H };
		SDL_RenderCopy(renderer, font, &src, &dst);
	}

	return cur->Advance;
}

void render_str(int x, int y, const char *s)
{
	while(*s)
	{
		x += render_char(x, y, *s);
		++s;
	}
}

void gfx_clear(void)
{
	SDL_RenderClear(renderer);
}

void gfx_update(void)
{
	SDL_RenderPresent(renderer);
}

void fill_rect(int x, int y, int w, int h)
{
	SDL_Rect rect = { ox + x, oy + y, w, h };
	SDL_RenderFillRect(renderer, &rect);
}

void set_color(int r, int g, int b)
{
	SDL_SetTextureColorMod(font, r, g, b);
	SDL_SetRenderDrawColor(renderer, r, g, b, 255);
}

void set_font(int font)
{
	offset_render = NUM_CHARS * font;
}

void main_thread_notify(void)
{
	SDL_Event event;
	memset(&event, 0, sizeof(event));
	event.type = user_event;
	SDL_PushEvent(&event);
}

void gfx_send_quit_event(void)
{
	SDL_Event event;
	memset(&event, 0, sizeof(event));
	event.type = SDL_QUIT;
	SDL_PushEvent(&event);
}

void gfx_origin_move(int x, int y)
{
	ox += x;
	oy += y;
}