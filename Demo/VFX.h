#ifndef VFX_H_INCLUDED
#define VFX_H_INCLUDED

#include "basics.h"
#include "vec2d.h"
#include "transform.h"
#include "types.h"
#include "global.h"

SDL_Rect src_in_sheet( SDL_Rect layout, int ID, int margin );

typedef struct Generic_Sprite_data_struct{
	SDL_Texture *texture;
	SDL_FPoint offset;
	SDL_Rect *srcs;
	int frames_N;
	int period;
	float scale;
	
} Generic_Sprite_data;

typedef struct Animated_Sprite_inst_struct {

	Generic_Sprite_data *data;
	int frame;
	clock_t next_frame;
		
} Animated_Sprite_inst;

void render_animated_sprite( SDL_Renderer *R, Animated_Sprite_inst *ASi, float x, float y );



typedef struct box_9slice_struct{

	int SI, SJ;//source indices (horz, vert)
	SDL_Rect dst;
	int scale;
	int c, end;//animation parameters

} Box_9Slice;

void build_Box_9Slice( Box_9Slice *B9S, Mundo *M, int id, SDL_Rect dst, int scale, int frames );
void render_Box_9Slice( SDL_Renderer *R, Box_9Slice *B9S, Mundo *M );

typedef struct tela_abaulada_struct{
	
	SDL_Vertex *grid;
	int grid_len;
	int *indices;
	int ind_len;

	SDL_FPoint C;//center
	double fix;
	double sc;//size correction factor
	double m, b;// line for reverse-guestimating the orginal distance

} tela_abaulada;

void build_tela_abaulada( tela_abaulada *A, int cols, SDL_Rect *rect, float factor );
vec2d abaulado_reverso( tela_abaulada *A, vec2d v );
void render_tela_abaulada( SDL_Renderer *R, SDL_Texture *T, tela_abaulada *A );
void destroy_tela_abaulada( tela_abaulada *A );

#endif