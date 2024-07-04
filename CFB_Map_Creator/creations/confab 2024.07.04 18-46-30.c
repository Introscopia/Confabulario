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
			case 0:
				//Bem vindo ao demo de CONFABULÁRIO!/nClique para fechar essa caixa e depois clique no "?" para avançar para o próximo Nó.
				break;
				
			case 1:
				//Confabulário é uma suite de ferramentas para a criação de jogos e outros tipos de obras interativas no computador.
				break;
				
			case 2:
				//Grafo é o nome matemático dessa estrutura que você está vendo, esse "mapa" de 'nó's conectados por linhas. No mundo dos RPGs, esse estílo de organização se chama de "Point-Crawl". É uma maneira de representar e 'discretizar' um espaço.
				break;
				
			case 3:
				//Uma Biblioteca, no âmbito da programação, é uma coleção de código pronto e bem 'embalado' para realizar alguma tarefa específica. São 'módulos' de código, basicamente.
// Confabulário é uma "lib" simples, e ainda com muito espaço para crescer. Ela consiste basicamente da infraestrutura do grafo, ou 'mapa' se preferir, e dos elementos interativos. Você está lendo esse texto em um desses elementos! Tudo que aparece quando você navega a um novo nó do grafo é uma chamada à uma função CFB.
				break;
				
			case 4:
				//por que C.txt
				break;
				
			case 5:
				//Aqui ao lado você pode ver a variedade de dados platonicos disponíveis! Temos também um exótico poliedro de 16 lados, para quem interessar...
				break;
				
			case 6:
				// Dialogos
				break;
				
			case 7:
				O projeto consiste de ferramenta de criação de grafos, e uma biblioteca para a linguagem C de programação contendo toda a infraestrutura de renderização e interatividade e também diversos elementos lúdicos prontos, como dados tridimensionais, um sistema de árvores de diálogo, entre outros.
	> O que é um grafo?
		* RETURN 2
	> Me fale mais sobre a tal "biblioteca"
		* RETURN 3
	> Por que linguagem C?
		* RETURN 4
	> Quero ver os dados 3D!
		* RETURN 5
	> Sistema de diálogo?
		* RETURN 6
	> Seguir em Frente.
		* RETURN 160
				break;
				
			case 8: break;
				
			case 9: break;
				
			case 10: break;
				
			case 11:
				
				break;
				
			case 12: break;
				
			case 13:
				//40 nodes selected.
{8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159};
				break;
				
			case 14: break;
				
			case 15:
				
				break;
				
			case 16: break;
				
			case 17:
				//paus
				break;
				
			case 18: break;
				
			case 19: break;
				
			case 20: break;
				
			case 21: break;
				
			case 22:
				
				break;
				
			case 23: break;
				
			case 24: break;
				
			case 25: break;
				
			case 26: break;
				
			case 27: break;
				
			case 28:
				18 nodes selected.
25, 26, 27, 28, 29, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44.
				break;
				
			case 29:
				
				break;
				
			case 30:
				//à direita temos um exemple de grafos "orgânicos", feitos a partir de um gerador pseudo-físico escrito especialmente para o Confabulário.
//à Esquerda temos um labirinto, também gerado dentro do editor do Confabulário!
				break;
				
			case 31:
				//Aqui, à esquerda temos um padrão geométrico. Confabulário contém uma biblioteca com centenas de ladrilhos desse tipo para você usar nas suas criações!
