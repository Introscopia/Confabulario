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
			case 0: break;
				
			case 1: break;
				
			case 2: break;
				
			case 3: break;
				
			case 4: break;
				
			case 5: break;
				
			case 6: break;
				
			case 7: break;
				
			case 8: break;
				
			case 9: break;
				
			case 10: break;
				
			case 11: break;
				
			case 12: break;
				
			case 13: break;
				
			case 14: break;
				
			case 15: break;
				
			case 16: break;
				
			case 17: break;
				
			case 18: break;
				
			case 19: break;
				
			case 20: break;
				
			case 21: break;
				
			case 22: break;
				
			case 23: break;
				
			case 24: break;
				
			case 25:
				beco
				break;
				
			case 26: break;
				
			case 27: break;
				
			case 28: break;
				
			case 29: break;
				
			case 30: break;
				
			case 31: break;
				
			case 32: break;
				
			case 33: break;
				
			case 34: break;
				
			case 35: break;
				
			case 36: break;
				
			case 37: break;
				
			case 38: break;
				
			case 39: break;
				
			case 40: break;
				
			case 41: break;
				
			case 42: break;
				
			case 43: break;
				
			case 44: break;
				
			case 45: break;
				
			case 46: break;
				
			case 47: break;
				
			case 48: break;
				
			case 49: break;
				
			case 50: break;
				
			case 51:
				
				break;
				
			case 52: break;
				
			case 53: break;
				
			case 54: break;
				
			case 55: break;
				
			case 56: break;
				
			case 57: break;
				
			case 58:
				fhsdkjfhskjfhsdjkfhsjkfhsdjkfhsdjkfh
				break;
				
			case 59:
				beco
				break;
				
			case 60: break;
				
			case 61: break;
				
			case 62: break;
				
			case 63: break;
				
			case 64: break;
				
			case 65: break;
				
			case 66: break;
				
			case 67: break;
				
			case 68: break;
				
			case 69: break;
				
			case 70: break;
				
			case 71: break;
				
			case 72: break;
				
			case 73: break;
				
			case 74: break;
				
			case 75: break;
				
			case 76:
				beco
				break;
				
			case 77: break;
				
			case 78: break;
				
			case 79: break;
				
			case 80: break;
				
			case 81: break;
				
			case 82: break;
				
			case 83: break;
				
			case 84: break;
				
			case 85: break;
				
			case 86: break;
				
			case 87: break;
				
			case 88:
				beco
				break;
				
			case 89: break;
				
			case 90: break;
				
			case 91: break;
				
			case 92: break;
				
			case 93:
				
				break;
				
			case 94: break;
				
			case 95: break;
				
			case 96: break;
				
			case 97: break;
				
			case 98: break;
				
			case 99: break;
				
			case 100: break;
				
			case 101: break;
				
			case 102:
				
				break;
				
			case 103: break;
				
			case 104: break;
				
			case 105: break;
				
			case 106: break;
				
			case 107: break;
				
			case 108: break;
				
			case 109: break;
				
			case 110: break;
				
			case 111: break;
				
			case 112: break;
				
			case 113: break;
				
			case 114: break;
				
			case 115: break;
				
			case 116: break;
				
			case 117: break;
				
			case 118: break;
				
			case 119: break;
				
			case 120: break;
				
			case 121: break;
				
			case 122: break;
				
			case 123: break;
				
			case 124: break;
				
			case 125: break;
				
			case 126: break;
				
			case 127: break;
				
			case 128: break;
				
			case 129: break;
				
			case 130: break;
				
			case 131: break;
				
			case 132: break;
				
			case 133: break;
				
			
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

