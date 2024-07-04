#ifndef TOOLS_H_INCLUDED
#define TOOLS_H_INCLUDED

#include "basics.h"
#include "vec2d.h"
#include "transform.h"


#define DELETED 0x80000000
#define KNOWLEDGE_NONE 0x00000001
#define KNOWLEDGE_SOME 0x00000002
#define KNOWLEDGE_FULL 0x00000004

typedef struct maplabel_struct{

	STRB strb;
	vec2d pos;
	double scale;
	double angle;
	double curvature;

	int connections_N;
	int connections [16];

} Maplabel;




typedef struct nodemap_struct{

	  int   N;
	vec2d  *points;
  int16_t **connections;
  	  int   max_cnx;
  int32_t  *status;
  	  int   deleted_N;
	  int  *icon_IDs;
	 char **contents;

} Nodemap;

bool nodes_connected( Nodemap *M, int i, int j );

#define MAX_NODE_CONNECTIONS 16
int node_get_connections( Nodemap *NM, int i, int buffer[MAX_NODE_CONNECTIONS] );

void nodes_set_connection( Nodemap *M, int i, int j, bool value );

void connectify( Nodemap *M, int *selected, int selected_N, float distsq );

void gen_nodemap( int N, Nodemap *M, float edge );
void destroy_nodemap( Nodemap *M );
void translate_nodemap( Nodemap *M, vec2d c );
void merge_nodemaps( Nodemap *A, Nodemap *B );

int create_node( Nodemap *M, vec2d p ); // returns index of new node

SDL_Rect src_in_sheet( SDL_Rect layout, int ID, int margin );

//PROPORTIONAL,
enum { NONE = 0, PANNING, SELECTING, TESSELATING, POLYGONNING, 
	   MAZING, LABELING, LABEL_EDITING, KNOWING, LINKING, RELAXING, 
	   TRANSLATING, SCALING, ROTATING };

typedef struct action_data_struct{
	int ACTION;
	int *selected;
	int selected_N;
	vec2d cursor;
	vec2d offset;
	double baseline;
} action_data;

void record_action( action_data *AD, int ACTION, int *selected, int selected_N, vec2d cursor, vec2d offset, double baseline );
void undo_action( action_data *AD, vec2d *points );
void redo_action( action_data *AD, vec2d *points );

typedef void (*transform_func) ( int *selected, int selected_N, vec2d *Psrc, vec2d *Pdst, vec2d cursor, vec2d offset, double baseline );

void apply_translation( int *selected, int selected_N, vec2d *Psrc, vec2d *Pdst, vec2d cursor, vec2d offset, double baseline );
void       apply_scale( int *selected, int selected_N, vec2d *Psrc, vec2d *Pdst, vec2d cursor, vec2d offset, double baseline );
void    apply_rotation( int *selected, int selected_N, vec2d *Psrc, vec2d *Pdst, vec2d cursor, vec2d offset, double baseline );

int mouse_on_node( vec2d rtm, Nodemap *M, Transform *T, int nirsq );
int mouse_on_label( vec2d rtm, Maplabel **labels, int labels_N, Transform *T, int nirsq );
bool node_label_connected( Maplabel *label, int node_id );

int consolidate_nodemap( Nodemap *M );

void export_nodemap( char *buf, Nodemap *M, Maplabel **labels, int labels_N );
void import_nodemap( char *filename, Nodemap *M, Maplabel ***labels, int *labels_N );

Nodemap build_maze( float edge, SDL_Rect dest );

#endif