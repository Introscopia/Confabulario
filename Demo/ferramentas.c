#include "ferramentas.h"
#include "ok_lib.h"
#include "vec2d.h"
#include "transform.h"
#include "primitives.h"
#include "VFX.h"


void nodes_set_connection( Nodemap *NM, int i, int j, bool value ){
	if( j < i ){
		nodes_set_connection( NM, j, i, value );
		return;
	}
	if( value ){
		NM->connections[i][0] += 1;
		NM->connections[i][ NM->connections[i][0] ] = j;
	}
	else{
		int ji = 0;
		for (int c = 1; c <= NM->connections[i][0]; ++c ){
			if( NM->connections[i][c] == j ){
				ji = c;
				break;
			}
		}
		if( ji ){
			if( ji < NM->connections[i][0] ){
				memmove( NM->connections[i]+ji, NM->connections[i]+ji+1, (NM->connections[i][0]-ji) * sizeof(int16_t) );
			}
			NM->connections[i][0] -= 1;
		}
	}
}

bool nodes_connected( Nodemap *NM, int i, int j ){
	if( j < i ){
		return nodes_connected( NM, j, i );
	}
	for (int c = 1; c <= NM->connections[i][0]; ++c ){
		if( NM->connections[i][c] == j ) return 1;
	}
	return 0;
}

int node_get_connections( Nodemap *NM, int i, int buffer[MAX_NODE_CONNECTIONS] ){
	int b = 0;
	for (int n = 0; n < i; ++n ){
		if( nodes_connected( NM, n, i ) ){
			buffer[b++] = n;
		}
	}
	for (int c = 1; c <= NM->connections[i][0]; ++c ){
		buffer[b++] = NM->connections[i][c];
	}
	if( b < MAX_NODE_CONNECTIONS ) buffer[b] = -1;
	return b;
}

typedef struct pfqe_struct{
	int prev;// index into queue
	int node;// index into nodemap
} pfqe;// path finder queue element

typedef struct ok_vec_of(pfqe) path_finder_queue;

int* nodemap_path( Nodemap *NM, int origin, int destination, bool(*check_traversible)(Nodemap*, int) ){

	path_finder_queue queue = OK_VEC_INIT;
	ok_vec_push( &queue, ((pfqe){ -1, origin }) );
	int Qpos = 0;
	int Qlen = 1;

	do {
		pfqe *E = ok_vec_get_ptr( &queue, Qpos );
		int connections [MAX_NODE_CONNECTIONS];
		int cn = node_get_connections( NM, E->node, connections );
		for (int c = 0; c < cn; ++c ){
			int n = connections[c];

			bool not_already_in_queue = 1;
			for (int q = Qpos + 1; q < Qlen; ++q ){
				pfqe *e = ok_vec_get_ptr( &queue, q );
				if( e->node == n ){
					not_already_in_queue = 0;
					break;
				}
			}

			if( not_already_in_queue &&
			    check_traversible( NM, n ) ){
				ok_vec_push( &queue, ((pfqe){ Qpos, n }) );
				if( n == destination ){
					goto dest_found;
				}
				Qlen++;
			}
		}
		Qpos++;
		//printf( "<%d:%d> ", Qpos, Qlen );

	} while( Qpos < Qlen );

	dest_found:;

	//puts("!!!");


	int *path = NULL;

	pfqe *E = ok_vec_last( &queue );
	if( E->node == destination ){// did get there?
		int cap = 2;
		path = malloc( cap * sizeof(int) );
		int len = 0;
		path[len++] = -1;
		do{
			if( len >= cap ){
				cap *= 2;
				path = realloc( path, cap * sizeof(int) );
			}
			path[len++] = E->node;
			E = ok_vec_get_ptr( &queue, E->prev );
		} while( E->prev >= 0 );//check the prev of the prev cause we don't want 'origin' on the path.
		path = realloc( path, len * sizeof(int) );
		reverse_list( path, len );
	}
	ok_vec_deinit( &queue );
	return path;
}

bool traversible_if_known(Nodemap* NM, int i){
	return ( NM->knowledge[i] > 0 );
}

void import_nodemap( char *filename, Nodemap *NM ){

	int fnlen = strlen( filename );
	printf("importing filename: %s, len: %d\n", filename, fnlen );

	if( strcmp( filename + fnlen -3, "bin" ) == 0 ){
		//puts("bin");

		setlocale(LC_ALL, "C");
		FILE *f = fopen( filename, "rb" );

		fseek( f, 8, SEEK_SET );
		int version = 0;
		fscanf( f, "%d", &version );
		//printf("version: %d", version );
		if( fgetc(f) == ' ' ) puts("_");
		
		fread( &(NM->N), sizeof(int), 1, f );
		printf("NM->N: %d\n", NM->N );

		     NM->points = malloc( NM->N * sizeof(vec2d) );
		NM->connections = malloc( NM->N * sizeof(int16_t*) );
		   NM->icon_IDs = calloc( NM->N,  sizeof(int) );
		  NM->knowledge = calloc( NM->N,  sizeof(int32_t) );
		
		fread(    NM->points, sizeof(vec2d)  , NM->N, f );
		fread( NM->knowledge, sizeof(int32_t), NM->N, f );
		fread(  NM->icon_IDs, sizeof(int)    , NM->N, f );

		//translate the 'status' values we have in the editor to 'knowledge' values
		for (int a = 0; a < NM->N; ++a ){
			NM->knowledge[a] -= 1;
			if( NM->knowledge[a] == 3 ){
				NM->knowledge[a] = 2;
			}
			if( NM->knowledge[a] > 2 ){
				NM->knowledge[a] = 0;
			}
		}

		//puts("connections:");
		for (int a = 0; a < NM->N; ++a ){
			int count = 0;
			fread( &count, sizeof(int16_t), 1, f );
			NM->connections[a] = calloc( count+1, sizeof(int16_t) );
			NM->connections[a][0] = count;
			fread( NM->connections[a]+1, sizeof(int16_t), count, f );
		}

		//puts("contents:");
		for (int a = 0; a < NM->N; ++a ){
			int len = 0;
			fread( &len, sizeof(int), 1, f );
			if( len > 0 ){
				//NM->contents[a] = malloc( len * sizeof(char) );
				//fread( NM->contents[a], sizeof(char), len, f );
				fseek( f, len, SEEK_CUR );
			}
		}

		fread( &(NM->labels_N), sizeof(int), 1, f );

		if( !feof(f) ){

			printf("NM->labels_N: %d\n", NM->labels_N );

			NM->labels = malloc( NM->labels_N * sizeof(Maplabel) );

			for (int l = 0; l < NM->labels_N; ++l ){
				fread( &(NM->labels[l].pos), sizeof(vec2d), 1, f );
				fread( &(NM->labels[l].scale), sizeof(double), 1, f );
				fread( &(NM->labels[l].angle), sizeof(double), 1, f );
				fread( &(NM->labels[l].curvature), sizeof(double), 1, f );
				fread( &(NM->labels[l].connections_N), sizeof(int), 1, f );
				//printf("label[%d]: %d connections", l, NM->labels[l].connections_N );
				NM->labels[l].show = 0;
				if( NM->labels[l].connections_N > 0 ){
					fread( NM->labels[l].connections, sizeof(int), NM->labels[l].connections_N, f );
					for (int c = 0; c < NM->labels[l].connections_N; ++c ){// start off connected to a known node?
						if( NM->knowledge[ NM->labels[l].connections[c] ] > 1 ){
							NM->labels[l].show = 1;
						}
					}
				}
				else{// no connections? show by default!
					NM->labels[l].show = 1;
				}
				int len = 0;
				fread( &len, sizeof(int), 1, f );
				NM->labels[l].str = calloc( len+1, sizeof(char) );
				fread( NM->labels[l].str, sizeof(char), len, f );
				NM->labels[l].str[ len ] = '\0';
				//printf("NM->labels[%d].str: %s\n", l, NM->labels[l].str );
			}
		}
		else{
			NM->labels_N = 0;
		}


		fclose (f);
	}
}

void destroy_nodemap( Nodemap *NM ){

	for (int a = 0; a < NM->N; ++a ){
		free( NM->connections[a] );
	}
	free( NM->points      );
	free( NM->connections );
	free( NM->icon_IDs    );
	free( NM->knowledge   );
}

