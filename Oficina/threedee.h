#ifndef THREEDEE_INCLUDED
#define THREEDEE_INCLUDED

#include "basics.h"
#include <float.h>
#include "ok_lib.h"
#include "TX_Font.h"

typedef struct vec3d_struct {
	float x, y, z;
} vec3d;

typedef struct tri_struct {
	int vid[3];//vertex id
	SDL_FPoint uv [3];
	vec3d normal;
	SDL_Color C;
} Tri;

typedef struct mesh_struct {
	vec3d *verts;
	int verts_N;
	Tri *tris;
	int tris_N;
} Mesh;

typedef struct mat4x4_struct {
	float m[4][4];
} mat4x4;

typedef struct transform_3d_struct{
	vec3d basis_x, basis_y, basis_z;
	vec3d origin;
} Transform_3d;

typedef struct camera_struct{
	
	mat4x4 matProj;
	vec3d pos;
	vec3d light_direction;

} Camera_3d;

typedef struct dice_3d_struct Dice_3d;

typedef int (*dice_func)( Dice_3d *D );

typedef struct dice_3d_struct{

	Mesh *mesh;
	float scale;
	Transform_3d T;
	vec3d rotvel;
	float inertia;
	bool rolling;
	int *face_values;
	int value;
	SDL_Texture *tex;

	dice_func roll;
	dice_func read;

} Dice_3d;

void print_Dice_3d( Dice_3d *D );

typedef struct ok_vec_of(Dice_3d*) Dice_vec;
typedef struct ok_map_of(int, Dice_3d*) Dice_map;

void v3d_normalize( vec3d *v );

void reset_Transform( Transform_3d *T );

void rotate_transform( Transform_3d *T, vec3d rots );
void step_rotate_transform( Transform_3d *T, vec3d rots, float steps );

int compare_Tris_Z(const void* p1, const void* p2);

Tri newTri( float x0, float y0, float z0, float x1, float y1, float z1, float x2, float y2, float z2, 
			float uvx0, float uvy0, float uvx1, float uvy1, float uvx2, float uvy2 );

void init_Cam( Camera_3d *Cam, float fov );

Mesh load_obj( char *filename );
void init_dice_3d( Dice_3d *D, char *obj_file, SDL_Texture *tex, float zdist, float diameter );
void rotate_dice( Dice_3d *D, float dx, float dy );
int roll_dice( Dice_3d *D );
int read_dice( Dice_3d *D );

void create_prismatic_dice_3d( SDL_Renderer *R, Dice_3d *D, int sides, TX_Font *font, 
	                           float zdist, float diameter, float width_factor );
int roll_prism( Dice_3d *D );
int read_prism( Dice_3d *D );

void create_coin_3d( SDL_Renderer *R, Dice_3d *D, int heads, int tails, int res, 
					 TX_Font *font, float zdist, float diameter, float thickness_factor );

void scramble_dice( Dice_3d *D, int N );

void render_dice_3d( SDL_Renderer *R, Dice_3d *D, Camera_3d *Cam, float tx, float ty );

void test_dice( Dice_3d *D, int faces, int N, bool reset );

int destroy_dice( Dice_3d *D );

#endif