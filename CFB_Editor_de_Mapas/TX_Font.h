#ifndef TX_FONT_H_INCLUDED
#define TX_FONT_H_INCLUDED

#include "basics.h"
#include <ctype.h>
#include <float.h>
#include <SDL_ttf.h>
#include "ok_lib.h"

#define TAB_SIZE 4

enum{ TX_ALIGN_LEFT,
	  TX_ALIGN_CENTER,
	  TX_ALIGN_RIGHT,
	  TX_JUSTIFY };

typedef struct {
	char *name;
	int ptsize;
	int16_t x [94];
	int16_t adv [94];
	//int *bx;
	//int *w;
	float space;
	int widest_char;
	int ascent, descent;
	int h;
	int line_skip;
	float scale;
	SDL_Texture *texture;

	TTF_Font *ttf;
	TTF_Font *ttf2;//secondary, to cover other unicode blocks..
	SDL_Color fg;

	  map_int_int trans_ascii_map;//maps codepoints to entries in the arrays
	          int trans_ascii_size;
	SDL_Texture **trans_ascii_textures;
			 int *trans_ascii_x;
	         int *trans_ascii_adv;
	         int *trans_ascii_h;

} TX_Font;

TX_Font render_TX_Font( SDL_Renderer *R, char *font_filename, int size, SDL_Color fg );

void destroy_TX_Font( TX_Font *font );

void TX_set_kerning( TX_Font *font, int delta );

//returns number of bytes rendercopied, and moves x forward
int TX_rendercopy_char( SDL_Renderer *R, TX_Font *font, char *string, float *x, float y );
float TX_glyph_width( TX_Font *font, char *string, int *bytes );

void TX_render_glyph( SDL_Renderer *R, TX_Font *font, char c, float x, float y, float s );

void TX_render_string( SDL_Renderer *R, TX_Font *font, char *string, float x, float y );

// This family aligns to center.
void TX_render_string_centered( SDL_Renderer *R, TX_Font *font, char *string, double x, double y );
void TX_render_string_rotated ( SDL_Renderer *R, TX_Font *font, char *string, double x, double y, double angle );
void TX_render_string_curved  ( SDL_Renderer *R, TX_Font *font, char *string, double x, double y, double angle, double curvature );

void TX_render_section( SDL_Renderer *R, TX_Font *font, char *string, int start, int end, float x, float y );

void TX_render_string_wrapped( SDL_Renderer *R, TX_Font *font, char *string, float x, float y, float width );
void TX_render_string_wrapped_aligned( SDL_Renderer *R, TX_Font *font, char *string, int x, int y, int width, int alignment );

void TX_render_wrapped_section( SDL_Renderer *R, TX_Font *font, char *string, int stop, int x, int y, int width, int alignment );

void TX_cursor_after( TX_Font *font, char *string, int stop, float *x, float *y );
void TX_cursor_after_wrapped_aligned( TX_Font *font, char *string, int stop, int width, float *x, float *y, int alignment );

// returns index of the longest line IF you pass in a valid w.
int TX_SizeText( TX_Font *font, char *string, float *w, float *h );
void TX_SizeTextUntil( TX_Font *font, char *string, int stop, float *w, float *h );

float TX_wrapped_string_height( TX_Font *font, char *string, int width );

int *TX_wrapping_indices( TX_Font *font, char *string, int width, int *lines );



typedef struct typist_struct{

	char *str;
	int len;
	int I;
	int P;  // Period: frames per character "typed"
	int fc; // frame counter, counting from P to 0 every char
	SDL_Color *cursor_color;

} Typist;

//(Typist){ NULL, NULL, 0, 0, 0, 0, NULL };

void init_typist( Typist *T, char *string, int period, SDL_Color *c );
void reinit_typist( Typist *T, char *string, int period );
void clear_typist( Typist *T );
bool typing_done( Typist *T );
void typist_step( Typist *T );
void render_typist( SDL_Renderer *R, TX_Font *font, Typist *T, float X, float Y, int w, int alignment );




#endif

/*Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut 
labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris 
nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit 
esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt 
in culpa qui officia deserunt mollit anim id est laborum.*/