#ifndef GLOBAL_H_INCLUDED
#define GLOBAL_H_INCLUDED


#include <stdbool.h>
#include "types.h"
#include "ok_lib.h"
#include "UI.h"
#include "transform.h"
#include "threedee.h"
#include "VFX.h"

#define DEBUGMODE

#define DICE_STYLE "pips"
//#define DICE_STYLE "nums"

#define MAX_NODE_CONNECTIONS 16

typedef struct maplabel_struct{

	char *str;
	vec2d pos;
	double scale;
	double angle;
	double curvature;
	bool show;
	int connections_N;
	int connections [MAX_NODE_CONNECTIONS];

} Maplabel;

typedef struct nodemap_struct{

	  int   N;
	vec2d  *points;
  int16_t **connections;
  	  int  *knowledge;
	  int  *icon_IDs;
	
	int labels_N;
	Maplabel *labels;

} Nodemap;


typedef struct mundo_struct {

	char buf [512];

	SDL_Cursor *hand_cursor;

	/*CAMERA, tela*/
	int width, height;
	int AA;
	SDL_Texture *AAtexture;
	Transform T, AAT;
	int scaleI;
	tela_abaulada *TA;
	SDL_Rect map_frame;
	SDL_Rect popup_rct;
	SDL_Rect popup_rct_gui;// popup_rct in GUI grid units.
	float nid; //node_icon_diameter
	float nir; //node_icon_radius
	float nirsq;
	float nidT;
	float nirT;

	SDL_Texture *map_icons;
	SDL_Rect map_icon_layout;

	SDL_Color pal_lo, pal_hi, pal_mo;

	Generic_Sprite_data *loc_indicator_data;
	Animated_Sprite_inst *loc_indicator;

	Camera_3d cam3d;
	Dice_vec dados_vec;
	Dice_map dados_map;

	Box_9Slice *popup_moldura;

	TX_Font font;
	TX_Font antifont;

	SDL_Texture *borders;
	int borders_cols, borders_rows;
	int borders_cw, borders_slice;

	UI_Set GUI;
	bool quit_flag;

	Nodemap grafo;
	int *visitado; 

	// variáveis específicas do Demo:
	bool COPAS;
	bool OUROS;
	int petalas;
	bool PAUS;
	bool ESPADAS;

} Mundo;


Dice_3d *dado_de_N_lados( Mundo *M, int N, SDL_Renderer *R );

void montar_mundo( SDL_Renderer *R, Mundo *M, int width, int height );
void destruir_mundo( Mundo *M );



#endif