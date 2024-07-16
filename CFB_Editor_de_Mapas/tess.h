#ifndef TESS_H_INCLUDED
#define TESS_H_INCLUDED

#include "basics.h"
#include "vec2d.h"


typedef struct wcoord_struct {
	int w [4];
} Wcoord;

Wcoord wc(int w0, int w1, int w2, int w3);
Wcoord wc_sum( Wcoord A, Wcoord B );
Wcoord wc_plus_warr( int *A, Wcoord B );
Wcoord wc_scaled( int *A, int k ) ;
vec2d wc_to_v2d( Wcoord A );
vec2d warr_to_v2d( int *A );
void sprint_wc( Wcoord A, char *buf );



typedef struct{

	//char *name;
	//char *tags;
	int T1 [4];
	int T2 [4];
	int **seed;
	int seed_count;

} Tess;


typedef struct{
	vec2d *V;
} geo;

typedef struct regpol{
	
	int sides;
	vec2d center;
	//double angle;
	//float radius;
	geo *geometry;
	float activation;
	SDL_Color base_color;
	SDL_Color current_color;

} regular_poly;

//void load_tess_as_regpols();

bool load_tess_as_nodemap( char *code, vec2d **points, int16_t ***connections, int *N, int max_cnx, 
						   SDL_Rect dest, float edge );


#endif