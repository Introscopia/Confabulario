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
	//SDL_SetRenderDrawBlendMode( R, SDL_BLENDMODE_BLEND );

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
			case 0:
				CFB_Mensagem( R, &M, "Bem vindo ao demo de CONFABULÁRIO!\nClique para fechar essa caixa e depois clique no \"?\" para avançar para o próximo Nó." );
				break;
				
			case 1:
				CFB_Mensagem( R, &M, "Confabulário é uma suite de ferramentas para a criação de jogos e outros tipos de obras interativas no computador." );
				break;
				
			case 2:
				CFB_Mensagem( R, &M, "Grafo é o nome matemático dessa estrutura que você está vendo, esse \"mapa\" de 'nós' conectados por linhas. No mundo dos RPGs, esse estílo de organização se chama de \"Point-Crawl\". É uma maneira de representar e *discretizar* um espaço." );
				break;
				
			case 3:
				CFB_Dialogo( R, &M, "Biblioteca?", "content/Biblioteca.txt");
				break;
				
			case 4:
				CFB_Dialogo( R, &M, "Por que C?", "content/por que C.txt" );
				break;
				
			case 5:
				CFB_Mensagem( R, &M, "Aqui ao lado você pode ver a variedade de dados platonicos disponíveis! Temos também um exótico poliedro de 16 lados, para quem interessar...");
				break;
				
			case 6:{

				int var0 = -1;
				int var1 = -1;
				int final = CFB_Dialogo( R, &M,"Sobre o Sistema de Diálogo", "content/dialog-01.txt", &var0, &var1 );
				printf("var0: %d, var1: %d, final: %d\n", var0, var1, final );

				}break;
				
			case 7:{
				int prox = CFB_Dialogo( R, &M, "Ferramentas?", "content/O projeto consiste.txt" );
				if( prox > 0 ){
					nodes_set_connection( &(M.grafo), 7, prox, true );
					M.grafo.knowledge[ prox ] = 2;
				}
				} break;
				
			case 13:{// Revelar a zona Orgânica
				const int vizinhos [] = {8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159};
				for (int i = 0; i < 40; ++i ){
					M.grafo.knowledge[ vizinhos[i] ] = 2;
				}
				} break;
				
			case 17:
			case 148:
			case 156:
				M.petalas += 1;
				if( M.petalas == 3 ){
					M.PAUS = true;
					CFB_Mensagem( R, &M, "NAIPE DE PAUS ADQUIRIDO!" );
				}
				else{
					char buf [32];
					sprintf( buf, "você agora possui (%d) petalas...", M.petalas );
					CFB_Mensagem( R, &M, buf );
				}
				break;
				
			case 28:{// revelar a zona do Ladrilho Geométrico
				const int vizinhos [] = {25, 26, 27, 28, 29, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44};
				for (int i = 0; i < 18; ++i ){
					M.grafo.knowledge[ vizinhos[i] ] = 2;
				}
				}break;
				
			case 30:
				CFB_Mensagem( R, &M, "à direita temos um exemple de grafos \"orgânicos\", feitos a partir de um gerador pseudo-físico escrito especialmente para o Confabulário.\nÀ Esquerda temos um labirinto, também gerado dentro do editor do Confabulário!");
				break;
				
			case 31:
				CFB_Mensagem( R, &M, "Aqui, à esquerda temos um padrão geométrico. Confabulário contém uma biblioteca com centenas de ladrilhos desse tipo para você usar nas suas criações!\nÀ direita, temos algumas estrelas. Podem ser feitas com a função \"Polígono\" do editor.");
				break;
				
			case 42:
				M.COPAS = true;
				CFB_Mensagem( R, &M, "NAIPE DE COPAS ADQUIRIDO!" );
				break;
				
			case 115: //Fim do labirinto (ouros)
				M.OUROS = true;
				CFB_Mensagem( R, &M, "NAIPE DE OUROS ADQUIRIDO!" );
				break;

			case 119:{//Revelar a zona das estrelas
				const int vizinhos [] = {116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136};
				for (int i = 0; i < 21; ++i ){
					M.grafo.knowledge[ vizinhos[i] ] = 2;
				}
				} break;
				
			case 160:
				CFB_Mensagem( R, &M, "\"Ok, entendi os elementos... mas o que é possível com essas ferramentas?\"" );
				break;
				
			case 161:{
				DEVal resultado = CFB_Rolar_Dados( R, &M, "Platônicos", "1d4 + 1d6 + 1d8 + 1d12 + 1d20", NULL );
				} break;
				
			case 162:
				CFB_Mensagem( R, &M, "Precisa de um número aleatório em algum intervalo diferente de 4, 6, 8, 12, 16 e 20? Veja ao lado as \"roletas\" ou dados prismáticos!");
				break;
				
			case 163:{ //Roletas
				DEVal resultado = CFB_Rolar_Dados( R, &M, "Roletas", "1d3 + 1d5 + 1d10", NULL );
				} break;
				
			case 164:
				CFB_Mensagem( R, &M, "Mas e se você só precisa escolher entre duas opções aleatóriamente? A melhor escolha é uma moeda, é claro!");
				break;

			case 165:{
				DEVal resultado = CFB_Rolar_Dados( R, &M, "Moedas", "4d1", NULL );
				} break;
				
			case 166:{
				DEVal resultado = CFB_Rolar_Dados( R, &M, "Dado de 16 lados", "1d16", NULL );
				} break;
				
			case 167:
				CFB_Mensagem( R, &M, "E o sistema de rolagem de dados do confabulário faz mais que apenas gerar números aleatórios, ele também pode realizar diversos tipos de operações logicas e aritméticas!" );
				break;
				
			case 168:{
				int tentativas = 3;
				DEVal resultado = CFB_Rolar_Dados( R, &M, "Boa Sorte!", "1d10 + 1d20 + 1d1 > 15", &tentativas );
				if( resultado.total ){
					nodes_set_connection( &(M.grafo), 168, 171, true );
					M.grafo.knowledge[ 171 ] = 1;
				}
				}break;
				
			case 169:
				CFB_Mensagem( R, &M, "Pode parecer um fundamento simples, mas com essas simples ferramentas já é possível uma infinidade conceitos, desde uma linha mais tradicional de jogos, como RPGs, passando por todo o mundo da ficção interativa, e além.");
				break;
				
			case 170:{

				int resposta = CFB_Dialogo( R, &M, "O Desafio", "content/desafio.txt" );
				if( resposta == 2 ){// checar se já completou o desafio
					if( M.COPAS && M.OUROS && M.PAUS && M.ESPADAS ){
						nodes_set_connection( &(M.grafo), 170, 172, true );
						M.grafo.knowledge[ 172 ] = 1;
					}
					else{
						CFB_Mensagem( R, &M, "Não... ainda não achastes todos os itens..." );
					}
				}
				} break;
				
			case 171:
				M.ESPADAS = true;
				CFB_Mensagem( R, &M, "NAIPE DE ESPADAS ADQUIRIDO!" );
				break;
				
			case 172:{
				CFB_Mensagem( R, &M, "Parabens! Obrigado pela atenção! Vamos criar coisas legais juntos!!!");
				const int vizinhos [] = {173, 174, 175, 176, 177, 178, 179, 180, 181, 182};
				for (int i = 0; i < 21; ++i ){
					M.grafo.knowledge[ vizinhos[i] ] = 2;
				}
				} break;
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

