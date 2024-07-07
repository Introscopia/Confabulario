#include "basics.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include "ok_lib.h"
#include "vec2d.h"
#include "transform.h"
#include "primitives.h"
#include "UI.h"
#include "global.h"
#include "ferramentas.h"
#include "VFX.h"

#ifdef WIN32
  #define _WIN32_WINNT 0x0500
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
#endif





int main( int argc, char *argv[] ){

	//HWND hwnd_win = GetConsoleWindow();
	//ShowWindow(hwnd_win,SW_HIDE);
	SDL_Window *window;
	SDL_Renderer *R;
	int width = 640;
	int height = 480;
	bool loop = 1;

	SDL_SetHint( SDL_HINT_RENDER_DRIVER, "opengl" );

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
		return 3;
	}
	if (SDL_CreateWindowAndRenderer(width, height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED, &window, &R)) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window and renderer: %s", SDL_GetError());
		return 3;
	}
	//SDL_MaximizeWindow( window );
	SDL_GetWindowSize( window, &width, &height );

	SDL_SetWindowTitle( window, "Fábula" );

	srand(time(NULL));//Inicializar o gerador de números aleatórios

	IMG_Init(IMG_INIT_PNG);//Inicializar a lib de imagem

	//Inicializar a lib de texto
	if(TTF_Init() == -1){
		printf("TTF_Init: %s\n", TTF_GetError());
	}
 
	Mundo M;
	montar_mundo( R, &M, width, height );

	int LOC = 0; // indice da posição atual do jogador no grafo 

	puts("<<Entrando no Mundo>>");
	while ( loop ) { //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> L O O P <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

		discover_node( &(M.grafo), LOC );

		switch( LOC ){
			
			default:
				break;
		}

		CFB_Navegar_Mapa( R, &M, &LOC );

		if( M.quit_flag ) break;
	}
	exit:; 
	puts("<<Saindo>>");

	destruir_mundo( &M );

	TTF_Quit();

	SDL_DestroyRenderer(R);
	SDL_DestroyWindow(window);

	SDL_Quit();
	return 0;
}