void discover_node( Nodemap *NM, int LOC ){
	NM->knowledge[ LOC ] = 2;

	int connections [MAX_NODE_CONNECTIONS];
	int cn = node_get_connections( NM, LOC, connections );
	for (int c = 0; c < cn; ++c ){
		if( NM->knowledge[ connections[c] ] < 1 ){
			NM->knowledge[ connections[c] ] = 1;
		}
	}

	for (int l = 0; l < NM->labels_N; ++l ){
		if( NM->labels[l].show ) continue;
		for (int lc = 0; lc < NM->labels[l].connections_N; ++lc ){
			if( NM->labels[l].connections[lc] == LOC ){
				NM->labels[l].show = 1;
				return;
			}
		}
	}
}

static int mouse_on_node( vec2d *mouse, Nodemap *NM, Transform *T, int nirsq ){
	vec2d rtm = reverse_transform_v2d( mouse, T );
	for (int i = 0; i < NM->N; ++i){
		//if( NM->status[i] & DELETED ) continue;
		if( NM->knowledge[i] < 1 ) continue;
		if( v2d_distsq( rtm, NM->points[i] ) < nirsq ){
			return i;
		}
	}
	return -1;
}


#define ABRIR_MOLDURA() SDL_SetRenderDraw_SDL_Color( R, &(M->pal_lo) );       \
						int af = M->popup_moldura->end - M->popup_moldura->c;   \
						for (int i = 0; i < af; ++i ){                        \
							render_Box_9Slice( R, M->popup_moldura, M );   \
							SDL_RenderPresent( R );                           \
							SDL_framerateDelay( 16 );                         \
						}


void CFB_Mensagem( SDL_Renderer *R, Mundo *M, char *msg ){

	Typist typ;
	init_typist( &typ, msg, 1, &(M->pal_hi) );

	M->GUI.Hx = M->popup_rct_gui.w-2;
	M->GUI.Vx = M->popup_rct_gui.h-2;
	SDL_Rect rct = calc_rect( &(M->GUI), M->popup_rct_gui.x + 1, M->popup_rct_gui.y + 1 );
	M->font.scale = 0.5;
	int th = TX_wrapped_string_height( &(M->font), msg, rct.w );
	while( th > rct.h ){
		M->font.scale -= 0.05;
		th = TX_wrapped_string_height( &(M->font), msg, rct.w );
	}
	rct.y += round( 0.5 * ( rct.h - th ) );

	ABRIR_MOLDURA();

	puts("<entering mensagem loop>");
	while( 1 ) {

		SDL_Event event;
		while( SDL_PollEvent(&event) ){
			switch( event.type ){
				case SDL_QUIT:
					M->quit_flag = 1;
					goto mensagem_exit;
					break;
				case SDL_KEYDOWN: 
				case SDL_MOUSEBUTTONUP:
					if ( typing_done( &typ ) ) {
						goto mensagem_exit;
					} else {
						for (int i = 0; i < 10; ++i ) typist_step( &typ );
					}
					break;
				case SDL_KEYUP:
					if( event.key.keysym.sym == SDLK_ESCAPE || 
						event.key.keysym.sym == SDLK_RETURN ||
						event.key.keysym.sym == SDLK_SPACE  ){
						typ.I = typ.len;
					}
					break;
			}
		}

		SDL_SetRenderDraw_SDL_Color( R, &(M->pal_lo) );
		render_Box_9Slice( R, M->popup_moldura, M );

		SDL_RenderSetClipRect( R, &(M->popup_rct) );
		render_typist( R, &(M->font), &typ, rct.x, rct.y, rct.w, TX_ALIGN_CENTER ); 
		SDL_RenderSetClipRect( R, NULL );

		SDL_RenderPresent(R);
		SDL_framerateDelay(16);
	}
	mensagem_exit:

	clear_typist( &typ );

	M->popup_moldura->c = 0;
}

void CFB_Imagem( SDL_Renderer *R, Mundo *M, char *filename ){

	SDL_Texture *imagem = IMG_LoadTexture( R, filename );
	SDL_Rect dst;
	if( imagem != NULL ){
		int w, h;
		SDL_QueryTexture( imagem, NULL, NULL, &w, &h );
		//printf("w: %d, h: %d\n", w, h );
		dst = (SDL_Rect){ 0, 0, w, h };
		fit_rect( &dst, &(M->popup_rct) );
		//printf("dst.x: %d, dst.y: %d, dst.w: %d, dst.h: %d\n", dst.x, dst.y, dst.w, dst.h);
	}
	else puts("Couldn't load image!!!!");

	ABRIR_MOLDURA();

	puts("<entering imagem loop>");
	while( 1 ) {

		SDL_Event event;
		while( SDL_PollEvent(&event) ){
			switch( event.type ){
				case SDL_QUIT:
					M->quit_flag = 1;
					goto imagem_exit;
					break;

				case SDL_KEYDOWN: 
				case SDL_MOUSEBUTTONUP:
				case SDL_KEYUP:

					goto imagem_exit;
					break;
			}
		}

		//SDL_SetRenderDraw_SDL_Color( R, &(M->pal_lo) );
		//SDL_RenderClear(R);
		render_Box_9Slice( R, M->popup_moldura, M );

		SDL_RenderCopy( R, imagem, NULL, &dst );

		SDL_RenderPresent(R);
		SDL_framerateDelay(16);
	}
	imagem_exit:

	M->popup_moldura->c = 0;
}

int CFB_Escolhas( SDL_Renderer *R, Mundo *M, char *titulo, int num, ... ){
	
	UI_Set ui;
	build_UI_Set_Copy( &ui, &(M->GUI) );

	UI_init_static_set( &ui, 1 );
	ui.Hx = M->popup_rct_gui.w;
	ui.Vx = 1;
	M->font.scale = 0.4;
	UI_build_static_label( &ui, M->popup_rct_gui.x, M->popup_rct_gui.y, R, titulo, "CC", &(M->font) );

	UI_init_interactive_set( &ui, num );

	int ls = 1;
	if( num < ((M->popup_rct_gui.h-2) / 2) ) ls = 2;
	int ty = M->popup_rct_gui.y + 2 + ((M->popup_rct_gui.h-2 -((num*2)-1)) / 2 );
	int response = -1;
	va_list VAL;
	va_start(VAL, num);
	for (int i = 0; i < num; ++i ){
		UI_build_numset( &ui, M->popup_rct_gui.x, ty + (ls * i), &response, i, M->pal_lo, M->pal_mo, M->pal_hi );
		numset_set_label( UI_last_element( &ui ), &(M->font), va_arg( VAL, char* ), "CC" );
	}
	va_end(VAL);

	ABRIR_MOLDURA();

	puts("<entering escolhas loop>");
	goto escolhas_refresh;
	while( 1 ) {

		bool refresh = 0;
		SDL_Event event;
		while( SDL_PollEvent(&event) ){
			if(event.type == SDL_QUIT){
				M->quit_flag = 1;
				goto escolhas_exit;
			}

			UI_event_handler( &ui, &event );

			if( response > -1 ) goto escolhas_exit;

			refresh = 1;
		}

		if( refresh ){
			escolhas_refresh:
			SDL_SetRenderDraw_SDL_Color( R, &(M->pal_lo) );
			render_Box_9Slice( R, M->popup_moldura, M );
			//  SDL_SetRenderDraw_SDL_Color( R, &(M->pal_mo) );
			//  UI_display_grid( R, &ui, M->width, M->height );
			SDL_SetRenderDraw_SDL_Color( R, &(M->pal_hi) );
			UI_display( R, &ui );

			SDL_RenderPresent(R);
		}
		SDL_framerateDelay(16);
	}
	escolhas_exit:

	destroy_UI_Set( &ui );

	M->popup_moldura->c = 0;

	return response;
}

