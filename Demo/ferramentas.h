#ifndef FERRAMENTAS_H_INCLUDED
#define FERRAMENTAS_H_INCLUDED

#include "basics.h"
#include "global.h"
#include "DExpr.h"
#include "threedee.h"

#define DEBUGMODE


void nodes_set_connection( Nodemap *M, int i, int j, bool value );

bool nodes_connected( Nodemap *M, int i, int j );

// fills buffer with the list of connections and returns the count.
int node_get_connections( Nodemap *NM, int i, int buffer[MAX_NODE_CONNECTIONS] );

int* nodemap_path( Nodemap *NM, int origin, int destination, bool(*check_traversible)(Nodemap*, int) );

bool traversible_if_known(Nodemap* NM, int i);

void import_nodemap( char *filename, Nodemap *M );

void destroy_nodemap( Nodemap *M );


void CFB_Mensagem( SDL_Renderer *R, Mundo *M, char *msg );


int CFB_Escolhas( SDL_Renderer *R, Mundo *M, char *titulo, int num, ... );


bool CFB_Senha( SDL_Renderer *R, Mundo *M, char *titulo, char *senha, int *tentativas );


DEVal CFB_Rolar_Dados( SDL_Renderer *R, Mundo *M, char *titulo, char *expressao, int *tentativas );


int CFB_Dialogo( SDL_Renderer *R, Mundo *M, char *titulo, char *arquivo, ... );


void CFB_Navegar_Mapa( SDL_Renderer *R, Mundo *M, int *LOC );

#endif