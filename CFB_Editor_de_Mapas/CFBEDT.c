#include "basics.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include "ok_lib.h"
#include "vec2d.h"
#include "transform.h"
#include "primitives.h"
#include "tess.h"
#include "UI.h"
#include "tools.h"

#ifdef WIN32
  #define _WIN32_WINNT 0x0500
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
#endif


void renderDrawLineT( SDL_Renderer *R, vec2d *a, vec2d *b, Transform *T ){
	SDL_RenderDrawLineF( R, atfX(a->x, T), atfY(a->y, T), atfX(b->x, T), atfY(b->y, T) );
}

void select_linked( int i, int *selected, int *selected_N, Nodemap *M ){
	for (int c = 1; c <= M->connections[i][0]; ++c ){
		int j = M->connections[i][c];
		if( !list_contains( selected, *selected_N, j ) ){
			insert_sorted( selected, selected_N, j );
			select_linked( j, selected, selected_N, M );
		}
	}
	for (int j = 0; j < i; ++j ){
		for (int c = 1; c <= M->connections[j][0]; ++c ){
			if( M->connections[j][c] == i && !list_contains( selected, *selected_N, j ) ){
				insert_sorted( selected, selected_N, j );
				select_linked( j, selected, selected_N, M );
			}
		}
	}
}