bool CFB_Senha( SDL_Renderer *R, Mundo *M, char *titulo, char *senha, int *tentativas ){
	UI_Set ui;
	build_UI_Set_Copy( &ui, &(M->GUI) );

	UI_init_static_set( &ui, 1 );
	ui.Hx = M->popup_rct_gui.w - 2;
	ui.Vx = 1;
	int ty = M->popup_rct_gui.y + 2 + ((M->popup_rct_gui.h-2) / 2 ) -1;
	M->font.scale = 0.4;
	//UI_build_static_label( &ui, M->popup_rct_gui.x + 1, M->popup_rct_gui.y, R, titulo, "CC", &(M->font) );

	UI_build_static_label( &ui, M->popup_rct_gui.x + 1, ty-3, R, "Digite sua resposta:", "TL", &(M->font) );

	UI_init_interactive_set( &ui, 2 );
	memset( M->buf, 0, 512 );
	UI_build_text_line_input( &ui, M->popup_rct_gui.x + 1, ty-1, M->buf, 512, &(M->font), M->pal_lo, M->pal_mo, M->pal_hi );

	bool confirm = 0;
	UI_build_toggle( &ui, M->popup_rct_gui.x + 1, ty + 1, &confirm, M->pal_lo, M->pal_mo, M->pal_hi );
	toggle_set_label( UI_last_element( &ui ), &(M->font), "Confirma", "CC" );

	if( tentativas != NULL ){
		UI_init_dynamic_set( &ui, 1 );
		ui.Hx = 3;
		UI_build_dynamic_int_label( &ui, M->popup_rct_gui.x + M->popup_rct_gui.w -3, 
										 M->popup_rct_gui.y + M->popup_rct_gui.h -1, 
										 "Tentativas: %d", "CL", tentativas, &(M->font) );
	}

	bool result = 0;

	ABRIR_MOLDURA();

	puts("<entering senha loop>");
	goto senha_refresh;
	while( 1 ) {

		bool refresh = 0;
		SDL_Event event;
		while( SDL_PollEvent(&event) ){
			if(event.type == SDL_QUIT){
				M->quit_flag = 1;
				goto senha_exit;
			}

			UI_event_handler( &ui, &event );

			if( confirm ){
				// String Case-Insensitive comparison
				if( strcicmp( M->buf, senha ) == 0 ){
					result = 1;
					goto senha_exit;
				}
				else{
					strcpy( M->buf, " * INCORRETO * " );
					if( tentativas != NULL && *tentativas > 0 ){
						*tentativas -= 1;
						if( *tentativas <= 0 ){
							goto senha_exit;
						}
					}
				}
				confirm = 0;
			}

			refresh = 1;
		}

		if( refresh ){
			senha_refresh:
			SDL_SetRenderDraw_SDL_Color( R, &(M->pal_lo) );
			render_Box_9Slice( R, M->popup_moldura, M );

			SDL_SetRenderDraw_SDL_Color( R, &(M->pal_hi) );
			UI_display( R, &ui );

			TX_render_string_wrapped_aligned( R, &(M->font), titulo, M->popup_rct.x, M->popup_rct.y, M->popup_rct.w, TX_ALIGN_CENTER );

			SDL_RenderPresent(R);
		}
		SDL_framerateDelay(16);
	}
	senha_exit:

	destroy_UI_Set( &ui );

	M->popup_moldura->c = 0;

	return result;
}




typedef struct dice_pool_struct{

	Camera_3d *cam;
	DExpr *pool;
	Dice_3d *dice;
	vec2d *center;
	vec2d tot_lab;// position of the (total) label
	float ring_rad, label_rad;
	TX_Font *font;
	SDL_Texture *moldura;
	float mw;
	SDL_Color *lo;
	SDL_Color *hi;

} Dice_pool;

static void build_dice_grid( Dice_pool *dp, DExpr *pool, Mundo *M, SDL_Renderer *R, float ox, float oy, 
																					float cw, float cols ){

	dp->pool = pool;
	int N = pool->dat.dpl.num;
	pool->dat.dpl.values = malloc( N * sizeof(int*) );
	dp->dice = malloc( N * sizeof(Dice_3d) );
	dp->center = malloc( N * sizeof(vec2d) );
	dp->label_rad = 0.5 * (M->font.h * M->font.scale * 1.025);
	dp->ring_rad = 0.5 * cw;//(cw - dp->label_rad) - 2.5;
	int o = 0;
	for (int i = 0; i < cols; ++i ){
		float q = 0;
		if( N-o < cols ){
			q = 0.5 * (cols-(N-o));
		}
		for (int j = 0; j < cols; ++j ){
			dp->dice[o] = *dado_de_N_lados( M, pool->dat.dpl.sides, R );
			//print_Dice_3d( dp->dice + o );
			dp->dice[o].scale *= 0.91 * cw;
			scramble_dice( dp->dice + o, 3 );
			pool->dat.dpl.values[o] = &(dp->dice[o].value);//grabbing reference to the 'physical' dice value
			dp->center[o] = v2d( ox + dp->ring_rad + (i * cw), oy + dp->ring_rad + ((j+q) * cw) );
			o++;
			if( o >= N ) goto done_gridding;
		}
	}

	done_gridding:

	dp->mw = 2 * (cw + (0.5 * dp->label_rad) + 5);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");
	dp->moldura = SDL_CreateTexture( R, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, dp->mw, dp->mw );
	SDL_SetTextureBlendMode( dp->moldura, SDL_BLENDMODE_BLEND );
	SDL_SetRenderTarget( R, dp->moldura );
	SDL_SetRenderDrawColor( R, 0,0,0,0 );
	SDL_RenderClear( R );
	SDL_SetRenderDraw_SDL_Color( R, &(M->pal_hi) );
	gp_drawthick_Ncircle( R, cw + 5, cw + 5, cw, 5, 48 );
	float lx = 2 * (cw - 0.5 * dp->label_rad) + 5;
	SDL_SetRenderDraw_SDL_Color( R, &(M->pal_lo) );
	gp_fill_Ncircle( R, lx, lx, 2 * dp->label_rad, 48 );
	SDL_SetRenderDraw_SDL_Color( R, &(M->pal_hi) );
	gp_drawthick_Ncircle( R, lx, lx, 2 * dp->label_rad, 3, 48 );
	SDL_SetRenderTarget( R, NULL );
	dp->mw *= 0.5;

	dp->cam = &(M->cam3d);
	dp->font = &(M->font);
	dp->lo = &(M->pal_lo);
	dp->hi = &(M->pal_hi);
	float actual_cols = ceil( (float)(N) / cols);
	dp->tot_lab = v2d( ox + (actual_cols * 0.54 * cw), oy + (cols * cw) + (M->font.scale * 0.6 * M->font.line_skip) );
}

static void display_Dice_pool( SDL_Renderer *R, Dice_pool *dp ){
	for (int i = dp->pool->dat.dpl.num-1; i >= 0; --i ){
		//SDL_SetRenderDraw_SDL_Color( R, dp->hi );
		//gp_drawthick_fastcircle( R, dp->center[i].x, dp->center[i].y, dp->ring_rad, 2.5 );
		render_dice_3d( R, dp->dice + i, dp->cam, dp->center[i].x, dp->center[i].y );

		SDL_RenderCopyF( R, dp->moldura, NULL, &(SDL_FRect){ dp->center[i].x - dp->ring_rad -2.5, 
															 dp->center[i].y - dp->ring_rad -2.5, 
															 dp->mw, dp->mw } );
	}
	int total = 0;
	for (int i = 0; i < dp->pool->dat.dpl.num; ++i ){
		float lx = dp->center[i].x + dp->ring_rad - 0.5 * dp->label_rad;
		float ly = dp->center[i].y + dp->ring_rad - 0.5 * dp->label_rad;
		//SDL_SetRenderDraw_SDL_Color( R, dp->lo );
		//gp_fill_16circle( R, lx, ly, dp->label_rad );
		char lab [4]; sprintf(lab, "%d", dp->dice[i].value );
		TX_render_string_centered( R, dp->font, lab, lx+1, ly+1 );
		total += dp->dice[i].value;
		//SDL_SetRenderDraw_SDL_Color( R, dp->hi );
		//gp_drawthick_16circle( R, lx, ly, dp->label_rad, 1.5 );
	}
	if( dp->pool->dat.dpl.num > 1 ){
		char tot [8]; sprintf(tot, "(%d)", total );
		TX_render_string_centered( R, dp->font, tot, dp->tot_lab.x, dp->tot_lab.y );
	}
}

