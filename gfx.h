#ifndef __GFX_H__
#define __GFX_H__

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

extern SDL_Window *window;
extern SDL_Renderer *renderer;

int gfx_init(int width, int height, const char *title);
void gfx_destroy(void);
void gfx_clear(void);
void gfx_update(void);
int font_load(const char *file, int size);
void set_color(int r, int g, int b);
void fill_rect(int x, int y, int w, int h);
int render_char(int x, int y, int c);
void render_str(int x, int y, const char *s);
void main_thread_notify(void);
void set_font(int font);
void gfx_send_quit_event(void);

#endif
