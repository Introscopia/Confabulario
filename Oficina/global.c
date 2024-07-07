#include "global.h"
#include "ferramentas.h"


void montar_mundo( SDL_Renderer *R, Mundo *M, int width, int height ){

	M->width = width;
	M->height = height;

	M->hand_cursor = SDL_CreateSystemCursor( SDL_SYSTEM_CURSOR_HAND );

	build_UI_Set( &(M->GUI), 0, 8, 14, 18, 2, 0.95, width, height );
	#ifdef DEBUGMODE
	puts("- GUI construida."); 
	#endif
	/*UI_init_static_set( &(M->GUI), 1 );
	  UI_build_static_label( &(M->GUI), 0, 0, R, "Confabulario", "TL", &font );//puts("o");*/
	/*
	M->GUI.Vx = 0.95;
	M->GUI.Hx = 1.95;
	bool import = 0;
	UI_build_toggle( &(M->GUI), 10, 0, &import, pal_lo, pal_mo, pal_hi );
	toggle_set_label( UI_last_element( &(M->GUI) ), &font, "Importar", "CC" );
	*/
	M->GUI.Hx = M->GUI.columns - 0.05;// 13.95;
	M->GUI.Vx = M->GUI.rows    - 0.05;// 17.95;
	M->map_frame = calc_rect( &(M->GUI), 0, 0 );

	SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "2" );
	//CARREGAR ICONES
	M->map_icons = IMG_LoadTexture( R, "Assets/map icons.png" );
	SDL_SetTextureBlendMode( M->map_icons, SDL_BLENDMODE_BLEND );
	M->map_icon_layout = (SDL_Rect){ 6, 6, 64, 64 };

	#ifdef DEBUGMODE
	puts("- icones carregados."); 
	#endif

	//PALETA DE CORES
	M->pal_lo = Uint32_to_SDL_Color(0x111210FF);
	M->pal_hi = Uint32_to_SDL_Color(0xe0e3d9FF);
	M->pal_mo = lerp_SDL_Color( M->pal_lo, M->pal_hi, 0.5 );

	//INDICADOR DE LOCAL NO MAPA
	M->loc_indicator_data = malloc( sizeof(Generic_Sprite_data) );
	M->loc_indicator_data->texture = IMG_LoadTexture( R, "Assets/GESBP pointer blue ver.png" ); // Arrow 02 GESBP pointer
	int loc_indicator_cols = 6;
	int loc_indicator_rows = 5;
	M->loc_indicator_data->frames_N = loc_indicator_cols * loc_indicator_rows;
	M->loc_indicator_data->period = 1000.0 / 16.0;
	M->loc_indicator_data->srcs = malloc( M->loc_indicator_data->frames_N * sizeof(SDL_Rect) );
	SDL_QueryTexture( M->loc_indicator_data->texture, NULL, NULL, &(M->loc_indicator_data->srcs[0].w), &(M->loc_indicator_data->srcs[0].h) );
	M->loc_indicator_data->srcs[0].w /= loc_indicator_cols;
	M->loc_indicator_data->srcs[0].h /= loc_indicator_rows;
	//printf("w: %d, h: %d\n", M->loc_indicator_data->srcs[0].w, M->loc_indicator_data->srcs[0].h );
	for (int j = 0; j < loc_indicator_rows; ++j ){
		for (int i = 0; i < loc_indicator_cols; ++i ){
			int o = i + j*loc_indicator_cols;
			M->loc_indicator_data->srcs[ o ] = (SDL_Rect){ i * M->loc_indicator_data->srcs[0].w,
														   j * M->loc_indicator_data->srcs[0].h,
														       M->loc_indicator_data->srcs[0].w,
														       M->loc_indicator_data->srcs[0].h };
		}
	}
	M->loc_indicator_data->offset.x = -(M->loc_indicator_data->srcs[0].w * 0.5);
	M->loc_indicator_data->offset.y = -(M->loc_indicator_data->srcs[0].h);
	M->loc_indicator_data->scale = 1;

	M->loc_indicator = malloc( sizeof(Animated_Sprite_inst) );
	M->loc_indicator->data = M->loc_indicator_data;
	M->loc_indicator->frame = 0;
	M->loc_indicator->next_frame = 0;

	#ifdef DEBUGMODE
	puts("- indicador carregado."); 
	#endif

	//FONTES TIPOGRAFICAS
	M->font =     render_TX_Font( R, "Assets/Jost-Medium.ttf", 80, M->pal_hi );
	M->antifont = render_TX_Font( R, "Assets/Jost-Medium.ttf", 80, M->pal_lo );

	//CAIXA "POPUP"
	
	M->popup_rct_gui.w = lrint( 0.6 * M->GUI.columns );
	M->popup_rct_gui.h = lrint( 0.8 * M->GUI.rows    );
	M->popup_rct_gui.x = lrint( (M->GUI.columns - M->popup_rct_gui.w) / 2.0 );
	M->popup_rct_gui.y = lrint( (M->GUI.rows    - M->popup_rct_gui.h) / 2.0 );
	M->GUI.Hx = M->popup_rct_gui.w;
	M->GUI.Vx = M->popup_rct_gui.h;
	M->popup_rct = calc_rect( &(M->GUI), M->popup_rct_gui.x, M->popup_rct_gui.y );
	/*(SDL_Rect){ M->map_frame.x + 0.2 * M->map_frame.w, 
	  M->map_frame.y + 0.1 * M->map_frame.h,
	  M->map_frame.w * 0.6, M->map_frame.h * 0.8 };*/

	
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
	M->borders = IMG_LoadTexture( R, "Assets/Kenney borders.png" );
	M->borders_cols = 8;
	M->borders_rows = 4;
	M->borders_cw = 24;
	M->borders_slice = 10;	

	int moldscl = 5;
	SDL_Rect moldrct = M->popup_rct;
	moldrct.x -= moldscl * M->borders_slice;
	moldrct.y -= moldscl * M->borders_slice;
	moldrct.w += 2 * (moldscl * M->borders_slice);
	moldrct.h += 2 * (moldscl * M->borders_slice);

	M->popup_moldura = malloc( sizeof(Box_9Slice) );
	build_Box_9Slice( M->popup_moldura, M, rand() % 32, moldrct, moldscl, 60 );

	// DADOS
	init_Cam( &(M->cam3d), 55 );
	//M->D6_pips = IMG_LoadTexture( R, "Assets/d6 pips.png" );
	ok_vec_init( &(M->dados_vec) );
	ok_map_init( &(M->dados_map) );


	// CARREGAR O MAPA
	import_nodemap( "data.bin", &(M->grafo) );
	#ifdef DEBUGMODE
	puts("- Nodemap carregado."); 
	#endif

	// CENTRALIZAR CAMERA NO NÓ INICIAL
	M->T = (Transform){ 0 };
	M->scaleI = 11;
	set_scale( &(M->T), pow(1.1, M->scaleI) );
	M->T.cx = (width /2) - (M->T.s * M->grafo.points[0].x);
	M->T.cy = (height/2) - (M->T.s * M->grafo.points[0].y);

	#ifdef DEBUGMODE
	/* REVELAR TODO O MAPA 
	for (int i = 0; i < M->grafo.N; ++i ){
		M->grafo.knowledge[i] = 2;
		//M->grafo.icon_IDs[i] = i % 8;
		//if( M->grafo.icon_IDs[i] > 3 ) M->grafo.icon_IDs[i] += 20;
		//else M->grafo.icon_IDs[i] = 0;
	}*/
	#endif

	//ANTI-ALIASING
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");
	M->AA = 2;
	M->AAtexture = SDL_CreateTexture( R, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 
									  M->AA * M->map_frame.w, M->AA * M->map_frame.h );
	SDL_SetTextureBlendMode( M->AAtexture, SDL_BLENDMODE_BLEND );
	M->AAT = (Transform){ 0, 0, M->T.cx * M->AA, M->T.cy * M->AA, M->T.s * M->AA, M->T.invs * (1.0/M->AA) };

	M->nid = 24; //node_icon_diameter
	M->nir = M->nid * 0.5; //node_icon_radius
	M->nirsq = sq( M->nir );
	M->nidT = M->nid * M->AAT.s;
	M->nirT = M->nir * M->AAT.s;

	M->TA = malloc( sizeof(tela_abaulada) );
	build_tela_abaulada( M->TA, 24, &(M->map_frame), 0.2 );

	M->quit_flag = 0;

	#ifdef DEBUGMODE
	puts("- Mundo montado.");
	#endif




	// variáveis específicas do Demo:
	M->COPAS = false;
	M->OUROS = false;
	M->petalas = 0;
	M->PAUS = false;
	M->ESPADAS = false;
}