DEVal CFB_Rolar_Dados( SDL_Renderer *R, Mundo *M, char *titulo, char *expressao, int *tentativas ){

	ABRIR_MOLDURA();

	SDL_Event event;
	while( SDL_PollEvent(&event) ){ }// throw away inputs

	float TFS = 0.72; // title font scale
	float DFS = 0.65; // dice font scale
	float PFS = 0.30; // dicepool font scale
	float MFS = 0.50; // msg font scale
	float BFS = 0.28; // buttons font scale

	#ifdef DEBUGMODE
	puts("compiling DExpr..."); 
	#endif
	DExpr *dexpr = compile_DExpr( expressao );
	#ifdef DEBUGMODE
	puts("compiled."); 
	#endif

	int pools_N = count_dpools( dexpr );

	char rde [256]; memset( rde, 0, 256 );// recompiled Dice expression
	sprint_DExpr_dpl_ids( rde, dexpr ); //printf("rde: \"%s\"\n", rde );
	int blen = strlen(rde);
	float tw, s2w;
	M->font.scale = DFS;
	TX_SizeText( &(M->font), "$2", &s2w, NULL );
	TX_SizeText( &(M->font), rde, &tw, NULL );
	int ettw = round(tw - (s2w * pools_N));// expression text total with
	float max_budget = (0.96 * M->popup_rct.w);
	if( ettw > max_budget ){
		DFS *= max_budget / ettw;
		ettw = max_budget;
	}
	max_budget -= ettw;
	float budget = max_budget;
	float max_h = 0.98 * ( M->popup_rct.h - (TFS * M->font.line_skip) - (MFS * M->font.line_skip) );//0.6 * M->popup_rct.h;
	float *pool_widths = malloc( pools_N * sizeof(float) );
	int *cols = malloc( pools_N * sizeof(int) );
	for (int p = 0; p < pools_N; ++p ){
		DExpr *pool = fetch_dpool( dexpr, p );
		cols[p] = ceil( sqrt(pool->dat.dpl.num) );
		if( cols[p] > 5 ) pool_widths[p] = max_h;
		else pool_widths[p] = ellipticalMap(cols[p], 5, 0.25, max_h, 0);
		float cw = pool_widths[p] / cols[p];
		float actual_cols = ceil( (float)(pool->dat.dpl.num) / cols[p]);
		budget -= (actual_cols * cw);
	}
	if( budget < 0 ){
		float S = max_budget / ( max_budget - budget );
		for (int p = 0; p < pools_N; ++p ){
			pool_widths[p] *= S;
		}
		budget = 0;
	}
	#ifdef DEBUGMODE
	puts("- cols and widths calculated."); 
	#endif

	float cx = M->popup_rct.x + (budget * 0.5) + (0.02 * M->popup_rct.w);//cursor x
	//expression text position
	int *etx = malloc( (pools_N+1) * sizeof(int) );
	etx[0] = cx;
	int ety = M->popup_rct.y + (TFS * M->font.line_skip) + (0.5 * max_h);
	int ei = strfchr(rde, '$');// expression index
	TX_SizeTextUntil( &(M->font), rde, ei,  &tw, NULL );
	cx += tw;
	ei+=2;

	//printf("mallocking %d pools.\n", pools_N );
	Dice_pool *dicepools = malloc( pools_N * sizeof(Dice_pool) );
	for (int p = 0; p < pools_N; ++p ){
		DExpr *pool = fetch_dpool( dexpr, p );
		float cw = pool_widths[p] / cols[p];
		float x = cx;
		float y = ety - (0.5 * pool_widths[p]);
		M->font.scale = PFS;
		build_dice_grid( dicepools + p, pool, M, R, x, y, cw, cols[p] );
		float actual_cols = ceil( (float)(pool->dat.dpl.num) / cols[p]);
		cx += (actual_cols * cw);
		etx[p+1] = cx + 3;
		int nei = strfchr(rde+ei, '$');
		if( nei < 0 ) nei = blen-ei;
		M->font.scale = DFS;
		TX_SizeTextUntil( &(M->font), rde+ei, nei,  &tw, NULL );
		ei += nei+2;
		cx += tw;
	}
	free( pool_widths );
	free( cols );
	ety -= (0.5 * DFS * M->font.h);
	#ifdef DEBUGMODE
	puts("- dicepools built."); 
	#endif

	bool done = 0;
	UI_Interactive done_button;
	bool retry = 0;
	UI_Interactive retry_button;
	{
		int bw = 0.3 * M->popup_rct.w;
		int bx = M->popup_rct.x + M->popup_rct.w - bw;
		M->font.scale = BFS;
		int bh = 1.2 * M->font.h * M->font.scale;
		int by = M->popup_rct.y + M->popup_rct.h - bh;
		retry_button = build_orphan_toggle( bx, by -bh - 4, bw, bh, &retry, M->pal_lo, M->pal_mo, M->pal_hi );
		toggle_set_label( &retry_button, &(M->font), "Tentar Novamente", "CC" );
		
		done_button = build_orphan_toggle( bx, by, bw, bh, &done, M->pal_lo, M->pal_mo, M->pal_hi );
		toggle_set_label( &done_button, &(M->font), "Seguir Adiante", "CC" );
	}
	
	DEVal resultado;

	char result_str [64];
	Typist result_msg;

	char retries_str [64];
	if( tentativas != NULL ){
		sprintf( retries_str, "Tentativas: %d", *tentativas );
	}

	int moment = 0; /* 0: waiting for input, 1: dice in motion, 2: results are in */ 
	
	puts("<entering dice roller>");
	while( 1 ) {

		while( SDL_PollEvent(&event) ){
			if( moment == 2 ){
				char captured = 0;
				if( resultado.total == 0 ){
					captured += UI_orphan_event_handler( &retry_button, &event );
				}
				captured += UI_orphan_event_handler( &done_button, &event );
				if( captured ){
					if( retry ){
						retry = 0;
						if( tentativas != NULL && *tentativas > 0 ){
							*tentativas -= 1;
							sprintf( retries_str, "Tentativas: %d", *tentativas );
							clear_typist( &result_msg );
							goto rollemup;
						}
					}
					if( done ){
						goto rolar_dados_exit;
					}
					continue;
				}
			}
			switch (event.type) {

				case SDL_QUIT:
					M->quit_flag = 1;
					goto rolar_dados_exit;

				case SDL_KEYUP:
				case SDL_MOUSEBUTTONUP:

					if( moment == 0 ){
						rollemup:
						for (int p = 0; p < pools_N; ++p ){
							for (int i = 0; i < dicepools[p].pool->dat.dpl.num; ++i ){
								scramble_dice( dicepools[p].dice + i, 3 );
								dicepools[p].dice[i].roll( dicepools[p].dice + i );
							}
						}
						moment = 1;
					}
					if( moment == 2 ){
						if( tentativas != NULL && *tentativas > 0 && resultado.total <= 0 ){
							
							*tentativas -= 1;
							sprintf( retries_str, "Tentativas: %d", *tentativas );
							clear_typist( &result_msg );
							goto rollemup;
						}
						else{
							goto rolar_dados_exit;
						}
					}

					break;
			}
		}

		if( moment == 1 ){
			moment = 2;
			for (int p = 0; p < pools_N; ++p ){
				for (int i = 0; i < dicepools[p].pool->dat.dpl.num; ++i ){
					if( dicepools[p].dice[i].rolling ){
						moment = 1;
						break;
					}
				}
			}
			if( moment == 2 ){
				resultado = evaluate_DExpr( dexpr );
				if( resultado.total == 0 ){
					sprintf( result_str, "Fracasso." );
				}else if ( resultado.total == 1 ){
					sprintf( result_str, "Sucesso!" );
				}else if ( resultado.total > 1 ){
					sprintf( result_str, "{%d}", resultado.total );
				}else{
					sprintf( result_str, "???????" );
				}
				init_typist( &result_msg, result_str, 2, &(M->pal_hi) );
			}
		}

		SDL_SetRenderDraw_SDL_Color( R, &(M->pal_lo) );
		//SDL_RenderFillRect( R, &M->popup_rct );
		render_Box_9Slice( R, M->popup_moldura, M );
		
		SDL_SetRenderDraw_SDL_Color( R, &(M->pal_hi) );
		M->font.scale = PFS;
		for (int p = pools_N-1; p >= 0; --p ){
			display_Dice_pool( R, dicepools + p );
		}

		ei = 0;
		M->font.scale = DFS;
		for (int i = 0; i < pools_N+1; ++i ){
			int nei = strfchr(rde+ei, '$'); 
			nei = ( nei < 0 )? blen : nei+ei;
			TX_render_section( R, &(M->font), rde, ei, nei, etx[i], ety );
			ei = nei+2;
		}
		//TX_render_string( R, &(M->font), rde, etx[0], ety + 65 );
		
		M->font.scale = TFS;
		TX_render_string_wrapped_aligned( R, &(M->font), titulo, M->popup_rct.x, M->popup_rct.y, M->popup_rct.w, TX_ALIGN_CENTER );

		if( moment == 2 ){
			M->font.scale = MFS;
			render_typist( R, &(M->font), &result_msg, M->popup_rct.x, 
						   M->popup_rct.y + M->popup_rct.h - (M->font.scale * M->font.line_skip), 
						   M->popup_rct.w, TX_ALIGN_CENTER );
			
			M->font.scale = BFS;
			if( tentativas != NULL && *tentativas > 0 && resultado.total == 0 ){
				TX_render_string_wrapped_aligned( R, &(M->font), retries_str, retry_button.rect.x, 
								                  retry_button.rect.y - (M->font.scale * M->font.line_skip), 
								                  retry_button.rect.w, TX_ALIGN_CENTER );
				toggle_display( R, &retry_button );
			}
			toggle_display( R, &done_button );
		}

		SDL_RenderPresent(R);
		SDL_framerateDelay( 16 );
	}
	rolar_dados_exit:

	destroy_DExpr( dexpr );

	for (int p = 0; p < pools_N; ++p ){
		free( dicepools[p].dice );
		free( dicepools[p].center );
		SDL_DestroyTexture( dicepools[p].moldura );
	}
	free( dicepools );
	free( etx );
	M->popup_moldura->c = 0;

	clear_typist( &result_msg );
	toggle_destroy( &retry_button );
	toggle_destroy( &done_button );

	return resultado;
}