À direita, temos algumas estrelas. Podem ser feitas com a função "Polígono" do editor.
				break;
				
			case 32: break;
				
			case 33: break;
				
			case 34: break;
				
			case 35: break;
				
			case 36: break;
				
			case 37:
				
				break;
				
			case 38: break;
				
			case 39: break;
				
			case 40: break;
				
			case 41: break;
				
			case 42:
				// copas
				break;
				
			case 43: break;
				
			case 44: break;
				
			case 45: break;
				
			case 46: break;
				
			case 47: break;
				
			case 48: break;
				
			case 49: break;
				
			case 50: break;
				
			case 51: break;
				
			case 52: break;
				
			case 53: break;
				
			case 54: break;
				
			case 55: break;
				
			case 56: break;
				
			case 57: break;
				
			case 58: break;
				
			case 59: break;
				
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
				
			case 76: break;
				
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
				
			case 88: break;
				
			case 89: break;
				
			case 90: break;
				
			case 91: break;
				
			case 92: break;
				
			case 93: break;
				
			case 94: break;
				
			case 95: break;
				
			case 96: break;
				
			case 97: break;
				
			case 98: break;
				
			case 99: break;
				
			case 100: break;
				
			case 101: break;
				
			case 102: break;
				
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
				
			case 114:
				
				break;
				
			case 115:
				//Fim do labirinto
				break;
				
			case 116: break;
				
			case 117:
				
				break;
				
			case 118:
				
				break;
				
			case 119:
				21 nodes selected.
116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136.
				break;
				
			case 120: break;
				
			case 121: break;
				
			case 122: break;
				
			case 123: break;
				
			case 124: break;
				
			case 125: break;
				
			case 126:
				
				break;
				
			case 127: break;
				
			case 128: break;
				
			case 129: break;
				
			case 130: break;
				
			case 131: break;
				
			case 132: break;
				
			case 133: break;
				
			case 134: break;
				
			case 135: break;
				
			case 136: break;
				
			case 137: break;
				
			case 138:
				//paus
				break;
				
			case 139: break;
				
			case 140: break;
				
			case 141: break;
				
			case 142: break;
				
			case 143:
				
				break;
				
			case 144: break;
				
			case 145: break;
				
			case 146: break;
				
			case 147: break;
				
			case 148:
				
				break;
				
			case 149: break;
				
			case 150: break;
				
			case 151: break;
				
			case 152: break;
				
			case 153:
				//paus
				break;
				
			case 154: break;
				
			case 155: break;
				
			case 156:
				//paus
				break;
				
			case 157: break;
				
			case 158: break;
				
			case 159: break;
				
			case 160:
				>"Ok, entendi os elementos... mas o que é possível com essas ferramentas?"
				break;
				
			case 161:
				//Platonicos
				break;
				
			case 162:
				//Precisa de um número aleatório em algum intervalo diferente de 4, 6, 8, 12, 16 e 20? Veja ao lado as "roletas" ou dados prismáticos!
				break;
				
			case 163:
				//Roletas
				break;
				
			case 164:
				Mas e se você só precisa escolher entre duas opções aleatóriamente? A melhor escolha é uma moeda, é claro! 
				break;
				
			case 165:
				
				break;
				
			case 166:
				//16-sided
				break;
				
			case 167:
				// E o sistema de rolagem de dados do confabulário faz mas que apenas gerar números aleatórios, ele também pode realizar diversos tipos de operações logicas e aritméticas!
				break;
				
			case 168:
				//Composicao + - > ...
// abre o caminho para 171
				break;
				
			case 169:
				//Pode parecer um fundamento simples, mas com essas simples ferramentas já é possível uma infinidade conceitos, desde uma linha mais tradicional de jogos, como RPGs, passando por todo o mundo da ficção interativa, e além.
				break;
				
			case 170:
				// Como um exemplo, este demo que você está lendo neste instante também é um jogo! Sim, isso mesmo! Tem quatro itens espalhados pelo mapa, veja se consegue encontrar todos eles!
				break;
				
			case 171:
				//espada
				break;
				
			case 172:
				// Parabens! Obrigado pela atenção! Vamos criar coisas legais juntos!!!
{173, 174, 175, 176, 177, 178, 179, 180, 181, 182}
				break;
				
			case 173:
				
				break;
				
			case 174: break;
				
			case 175: break;
				
			case 176: break;
				
			case 177:
				
				break;
				
			case 178: break;
				
			case 179: break;
				
			case 180: break;
				
			case 181: break;
				
			case 182: break;
				
			
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