//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~O~~~~~~~~~~| M A I N |~~~~~~~~~~~O~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int main( int argc, char *argv[] ){

	//HWND hwnd_win = GetConsoleWindow();
	//ShowWindow(hwnd_win,SW_HIDE);
	SDL_Window *window;
	SDL_Renderer *R;
	int width = 640;
	int height = 480;
	bool loop = 1;
	//int mouseX, mouseY, pmouseX, pmouseY;


	/*
	If you use the default D3D9 renderer then you need to handle the SDL_RENDER_TARGETS_RESET event by re-creating all of your textures.
	If you want to avoid that, you can use the OpenGL or D3D11 renderers, by setting one of these hints before your SDL_CreateWindowAndRenderer() call:
	SDL_SetHint( SDL_HINT_RENDER_DRIVER, "opengl" );
	SDL_SetHint( SDL_HINT_RENDER_DRIVER, "direct3d11" );
    direct3d
    opengl	
    opengles2	
    opengles	
    metal
    software
	*/
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

	SDL_SetWindowTitle( window, "Criador de Mapas do Confabulário" );

	srand(time(NULL));// prime the random number generator

	char buf [1024];

	puts("...");

	IMG_Init(IMG_INIT_PNG);

	SDL_Texture *map_icons = IMG_LoadTexture( R, "Assets/map icons.png" );
	SDL_SetTextureBlendMode( map_icons, SDL_BLENDMODE_BLEND );
	SDL_Rect map_icon_layout = (SDL_Rect){ 6, 6, 64, 64 };
	puts("did icons");

	SDL_Color pal_lo = Uint32_to_SDL_Color(0x111210FF);
	SDL_Color pal_hi = Uint32_to_SDL_Color(0xe0e3d9FF);
	SDL_Color pal_mo = lerp_SDL_Color( pal_lo, pal_hi, 0.5 );

	if(TTF_Init() == -1){
		printf("TTF_Init: %s\n", TTF_GetError());
	}


	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
	TX_Font     font = render_TX_Font( R, "Assets/Jost-Medium.ttf", 18, pal_hi );
	TX_Font antifont = render_TX_Font( R, "Assets/Jost-Medium.ttf", 18, pal_lo );
	TX_Font labelfont = render_TX_Font( R, "Assets/Jost-Medium.ttf", 80, pal_hi );


	enum { XX, TB, IMPO, LUND, SS, KNOW, TESS, POLY, MAZE, LABEL };
	int DOCKED = TB;
 

	UI_Set ui;
	build_UI_Set( &ui, 10, 8, 14, 17, 2, 0.95, width, height );
	puts("built ui set");//debug

	//UI_init_static_set( &ui, 1 );
	//UI_build_static_label( &ui, 0, 0, R, "Confabulario", "TL", &font );//puts("o");

	ui.Hx = 1.95;
	ui.Vx = 0.95;
	UI_build_numset( &ui, 10, 0, &DOCKED, IMPO, TB, pal_lo, pal_mo, pal_hi );
	numset_set_label( UI_last_element( &ui ), &font, &antifont, "Importar", "CC" );
	bool export = 0;
	UI_build_toggle( &ui, 12, 0, &export, pal_lo, pal_mo, pal_hi );
	toggle_set_label( UI_last_element( &ui ), &font, &antifont, "Exportar", "CC" );

	//ui.Hx = 3.95;
	//UI_build_numset( &ui, 10, 1, &DOCKED, LUND, TB, pal_lo, pal_mo, pal_hi );
	//numset_set_label( UI_last_element( &ui ), &font, &antifont, "Carregar imagem de fundo", "CC" );
	
	ui.Hx = 1.95;
	ui.Vx = 0.95;
	UI_build_numset( &ui, 10, 12, &DOCKED, TESS, TB, pal_lo, pal_mo, pal_hi );
	numset_set_label( UI_last_element( &ui ), &font, &antifont, "Ladrilhar", "CC" );
	
	UI_build_numset( &ui, 12, 12, &DOCKED, POLY, TB, pal_lo, pal_mo, pal_hi );
	numset_set_label( UI_last_element( &ui ), &font, &antifont, "Polígono", "CC" );

	UI_build_numset( &ui, 10, 13, &DOCKED, MAZE, TB, pal_lo, pal_mo, pal_hi );
	numset_set_label( UI_last_element( &ui ), &font, &antifont, "Labirinto", "CC" );

	UI_build_numset( &ui, 12, 13, &DOCKED, LABEL, TB, pal_lo, pal_mo, pal_hi );
	numset_set_label( UI_last_element( &ui ), &font, &antifont, "Legenda", "CC" );

	ui.Hx = 3.95;
	UI_build_numset( &ui, 10, 14, &DOCKED, SS, TB, pal_lo, pal_mo, pal_hi );
	numset_set_label( UI_last_element( &ui ), &font, &antifont, "Selecionar ícone", "CC" );

	ui.Vx = 0.95;
	ui.Hx = 1.95;
	int node_view = 0;
	UI_build_numset( &ui, 10, 15, &node_view, 0, 1, pal_lo, pal_mo, pal_hi );
	numset_set_label( UI_last_element( &ui ), &font, &antifont, "Ícones", "CC" );
	UI_build_numset( &ui, 12, 15, &node_view, 1, 0, pal_lo, pal_mo, pal_hi );
	numset_set_label( UI_last_element( &ui ), &font, &antifont, "Índices", "CC" );

	ui.Hx = 3.95;
	UI_build_numset( &ui, 10, 16, &DOCKED, KNOW, TB, pal_lo, pal_mo, pal_hi );
	numset_set_label( UI_last_element( &ui ), &font, &antifont, "Conhecimento", "CC" );

	ui.Hx = 9.95;
	ui.Vx = 16.95;
	SDL_Rect map_frame_dst = calc_rect( &ui, 0, 0 );


	UI_Set TB_ui;
	build_UI_Set( &TB_ui, 1, 8, 14, 17, 3.95, 10.95, width, height );
	int tb_state = 0;
	char *instructions = NULL;
	load_file_as_str( "Assets/instrucoes.txt", &instructions );
	int ibs = (strlen( instructions )+1);
	STRB TB_STRB;
	STRB_init( &TB_STRB, ibs );
	UI_build_textbox( &TB_ui, 10, 1, &TB_STRB, &font, 16, R, pal_lo, pal_mo, pal_hi );
	UI_Interactive *textbox = UI_last_element( &TB_ui );
	textbox_data *tbdat = (textbox_data*) (textbox->data);
	//puts("textbox built.");

	UI_Set FB_ui;
	build_UI_Set( &FB_ui, 1, 8, 14, 17, 3.95, 10.95, width, height );
	bool file_browser_bang = 0;
	char browsen_path [512]; browsen_path[0] = '\0';
	UI_build_file_browser( &FB_ui, 10, 1, &font, &antifont, &file_browser_bang, browsen_path, pal_lo, pal_mo, pal_hi );
	puts("file_browser built");
	//sprintf( buf, "%s%s", SDL_GetBasePath(), "creations" );
	//file_browser_set_path( FB_ui.set + 0, buf );
	//puts("file_browser path set");

	UI_Set SS_ui;
	build_UI_Set( &SS_ui, 2, 8, 14, 17, 3.95, 9.95, width, height );
	int selected_icon = -1;
	bool *icon_selection = calloc( 36, sizeof(bool) );
	UI_build_sprite_picker( &SS_ui, 10, 1, map_icons, 64, 64, 0, 1, &selected_icon, icon_selection, pal_lo, pal_mo, pal_hi );
	bool randomize_icons = 0;
	SS_ui.Vx = 0.95;
	UI_build_toggle( &SS_ui, 10, 11, &randomize_icons, pal_lo, pal_mo, pal_hi );
	toggle_set_label( UI_last_element( &SS_ui ), &font, &antifont, "Aleatorizar ícones", "CC" );
	
	#define faves_N 14
	char fave_tesses [faves_N][8] = { "KNPQU_2", "NP_1", "NP_3", "NPS_1", "P", "t6020", "t6022", 
									  "t6373", "t6477", "t2002", "t1009", "NQTUVW", "t4u004", "t4u024" };
	UI_Set TESS_ui;
	build_UI_Set( &TESS_ui, 2, 8, 14, 17, 1.95, 0.95, width, height );
	UI_init_static_set( &TESS_ui, 2 );
	UI_build_static_label( &TESS_ui, 10, 4, R, "Código do ladrilho:", "TL", &font );
	bool random_tess = 0;
	UI_build_toggle( &TESS_ui, 12, 4, &random_tess, pal_lo, pal_mo, pal_hi );
	toggle_set_label( UI_last_element( &TESS_ui ), &font, &antifont, "Sortear", "CC" );
	TESS_ui.Hx = 3.95;
	TESS_ui.Vx = 4;
	UI_build_static_label( &TESS_ui, 10, 6, R, "(o \"tiling\" conforme http://chequesoto.info/tiling/)\nXadrez padrão: S\nColmeia hexagonal: K\nTriângulos: t1011", "TL", &font );
	TESS_ui.Vx = 0.95;
	char tess_code [16] = "";
	strcpy( tess_code, fave_tesses + (rand() % faves_N) );
	UI_build_text_line_input( &TESS_ui, 10, 5, tess_code, 16, &font, pal_lo, pal_mo, pal_hi );

	UI_Set POLY_ui;
	build_UI_Set( &POLY_ui, 2, 8, 14, 17, 3.95, 0.95, width, height );
	UI_init_static_set( &POLY_ui, 2 );
	UI_build_static_label( &POLY_ui, 10, 4, R, "Lados:", "TL", &font );
	int poly_sides = 3;
	UI_build_number_input( &POLY_ui, 10, 5, &poly_sides, "%d", &font, pal_lo, pal_mo, pal_hi );
	UI_build_static_label( &POLY_ui, 10, 6, R, "Pulos:", "TL", &font );
	int poly_skip = 1;
	UI_build_number_input( &POLY_ui, 10, 7, &poly_skip, "%d", &font, pal_lo, pal_mo, pal_hi );

	UI_Set LABEL_ui;
	build_UI_Set( &LABEL_ui, 0, 8, 14, 17, 3.95, 4, width, height );
	UI_init_static_set( &LABEL_ui, 1 );
	UI_build_static_label( &LABEL_ui, 10, 4, R, "Clique para colocar uma Legenda", "C", &font );


	UI_Set KNOW_ui;
	build_UI_Set( &KNOW_ui, 6, 8, 14, 17, 3.95, 0.95, width, height );
	UI_init_static_set( &KNOW_ui, 3 );
	UI_build_static_label( &KNOW_ui, 10, 2, R, "Conhecimento inicial do mapa", "CC", &font );
	
	UI_build_static_label( &KNOW_ui, 10, 3, R, "Visão:", "TL", &font );
	int knowledge_view = 0;
	KNOW_ui.Hx = 1.95;
	UI_build_numset( &KNOW_ui, 10, 4, &knowledge_view, 0, 1, pal_lo, pal_mo, pal_hi );
	numset_set_label( UI_last_element( &KNOW_ui ), &font, &antifont, "Simbólica", "CC" );
	UI_build_numset( &KNOW_ui, 12, 4, &knowledge_view, 1, 0, pal_lo, pal_mo, pal_hi );
	numset_set_label( UI_last_element( &KNOW_ui ), &font, &antifont, "Simulada", "CC" );

	SDL_Texture *knowledge_icons = IMG_LoadTexture(R, "Assets/knowledge icons.png");

	KNOW_ui.Hx = 3.95;
	UI_build_static_label( &KNOW_ui, 10, 6, R, "Aplicar:", "TL", &font );
	int knowledge_brush = -1;
	UI_build_numset( &KNOW_ui, 10, 7, &knowledge_brush, KNOWLEDGE_FULL, -1, pal_lo, pal_mo, pal_hi );
	numset_set_label( UI_last_element( &KNOW_ui ), &font, &antifont, "Total [T]", "CC" );
	UI_build_numset( &KNOW_ui, 10, 8, &knowledge_brush, KNOWLEDGE_SOME, -1, pal_lo, pal_mo, pal_hi );
	numset_set_label( UI_last_element( &KNOW_ui ), &font, &antifont, "Parcial [P]", "CC" );
	UI_build_numset( &KNOW_ui, 10, 9, &knowledge_brush, KNOWLEDGE_NONE, -1, pal_lo, pal_mo, pal_hi );
	numset_set_label( UI_last_element( &KNOW_ui ), &font, &antifont, "Nenhum [N]", "CC" );
	bool detect_neighbors = 0;
	UI_build_toggle( &KNOW_ui, 10, 10, &detect_neighbors, pal_lo, pal_mo, pal_hi );
	toggle_set_label( UI_last_element( &KNOW_ui ), &font, &antifont, "Detectar Vizinhos", "CC" );

	int scaleI = 5;
	Transform T = { 0, 0, width * 0.3, height/2, 1.61051, 0.62092 };
	vec2d mouse, rtm;
	vec2d mf_mouse;


	Nodemap NM;
	float edge = 45;
	float edgesq = sq(1.333*edge);

	int labels_N = 0;
	Maplabel **labels = NULL;

	if( argc < 2 ){
		NM.N = random( 64, 128 );
		gen_nodemap( NM.N, &NM, edge );
	}
	else{
		import_nodemap( argv[1], &NM, &labels, &labels_N );
	}
	bool gen_latch = 1;

	bool dupe_latch = 1;

	
	SDL_Cursor *defaultcursor = SDL_GetCursor();
	SDL_Cursor *crosshair = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
	

	//floating label edit buttons
	// [T]ext, [P]os, [S]cale, [R]angle, [C]urvature
	SDL_Texture *label_icons = IMG_LoadTexture(R, "Assets/label icons.png");
	SDL_Rect label_icons_def_src = (SDL_Rect){ 0, 0, 180, 36 };
	int selected_label = -1;
	int label_icons_MO = -1;
	vec2d label_anchor;
	int label_cursor = -1;
	enum { lNON=0, lTXE, lTSL, lSCL, lROT, lCRV };
	int label_cmd = lNON;
	STRB cursor_mask;
	STRB_init( &cursor_mask, 0 );

	SDL_Texture *trash_texture = IMG_LoadTexture(R, "Assets/trash.png");
	SDL_Rect trash_src = (SDL_Rect){ 0, 0, 70, 70 };
	SDL_Rect trash_dst = (SDL_Rect){ 0, height-70, 70, 70 };
	

	/*for (int i = 0; i < NM.N; ++i ){
		NM.icon_IDs[i] = random( 24, 28 );
	}*/

	//int grabbing = -1;
	//bool dragged = 0;
	//int linking = -1;
	int ACTION = NONE;
	transform_func apply_transform;

	#define action_history_len 16
	action_data action_history [action_history_len];
	int ah_index = 0;
	for (int i = 0; i < action_history_len; ++i ){
		action_history[i].ACTION = -1;
		action_history[i].selected = NULL;
		action_history[i].selected_N = 0;
	}
	int a_done = 0;
	int a_undone = 0;
	
	vec2d st_rtm; //mouse coords at start_transform time (for resetting)

	int *selected = malloc( NM.N * sizeof(int) );
	int selected_N = 0;

	vec2d offset;
	double baseline = 0;

	//temporary transformed points, for the transformation actions.
	vec2d *ttps = malloc( NM.N * sizeof(vec2d) );

	/*float proportional_radius = 3 * edge;
	float prop_rad_sq = sq( proportional_radius );
	int *affected = malloc( NM.N * sizeof(int) );
	int affected_N = 0;*/


	STRB_copy( &TB_STRB, instructions );
	tbdat->render_flag = 1;

	#define COPY_TB_TO_NODE() if( TB_STRB.len > 0 ){\
								  NM.contents[selected[0]] = realloc( NM.contents[selected[0]], (TB_STRB.len + 1) );\
								  strcpy( NM.contents[selected[0]], TB_STRB.str );\
							  } else if( NM.contents[selected[0]] != NULL ){\
							  	  free( NM.contents[selected[0]] );\
							  	  NM.contents[selected[0]] = NULL;\
							  }
	
	tb_state = 0;

	vec2d *V = NULL;
	vec2d *F = NULL;
	int VFN = 0;
	float range_factor = 3;
	double rangemax = range_factor * edge;
	double rangemax_sq = sq( rangemax );
	double force_lo = 2;
	double force_hi = -(range_factor-1) * force_lo;

	int AA = 2;
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");
	SDL_Texture *AAtexture = SDL_CreateTexture( R, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 
												AA * map_frame_dst.w, AA * map_frame_dst.h );
	SDL_SetTextureBlendMode( AAtexture, SDL_BLENDMODE_BLEND );
	Transform AAT = { 0, 0, T.cx * AA, T.cy * AA, T.s * AA, T.invs * (1.0/AA) };
	//SDL_Rect AAdst = (SDL_Rect){ 0, 0, width, height };

	float nid = 24; //location_icon_diameter
	float nir = nid * 0.5; //location_icon_radius
	float nirsq = sq( nir );
	float nidT = nid * AAT.s;
	float nirT = nir * AAT.s;

	bool refresh = 1;

	SDL_SetRenderDrawBlendMode( R, SDL_BLENDMODE_BLEND );

	
	puts("<<Entering Main Loop>>");
	//\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\         ////////////////////////////////|||
	while ( loop ) { //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> L O O P <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<|||
	//////////////////////////////////////////////////////////         \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\|||	
		SDL_Event event;
		while( SDL_PollEvent(&event) ){

			if(event.type == SDL_QUIT){
				goto exit;
			}

			refresh = 1;

			if( label_cmd == lTXE ){
				//printf("lTXE, cursor: %d, len: %d\n", label_cursor, labels[ selected_label ]->strb.len );
				STRB_event_handler( &(labels[ selected_label ]->strb), &label_cursor, &event );
				STRB_copy( &cursor_mask, labels[ selected_label ]->strb.str );
				STRB_insert_char( &cursor_mask, '|', label_cursor );
				//event handler sets the cursor negative to tell us user has clicked, ending the text editing.
				if( label_cursor < 0 ){
					label_cmd = lNON;
					ACTION = NONE;
					SDL_StopTextInput();
				}
				continue;
			}

			int pDOCKED = DOCKED;

			char captured = UI_event_handler( &ui, &event );

			if( DOCKED != pDOCKED ){
				if( DOCKED >= TESS ){
					SDL_SetCursor(crosshair);
				}
				if( DOCKED == IMPO ){
					sprintf( buf, "%s%s", SDL_GetBasePath(), "creations" );
					file_browser_set_path( FB_ui.set + 0, buf );
				}
			}

			switch( DOCKED ){
				case TB:
				case MAZE:
					captured += UI_event_handler( &TB_ui, &event );
					break;
				case IMPO:
				case LUND:
					captured += UI_event_handler( &FB_ui, &event );
					break;
				case SS:
					captured += UI_event_handler( &SS_ui, &event );
					break;
				case KNOW:
					captured += UI_event_handler( &KNOW_ui, &event );
					break;
				case TESS:
					captured += UI_event_handler( &TESS_ui, &event );
					break;
				case POLY:
					captured += UI_event_handler( &POLY_ui, &event );
					break;
			}

			if( captured ){
				
				if( export ){

					export_nodemap( buf, &NM, labels, labels_N );

					selected_N = 0;
					int len = strlen(buf);
					buf[len-2]='\0';
					sprintf( buf+256, "\n\n\nProjeto \"%s\" exportado com sucesso.", buf );
					STRB_copy( &TB_STRB, buf+256 );
					tbdat->render_flag = 1;
					DOCKED = TB;
					tb_state = 0;
					export = 0;
				}
				else if( DOCKED == IMPO ){
					if( file_browser_bang ){
						printf("bang!! browsen_path: \"%s\"\n", browsen_path );
						int bpl = strlen( browsen_path );
						if( strcmp( browsen_path + bpl -3, "bin" ) == 0 ){
							destroy_nodemap( &NM );
							puts("destroyed old one");
							import_nodemap( browsen_path, &NM, &labels, &labels_N );
							puts("imported new one");
							selected = realloc( selected, NM.N * sizeof(int) );
							selected_N = 0;
							for (int i = selected_N; i < NM.N; ++i ) selected[i] = INT32_MAX;
							ttps = realloc( ttps, NM.N * sizeof(vec2d) );
							DOCKED = TB;
						}
						file_browser_bang = 0;
					}
				}
				else if( DOCKED == SS ){
					if( selected_icon >= 0 ){
						for ( int si = 0; si < selected_N; ++si ){
							int i = selected[si];
							NM.icon_IDs[i] = selected_icon;
						}
						selected_icon = -1;
					}
					if( randomize_icons ){
						int icon_selection_N = 0;
						for (int s = 0; s < 36; ++s ){
							if( icon_selection[s] ) icon_selection_N += 1;
						}
						int *icon_selection_ids = calloc( icon_selection_N, sizeof(int) );
						icon_selection_N = 0;
						for (int s = 0; s < 36; ++s ){
							if( icon_selection[s] ){
								icon_selection_ids[icon_selection_N++] = s;
							}
						}
						for ( int si = 0; si < selected_N; ++si ){
							int i = selected[si];
							NM.icon_IDs[i] = icon_selection_ids[ rand() % icon_selection_N ];
						}						
						free( icon_selection_ids );
						randomize_icons = 0;
					}
				}
				else if( DOCKED == KNOW ){
					if( detect_neighbors ){
						detect_neighbors = 0;
						for (int i = 0; i < NM.N; ++i ){
							if( NM.status[i] & DELETED ) continue;
							if( (NM.status[i] & KNOWLEDGE_NONE) || (NM.status[i] & 0x00000007) == 0 ){
								int cons [MAX_NODE_CONNECTIONS];
								int cn = node_get_connections( &NM, i, cons );
								for (int c = 0; c < cn; ++c ){
									if( NM.status[ cons[c] ] & KNOWLEDGE_FULL ){
										NM.status[i] &= 0xFFFFFFF8;
										NM.status[i] |= KNOWLEDGE_SOME;
										break;
									}
								}
							}
						}
					}
				}
				else if( DOCKED == TESS ){
					if( random_tess ){
						strcpy( tess_code, fave_tesses + (rand() % faves_N) );
						random_tess = 0;
					}
				}
			}
			else{
				const Uint8 *state = SDL_GetKeyboardState(NULL);

				bool selection_altered = 0;

				switch (event.type) {

					case SDL_KEYDOWN:

						if( selected_N > 0 ){

							int pA = ACTION;
							transform_func pAf = apply_transform;

							if( event.key.keysym.sym == SDLK_g ){
								ACTION = TRANSLATING;
								apply_transform = apply_translation;
							}
							/*else if( event.key.keysym.sym == SDLK_h ){
								ACTION = PROPORTIONAL;
								apply_transform = apply_translation;
							}*/
							else if( event.key.keysym.sym == SDLK_s ){
								ACTION = SCALING;
								apply_transform = apply_scale;
							}
							else if( event.key.keysym.sym == SDLK_r ){
								ACTION = ROTATING;
								apply_transform = apply_rotation;
							}
							else if( event.key.keysym.sym == SDLK_ESCAPE ){
								// CANCEL TRANSFORM
								if( pA >= TRANSLATING ){
									apply_transform( selected, selected_N, NM.points, NM.points, st_rtm, offset, baseline );
								}
								pA = NONE;
								ACTION = NONE;
							}
							// Duplicate
							else if( event.key.keysym.sym == SDLK_d && dupe_latch ){
								dupe_latch = 0;
								int *noobs = malloc( selected_N * sizeof(int) );
								int pN = NM.N;
								for (int s = 0; s < selected_N; ++s ){
									noobs[s] = create_node( &NM, NM.points[selected[s]] );
									NM.icon_IDs[noobs[s]] = NM.icon_IDs[selected[s]];
									NM.status[noobs[s]] = NM.status[selected[s]];
								}
								for( int s = 0; s < selected_N; ++s ){
									int i = selected[s];
									for(int c = 1; c <= NM.connections[i][0]; ++c ){
										int j = NM.connections[i][c];
										int sj = find_in_list( selected, selected_N, j );
										if( sj >= 0 ){
											nodes_set_connection( &NM, noobs[s], noobs[sj], 1 );
										}
									}
								}
								if( NM.N > pN ){
									selected = realloc( selected, NM.N * sizeof(int) );
									for (int i = pN; i < NM.N; ++i ) selected[i] = INT32_MAX;
									ttps = realloc( ttps, NM.N * sizeof(vec2d) );
								}
								for ( int s = 0; s < selected_N; ++s ){
									selected[s] = noobs[s];
								}
								free( noobs );
								ACTION = TRANSLATING;
								apply_transform = apply_translation;
							}
							// Apply relaxing forces
							else if( event.key.keysym.sym == SDLK_f ){
								if( ACTION != RELAXING ){
									ACTION = RELAXING;
									if( selected_N > VFN ){
										V = realloc( V, selected_N * sizeof(vec2d) );
										F = realloc( F, selected_N * sizeof(vec2d) );
										VFN = selected_N;
									}
									for (int i = 0; i < selected_N; ++i ){
										V[i] = v2dzero;
									}
								}
							}
							// Select all linked
							else if( event.key.keysym.sym == SDLK_l ){
								for (int si = selected_N-1; si >= 0; --si ){
									select_linked( selected[si], selected, &selected_N, &NM );
								}
								selection_altered = 1;
							}
							// Cut: Sever all links to/from selection
							else if( event.key.keysym.sym == SDLK_c ){
								int Si = 0;
								for (int i = 0; i < NM.N; ++i ){
									bool i_sel = 0;
									if( Si < selected_N && i == selected[Si] ){
										i_sel = 1; Si++;
									}
									for (int c = 1; c <= NM.connections[i][0]; ++c ){
										int j = NM.connections[i][c];
										if( i_sel != list_contains( selected, selected_N, j ) ){
											nodes_set_connection( &NM, i, j, 0 );
											c--;
										}
									}
								}
							}
							// connectify
							else if( event.key.keysym.sym == SDLK_a ){
								connectify( &NM, selected, selected_N, edgesq );
							}

							if( ACTION >= TRANSLATING && ACTION != pA ){

								if( pA >= TRANSLATING ){
									// FINALIZE TRANSFORM
									pAf( selected, selected_N, NM.points, NM.points, rtm, offset, baseline );
									record_action( action_history + ah_index, pA, selected, selected_N, rtm, offset, baseline );
									ah_index = cycle( ah_index + 1, 0, action_history_len-1 );
									a_done = constrain( a_done + 1, 0, action_history_len );
								}
	
								st_rtm = rtm;

								if( ACTION == TRANSLATING ){
									offset = rtm;
								}
								else{ //if( ACTION == SCALING || ACTION == ROTATING ){
									vec2d min = v2d(  999999,  999999 );
									vec2d max = v2d( -999999, -999999 );
									for (int s = 0; s < selected_N; ++s ){
										if( NM.points[ selected[s] ].x < min.x ) min.x = NM.points[ selected[s] ].x;
										if( NM.points[ selected[s] ].y < min.y ) min.y = NM.points[ selected[s] ].y;
										if( NM.points[ selected[s] ].x > max.x ) max.x = NM.points[ selected[s] ].x;
										if( NM.points[ selected[s] ].y > max.y ) max.y = NM.points[ selected[s] ].y;
									}
									offset = v2d_lerp( min, max, 0.5 );

									for (int s = 0; s < selected_N; ++s ){
										//POLARIZE POINTS
										vec2d diff = v2d_diff( NM.points[ selected[s] ], offset );
										NM.points[ selected[s] ] = v2d_to_polar(diff);//v2d( v2d_mag(diff), v2d_heading(diff) );
									}
									if( ACTION == SCALING ){
										baseline = v2d_dist( rtm, offset );
									}
									else if( ACTION == ROTATING ){
										baseline = v2d_heading( v2d_diff( rtm, offset ) );
									}
								}
							}
						}
						else{
							if( event.key.keysym.sym == SDLK_n && DOCKED != KNOW ){
								if( gen_latch ){
									gen_latch = 0;
									Nodemap nNM;
									vec2d ct = mouse;
									if( ACTION == SELECTING ){
										ct = v2d_lerp( offset, mouse, 0.5 );
										float rad = T.invs * v2d_dist( ct, mouse );
										float A = PI * sq( rad / edge );
										nNM.N = lrint( 1.25 * A );
									}
									else{
										nNM.N = 20 * random_gaussian() + 52;
									}
									//printf(">>genning new map, %d nodes.\n", nNM.N );

									gen_nodemap( nNM.N, &nNM, edge );
									translate_nodemap( &nNM, reverse_transform_v2d( &ct, &T ) );
									int pN = NM.N;
									merge_nodemaps( &NM, &nNM );
									//printf(">>>NM.N: %d\n", NM.N );
									selected = realloc( selected, NM.N * sizeof(int) );
									for (int i = pN; i < NM.N; ++i ) selected[i] = INT32_MAX;
									ttps = realloc( ttps, NM.N * sizeof(vec2d) );
									//affected = realloc( affected, NM.N * sizeof(int) );
									ACTION = NONE;
								}
							}
							else if( state[SDL_SCANCODE_LCTRL] || state[SDL_SCANCODE_RCTRL] ){
								if( event.key.keysym.sym == SDLK_l ){
									DOCKED = TESS;
								} else if( event.key.keysym.sym == SDLK_p ){
									DOCKED = POLY;
								} else if( event.key.keysym.sym == SDLK_m ){
									DOCKED = MAZE;
								}
							}
						}
						
						break;
					
					case SDL_KEYUP:

						if( DOCKED == KNOW ){
								 if( event.key.keysym.sym == SDLK_t ) knowledge_brush = KNOWLEDGE_FULL;
							else if( event.key.keysym.sym == SDLK_p ) knowledge_brush = KNOWLEDGE_SOME;
							else if( event.key.keysym.sym == SDLK_n ) knowledge_brush = KNOWLEDGE_NONE;
						}
						else if( (event.key.keysym.sym == SDLK_x || 
								  event.key.keysym.sym == SDLK_DELETE) &&
								 selected_N > 0 ){
							//DELETE NODES
							NM.deleted_N += selected_N;
							for(int s = 0; s < selected_N; s++){
								NM.status[ selected[s] ] = DELETED;
								NM.connections[ selected[s] ][0] = 0;									
								for (int i = 0; i < selected[s]; ++i ){
									nodes_set_connection( &NM, i, selected[s], 0 );
								}
								if(  NM.contents[ selected[s] ] != NULL ) free( NM.contents[ selected[s] ] );
								NM.contents[ selected[s] ] = NULL;
							}
							selected_N = 0;
						}
						else if( event.key.keysym.sym == SDLK_f ){
							ACTION = NONE;
						}
						else if( event.key.keysym.sym == SDLK_n ){
							gen_latch = 1;
						}
						else if( event.key.keysym.sym == SDLK_d ){
							dupe_latch = 1;
						}
						else if( event.key.keysym.sym == SDLK_p ){
							save_texture( "print.png", R, AAtexture );
						}
						else if( state[SDL_SCANCODE_LCTRL] || state[SDL_SCANCODE_RCTRL] ){

							if( event.key.keysym.sym == SDLK_z ){
								if( a_undone < a_done ){
									ah_index = cycle( ah_index - 1, 0, action_history_len-1 );
									undo_action( action_history + ah_index, NM.points );
									a_undone += 1;
								}
							}
							if( event.key.keysym.sym == SDLK_y ){
								if( a_undone > 0 ){
									redo_action( action_history + ah_index, NM.points );
									ah_index = cycle( ah_index + 1, 0, action_history_len-1 );
									a_undone -= 1;
								}
							}
						}

						

						break;

					case SDL_MOUSEMOTION:;

						vec2d pmouse = mouse;
						mouse = v2d( event.motion.x, event.motion.y );
						mf_mouse.x = mouse.x - map_frame_dst.x;
						mf_mouse.y = mouse.y - map_frame_dst.y;
						rtm = reverse_transform_v2d( &mf_mouse, &T );

						if( ACTION == PANNING ){
							T.cx += event.motion.xrel;
							T.cy += event.motion.yrel;
							AAT.cx = T.cx * AA;
							AAT.cy = T.cy * AA;
						}
						else if( ACTION == KNOWING ){
							int X = mouse_on_node( rtm, &NM, &T, nirsq );
							if( X >= 0 && knowledge_brush > -1 ){
								NM.status[ X ] &= 0xFFFFFFF8;
								NM.status[ X ] |= knowledge_brush;
							}
						}

						if( label_cmd <= lNON ){
							selected_label = -1;
							double min = 10101;
							for (int i = 0; i < labels_N; ++i ){
								double dsq = v2d_distsq( mouse, apply_transform_v2d( &(labels[i]->pos), &T ) );
								if( dsq < 10100 && dsq < min ){
									min = dsq;
									selected_label = i;
									label_icons_MO = -1;
									SDL_Rect dst = label_icons_def_src;
									dst.x = atfX( labels[i]->pos.x , &T ) - 0.50 * label_icons_def_src.w;
									dst.y = atfY( labels[i]->pos.y , &T ) - 1.66 * label_icons_def_src.h;
									if( cursor_in_rect( &event, &dst ) ){
										label_icons_MO = floor(( mouse.x - dst.x ) / label_icons_def_src.h);
										//if( label_icons_MO > 4 ) label_icons_MO = -1;
									}
								}
							}
						}


						break;
					
					case SDL_MOUSEBUTTONDOWN:{

						if( ACTION < TRANSLATING ){
							int X = mouse_on_node( rtm, &NM, &T, nirsq );
							bool clicked_inside = 1;
							if( !cursor_in_rect( &event, &map_frame_dst ) ){
								X = -1;
								clicked_inside = 0;
							}

							if( DOCKED == KNOW && clicked_inside && 
								event.button.button == SDL_BUTTON_LEFT ){
								ACTION = KNOWING;
								if( X >= 0 && knowledge_brush > -1 ){
									NM.status[ X ] &= 0xFFFFFFF8;
									NM.status[ X ] |= knowledge_brush;
								}
							}
							else if( X >= 0 ){
								if ( event.button.button == SDL_BUTTON_LEFT ){

									//ADDING OR REMOVING FROM SELECTION WITH SHIFT
									if( selected_N > 0 && (state[SDL_SCANCODE_LSHIFT] || state[SDL_SCANCODE_RSHIFT]) ){
										int sx = find_in_list( selected, selected_N, X );
										if( sx >= 0 ){
											delete_repack( selected, &selected_N, sx );
										}
										else{
											insert_sorted( selected, &selected_N, X );
										}
										selection_altered = 1;
									}
									//GRABBING NODE
									else if( !state[SDL_SCANCODE_LCTRL] && !state[SDL_SCANCODE_RCTRL] ){

										if( tb_state == 1 ){
											COPY_TB_TO_NODE();
										}
										
										int sx = find_in_list( selected, selected_N, X );
										if( sx < 0 ){
											selected[0] = X;
											selected_N = 1;
											selection_altered = 1;
										}

										if( NM.contents[X] == NULL ){
											STRB_clear( &TB_STRB );
										} else {
											STRB_copy( &TB_STRB, NM.contents[X] );
										}
										tbdat->render_flag = 1;
										tb_state = 1;

										offset = rtm;// v2d_diff( NM.points[X], rtm );
										ACTION = TRANSLATING;
										apply_transform = apply_translation;
									}
								}
								else if ( event.button.button == SDL_BUTTON_RIGHT ){
									selected[0] = X;
									selected_N = 0;
									ACTION = LINKING;
								}
							}
							else if( clicked_inside ){
								if ( event.button.button == SDL_BUTTON_LEFT  ){
									// CLICKED ON THE BG, DESELECT ALL
									if( !(state[SDL_SCANCODE_LSHIFT] || state[SDL_SCANCODE_RSHIFT]) ){
										selected_N = 0;
										if( tb_state == 1 ){
											COPY_TB_TO_NODE();
										}
										tb_state = 0;
										STRB_copy( &TB_STRB, instructions );
										tbdat->render_flag = 1;
									}
									if( !(state[SDL_SCANCODE_LCTRL] || state[SDL_SCANCODE_RCTRL]) ){
										if( DOCKED == TESS ){
											ACTION = TESSELATING;
											offset = rtm;
										}
										else if( DOCKED == POLY ){
											ACTION = POLYGONNING;
											offset = rtm;
										}
										else if( DOCKED == MAZE ){
											ACTION = MAZING;
											offset = rtm;
										}
										else if( DOCKED == LABEL ){
											ACTION = LABELING;
											//offset = rtm;
										}
										else{
											ACTION = SELECTING;
											offset = mouse;
										}
									}
								}
								else if ( (event.button.button == SDL_BUTTON_RIGHT || event.button.button == SDL_BUTTON_MIDDLE ) ){
									ACTION = PANNING;
								}
							}
						}

						if( selected_label >= 0 && label_icons_MO >= 0 ){
							//enum { lNON, lTXE, lTSL, lSCL, lROT, lCRV };
							ACTION = LABEL_EDITING;
							label_cmd = label_icons_MO + 1;
							label_anchor = mouse;
							label_cursor = labels[ selected_label ]->strb.len;
							if( label_cmd == lTXE ){
								SDL_StartTextInput();
								STRB_copy( &cursor_mask, labels[ selected_label ]->strb.str );
								STRB_insert_char( &cursor_mask, '|', label_cursor );
							}
							else if( label_cmd == lROT ){
								label_anchor.x = v2d_heading( v2d_diff( rtm, labels[ selected_label ]->pos ) );
							}
						}

						} break;
					
					case SDL_MOUSEBUTTONUP:{

						if( ACTION >= TRANSLATING ){
							// FINALIZE TRANSFORMS
							apply_transform( selected, selected_N, NM.points, NM.points, rtm, offset, baseline );
							record_action( action_history + ah_index, ACTION, selected, selected_N, rtm, offset, baseline );
							ah_index = cycle( ah_index + 1, 0, action_history_len-1 );
							a_done = constrain( a_done + 1, 0, action_history_len );
						}
						else{
							int X = mouse_on_node( rtm, &NM, &T, nirsq );

							if( ACTION == NONE &&
								(state[SDL_SCANCODE_LCTRL] || state[SDL_SCANCODE_RCTRL]) ){
								//DELETE NODE
								if( X >= 0 ){ 
									NM.deleted_N += 1;
									NM.status[ X ] = DELETED;
									NM.connections[ X ][0] = 0;
									for (int i = 0; i < X; ++i ){
										nodes_set_connection( &NM, i, X, 0 );
									}
									if(  NM.contents[ X ] != NULL ) free( NM.contents[ X ] );
									NM.contents[ X ] = NULL;

									selected_N = 0;

									if( tb_state == 1 ){
										COPY_TB_TO_NODE();
									}
									tb_state = 0;
									STRB_copy( &TB_STRB, instructions );
									tbdat->render_flag = 1;
								}
								//CREATE NODE
								else{
									int pN = NM.N;
									selected[0] = create_node(&NM, rtm);
									selected_N = 1;
									if( NM.N > pN ){
										selected = realloc( selected, NM.N * sizeof(int) );
										for (int i = selected_N; i < NM.N; ++i ) selected[i] = INT32_MAX;
										ttps = realloc( ttps, NM.N * sizeof(vec2d) );
									}
								}
							}
							else if( ACTION == LINKING ){
								if( X >= 0 && X != selected[0] ){
									bool Q = !nodes_connected( &NM, selected[0], X );
									nodes_set_connection( &NM, selected[0], X, Q );
								}
								else{
									X = mouse_on_label( rtm, labels, labels_N, &T, nirsq );
									if( X >= 0 ){
										bool Q = !node_label_connected( labels[X], selected[0] );
										label_set_connection( labels[X], selected[0], Q );
									}
								}
							}
							else if( ACTION == SELECTING ){

								vec2d ct = v2d_lerp( offset, mouse, 0.5 );
								ct.x -= map_frame_dst.x; ct.y -= map_frame_dst.y;
								//printf("off: %g, %g   %g, %g    %g, %g\n"
								float sqrad = sq( T.invs * v2d_dist( ct, mouse ) );
								vec2d rtct = reverse_transform_v2d( &ct, &T );
								//printf("\nSelecting. NM.N: %d. ", NM.N );
								for(int i = 0; i < NM.N; i++){
									if( NM.status[i] & DELETED ) continue;
									if( v2d_distsq( NM.points[i], rtct ) < sqrad ){
										int si = find_in_list(selected, selected_N, i );
										if( si >= 0 ){
											delete_repack( selected, &selected_N, si );
										}else{
											insert_sorted( selected, &selected_N, i );
										}
									}
								}
								selection_altered = 1;
							}
							else if( ACTION == TESSELATING ){

								//sprintf(buf, "t6020");
								SDL_Rect tessbox = (SDL_Rect){ min( offset.x, rtm.x), min( offset.y, rtm.y),
															   abs( offset.x - rtm.x), abs( offset.y - rtm.y) };
								Nodemap TNM = {0};
								if( load_tess_as_nodemap( tess_code, &(TNM.points), &(TNM.connections), 
														  &(TNM.N), 16, tessbox, edge ) ){
									puts("TESSED");
									bool already_did_selected = 0;
									if( TNM.N > NM.N ){
										selected = realloc( selected, (NM.N + TNM.N) * sizeof(int) );
										for (int i = selected_N; i < NM.N; ++i ) selected[i] = INT32_MAX;
										already_did_selected = 1;
									}
									for (int i = 0; i < TNM.N; ++i ){
										selected[i] = i;
									}
									selected_N = TNM.N;
									connectify( &TNM, selected, selected_N, edgesq );
									selected_N = 0;
									puts("connectified tess");
									merge_nodemaps( &NM, &TNM );
									puts("merged.");
									if( !already_did_selected ){
										selected = realloc( selected, NM.N * sizeof(int) );
									}
									ttps = realloc( ttps, NM.N * sizeof(vec2d) );
									DOCKED = TB;
								}
								else{
									if( tb_state == 1 ){
										COPY_TB_TO_NODE();
									}
									tb_state = 0;
									STRB_copy( &TB_STRB, "\n\n\nCódigo de ladrilho não reconhecido!" );
									tbdat->render_flag = 1;
								}
								
								SDL_StopTextInput();
							}
							else if( ACTION == POLYGONNING ){
								double d = v2d_dist( offset, rtm );
								if( d > 5 ){
									double a = atan2( rtm.y - offset.y, rtm.x - offset.x );
									double s = TWO_PI / poly_sides;
									int *inds = malloc( poly_sides * sizeof(int) );
									int pN = NM.N;
									for (int i = 0; i < poly_sides; ++i){
										vec2d v = v2d_sum( offset, v2d( d * cos(a+(i*s) ), d * sin(a+(i*s) ) ) );
										inds[i] = create_node( &NM, v );
									}
									if( poly_skip > 0 ){
										for (int i = 0; i < poly_sides; ++i ){
											nodes_set_connection( &NM, inds[i], inds[(i + poly_skip) % poly_sides], 1 );
										}
									}
									free( inds );

									if( NM.N > pN ){
										selected = realloc( selected, NM.N * sizeof(int) );
										selected_N = 0;
										for (int i = selected_N; i < NM.N; ++i ) selected[i] = INT32_MAX;
										ttps = realloc( ttps, NM.N * sizeof(vec2d) );
									}

									DOCKED = TB;
									SDL_StopTextInput();
								}
							}
							else if( ACTION == MAZING ){

								SDL_Rect box = (SDL_Rect){ min( offset.x, rtm.x), min( offset.y, rtm.y),
														   abs( offset.x - rtm.x), abs( offset.y - rtm.y) };
								if( box.w > edge && box.h > edge ){
									puts("mazin'");
									int pN = NM.N;
									Nodemap MNM = build_maze( edge, box );
									merge_nodemaps( &NM, &MNM );
									puts("merged.");
									if( NM.N > pN ){
										selected = realloc( selected, NM.N * sizeof(int) );
										selected_N = 0;
										for (int i = 0; i < NM.N; ++i ) selected[i] = INT32_MAX;
										ttps = realloc( ttps, NM.N * sizeof(vec2d) );
									}
									DOCKED = TB;
								}
							}
							else if( ACTION == LABELING ){

								int I = labels_N;
								labels_N += 1;
								labels = realloc( labels, labels_N * sizeof(Maplabel*) );

								labels[I] = calloc( 1, sizeof(Maplabel) );

								STRB_init( &(labels[I]->strb), 8 );
								STRB_copy( &(labels[I]->strb), "Legenda" );
								labels[I]->pos = rtm;
								labels[I]->scale = 0.25;
								labels[I]->angle = -HALF_PI;
								labels[I]->curvature = 0;
								labels[I]->connections_N = 0;
								for (int con = 0; con < 16; ++con ) labels[I]->connections[con] = -1;
								DOCKED = TB;
							}
							else if( ACTION == LABEL_EDITING ){
								vec2d dif = v2d_diff( mouse, label_anchor );
								#define SL selected_label
								switch( label_cmd ){

									case lTSL:
										if( coordinates_in_Rect( mouse.x, mouse.y, &trash_dst ) ){
											//Delete label.
											STRB_reset( &(labels[ SL ]->strb), 0 );
											free( labels[ SL ] );
											if( SL < labels_N-1 ){
												memmove( labels + SL, labels + SL + 1, 
														(labels_N - (SL+1)) * sizeof(Maplabel*) );
											}
											labels_N -= 1;
										}
										else{
											labels[ SL ]->pos.x += T.invs * dif.x;
											labels[ SL ]->pos.y += T.invs * dif.y;
										}
										break;

									case lSCL:
										if( mouse.x > label_anchor.x ){
											labels[ SL ]->scale *= 1 + (fabs(dif.x) * 0.03125);
										} else{
											labels[ SL ]->scale /= 1 + (fabs(dif.x) * 0.03125);
										}
										labels[ SL ]->scale = constrainD( labels[ SL ]->scale, 0.001, 1 );
										break;

									case lROT:
										labels[ SL ]->angle += v2d_heading( v2d_diff( rtm, labels[ SL ]->pos ) ) - label_anchor.x;
										break;

									case lCRV:
										if( mouse.x > label_anchor.x ){
											labels[ SL ]->curvature += map( v2d_mag(dif), 0, 300, 0, TWO_PI );
										}else{
											labels[ SL ]->curvature -= map( v2d_mag(dif), 0, 300, 0, TWO_PI );
										}
										break;
								}
								label_cmd = lNON;
							}
						}

						SDL_SetCursor(defaultcursor);
						ACTION = NONE;
						
						} break;
					
					case SDL_MOUSEWHEEL:;

						/*if( ACTION == PROPORTIONAL ){
							proportional_radius += 10 * event.wheel.y;
							prop_rad_sq = sq( proportional_radius );
						}*/
						float xrd = (mouse.x - T.cx) * T.invs;
						float yrd = (mouse.y - T.cy) * T.invs;
						scaleI -= event.wheel.y;
						set_scale( &T, pow(1.1, scaleI) );
						T.cx = mouse.x - xrd * T.s;
						T.cy = mouse.y - yrd * T.s;
						
						AAT.cx = T.cx * AA;
						AAT.cy = T.cy * AA;
						AAT.s = T.s * AA;
						AAT.invs = T.invs * (1.0/AA);
						nirT = nir * AAT.s;
						nidT = 2 * nirT;
						/*
							if( AAT.s > 1 ){
								nirT = round( nir * (1 + ((AAT.s-1)*0.5)) );
								nidT = 2 * nirT;
							}
							else{
							nirT = round( nir * AAT.s );
							nidT = nid * AAT.s;
						}*/
						//nirsq = sq( nirT * AAT.invs );
						//printf("%d %d\n", nirsq, nirT );
						break;
				}

				if( selection_altered && selected_N > 1 ){

					if( tb_state == 1 ){
						COPY_TB_TO_NODE();
					}
					sprintf( buf, "%d nodes selected.\n", selected_N );
					STRB_copy( &TB_STRB, buf );
					for (int i = 0; i < selected_N; ++i ){
						if( i < selected_N-1 ) sprintf( buf, "%d, ", selected[i] );
						else sprintf( buf, "%d.", selected[i] );
						STRB_append_str( &TB_STRB, buf );
					}
					DOCKED = TB;
					tbdat->render_flag = 1;
					tb_state = 0;
				}
			}
		}
		if( !refresh ){
			if( SDL_GetMouseState(NULL, NULL) ) refresh = 1;
		}


		if( ACTION == RELAXING ){
			//resetting forces
			for (int i = 0; i < selected_N; ++i) F[i] = v2dzero;

			//Add up repelling/attracting forces
			for ( int si = 0; si < selected_N; ++si ){
				int i = selected[si];
				for ( int sj = si+1; sj < selected_N; ++sj ){
					int j = selected[sj];
					double d = v2d_distsq( NM.points[i], NM.points[j] );
					if( d < rangemax_sq ){
						if( d == 0 ) d = 0.03125;
						vec2d f = v2d_normalize( v2d_diff( NM.points[i], NM.points[j] ) );
						d = sqrt(d);
						double mag = map( d, 0, rangemax, force_lo, force_hi );
						if( !( !nodes_connected( &NM, i, j ) && mag < 0 ) ){//never attract if we're not connected
							v2d_mult( &f, mag );
							v2d_add( F + si, f );
							v2d_sub( F + sj, f );
						}
					}
				}
			}
			//effect motion
			for ( int si = selected_N-1; si >= 0; --si ){
				v2d_add( V + si, F[si] );
				v2d_add( NM.points + selected[si], V[si] );
				v2d_mult( V + si, 0.5 );
			}

			refresh = 1;
		}


		if( refresh ){

			refresh = 0;

			SDL_SetRenderDraw_SDL_Color( R, &pal_lo );
			SDL_RenderClear( R );

			SDL_SetRenderTarget( R, AAtexture );

			SDL_SetRenderDraw_SDL_Color( R, &pal_lo );
			SDL_RenderClear( R );

			if( DOCKED == KNOW && knowledge_view == 1 ){// visão simulada
				//Draw Links
				for (int i = 0; i < NM.N; ++i){
					if( NM.status[i] & DELETED ) continue;
					vec2d *vi = NM.points + i;
					for (int c = 1; c <= NM.connections[i][0]; ++c ){
						int j = NM.connections[i][c];

						if( NM.status[j] & DELETED ) continue;

						if( (!(NM.status[i] & KNOWLEDGE_FULL) && 
							 !(NM.status[j] & KNOWLEDGE_FULL)) ) continue;

						vec2d *vj = NM.points + j;
						
						if( (NM.status[i] & KNOWLEDGE_FULL) && (NM.status[j] & KNOWLEDGE_FULL) ){
							SDL_SetRenderDraw_SDL_Color( R, &pal_hi );
						} else {
							SDL_SetRenderDraw_SDL_Color( R, &pal_mo );
						}						
						gp_draw_thickLine( R, atfX(vi->x, &AAT), atfY(vi->y, &AAT), atfX(vj->x, &AAT), atfY(vj->y, &AAT), AAT.s );
					}
				}
				//Draw Nodes
				for (int i = 0; i < NM.N; ++i){
					if( NM.status[i] & DELETED ) continue;
					if( (NM.status[i] & KNOWLEDGE_NONE) || (NM.status[i] & 0x00000007) == 0 ) continue;
					vec2d vt = apply_transform_v2d(NM.points + i, &AAT);
					
					int ID = NM.icon_IDs[ i ];
					if( NM.status[i] & KNOWLEDGE_SOME ) ID = 2;// the "?" icon
					
					SDL_Rect src = src_in_sheet( map_icon_layout, ID, 1 );
					SDL_FRect dst = (SDL_FRect){ vt.x - nirT, vt.y - nirT, nidT, nidT };
					SDL_RenderCopyF( R, map_icons, &src, &dst );
					
				}
			}
			else{
				bool transforming = ACTION >= TRANSLATING;
				if( transforming ){
					apply_transform( selected, selected_N, NM.points, ttps, rtm, offset, baseline );
				}

				//Draw Label-Node connections
				SDL_SetRenderDrawColor( R, 90,90,90,255 );
				for (int i = 0; i < labels_N; ++i ){
					for (int con = 0; con < labels[i]->connections_N; ++con ){
						vec2d A = NM.points[labels[i]->connections[con]];//v2d_lerp( NM.points[labels[i]->connections[con]], labels[i]->pos, 0.25 );
						A = apply_transform_v2d( &A, &AAT );
						vec2d B = v2d_lerp( NM.points[labels[i]->connections[con]], labels[i]->pos, 0.82 );
						B = apply_transform_v2d( &B, &AAT );
						gp_draw_thickLine( R, A.x, A.y, B.x, B.y, AAT.s * 0.8 );
					}
				}

				SDL_SetRenderDraw_SDL_Color( R, &pal_hi );
				int Si = 0;
				//Draw Links
				for (int i = 0; i < NM.N; ++i){
					if( NM.status[i] & DELETED ) continue;
					vec2d *vi = NM.points + i;
					if( transforming && Si < selected_N && i == selected[Si]){
						vi = ttps + i;
						Si++;
					}
					for (int c = 1; c <= NM.connections[i][0]; ++c ){
						int j = NM.connections[i][c];

						if( NM.status[j] & DELETED ) continue;

						vec2d *vj = NM.points + j;
						int Sj = find_in_list(selected, selected_N, j );
						if(transforming && Sj >= 0 ){
							vj = ttps + j;
						}
						gp_draw_thickLine( R, atfX(vi->x, &AAT), atfY(vi->y, &AAT), atfX(vj->x, &AAT), atfY(vj->y, &AAT), AAT.s );
					}
				}

				if( ACTION == LINKING ){
					int snap = mouse_on_node( rtm, &NM, &T, nirsq );
					if( snap == selected[0] ) snap = -1;
					if( snap >= 0 ){
						if ( nodes_connected( &NM, snap, selected[0] ) ) SDL_SetRenderDrawColor( R, 255, 16, 12, 255 );
						else SDL_SetRenderDrawColor( R, 14, 255, 84, 255 ); //SDL_SetRenderDraw_SDL_Color( R, &pal_hi );
						gp_draw_thickLine( R, atfX(NM.points[selected[0]].x, &AAT), atfY(NM.points[selected[0]].y, &AAT), 
												 atfX(NM.points[snap].x, &AAT), atfY(NM.points[snap].y, &AAT), AAT.s * 1.5 ); //renderDrawLineT( R, NM.points + selected[0], NM.points + snap, &T );
					}
					else{
						snap = mouse_on_label( rtm, labels, labels_N, &T, nirsq );
						if( snap >= 0 ){
							if( node_label_connected( labels[snap], selected[0] ) ) SDL_SetRenderDrawColor( R, 255, 16, 12, 255 );
							else SDL_SetRenderDrawColor( R, 14, 255, 84, 255 );
							gp_draw_thickLine( R, atfX(NM.points[selected[0]].x, &AAT), atfY(NM.points[selected[0]].y, &AAT), 
												  atfX(labels[snap]->pos.x, &AAT), atfY(labels[snap]->pos.y, &AAT), AAT.s * 1.5 );
						}
						else{
							SDL_SetRenderDraw_SDL_Color( R, &pal_hi );
							int x = atfX(NM.points[selected[0]].x, &AAT); 
							int y = atfY(NM.points[selected[0]].y, &AAT);
							gp_draw_thickLine( R, x, y, AA * mf_mouse.x, AA * mf_mouse.y, AAT.s );
						}
					}
				}

				Si = 0;
				font.scale = AA;
				//Draw Nodes
				for (int i = 0; i < NM.N; ++i){
					if( NM.status[i] & DELETED ) continue;

					vec2d vt;
					bool am_sel = (Si < selected_N && i == selected[Si]);
					if( transforming && am_sel ){
						vt = apply_transform_v2d(ttps + i, &AAT);
					}else{
						vt = apply_transform_v2d(NM.points + i, &AAT);
					}
					if( am_sel ){
						SDL_SetRenderDrawColor( R, 14, 84, 255, 255 );
						gp_drawthick_fastcircle( R, vt.x, vt.y, nirT+4, AAT.s );
						Si++;
					}

					if( DOCKED == KNOW && knowledge_view == 0 ){// visão simbólica
						int KNO = 0;
						     if( NM.status[i] & KNOWLEDGE_SOME ) KNO = 1;
						else if( NM.status[i] & KNOWLEDGE_FULL ) KNO = 2;
						SDL_RenderCopyF( R, knowledge_icons, &(SDL_Rect){ KNO*64, 0, 64, 64 }, 
										 &(SDL_FRect){ vt.x-nirT, vt.y-nirT, nidT, nidT });
					}
					else if( node_view == 1 ){
						sprintf( buf, "%d", i );
						SDL_SetRenderDraw_SDL_Color( R, &pal_lo );
						gp_fill_fastcircle( R, vt.x, vt.y, nirT );
						TX_render_string_centered( R, &font, buf, vt.x, vt.y );
					}
					else{
						SDL_Rect src = src_in_sheet( map_icon_layout, NM.icon_IDs[ i ], 1 );
						SDL_FRect dst = (SDL_FRect){ vt.x - nirT, vt.y - nirT, nidT, nidT };
						SDL_RenderCopyF( R, map_icons, &src, &dst );
					}
				}
				font.scale = 1;
			}

			//SDL_RenderCopy( R, map_frame, NULL, &map_frame_dst );
			SDL_SetRenderTarget( R, NULL );

			SDL_RenderCopy( R, AAtexture, NULL, &map_frame_dst );

			if( ACTION == SELECTING ){
				SDL_SetRenderDrawColor( R, 14, 84, 255, 255 );
				gp_draw_8circle( R, offset.x, offset.y, 4 );
				vec2d ct = v2d_lerp( offset, mouse, 0.5 );
				gp_draw_fastcircle( R, ct.x, ct.y, v2d_dist( ct, mouse ) );
			}
			else if( ACTION == TESSELATING || ACTION == MAZING ){
				SDL_SetRenderDrawColor( R, 14, 84, 255, 255 );
				SDL_Rect box = (SDL_Rect){ atfX( min( offset.x, rtm.x), &T ) + map_frame_dst.x, 
										   atfY( min( offset.y, rtm.y), &T ) + map_frame_dst.y,
										   T.s * abs( offset.x - rtm.x), 
										   T.s * abs( offset.y - rtm.y) };
				SDL_RenderDrawRect( R, &box );
			}
			else if( ACTION == POLYGONNING ){
				SDL_SetRenderDrawColor( R, 14, 84, 255, 255 );
				double d = v2d_dist( offset, rtm );
				double a = atan2( rtm.y - offset.y, rtm.x - offset.x );
				double s = TWO_PI / poly_sides;
				vec2d p = v2d_sum( offset, v2d( d * cos(a + ((poly_sides-1)*s)), d * sin(a + ((poly_sides-1)*s)) ) );
				for (int i = 0; i < poly_sides; ++i){
					vec2d v = v2d_sum( offset, v2d( d * cos(a+(i*s) ), d * sin(a+(i*s) ) ) );
					SDL_RenderDrawLineF( R, atfX(p.x, &T)+ map_frame_dst.x, atfY(p.y, &T)+ map_frame_dst.y, 
											   atfX(v.x, &T)+ map_frame_dst.x, atfY(v.y, &T)+ map_frame_dst.y );
					p = v;
				}
			}

			// Draw Label trash can
			if( label_cmd == lTSL ){
				SDL_Rect src = trash_src;
				if( coordinates_in_Rect( mouse.x, mouse.y, &trash_dst ) ){
					src.y += 70;
				}
				SDL_RenderCopy( R, trash_texture, &src, &trash_dst );
			}

			//Draw Labels
			for (int i = 0; i < labels_N; ++i ){
				double lx = atfX( labels[i]->pos.x, &T) + map_frame_dst.x;
				double ly = atfY( labels[i]->pos.y, &T) + map_frame_dst.y;
				if( i == selected_label && label_cmd > lNON ){//lTXE, lTSL, lSCL, lROT, lCRV
					labelfont.scale = labels[i]->scale * T.s;
					vec2d dif = v2d_diff( mouse, label_anchor );
					switch( label_cmd ){
						case lTXE:
							TX_render_string_curved( R, &labelfont, cursor_mask.str,
													 lx, ly,
													 labels[i]->angle, labels[i]->curvature );
							break;
						case lTSL:
							TX_render_string_curved( R, &labelfont, labels[i]->strb.str,
													 lx + dif.x, ly + dif.y, 
													 labels[i]->angle, labels[i]->curvature );
							break;
						case lSCL:
							SDL_SetRenderDrawColor( R, 14, 84, 255, 255 );
							gp_draw_8circle( R, label_anchor.x, label_anchor.y, 4 );
							double scl = labels[i]->scale;
							if( mouse.x > label_anchor.x ){
								scl *= 1 + (fabs(dif.x) * 0.03125);
							}else{
								scl /= 1 + (fabs(dif.x) * 0.03125);
							}
							scl = constrainF( scl, 0.001, 1 );
							labelfont.scale = scl * T.s;
							TX_render_string_curved( R, &labelfont, labels[i]->strb.str,
													 lx, ly, labels[i]->angle, labels[i]->curvature );
							break;
						case lROT:;
							double A = labels[i]->angle + v2d_heading( v2d_diff( rtm, labels[i]->pos ) ) - label_anchor.x;
							TX_render_string_curved( R, &labelfont, labels[i]->strb.str,
													 lx, ly, A, labels[i]->curvature );
							break;
						case lCRV:;
							SDL_SetRenderDrawColor( R, 14, 84, 255, 255 );
							gp_draw_8circle( R, label_anchor.x, label_anchor.y, 4 );
							double C = labels[i]->curvature;
							if( mouse.x > label_anchor.x ){
								C += map( v2d_mag(dif), 0, 300, 0, TWO_PI );
							}else{
								C -= map( v2d_mag(dif), 0, 300, 0, TWO_PI );
							}
							TX_render_string_curved( R, &labelfont, labels[i]->strb.str,
													 lx, ly, labels[i]->angle, C );
							break;
					}
				}
				else{
					labelfont.scale = labels[i]->scale * T.s;
					TX_render_string_curved( R, &labelfont, labels[i]->strb.str,
											 lx, ly, labels[i]->angle, labels[i]->curvature );
				}
			}

			//Draw Floating label controls
			if( ACTION == NONE && selected_label >= 0 && label_cmd <= lNON && 
					labels[ selected_label ]->scale * T.s > 0.15 ){
				SDL_Rect dst = label_icons_def_src;
				dst.x = atfX( labels[ selected_label ]->pos.x , &T ) - 0.50 * label_icons_def_src.w;
				dst.y = atfY( labels[ selected_label ]->pos.y , &T ) - 1.66 * label_icons_def_src.h;
				SDL_RenderCopy( R, label_icons, &label_icons_def_src, &dst );
				if( label_icons_MO >= 0 ){
					SDL_Rect src = (SDL_Rect){ label_icons_MO * label_icons_def_src.h, label_icons_def_src.h, label_icons_def_src.h, label_icons_def_src.h };
					dst.x += src.x;
					dst.w = label_icons_def_src.h;
					SDL_RenderCopy( R, label_icons, &src, &dst );
				}
			}

			UI_display( R, &ui );

			switch( DOCKED ){
				case TB:
				case MAZE:
					UI_display( R, &TB_ui );
					break;
				case IMPO:
				case LUND:
					UI_display( R, &FB_ui );
					break;
				case SS:
					UI_display( R, &SS_ui );
					break;
				case KNOW:
					UI_display( R, &KNOW_ui );
					break;
				case TESS:
					UI_display( R, &TESS_ui );
					break;
				case POLY:
					UI_display( R, &POLY_ui );
					break;
				case LABEL:
					UI_display( R, &LABEL_ui );
					break;
			}

			//SDL_SetRenderDrawColor( R, 50, 50, 50, 50 );
			//UI_display_grid( R, &ui, width, height );

			SDL_RenderPresent(R);
		}

		SDL_framerateDelay(16);

	}//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> / L O O P <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


	exit:; 
	puts("exiting..");

	destroy_nodemap( &NM );

	destroy_TX_Font(&font);
	TTF_Quit();

	SDL_DestroyRenderer(R);
	SDL_DestroyWindow(window);

	SDL_Quit();

	puts("BYE");
	return 0;
}