enum { dn_NULL = 0, dn_TALK, dn_RESPONSE, dn_ASSIGNMENT, dn_RETURNER };
typedef struct dialog_node_struct Dialog_node;
typedef struct dialog_node_struct{

	int type;
	int line;
	int indent;
	bool visited;

	union{

		struct{
			char *content;
			int responses_N;
			Dialog_node **responses;
		} talking;

		struct{
			int va_id;
			int operation;
			int value;
		} assignment;

		int return_value;

	} bod;

	Dialog_node *parent;
	Dialog_node *next;

} Dialog_node;

static void print_DNTree( Dialog_node *D, int tabs ){

	for (int i = 0; i < tabs; ++i ){
		printf(" . ");
	}

	switch( D->type ){
		case dn_NULL:
			printf("NULL\n");
			break;
		case dn_TALK:
			printf("TALK(l %d,i %d): *{", D->line, D->indent );
			print_string_with_escape_chars_visible( D->bod.talking.content, 24 );
			printf("}\n");
			for (int r = 0; r < D->bod.talking.responses_N; ++r ){
				print_DNTree( D->bod.talking.responses[r], tabs + 1 );
			}
			break;
		case dn_RESPONSE:
			printf("RESPONSE(l %d,i %d): >{", D->line, D->indent );
			print_string_with_escape_chars_visible( D->bod.talking.content, 24 );
			printf("}\n");
			break;
		case dn_ASSIGNMENT:
			printf("ASSIGNMENT(l %d,i %d): [%d] %c %d\n", D->line, D->indent, D->bod.assignment.va_id, D->bod.assignment.operation, D->bod.assignment.value );
			break;
		case dn_RETURNER:
			printf("RETURNER(l %d,i %d): %d\n", D->line, D->indent, D->bod.return_value );
			break;
		default:
			printf("Unrecognized DN type: %d\n", D->type );
	}
	if( D->next != NULL ){
		if( D->type == dn_RESPONSE ) tabs += 1;
		//printf("NX%d ", tabs);
		print_DNTree( D->next, tabs );
	}
}

static void new_DN( Dialog_node ***cD, int lvl, int plvl, int line, int new_type ){

	if( **cD == NULL ){
		**cD = calloc( 1, sizeof(Dialog_node) );
		//printf("1: calloc'd (%p)\n", **cD );
		(**cD)->line = line;
		(**cD)->indent = lvl;
		(**cD)->type = new_type;
		return;
	}

	int mo = -1;
	if( lvl < plvl ){// previous branch is over, go into next
		//int up = plvl - lvl;
		//for (int u = 0; u < up; ++u ){//rewind..
		while( (**cD)->indent > lvl ){
			*cD = &((**cD)->parent);
			if( *cD == NULL ){
				puts("Dialog Warning: Excessive indentation, probably.");
				break;
			}
		}
		if( (**cD)->type == dn_RESPONSE ){
			*cD = &((**cD)->parent);//back up the cursor..
			mo = 'R';
		}
		else mo = 'N';
	}
	else if( lvl == plvl ){
		if( (**cD)->type == dn_TALK ){// we're next after a TALK with no responses
			mo = 'N';
		}
		else if( (**cD)->type == dn_RESPONSE ){// we're the next response
			if( new_type != dn_RESPONSE ) puts("Dialog Warning: Expected a simple >Response.");
			*cD = &((**cD)->parent);//back up the cursor..
			mo = 'R';
		}
		else{ // assigns or returns on the same lvl... ok
			mo = 'N';
		}
	}
	else if( lvl > plvl ){// branch is going deeper
		if( (**cD)->type == dn_TALK ){ // first response to this talk.
			mo = 'R';
		}
		else{
			mo = 'N';
		}
	}

	if( mo == 'N' ){
		Dialog_node *prev = **cD;
		*cD = &((**cD)->next);
		**cD = calloc( 1, sizeof(Dialog_node) );
		//printf("N: calloc'd (%p)\n", **cD );
		(**cD)->parent = prev;
	}
	else if( mo == 'R' ){
		int N = (**cD)->bod.talking.responses_N;
		(**cD)->bod.talking.responses_N += 1;
		(**cD)->bod.talking.responses = realloc( (**cD)->bod.talking.responses, (N+1) * sizeof(Dialog_node*) );
		(**cD)->bod.talking.responses[N] = calloc( 1, sizeof(Dialog_node) );//memset( RE, 0, sizeof(Dialog_node) );
		//printf("R: calloc'd (%p)\n", (**cD)->bod.talking.responses[N] );
		(**cD)->bod.talking.responses[N]->parent = **cD;
		*cD = (**cD)->bod.talking.responses + N;
	}
	//printf("(**cD): %p, type: %d\n", (**cD), new_type );
	(**cD)->line = line;
	(**cD)->indent = lvl;
	(**cD)->type = new_type;
}

static char *fscan_indented_paragraphs( FILE *F, int indent, int *line ){
	int size = 8;
	int len = 0;
	char *O = calloc( size, sizeof(char) );

	int c = fnext_nonspace( F );
	while( !feof(F) ){

		if( c == '\n' ){
			long int pos = ftell( F )-1;
			int tabs = 0;
			while( !feof(F) ){
				c = getgc(F);
				if( c == '\t' ) tabs += 1;
				else break;
			}
			if( tabs == indent ){
				if( c == '*' || c == '>' ){
					goto paragraph_over;
				}
				ungetc( c, F ); 
				c = '\n';
			}
			else{
				paragraph_over:
				fseek( F, pos, SEEK_SET );
				break;
			}
			(*line)++;
		}

		O[ len++ ] = c;
		if( len >= size ){
			size *= 2;
			O = realloc( O, size * sizeof(char) );
		}
		if( c == '\n' ) c = fnext_nonspace( F );
		else c = getgc(F);
	}
	O[ len++ ] = '\0';
	O = realloc( O, len * sizeof(char) );
	return O;
}

static void load_DNTree( Dialog_node **root, int *VAN, char *filename ){
	
	setlocale(LC_ALL, "C");
	FILE *F = fopen( filename, "rb" );

	if( F == NULL ){
		*root = NULL;
		printf( "ERROR: Could not open \"%s\"\n", filename );
	}
	else{
		int pp = '\n';
		int p = '\n';
		int c = fgetc( F );
		int line = 1;
		int plvl = 0, lvl = 0;
		Dialog_node **cD = root;//cursor
		Dialog_node *RE = NULL;//previous_response


		while( !feof(F) ){

			if( c == '\n' ){
				line++;
				plvl = lvl;
				lvl = 0;
			}
			else if( c == '\t' ) lvl++; 

			if( (pp == '\t' || pp == '\n') && p == '*' ){
				if( c == ' ' ) c = fnext_nonspace( F );
				if( c == '[' ){
					new_DN( &cD, lvl, plvl, line, dn_ASSIGNMENT );
					int n = 0;
					fscanf( F, "%d", &n );
					if( n+1 > *VAN ) *VAN = n+1;
					fgetc( F );//eat the ']'
					c = fnext_nonspace( F );
					if( !( c == '+' || c == '-' || c == '*' || c == '/' || c == '=' || 
						   c == '%' || c == '&' || c == '|' || c == '^') ){
						printf("Expected an operation (=, +, -, *, /, %, &, |, ^), but got '%c' instead.\n", c );
					}
					(*cD)->bod.assignment.va_id = n;
					(*cD)->bod.assignment.operation = c;
					fscanf( F, "%d", &((*cD)->bod.assignment.value) );
				}
				else if( c == 'R'){
					ungetc( c, F );
					if( fseek_string_before( F, "RETURN", "\n" ) ){
						new_DN( &cD, lvl, plvl, line, dn_RETURNER );
						fscanf( F, "%d", &((*cD)->bod.return_value) );
					}
					else goto not_return;
				}
				else{
					not_return:
					ungetc( c, F );
					new_DN( &cD, lvl, plvl, line, dn_TALK );
					(*cD)->bod.talking.content = fscan_indented_paragraphs( F, lvl, &line );
				}
			}
			else if( p == '\t' && c == '>' ){
				//printf( "RE, %d, %d\n", plvl, lvl );
				new_DN( &cD, lvl, plvl, line, dn_RESPONSE );
				(*cD)->bod.talking.content = fscan_indented_paragraphs( F, lvl, &line );
			}
			else{
				//printf("uhhhhh.... '%c' found??\n", c );
			}

			pp = p; p = c;
			c = getgc( F );
			//printf("<%c>\n", c );
		}
		//print_DNTree( *root, 0 );
	}
}