Dice_3d *dado_de_N_lados( Mundo *M, int N, SDL_Renderer *R ){

	Dice_3d *D = ok_map_get( &(M->dados_map), N );

	//printf( "~~~~~~You want a %d sided dice?? Why, I've got just the thing: %p!!\n", N, D );
	//if( D != NULL ) print_Dice_3d( D );

	if( D == NULL ){

		Dice_3d **DD = ok_vec_push_new( &(M->dados_vec) );
		*DD = calloc( 1, sizeof(Dice_3d) );
		D = *DD;

		switch( N ){
			case 1:
				M->antifont.scale = 0.65;
				create_coin_3d( R, D, 0, 1, 16, &(M->antifont), 10, 1, 0.1 );
				break;
			case 2:
				M->antifont.scale = 0.65;
				create_coin_3d( R, D, 1, 2, 16, &(M->antifont), 10, 1, 0.1 );
				break;
			case 4:
				init_dice_3d( D, "Assets/Dados/D4.obj", IMG_LoadTexture( R, "Assets/Dados/D4 "DICE_STYLE".png" ), 10, 1 );
				break;
			case 6:
				init_dice_3d( D, "Assets/Dados/D6.obj", IMG_LoadTexture( R, "Assets/Dados/D6 "DICE_STYLE".png" ), 10, 1 );
				break;
			case 8:
				init_dice_3d( D, "Assets/Dados/D8.obj", IMG_LoadTexture( R, "Assets/Dados/D8 "DICE_STYLE".png" ), 10, 1 );
				break;
			case 12:
				init_dice_3d( D, "Assets/Dados/D12.obj", IMG_LoadTexture( R, "Assets/Dados/D12 "DICE_STYLE".png" ), 10, 0.9 );
				break;
			case 16:
				init_dice_3d( D, "Assets/Dados/D16.obj", IMG_LoadTexture( R, "Assets/Dados/D16 nums.png" ), 10, 0.95 );
				break;
			case 20:
				init_dice_3d( D, "Assets/Dados/D20.obj", IMG_LoadTexture( R, "Assets/Dados/D20 "DICE_STYLE".png" ), 10, 1 );
				break;
			default:
				M->antifont.scale = 0.65;
				create_prismatic_dice_3d( R, D, N, &(M->antifont), 10, 1, 0.6 );
				break;
		}
		ok_map_put( &(M->dados_map), N, D );
	}

	return D;
}

void destruir_mundo( Mundo *M ){

	SDL_DestroyTexture( M->map_icons );
	//SDL_DestroyTexture( M->retratos );
	SDL_DestroyTexture( M->loc_indicator_data->texture );
	SDL_DestroyTexture( M->borders );

	SDL_FreeCursor( M->hand_cursor );

	ok_vec_foreach( &(M->dados_vec), Dice_3d *D ){
    	destroy_dice( D );
    	free( D );
 	}
 	ok_vec_deinit( &(M->dados_vec) );
 	ok_map_deinit( &(M->dados_map) );	

	destroy_TX_Font(&(M->font));
	destroy_TX_Font(&(M->antifont));
	SDL_DestroyTexture( M->AAtexture );
	destroy_UI_Set( &(M->GUI) );
}
