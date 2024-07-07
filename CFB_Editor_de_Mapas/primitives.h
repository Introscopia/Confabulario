#ifndef PRIMITIVES_H_INCLUDED
#define PRIMITIVES_H_INCLUDED

#include "vec2d.h"
/*
Graphics Primitives for SDL using RenderGeometry!
*/


//SDL_Color SDL_GetRender_SDL_Color( SDL_Renderer *R );


void gp_draw_thickLine( SDL_Renderer *R, int ax, int ay, int bx, int by, float thickness );
void gp_draw_roundedThickLine( SDL_Renderer *R, int ax, int ay, int bx, int by, float radius );
void gp_draw_arrow( SDL_Renderer *R, int ax, int ay, int bx, int by, float LT, float HB, float HH );
																//line thickness, head base, head height
void gp_draw_bezier1( SDL_Renderer *renderer, vec2d *a1, vec2d *a2, vec2d *c, int res );
void gp_draw_bezier2( SDL_Renderer *renderer, vec2d *a1, vec2d *a2, vec2d *c1, vec2d *c2, int res );
                                                              
void gp_draw_circle( SDL_Renderer *R, float x, float y, float radius );
void gp_fill_circle( SDL_Renderer *R, float x, float y, float radius );

void gp_draw_8circle(SDL_Renderer *R, float x, float y, float radius);
void gp_drawthick_8circle(SDL_Renderer *R, float x, float y, int C_radius, float L_radius );
void gp_fill_8circle(SDL_Renderer *R, float x, float y, float radius);

void gp_draw_16circle(SDL_Renderer *R, float x, float y, float radius);
void gp_drawthick_16circle(SDL_Renderer *R, float x, float y, int C_radius, float L_radius );
void gp_fill_16circle(SDL_Renderer *R, float x, float y, float radius);

void gp_draw_24circle(SDL_Renderer *R, float x, float y, float radius);
void gp_drawthick_24circle(SDL_Renderer *R, float x, float y, int C_radius, float L_radius );
void gp_fill_24circle(SDL_Renderer *R, float x, float y, float radius);

void gp_draw_36circle(SDL_Renderer *R, float x, float y, float radius);
void gp_drawthick_36circle(SDL_Renderer *R, float x, float y, int C_radius, float L_radius );
void gp_fill_36circle(SDL_Renderer *R, float x, float y, float radius);

#define circle_threshold8 8
#define circle_threshold16 50
#define circle_threshold24 180

void gp_draw_fastcircle(SDL_Renderer *R, float x, float y, float radius);
void gp_drawthick_fastcircle(SDL_Renderer *R, float x, float y, int C_radius, float L_radius );
void gp_fill_fastcircle(SDL_Renderer *R, float x, float y, float radius);

void gp_draw_pie(SDL_Renderer *R, float x, float y, float radius, float start, float end );

void gp_draw_roundedRect( SDL_Renderer *R, SDL_Rect *rect, float radius );
void gp_fill_roundedRect( SDL_Renderer *R, SDL_Rect *rect, float radius );

void gp_draw_poly( SDL_Renderer *R, vec2d *verts, int verts_count, bool close );
void gp_draw_thickRoundedPoly( SDL_Renderer *R, vec2d *verts, int verts_count, float thickness );
void gp_fill_poly( SDL_Renderer *R, vec2d *verts, int verts_count );



#endif