static void destroy_DNTree( Dialog_node *D ){

	if( D->next != NULL ){
		destroy_DNTree( D->next );
	}

	if( D->type == dn_TALK ){
		if( D->bod.talking.responses_N > 0 ){
			for (int i = 0; i < D->bod.talking.responses_N; ++i ){
				destroy_DNTree( D->bod.talking.responses[i] );
			}
			free( D->bod.talking.responses );
		}
		if( D->bod.talking.content != NULL ){
			free( D->bod.talking.content );
		}
	}
	else if( D->type == dn_RESPONSE ){
		if( D->bod.talking.content != NULL ){
			free( D->bod.talking.content );
		}
	}

	//printf("freeing (%p)\n", D );
	free( D );
}

static void perform_dlg_assignment( Dialog_node *D, int ** va_ptrs ){
	switch( D->bod.assignment.operation ){
		case '=':
			*(va_ptrs[ D->bod.assignment.va_id ]) = D->bod.assignment.value;
			break;
		case '+':
			*(va_ptrs[ D->bod.assignment.va_id ]) += D->bod.assignment.value;
			break;
		case '-':
			*(va_ptrs[ D->bod.assignment.va_id ]) -= D->bod.assignment.value;
			break;
		case '*':
			*(va_ptrs[ D->bod.assignment.va_id ]) *= D->bod.assignment.value;
			break;
		case '/':
			*(va_ptrs[ D->bod.assignment.va_id ]) /= D->bod.assignment.value;
			break;
		case '%':
			*(va_ptrs[ D->bod.assignment.va_id ]) %= D->bod.assignment.value;
			break;
		case '&':
			*(va_ptrs[ D->bod.assignment.va_id ]) &= D->bod.assignment.value;
			break;
		case '|':
			*(va_ptrs[ D->bod.assignment.va_id ]) |= D->bod.assignment.value;
			break;
		case '^':
			*(va_ptrs[ D->bod.assignment.va_id ]) ^= D->bod.assignment.value;
			break;
	}
}

typedef struct gistruct{

	UI_Interactive *textbox;
	UI_Interactive *list_select;

	char **responses;
	int responses_length;
	int responses_size;

} Dialog_interface;

static void UI_build_Dialog_interface( Dialog_interface *DI, UI_Set *S, int c, int r, int *incumbency, 
									   SDL_Renderer *R, SDL_Color *lo, SDL_Color *mo, SDL_Color *hi,
									   int sbw, TX_Font *font, float font_scale ){

	DI->responses = NULL;
	DI->responses_length = 0;
	DI->responses_size = 0;

	int I = S->build_index;

	float TVx = S->Vx;
	S->Vx = round( S->Vx * 0.5 ) + 0.5;// factor of the total taken up by the textbox. listselect gets the rest.
	//S->Vx -= 0.5;

	UI_build_textbox( S, c, r, font, font_scale, sbw, R, lo, hi );
	S->set[ I ].IEHFs[  key_down  ] = empty_IEHF;
	S->set[ I ].IEHFs[   key_up   ] = empty_IEHF;
	S->set[ I ].IEHFs[ text_input ] = empty_IEHF;
	S->set[ I ].IEHFs[text_editing] = empty_IEHF;
	DI->textbox = S->set + I;

	r += ceil( S->Vx );
	S->Vx = TVx - S->Vx - 0.5;
	UI_build_list_select( S, c, r, &(DI->responses), &(DI->responses_length), incumbency,
						  font, font_scale, lo, mo, hi, sbw );

	DI->list_select = S->set + I + 1;

	return DI;
}

void Dialog_interface_update( SDL_Renderer *R, Dialog_interface *DI, Dialog_node *DN ){

	if( DN->type == dn_TALK ){

		textbox_set_incumbency( DI->textbox, &(DN->bod.talking.content), 1 );
		//textbox_set_string( UI_Interactive *E, char *str );

		//printf("DN->bod.talking.responses_N: %d\n", DN->bod.talking.responses_N );
		DI->responses_length = DN->bod.talking.responses_N;
		if( DI->responses_length > DI->responses_size ){
			DI->responses_size = DI->responses_length;
			DI->responses = realloc( DI->responses, DI->responses_size * sizeof(char*) );
		}
		for (int i = 0; i < DI->responses_length; ++i ){
			if(  DN->bod.talking.responses[i]->type == dn_RESPONSE ){
				DI->responses[i] = DN->bod.talking.responses[i]->bod.talking.content;
			}
			else{
				printf("!!! Dialog TALK contains a non-RESPONSE response, (an assignment, a RETURN, etc).");
			}
		}
		list_select_update( R, DI->list_select );
	}
	else{
		printf("!!! attempting to update Dialog interface with a non-TALK Dialog_node!\n");
	}
}

int CFB_Dialogo( SDL_Renderer *R, Mundo *M, char *titulo, char *arquivo, ... ){
	
	

	Dialog_node *dlg_root = NULL;
	Dialog_node *dlg_now = NULL;

	int VAN = 0;// number of variable arguments
	va_list VAL;
	int **va_ptrs = NULL;

	load_DNTree( &dlg_root, &VAN, arquivo );

	if( dlg_root == NULL ){
		return -9999;
	}

	dlg_now = dlg_root;

	if( VAN > 0 ){
		va_ptrs = malloc( VAN * sizeof(int*) );
		va_start(VAL, arquivo);
		for (int i = 0; i < VAN; i++){
			va_ptrs[i] = va_arg( VAL, int* );
		}
		va_end(VAL);
	}

	int dlg_return_value = INT32_MIN;
   
	UI_Set dui;
	build_UI_Set_Copy( &dui, &(M->GUI) );

	UI_init_static_set( &dui, 1 );
	dui.Hx = M->popup_rct_gui.w;
	dui.Vx = 1;
	M->font.scale = 0.4;
	UI_build_static_label( &dui, M->popup_rct_gui.x, M->popup_rct_gui.y, R, titulo, "C", &(M->font) );

	UI_init_interactive_set( &dui, 2 );
	//float mmx = ((2 * (M->popup_moldura->scale * M->borders_slice)) / (float)(dui.column_width) );
	//float mmy = ((2 * (M->popup_moldura->scale * M->borders_slice)) / (float)(dui.row_height) );

	dui.Hx = M->popup_rct_gui.w;
	dui.Vx = M->popup_rct_gui.h - 2;
	printf( "::%d, %d, %g, %g\n", M->popup_rct_gui.w, M->popup_rct_gui.h, dui.Hx, dui.Vx );
	int response = -1;

	//UI_Interactive *widget = UI_build_dialog( &dui, M->popup_rct_gui.x, M->popup_rct_gui.y, 
	//										  &response, 16, &(M->font), 0.3 );

	Dialog_interface DI;
	UI_build_Dialog_interface( &DI, &dui, M->popup_rct_gui.x, M->popup_rct_gui.y + 2, 
							   &response, R, 
							   &(M->pal_lo), &(M->pal_mo), &(M->pal_hi),
						       20, &(M->font), 0.32 );
	puts("built dialog interface.");
	//int buffer_size = 256;
	//char *buffer = calloc( buffer_size, sizeof(char) );

	bool advance = 1;


	ABRIR_MOLDURA();

	puts("<entering dialog loop>");
	while( 1 ) {

		if( advance ){
			while( dlg_now->type != dn_TALK ){
				//printf( "running through Dnodes, dlg_now->type: %d\n", dlg_now->type );
				switch( dlg_now->type ){
					case dn_RESPONSE: //?????
						break;
					case dn_ASSIGNMENT:
						perform_dlg_assignment( dlg_now, va_ptrs );
						break;
					case dn_RETURNER:
						dlg_return_value = dlg_now->bod.return_value;
						goto dialogo_exit;
				}
				dlg_now->visited = 1;
				
				if( dlg_now->next == NULL ){
					do{
						if( dlg_now->parent == NULL ){
							goto dialogo_exit;
						}
						dlg_now = dlg_now->parent;

					} while( dlg_now->next == NULL || dlg_now->next->visited );
				}
				dlg_now = dlg_now->next;
			}

			/*int len = 0;
				len += strlen( dlg_now->bod.talking.content );
				for (int i = 0; i < dlg_now->bod.talking.responses_N; ++i ){
					len += strlen( dlg_now->bod.talking.responses[i]->bod.talking.content );
				}
				if( len > buffer_size ){
					buffer_size = len + 1;
					buffer = realloc( buffer, buffer_size * sizeof(char) );
				}
				sprintf( buffer, "%s\n\n", dlg_now->bod.talking.content );
				len = strlen( buffer );
				for (int i = 0; i < dlg_now->bod.talking.responses_N; ++i ){
					int pl;
					sprintf( buffer + len, "#(>%s)%n\n", dlg_now->bod.talking.responses[i]->bod.talking.content, &pl );
					len += pl+1;
				}
				dialog_ask( R, widget, buffer );*/

			dlg_now->visited = 1;

			Dialog_interface_update( R, &DI, dlg_now );

			advance = 0;
			response = -1;
		}

		SDL_Event event;
		while( SDL_PollEvent(&event) ){
			if(event.type == SDL_QUIT){
				M->quit_flag = 1;
				goto dialogo_exit;
			}

			UI_event_handler( &dui, &event );

			if( response >= 0 ){
				dlg_now = dlg_now->bod.talking.responses[ response ];
				advance = 1;
			}
		}

		SDL_SetRenderDraw_SDL_Color( R, &(M->pal_lo) );
		render_Box_9Slice( R, M->popup_moldura, M );
		SDL_SetRenderDraw_SDL_Color( R, &(M->pal_hi) );
		UI_display( R, &dui );

		SDL_RenderPresent(R);

		SDL_framerateDelay(16);
	}
	dialogo_exit:

	destroy_DNTree( dlg_root );

	destroy_UI_Set( &dui );

	M->popup_moldura->c = 0;

	if( DI.responses != NULL ) free( DI.responses );

	//free( buffer );
	free( va_ptrs );
	
	return dlg_return_value;
}





void CFB_Navegar_Mapa( SDL_Renderer *R, Mundo *M, int *LOC ){

	int mx, my;
	SDL_GetMouseState(&mx, &my);
	vec2d mouse = v2d(mx,my);
	vec2d mf_mouse = v2d(mouse.x - M->map_frame.x, mouse.y - M->map_frame.y);
	vec2d mar = abaulado_reverso( M->TA, mf_mouse );
	int MO = mouse_on_node( &mar, &(M->grafo), &(M->T), M->nirsq );
	bool panning = 0;
	int next_LOC = -1;
	int *path = NULL;
	int path_index = -1;
	vec2d path_vel;
	float path_speed = 2;
	int last_frames = 0;
	vec2d loc_indicator_pos, target_pos;

	//<Example GUI for the demo>
	Box_9Slice naipes_9s;
	SDL_Rect naipes_dst;
	naipes_dst.w = (M->map_icon_layout.w * 4) + 100;
	naipes_dst.h = M->map_icon_layout.h + 100;
	naipes_dst.x = 0.5 * (M->width - naipes_dst.w);
	naipes_dst.y = M->height - (M->map_icon_layout.h + 50);
	build_Box_9Slice( &naipes_9s, M, 4, naipes_dst, 5, 0 );
	//</>

	while ( 1 ) {

		SDL_Event event;
		while( SDL_PollEvent(&event) ){

			if(event.type == SDL_QUIT){
				M->quit_flag = 1;
				goto navegar_exit;
			}

			//refresh = 1;

			int captured = UI_event_handler( &(M->GUI), &event );

			if( captured ){
				
			}
			else{
				//const Uint8 *state = SDL_GetKeyboardState(NULL);

				switch (event.type) {

					case SDL_KEYDOWN:
						//if( event.key.keysym.sym == SDLK_n ){
						break;
					
					case SDL_KEYUP:
						if( event.key.keysym.sym == SDLK_p ){
							/* TAKE SCREENSHOT
							SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");
							SDL_Texture *expt  = SDL_CreateTexture( R, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 3840, 2160 );
							SDL_Texture *expta = SDL_CreateTexture( R, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 3840, 2160 );
							SDL_SetTextureBlendMode( expt, SDL_BLENDMODE_BLEND );
							Transform et = (Transform){ 0, 0, M->T.cx * 2.8111273, M->T.cy * 2.8111273, M->T.s * 2.8111273, M->T.invs * (1.0/2.8111273) };
							double nidT = M->nid * et.s;
							double nirT = M->nir * et.s;
							tela_abaulada *eta = malloc( sizeof(tela_abaulada) );
							SDL_Rect eb = (SDL_Rect){ 0, 0, 3840, 2160 };
							build_tela_abaulada( eta, 24, &eb, 0.2 );

							SDL_SetRenderTarget( R, expt );

							SDL_SetRenderDraw_SDL_Color( R, &(M->pal_lo) );
							SDL_RenderClear( R );

							SDL_SetRenderDraw_SDL_Color( R, &(M->pal_hi) );
							int Si = 0;
							for (int i = 0; i < M->grafo.N; ++i){
								for (int c = 1; c <= M->grafo.connections[i][0]; ++c ){
									int j = M->grafo.connections[i][c];

									if( nodes_connected( &(M->grafo), i, j ) ){
										gp_draw_thickLine( R, atfX(M->grafo.points[i].x, &et), 
															  atfY(M->grafo.points[i].y, &et), 
															  atfX(M->grafo.points[j].x, &et), 
															  atfY(M->grafo.points[j].y, &et), 
															  et.s );
									}
								}
							}

							Si = 0;
							for (int i = 0; i < M->grafo.N; ++i){

								vec2d vt = apply_transform_v2d(M->grafo.points + i, &et);
								
								SDL_RenderCopyF( R, M->map_icons, M->icon_srcs + M->grafo.icon_IDs[ i ], 
												 &(SDL_FRect){ vt.x - nirT, vt.y - nirT, nidT, nidT } );
							}

							SDL_SetRenderTarget( R, expta );							

							render_tela_abaulada( R, expt, eta );

							SDL_SetRenderTarget( R, NULL );
							
							save_texture( "print.png", R, expta );

							destroy_tela_abaulada( eta );
							SDL_DestroyTexture( expt );
							SDL_DestroyTexture( expta );
							*/
						}
						break;

					case SDL_MOUSEMOTION:;

						/*vec2d pmouse = mouse;*/
						mouse = v2d( event.motion.x, event.motion.y );
						mf_mouse.x = mouse.x - M->map_frame.x;
						mf_mouse.y = mouse.y - M->map_frame.y;
						mar = abaulado_reverso( M->TA, mf_mouse );
						MO = mouse_on_node( &mar, &(M->grafo), &(M->T), M->nirsq );

						if( panning ){
							M->T.cx += event.motion.xrel;
							M->T.cy += event.motion.yrel;
							M->AAT.cx = M->T.cx * M->AA;
							M->AAT.cy = M->T.cy * M->AA;
						}

						break;
					
					case SDL_MOUSEBUTTONDOWN:{

						if ( event.button.button == SDL_BUTTON_RIGHT ){
							panning = 1;
						}

						} break;
					
					case SDL_MOUSEBUTTONUP:{

						if ( event.button.button == SDL_BUTTON_RIGHT ){
							panning = 0;
						}
						else if( event.button.button == SDL_BUTTON_LEFT && MO >= 0  && next_LOC < 0 ){
							if( MO == *LOC ){
								last_frames = 3;
							}
							else if( nodes_connected( &(M->grafo), *LOC, MO ) ){
								next_LOC = MO;
							}
							else{
								path = nodemap_path( &(M->grafo), *LOC, MO, traversible_if_known );
								if( path != NULL ){
									next_LOC = path[0];
									path_index = 1;
									/*int i = 0;
										while(1){
											printf("path[%d]: %d\n", i, path[i] );
											if( path[i] < 0 ) break;
											i++;
										}
										free( path );*/
								}
							}
							if( next_LOC > -1 ){
								loc_indicator_pos = v2d( M->grafo.points[*LOC].x, 
														 M->grafo.points[*LOC].y - M->nir -1 );
								target_pos = v2d( M->grafo.points[ next_LOC ].x, 
												  M->grafo.points[ next_LOC ].y - M->nir -1 );
								path_vel = v2d_diff( target_pos, loc_indicator_pos );
								path_vel = v2d_setlen( path_vel, path_speed );
							}
						}
						
						} break;
					
					case SDL_MOUSEWHEEL:;

						float xrd = (mouse.x - M->T.cx) * M->T.invs;
						float yrd = (mouse.y - M->T.cy) * M->T.invs;
						M->scaleI -= event.wheel.y;
						set_scale( &(M->T), pow(1.1, M->scaleI) );
						M->T.cx = mouse.x - xrd * M->T.s;
						M->T.cy = mouse.y - yrd * M->T.s;
						
						M->AAT.cx = M->T.cx * M->AA;
						M->AAT.cy = M->T.cy * M->AA;
						M->AAT.s = M->T.s * M->AA;
						M->AAT.invs = M->T.invs * (1.0/M->AA);
						M->nirT = M->nir * M->AAT.s;
						M->nidT = 2 * M->nirT;
						break;
				}
			}
		}

		SDL_SetRenderDrawColor( R, 0,0,0,255 );
		SDL_RenderClear( R );

		SDL_SetRenderTarget( R, M->AAtexture );

		SDL_SetRenderDraw_SDL_Color( R, &(M->pal_lo) );
		SDL_RenderClear( R );

		// Draw Links
		int Si = 0;
		for (int i = 0; i < M->grafo.N; ++i){
			for (int c = 1; c <= M->grafo.connections[i][0]; ++c ){
				int j = M->grafo.connections[i][c];

				if( (M->grafo.knowledge[i] < 2) && (M->grafo.knowledge[j] < 2) ) continue;
				if( (M->grafo.knowledge[i] >= 2) && (M->grafo.knowledge[j] >= 2) ){
					SDL_SetRenderDraw_SDL_Color( R, &(M->pal_hi) );
				}else{
					SDL_SetRenderDraw_SDL_Color( R, &(M->pal_mo) );
				}

				if( nodes_connected( &(M->grafo), i, j ) ){
					gp_draw_thickLine( R, atfX(M->grafo.points[i].x, &(M->AAT)), 
										  atfY(M->grafo.points[i].y, &(M->AAT)), 
										  atfX(M->grafo.points[j].x, &(M->AAT)), 
										  atfY(M->grafo.points[j].y, &(M->AAT)), 
										  M->AAT.s );
				}
			}
		}

		// Draw Nodes
		Si = 0;
		for (int i = 0; i < M->grafo.N; ++i){

			if( M->grafo.knowledge[i] < 1 ) continue;

			vec2d vt = apply_transform_v2d(M->grafo.points + i, &(M->AAT));

			if( i == MO ){
				SDL_SetRenderDrawColor( R, 14, 84, 255, 255 );
				gp_drawthick_fastcircle( R, vt.x, vt.y, M->nirT+6, 2 );
			}
			int ID = M->grafo.icon_IDs[ i ];
			if( M->grafo.knowledge[i] < 2 ) ID = 2;// the "?" icon


			//#define NODEVIEW_INDICES
			#ifdef NODEVIEW_INDICES
				M->font.scale = 0.25;
				sprintf( M->buf, "%d", i );
				SDL_SetRenderDraw_SDL_Color( R, &(M->pal_lo) );
				gp_fill_fastcircle( R, vt.x, vt.y, M->nirT );
				TX_render_string_centered( R, &(M->font), M->buf, vt.x, vt.y );
			#else
				SDL_Rect src = src_in_sheet( M->map_icon_layout, ID, 1 );
				SDL_FRect dst = (SDL_FRect){ vt.x - M->nirT, vt.y - M->nirT, M->nidT, M->nidT };
				SDL_RenderCopyF( R, M->map_icons, &src, &dst );
			#endif
		}

		// Draw Labels
		for (int i = 0; i < M->grafo.labels_N; ++i ){
			if( M->grafo.labels[i].show ){
				double lx = atfX( M->grafo.labels[i].pos.x, &(M->AAT)); //+ map_frame_dst.x;
				double ly = atfY( M->grafo.labels[i].pos.y, &(M->AAT)); //+ map_frame_dst.y;
				M->font.scale = M->grafo.labels[i].scale * M->AAT.s;
				TX_render_string_curved( R, &(M->font), M->grafo.labels[i].str,
										 lx, ly, M->grafo.labels[i].angle, 
										 M->grafo.labels[i].curvature );
			}
		}

		// Draw Loc Marker
		if( next_LOC >= 0 ){

			vec2d vt = apply_transform_v2d( &loc_indicator_pos, &(M->AAT) );
			render_animated_sprite( R, M->loc_indicator, vt.x, vt.y );
			//loc_indicator_pos = v2d_lerp( loc_indicator_pos, target_pos, loc_walk );
			//loc_walk += 0.04;
			v2d_add( &loc_indicator_pos, path_vel );

			if( last_frames <= 0 && v2d_distsq( loc_indicator_pos, target_pos ) < sq(path_speed) ){
				if( path != NULL ){
					int pLOC = next_LOC;
					next_LOC = path[ path_index++ ];
					if( next_LOC < 0 ){
						*LOC = pLOC;
						free( path );
						path == NULL;
						last_frames = 3;
						panning = 0;
					}
					else{
						loc_indicator_pos = v2d( M->grafo.points[     pLOC ].x, 
												 M->grafo.points[     pLOC ].y - M->nir -1 );
							   target_pos = v2d( M->grafo.points[ next_LOC ].x, 
												 M->grafo.points[ next_LOC ].y - M->nir -1 );
						path_vel = v2d_diff( target_pos, loc_indicator_pos );
						path_vel = v2d_setlen( path_vel, path_speed );
					}
				}
				else{
					*LOC = next_LOC;
					next_LOC = -1;
					last_frames = 3;
					panning = 0;
					//goto navegar_exit;
				}
			}
		}
		else{
			vec2d vt = apply_transform_v2d( M->grafo.points + (*LOC), &(M->AAT) );
			//indicator above:
			render_animated_sprite( R, M->loc_indicator, vt.x, vt.y - M->nirT );
			//indicator to the right:
			//render_animated_sprite( R, M->loc_indicator, vt.x + M->nirT, vt.y );
		}


		SDL_SetRenderTarget( R, NULL );
		//SDL_RenderCopy( R, M->AAtexture, NULL, &(M->map_frame) );

		render_tela_abaulada( R, M->AAtexture, M->TA );

		UI_display( R, &(M->GUI) );

		//<Example GUI for the demo>
		if( M->ESPADAS || M->OUROS || M->COPAS || M->PAUS ){
			SDL_SetRenderDraw_SDL_Color( R, &(M->pal_lo) );
			render_Box_9Slice( R, &naipes_9s, M );
			if( M->ESPADAS ){
				SDL_Rect src = src_in_sheet( M->map_icon_layout, 24, 1 );
				SDL_FRect dst = (SDL_FRect){ naipes_dst.x + 50, naipes_dst.y+50, 64, 64 };
				SDL_RenderCopyF( R, M->map_icons, &src, &dst );
			}
			if( M->OUROS ){
				SDL_Rect src = src_in_sheet( M->map_icon_layout, 25, 1 );
				SDL_FRect dst = (SDL_FRect){ naipes_dst.x + 114, naipes_dst.y+50, 64, 64 };
				SDL_RenderCopyF( R, M->map_icons, &src, &dst );
			}
			if( M->COPAS ){
				SDL_Rect src = src_in_sheet( M->map_icon_layout, 26, 1 );
				SDL_FRect dst = (SDL_FRect){ naipes_dst.x + 178, naipes_dst.y+50, 64, 64 };
				SDL_RenderCopyF( R, M->map_icons, &src, &dst );
			}
			if( M->PAUS ){
				SDL_Rect src = src_in_sheet( M->map_icon_layout, 27, 1 );
				SDL_FRect dst = (SDL_FRect){ naipes_dst.x + 242, naipes_dst.y+50, 64, 64 };
				SDL_RenderCopyF( R, M->map_icons, &src, &dst );
			}

		}

		//</>

		//SDL_SetRenderDrawColor( R, 50, 50, 50, 50 );
		//UI_display_grid( R, &ui, width, height );

		SDL_RenderPresent(R);

		SDL_framerateDelay(16);

		if( last_frames > 0 ){
			last_frames -= 1;
			if( last_frames <= 0 ) goto navegar_exit;
		}
	}
	navegar_exit:;

}