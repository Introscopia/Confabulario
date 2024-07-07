#include "UI.h"
#include <locale.h>
#include "primitives.h"

void build_UI_Set( UI_Set *S, int set_size, float margin, float columns, float rows, float Hx, float Vx, int width, int height ){
	
	S->margin = margin;
	S->columns = columns;
	S->rows = rows;
	S->Hx = Hx;
	S->Vx = Vx;
	S->column_width = lrint((width - (2 * margin)) / columns);
	S->row_height = lrint((height - (2 * margin)) / rows);
	//UI_Label *labels;
	S->set_size = set_size;
	if( set_size > 0 ){
		S->set = malloc( set_size * sizeof(UI_Interactive) );
		S->indices = malloc( S->columns * sizeof(int*) );
		for (int i = 0; i < S->columns; ++i) S->indices[i] = malloc( S->rows * sizeof(int) );

		for (int i = 0; i < S->columns; ++i){
			for (int j = 0; j < S->rows; ++j){
				S->indices[i][j] = -1;
			}
		}
	}
	else{
		S->set =  NULL;
		S->indices =  NULL;
	}
	S->build_index = 0;
	S->dynamic_build_index = 0;
	S->static_build_index = 0;
	
	S->dynamic_set_size = 0;
	S->dynamic_set = NULL;
	S->static_set_size = 0;
	S->static_set = NULL;
}

void build_UI_Set2( UI_Set *S, int set_size, int column_width, int row_height, int width, int height ){
	
	S->margin = 0;
	S->column_width = column_width;
	S->row_height = row_height;
	S->columns = width / column_width;
	S->rows = height / row_height;
	S->Hx = 1;
	S->Vx = 1;
	//UI_Label *labels;
	
	S->indices = malloc( S->columns * sizeof(int*) );
	for (int i = 0; i < S->columns; ++i) S->indices[i] = malloc( S->rows * sizeof(int) );

	for (int i = 0; i < S->columns; ++i){
		for (int j = 0; j < S->rows; ++j){
			S->indices[i][j] = -1;
		}
	}

	S->set_size = set_size;
	if( set_size > 0 ){
		S->set = malloc( set_size * sizeof(UI_Interactive) );
	}
	else{
		S->set =  NULL;
	}
	S->build_index = 0;
	S->dynamic_build_index = 0;
	S->static_build_index = 0;
	
	S->dynamic_set_size = 0;
	S->dynamic_set = NULL;
	S->static_set_size = 0;
	S->static_set = NULL;
}

void build_UI_Set_Copy( UI_Set *S, UI_Set *model ){
	
	S->margin = model->margin;
	S->columns = model->columns;
	S->rows = model->rows;
	S->Hx = model->Hx;
	S->Vx = model->Vx;
	S->column_width = model->column_width;
	S->row_height = model->row_height;

	S->indices = malloc( S->columns * sizeof(int*) );
	for (int i = 0; i < S->columns; ++i) S->indices[i] = malloc( S->rows * sizeof(int) );

	for (int i = 0; i < S->columns; ++i){
		for (int j = 0; j < S->rows; ++j){
			S->indices[i][j] = -1;
		}
	}
	
	S->set_size = 0;
	S->set =  NULL;
	S->dynamic_set_size = 0;
	S->dynamic_set = NULL;
	S->static_set_size = 0;
	S->static_set = NULL;

	S->build_index = 0;
	S->dynamic_build_index = 0;
	S->static_build_index = 0;
}

void destroy_UI_Set( UI_Set *S ){

	for( int i = 0; i < S->set_size; ++i ){
		S->set[i].destroy_function( S->set + i );
	}
	free( S->set );

	for( int i = 0; i < S->dynamic_set_size; ++i ){
		S->dynamic_set[i].destroy_function( S->dynamic_set + i );
	}
	free( S->dynamic_set );

	for( int i = 0; i < S->static_set_size; ++i ){
		SDL_DestroyTexture( S->static_set[i].texture );
	}
	free( S->static_set );
	
}

void UI_init_interactive_set( UI_Set *S, int size ){
	S->set_size = size;
	S->set = calloc( size, sizeof(UI_Interactive) );
}

void UI_init_dynamic_set( UI_Set *S, int size ){
	S->dynamic_set = malloc( size * sizeof(UI_Dynamic) );
	S->dynamic_set_size = size;
	S->dynamic_build_index = 0;
}

void UI_init_static_set( UI_Set *S, int size ){
	S->static_set = calloc( size, sizeof(UI_Static) );
	S->static_set_size = size;
	S->static_build_index = 0;
}



UI_Interactive *UI_last_element( UI_Set *S ){
	return S->set + (S->build_index-1);
}




void log_indices( UI_Set *S, int c, int r ){
	if( S->build_index >= S->set_size ){
		printf("WARNING: Adding more ui_elements than you've allocated for! (%d/%d)", S->build_index, S->set_size );
		return;
	}
	for(int n = c; n < c + ceil(S->Hx); ++n){
		for(int m = r; m < r + ceil(S->Vx); ++m){
			if( n >= 0 && n < S->columns && m >= 0 && m < S->rows ) S->indices[n][m] = S->build_index;
			else printf("Tried to add to indices <%d, %d>, but grid size is <%g, %g>\n", n, m, S->columns, S->rows );
		}
	}
}

float calc_cell_x( UI_Set *S, int c  ){
	return S->margin + (c * S->column_width) + ( ( (ceil(S->Hx) - S->Hx) * S->column_width ) / 2.0 );
}
float calc_cell_y( UI_Set *S, int r  ){
	return S->margin + (r * S->row_height) + ( ( (ceil(S->Vx) - S->Vx ) * S->row_height) / 2.0 );
}
float calc_cell_w( UI_Set *S ){
	return S->Hx * S->column_width;
}
float calc_cell_h( UI_Set *S ){
	return S->Vx * S->row_height;
}
SDL_Rect calc_rect( UI_Set *S, int c, int r ){
	return (SDL_Rect){ round(calc_cell_x(S, c)), round(calc_cell_y(S, r)), ceil(calc_cell_w(S)), ceil(calc_cell_h(S)) };
}

/*
bool inside( UI_Interactive *E, float x, float y ){
	return ( x > E->x && x < E->r && y > E->y && y < E->b );
}*/



// --- --- --- --- --- --- --- --- --- --- --- --- UI_Set input

char UI_event_handler( UI_Set *S, SDL_Event *event ){
	if( S->set_size <= 0 ) return 0;
	int i = -1, j = -1;
	int type = -1;
	char captured = 0;
	switch( event->type ){
		case SDL_MOUSEMOTION:
			type = mouse_motion;
			S->mouseX = event->motion.x;
			S->mouseY = event->motion.y;
			i = floor((S->mouseX - S->margin) / S->column_width);
			j = floor((S->mouseY - S->margin) / S->row_height);
			break;
		case SDL_MOUSEBUTTONDOWN:
			type = mouse_down;
		case SDL_MOUSEBUTTONUP:
			if( type < 0 ) type = mouse_up;
			i = floor((event->button.x - S->margin) / S->column_width);
			j = floor((event->button.y - S->margin) / S->row_height);
			break;
		case SDL_MOUSEWHEEL:
			type = mouse_wheel;
			i = floor((S->mouseX - S->margin) / S->column_width);
			j = floor((S->mouseY - S->margin) / S->row_height);
			break;

		case SDL_KEYDOWN:
			type = key_down;
		case SDL_KEYUP:
			if( type < 0 ) type = key_up;
			//for key events mouse doesn't matter, so we just run all of them with IN=0, and return.
			for (int k = 0; k < S->set_size; k++) captured += S->set[k].IEHFs[type]( (S->set+k), event, 0 );
			return captured;

		case SDL_TEXTINPUT:
			for (int k = 0; k < S->set_size; k++) captured += S->set[k].IEHFs[text_input]( (S->set+k), event, 0 );
			return captured;
		case SDL_TEXTEDITING:
			for (int k = 0; k < S->set_size; k++) captured += S->set[k].IEHFs[text_editing]( (S->set+k), event, 0 );
			return captured;
	}
	if( type < 0 ) return -1;
	if( i >= 0 && j >= 0 && i < S->columns && j < S->rows ){
		int E = S->indices[i][j];
		//puts("UI_event_handler");
		if( E >= 0 ){
			for (int k = 0; k < E; k++){
				captured += S->set[k].IEHFs[type]( (S->set+k), event, 0 );
				//printf("set[%d]\n", k );
			}
			captured += S->set[ E ].IEHFs[type]( (S->set + E), event, 1 );
			//printf("set[%d]\n", E );
			for (int k = E+1; k < S->set_size; k++){
				captured += S->set[k].IEHFs[type]( (S->set+k), event, 0 );
				//printf("set[%d]\n", k );
			}
		}
		else for (int k = 0; k < S->set_size; k++) captured += S->set[k].IEHFs[type]( (S->set+k), event, 0 );
	}
	else for (int k = 0; k < S->set_size; k++) captured += S->set[k].IEHFs[type]( (S->set+k), event, 0 );
	return captured;
}


char UI_orphan_event_handler( UI_Interactive *E, SDL_Event *event ){

	int type = -1;
	int mouseX, mouseY;
	switch( event->type ){
		case SDL_MOUSEMOTION:
			type = mouse_motion;
			mouseX = event->motion.x;
			mouseY = event->motion.y;
			break;
		case SDL_MOUSEBUTTONDOWN:
			type = mouse_down;
		case SDL_MOUSEBUTTONUP:
			if( type < 0 ) type = mouse_up;
			mouseX = event->button.x;
			mouseY = event->button.y;
			break;
		case SDL_MOUSEWHEEL:
			type = mouse_wheel;
			SDL_GetMouseState( &mouseX, &mouseY );
			break;
		case SDL_KEYDOWN:
			type = key_down;
		case SDL_KEYUP:
			if( type < 0 ) type = key_up;
			return E->IEHFs[ type ]( E, event, 0 );
			break;
		case SDL_TEXTINPUT:
			return E->IEHFs[text_input]( E, event, 0 );
		case SDL_TEXTEDITING:
			return E->IEHFs[text_editing]( E, event, 0 );
	}
	if( type < 0 ) return 0;
	return E->IEHFs[ type ]( E, event, coordinates_in_Rect( mouseX, mouseY, &(E->rect) ) );
}

/*
void UI_mouse_motion( UI_Set *S, SDL_Event *event ){ UI_event_handling_function( S, event, mouse_motion ); }
void UI_mouse_down  ( UI_Set *S, SDL_Event *event ){ UI_event_handling_function( S, event, mouse_down ); }
void UI_mouse_up    ( UI_Set *S, SDL_Event *event ){ UI_event_handling_function( S, event, mouse_up ); }
void UI_mouse_wheel ( UI_Set *S, SDL_Event *event ){ UI_event_handling_function( S, event, mouse_wheel ); }
void UI_key_down    ( UI_Set *S, SDL_Event *event ){ UI_event_handling_function( S, event, key_down ); }
void UI_key_up      ( UI_Set *S, SDL_Event *event ){ UI_event_handling_function( S, event, key_up ); }
*/
// --- --- --- --- --- --- --- --- --- --- --- --- Set display

void UI_display(  SDL_Renderer *R, UI_Set *S ){
	//puts("UI_display");
	for( int i = 0; i < S->static_set_size; ++i ){
		//printf("static_set[%d]\n", i );
		SDL_RenderCopy( R, S->static_set[i].texture, NULL, &(S->static_set[i].rect) );
	}
	for( int i = 0; i < S->set_size; ++i ){
		//printf("set[%d]\n", i );
		S->set[i].display_function( R, S->set + i );
	}
	for( int i = 0; i < S->dynamic_set_size; ++i ){
		//printf("dynamic_set[%d]\n", i );
		S->dynamic_set[i].display_function( R, S->dynamic_set + i );
	}
	//puts("donezo!");
}

void UI_display_grid(  SDL_Renderer *R, UI_Set *S, int width, int height ){
	for (float x =  S->margin; x < width ; x += S->column_width ) {
		SDL_RenderDrawLineF( R, x, S->margin, x, height - S->margin );
	}
	for (float y = S->margin; y < height ; y += S->row_height ) {
		SDL_RenderDrawLineF( R, S->margin, y, width - S->margin, y );
	}
}






//    []o[]o[]o[]o[]o[]o[]o[]o[]o[]o[]o[]o[]o[]o   S T A T I C   o[]o[]o[]o[]o[]o[]o[]o[]o[]o[]o[]o[]o[]o[]o


char empty_IEHF( UI_Interactive *E, SDL_Event *event, bool IN ){ return 0; }

void UI_build_label( UI_Set *S, int c, int r, SDL_Texture *txtr ){
	S->static_set[ S->static_build_index ].rect = calc_rect( S, c, r );
	S->static_set[ S->static_build_index ].texture = txtr;
	S->static_build_index ++;
}


void UI_build_static_label( UI_Set *S, int c, int r, SDL_Renderer * R, char *str, char pos[2], TX_Font *font ){

	int I = S->static_build_index;
	S->static_set[ I ].rect = calc_rect( S, c, r );
	int cw = S->static_set[ I ].rect.w;
	int ch = S->static_set[ I ].rect.h;

	SDL_FRect dst = (SDL_FRect){ 0, 0, 0, 0 };
	TX_SizeText( font, str, &(dst.w), &(dst.h) ); 

	//printf( ">> %d, %g, %g, %d\n", ch, dst.h, font->scale, font->ascent );
	
	int pad = 2;
		 if( pos[0] == 'T' ) dst.y = pad;
	else if( pos[0] == 'B' ) dst.y = ch - (font->scale * font->ascent) - pad;
	else                     dst.y = round(0.5*(ch - (font->scale * font->ascent)));
		 if( pos[1] == 'L' ) dst.x = pad;
	else if( pos[1] == 'R' ) dst.x = cw - dst.w - pad;
	else                     dst.x = round(0.5*(cw - dst.w));

	S->static_set[ I ].texture = SDL_CreateTexture( R, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, dst.w + 2, dst.h + 2 );
	SDL_SetTextureBlendMode( S->static_set[ I ].texture, SDL_BLENDMODE_BLEND );
	SDL_SetRenderTarget( R, S->static_set[ I ].texture );
	SDL_SetRenderDrawColor( R, 0,0,0,0 );
	SDL_RenderClear( R );
	TX_render_string( R, font, str, 1, 1 );//dst.x, dst.y );
	SDL_SetRenderTarget( R, NULL );

	S->static_set[ I ].rect.x += dst.x;// S->static_set[ I ].rect.x + S->static_set[ I ].rect.w - (0.5 * dst.w);
	S->static_set[ I ].rect.y += dst.y;// S->static_set[ I ].rect.y + S->static_set[ I ].rect.h - (0.5 * dst.h);
	S->static_set[ I ].rect.w = dst.w + 2;
	S->static_set[ I ].rect.h = dst.h + 2;

	S->static_build_index ++;
}






//    []o[]o[]o[]o[]o[]o[]o[]o[]o[]o[]o[]o[]o[]o    D Y N A M I C   o[]o[]o[]o[]o[]o[]o[]o[]o[]o[]o[]o[]o[]o[]o
/*
typedef struct {
	int *content;
	char *fmt;
	TX_Font *font;
	char pos [2];
} dynamic_int_label;*/

int **UI_build_dynamic_int_label( UI_Set *S, int c, int r, char *fmt, char *pos, int *content, TX_Font *font ){
	int I = S->dynamic_build_index;
	S->dynamic_set[ I ].rect = calc_rect( S, c, r );
	dynamic_int_label *data = malloc( sizeof(dynamic_int_label) );
	int fmtlen = strlen(fmt) + 1;
	*data = (dynamic_int_label){ content, malloc( fmtlen ), font, {pos[0], pos[1]} };
	strcpy( data->fmt, fmt );
	S->dynamic_set[ I ].data = (void*)data;
	S->dynamic_set[ I ].display_function = dynamic_int_label_display;
	S->dynamic_set[ I ].destroy_function = dynamic_int_label_destroy;
	S->dynamic_build_index ++;
	return &(data->content);
}

void dynamic_int_label_display( SDL_Renderer *R, UI_Dynamic *E ){
	dynamic_int_label *data = (dynamic_int_label*) E->data;

	char str [128];
	sprintf( str, data->fmt, *(data->content) );
	float tw, th;
	TX_SizeText( data->font, str, &tw, &th );
	float x = E->rect.x, y = E->rect.y;
	float lh = (data->font->scale * (data->font->ascent - data->font->descent));
	int pad = 2;
		 if( data->pos[0] == 'T' ) y += pad;
	else if( data->pos[0] == 'B' ) y += E->rect.h - lh - pad;
	else                           y += 0.5*(E->rect.h - lh);
		 if( data->pos[1] == 'L' ) x += pad;
	else if( data->pos[1] == 'R' ) x += E->rect.w - tw - pad;
	else                           x += 0.5*(E->rect.w - tw);
	y -= data->font->scale * data->font->descent;

	SDL_RenderSetClipRect( R, &(E->rect) );
	TX_render_string( R, data->font, str, x, y );
	SDL_RenderSetClipRect( R, NULL );
}

void dynamic_int_label_destroy( UI_Dynamic *E ){
	dynamic_int_label *data = (dynamic_int_label*) E->data;
	free( data->fmt );
	free( E->data );
}



// --- --- --- --- --- --- --- --- --- --- --- dynamic_string_label
/*
typedef struct {
	char *content;
	TX_Font *font;
	char positioning [2];
} dynamic_string_label;
*/

char **UI_build_dynamic_string_label( UI_Set *S, int c, int r, char pos[2], TX_Font *font, char *content ){
	int I = S->dynamic_build_index;
	S->dynamic_set[ I ].rect = calc_rect( S, c, r );

	dynamic_string_label *data = malloc( sizeof(dynamic_string_label) );
	*data = (dynamic_string_label){ content, font, {pos[0], pos[1]} };
	S->dynamic_set[ I ].data = (void*)data;
	S->dynamic_set[ I ].display_function = dynamic_string_label_display;
	S->dynamic_set[ I ].destroy_function = dynamic_string_label_destroy;
	S->dynamic_build_index ++;
	return &(data->content);
}

void dynamic_string_label_display( SDL_Renderer *R, UI_Dynamic *E ){
	dynamic_string_label *data = (dynamic_string_label*) E->data;
	if( data->content == NULL ) return;
	int tw, th;
	TX_SizeText( data->font, data->content, &tw, &th );
	int pad = 4;
	int x = E->rect.x, y = E->rect.y;
		 if( data->pos[0] == 'T' ) y += pad;            
	else if( data->pos[0] == 'B' ) y += E->rect.h - th -pad;       
	else                           y += (E->rect.h / 2) - th/2;
		 if( data->pos[1] == 'L' ) x += pad;      
	else if( data->pos[1] == 'R' ) x += E->rect.w - tw -pad;
	else                    	   x += (E->rect.w / 2) - tw/2;

	SDL_RenderSetClipRect( R, &(E->rect) );
	TX_render_string( R, data->font, data->content, x, y );
	SDL_RenderSetClipRect( R, NULL );
}

void dynamic_string_label_destroy( UI_Dynamic *E ){
	free( E->data );
}



// --- --- --- --- --- --- --- --- --- --- --- TX_Label
/*
typedef struct {
	TX_Font *font;
	char *content;
	int tx, ty;
} TX_label;*/

void display_TX_label( SDL_Renderer *R, TX_label *L ){
	if( L != NULL && L->content != NULL ){
		TX_render_string( R, L->font, L->content, L->tx, L->ty );
	}
}




// []o[]o[]o[]o[]o[]o[]o[]o[]o[]o[]o[]o[]o[]o    I N T E R A C T I V E   o[]o[]o[]o[]o[]o[]o[]o[]o[]o[]o[]o[]o[]o[]o





// --- --- --- --- --- --- --- --- --- --- --- --- Toggle Element

/*
typedef struct {
	bool *incumbency;
	bool mouse_over;
	SDL_Color colors [3];
	TX_label label;
} toggle_data;*/


void UI_build_toggle( UI_Set *S, int c, int r, bool *incumbency, SDL_Color lo, SDL_Color mo, SDL_Color hi ){
	int I = S->build_index;
	S->set[ I ].rect = calc_rect( S, c, r );
	toggle_data *data = malloc( sizeof(toggle_data) );
	*data = (toggle_data){ incumbency, 0, {lo, mo, hi}, {NULL, NULL, 0, 0} };
	S->set[ I ].data = (void*)data;
	S->set[ I ].IEHFs[mouse_motion] = toggle_mouse_motion;
	S->set[ I ].IEHFs[ mouse_down ] = empty_IEHF;
	S->set[ I ].IEHFs[  mouse_up  ] = toggle_mouse_up;
	S->set[ I ].IEHFs[ mouse_wheel] = empty_IEHF;
	S->set[ I ].IEHFs[  key_down  ] = empty_IEHF;
	S->set[ I ].IEHFs[   key_up   ] = empty_IEHF;
	S->set[ I ].IEHFs[ text_input ] = empty_IEHF;
	S->set[ I ].IEHFs[text_editing] = empty_IEHF;
	S->set[ I ].display_function = toggle_display;
	S->set[ I ].destroy_function = toggle_destroy;
	log_indices( S, c, r );
	S->build_index ++;
}

UI_Interactive build_orphan_toggle( int x, int y, int w, int h, bool *incumbency, SDL_Color lo, SDL_Color mo, SDL_Color hi ){
	UI_Interactive E;
	E.rect = (SDL_Rect){x,y,w,h};
	toggle_data *data = malloc( sizeof(toggle_data) );
	*data = (toggle_data){ incumbency, 0, {lo, mo, hi}, {NULL, NULL, 0, 0} };
	E.data = (void*)data;
	E.IEHFs[mouse_motion] = toggle_mouse_motion;
	E.IEHFs[ mouse_down ] = empty_IEHF;
	E.IEHFs[  mouse_up  ] = toggle_mouse_up;
	E.IEHFs[ mouse_wheel] = empty_IEHF;
	E.IEHFs[  key_down  ] = empty_IEHF;
	E.IEHFs[   key_up   ] = empty_IEHF;
	E.IEHFs[ text_input ] = empty_IEHF;
	E.IEHFs[text_editing] = empty_IEHF;
	E.display_function = toggle_display;
	E.destroy_function = toggle_destroy;
	return E;
}

void toggle_set_label( UI_Interactive *E, TX_Font *font, char *str, char pos[2] ){
	toggle_data *data = (toggle_data*) E->data;
	SDL_FRect dst = (SDL_FRect){ 0 };
	TX_SizeText( font, str, &(dst.w), &(dst.h) );
	float lh = (font->scale * (font->ascent - font->descent));
	int pad = 2;
		 if( pos[0] == 'T' ) dst.y = pad;
	else if( pos[0] == 'B' ) dst.y = E->rect.h - lh - pad;
	else                     dst.y = 0.5*(E->rect.h - lh);
		 if( pos[1] == 'L' ) dst.x = pad;
	else if( pos[1] == 'R' ) dst.x = E->rect.w - dst.w - pad;
	else                     dst.x = 0.5*(E->rect.w - dst.w);
	dst.y -= font->scale * font->descent;
	data->label = (TX_label){ font, NULL, E->rect.x + dst.x, E->rect.y + dst.y };
	data->label.content = malloc( strlen(str)+1 );
	strcpy( data->label.content, str );
}

char toggle_mouse_motion( UI_Interactive *E, SDL_Event *event, bool IN ){
	toggle_data *data = (toggle_data*) E->data;
	char Q = data->mouse_over != IN;
	data->mouse_over = IN;
	return Q;
}

char toggle_mouse_up( UI_Interactive *E, SDL_Event *event, bool IN ){
	if( IN ){ //if( inside( E, x, y ) ){
		toggle_data *data = (toggle_data*) E->data;
		*(data->incumbency) = !(*(data->incumbency));
	}
	return IN;
}

void toggle_display( SDL_Renderer *R, UI_Interactive *E ){
	toggle_data *data = (toggle_data*) E->data;
	int Q = 0;
	if( *(data->incumbency) ) Q = 2;
	else if( data->mouse_over ) Q = 1;
	SDL_SetRenderDraw_SDL_Color( R, data->colors + Q );
	SDL_RenderFillRect( R, &(E->rect) );
	SDL_SetRenderDraw_SDL_Color( R, data->colors + 2 );
	SDL_RenderDrawRect( R, &(E->rect) );
	display_TX_label( R, &(data->label) );
}

void tickbox_display( SDL_Renderer *R, UI_Interactive *E ){
	toggle_data *data = (toggle_data*) E->data;
	SDL_SetRenderDraw_SDL_Color( R, data->colors + 0 );
	if( data->mouse_over ) SDL_SetRenderDraw_SDL_Color( R, data->colors + 1 );
	SDL_RenderFillRect( R, &(E->rect) );
	SDL_SetRenderDraw_SDL_Color( R, data->colors + 2 );
	SDL_RenderDrawRect( R, &(E->rect) );
	if( *(data->incumbency) ){
		float ox = E->rect.w * 0.1;
		float oy = E->rect.h * 0.1;
		float hw = E->rect.w * 0.5;
		float hh = E->rect.h * 0.5;
		SDL_Color C = SDL_GetRender_SDL_Color( R );
		SDL_Vertex verts[4];
		verts[0] = (SDL_Vertex){ { E->rect.x + hw,             E->rect.y + oy },             C, {0,0} };
		verts[1] = (SDL_Vertex){ { E->rect.x + E->rect.w - ox, E->rect.y + hh },             C, {0,0} };
	    verts[2] = (SDL_Vertex){ { E->rect.x + hw,             E->rect.y + E->rect.h - oy }, C, {0,0} };
	    verts[3] = (SDL_Vertex){ { E->rect.x + ox,             E->rect.y + hh },             C, {0,0} };
	    int indices[6] = { 0, 1, 2, 0, 2, 3 };
		SDL_RenderGeometry( R, NULL, verts, 4, indices, 6 );
	}
}

void toggle_destroy( UI_Interactive *E ){
	toggle_data *data = (toggle_data*) E->data;
	//for (int i = 0; i < 3; ++i)  SDL_DestroyTexture( data->textures[i] );
	free( data->label.content );
	free( E->data );
}
void destroy_toggle_and_textures( UI_Interactive *E ){
	toggle_data *data = (toggle_data*) E->data;
	//for (int i = 0; i < 3; ++i)  SDL_DestroyTexture( data->textures[i] );
	free( data->label.content );
	free( E->data );
}


// --- --- --- --- --- --- --- --- --- --- --- ---/Toggle Element
// --- --- --- --- --- --- --- --- --- --- --- --- NumSet Element




void UI_build_numset( UI_Set *S, int c, int r, int *incumbency, int set, SDL_Color lo, SDL_Color mo, SDL_Color hi ){
	int I = S->build_index;
	S->set[ I ].rect = calc_rect( S, c, r );//calc_rect( S, c, r ); //(SDL_FRect){ calc_cell_x( S, c ), calc_cell_y( S, r ), calc_cell_w( S ), calc_cell_h( S ) };
	numset_data *data = malloc( sizeof(numset_data) );
		*data = (numset_data){ incumbency, set, 0, {lo, mo, hi}, {NULL, NULL, 0, 0} };
	S->set[ I ].data = (void*)data;
	S->set[ I ].IEHFs[mouse_motion] = numset_mouse_motion;
	S->set[ I ].IEHFs[ mouse_down ] = empty_IEHF;
	S->set[ I ].IEHFs[  mouse_up  ] = numset_mouse_up;
	S->set[ I ].IEHFs[ mouse_wheel] = empty_IEHF;
	S->set[ I ].IEHFs[  key_down  ] = empty_IEHF;
	S->set[ I ].IEHFs[   key_up   ] = empty_IEHF;
	S->set[ I ].IEHFs[ text_input ] = empty_IEHF;
	S->set[ I ].IEHFs[text_editing] = empty_IEHF;
	S->set[ I ].display_function = numset_display;
	S->set[ I ].destroy_function = numset_destroy;
	log_indices( S, c, r );
	S->build_index ++;
}

void numset_set_label( UI_Interactive *E, TX_Font *font, char *str, char pos[2] ){
	numset_data *data = (numset_data*) E->data;
	SDL_FRect dst = (SDL_FRect){ 0 };
	TX_SizeText( font, str, &(dst.w), &(dst.h) );
	float lh = (font->scale * (font->ascent - font->descent));
	int pad = 2;
		 if( pos[0] == 'T' ) dst.y = pad;
	else if( pos[0] == 'B' ) dst.y = E->rect.h - lh - pad;
	else                     dst.y = 0.5*(E->rect.h - lh);
		 if( pos[1] == 'L' ) dst.x = pad;
	else if( pos[1] == 'R' ) dst.x = E->rect.w - dst.w - pad;
	else                     dst.x = 0.5*(E->rect.w - dst.w);
	dst.y -= font->scale * font->descent;
	//printf( ">> %g = 0.5*(%d - (%g * (%d - %d))), lh: %g, dst.h: %g\n", dst.y, E->rect.h, font->scale, font->ascent, font->descent, lh, dst.h );
	data->label = (TX_label){ font, NULL, E->rect.x + dst.x, E->rect.y + dst.y };
	data->label.content = malloc( strlen(str)+1 );
	strcpy( data->label.content, str );
}

char numset_mouse_motion( UI_Interactive *E, SDL_Event *event, bool IN  ){
	((numset_data*) E->data)->mouse_over = IN;
	return 0;
}

char numset_mouse_up( UI_Interactive *E, SDL_Event *event, bool IN  ){
	if( IN ){ //if( inside( E, x, y ) ){
		numset_data *data = (numset_data*) E->data;
		*(data->incumbency) = data->set;
	}
	return IN;
}

void numset_display( SDL_Renderer *R, UI_Interactive *E ){
	numset_data *data = (numset_data*) E->data;
	if( data->mouse_over ){
		SDL_SetRenderDraw_SDL_Color( R, data->colors + 1 );
	}else{
		SDL_SetRenderDraw_SDL_Color( R, data->colors + 0 );
	}
	SDL_RenderFillRect( R, &(E->rect) );
	if( *(data->incumbency) == data->set ){
		SDL_SetRenderDraw_SDL_Color( R, data->colors + 2 );
		SDL_RenderFillRect( R, &(E->rect) );
		SDL_SetRenderDraw_SDL_Color( R, data->colors + 0 );
		SDL_Rect inrct = (SDL_Rect){ E->rect.x +4, E->rect.y +4, E->rect.w -8, E->rect.h -8 };
		SDL_RenderFillRect( R, &inrct );
	}else{
		SDL_SetRenderDraw_SDL_Color( R, data->colors + 2 );
		SDL_RenderDrawRect( R, &(E->rect) );
	}
	display_TX_label( R, &(data->label) );
}

void numset_destroy( UI_Interactive *E ){
	numset_data *data = (numset_data*) E->data;
	//for (int i = 0; i < 3; ++i)  SDL_DestroyTexture( data->textures[i] );
	free( data->label.content );
	free( E->data );
}



// --- --- --- --- --- --- --- --- --- --- --- --- /Numset Element
// --- --- --- --- --- --- --- --- --- --- --- --- multi_NumSet Element

/*
typedef struct {
	int *incumbency;
	int N;
	int *set;
	int mouse_over;
	SDL_Texture *spritesheet;
	SDL_Rect *srcs;
	SDL_Rect *dsts;
} multi_numset_data;*/



void UI_build_multi_numset( UI_Set *S, int c, int r, int *incumbency, int N, int *set ){
	int I = S->build_index;
	S->set[ I ].rect = calc_rect( S, c, r );//calc_rect( S, c, r ); //(SDL_FRect){ calc_cell_x( S, c ), calc_cell_y( S, r ), calc_cell_w( S ), calc_cell_h( S ) };
	multi_numset_data *data = malloc( sizeof(multi_numset_data) );
	if( set == NULL ){
		set = malloc( N * sizeof(int) );
		for(int i = 0; i < N; i++){
			set[i] = i;
		}
	}
	*data = (multi_numset_data){ incumbency, N, set, -1, NULL, NULL, NULL };
	S->set[ I ].data = (void*)data;
	S->set[ I ].IEHFs[mouse_motion] = multi_numset_mouse_motion;
	S->set[ I ].IEHFs[ mouse_down ] = empty_IEHF;
	S->set[ I ].IEHFs[  mouse_up  ] = multi_numset_mouse_up;
	S->set[ I ].IEHFs[ mouse_wheel] = empty_IEHF;
	S->set[ I ].IEHFs[  key_down  ] = empty_IEHF;
	S->set[ I ].IEHFs[   key_up   ] = empty_IEHF;
	S->set[ I ].IEHFs[ text_input ] = empty_IEHF;
	S->set[ I ].IEHFs[text_editing] = empty_IEHF;
	S->set[ I ].display_function = multi_numset_display;
	S->set[ I ].destroy_function = multi_numset_destroy;
	log_indices( S, c, r );
	S->build_index ++;
}

char multi_numset_mouse_motion( UI_Interactive *E, SDL_Event *event, bool IN  ){
	multi_numset_data *data = (multi_numset_data*) E->data;
	data->mouse_over = -1;
	if( IN ){
		for(int i = 0; i < data->N; i++){
			if( cursor_in_rect( event, data->dsts + i ) ){
				data->mouse_over = i;
				return 0;
			}
		}
	}
	return 0;
}

char multi_numset_mouse_up( UI_Interactive *E, SDL_Event *event, bool IN  ){
	multi_numset_data *data = (multi_numset_data*) E->data;
	if( data->mouse_over >= 0 ){
		*(data->incumbency) = data->set[ data->mouse_over ];
		return 1;
	}
	return 0;
}

void multi_numset_display( SDL_Renderer *R, UI_Interactive *E ){
	multi_numset_data *data = (multi_numset_data*) E->data;
	
	SDL_Rect dst = (SDL_Rect){ data->dsts[0].x, data->dsts[0].y, 
							   data->dsts[data->N - 1].x + data->dsts[data->N - 1].w - data->dsts[0].x, 
							   data->dsts[0].h };
	SDL_RenderCopy( R, data->spritesheet, data->srcs + 0, &dst );

	if( data->mouse_over >= 0 ){
		int Q = 1 + data->mouse_over;
		SDL_RenderCopy( R, data->spritesheet, data->srcs + Q, data->dsts + data->mouse_over );
	}
	int the = -1;
	for(int i = 0; i < data->N; i++){
		if( *(data->incumbency) == data->set[i] ){
			the = i;
			break;
		}
	}
	if( the >= 0 ){
		int Q = 1 + data->N + the;
		SDL_RenderCopy( R, data->spritesheet, data->srcs + Q, data->dsts + the );
	}
}

void multi_numset_destroy( UI_Interactive *E ){
	multi_numset_data *data = (multi_numset_data*) E->data;
	SDL_DestroyTexture( data->spritesheet );
	free( data->set );
	free( data->srcs );
	free( data->dsts );
	free( E->data );
}




// --- --- --- --- --- --- --- --- --- --- --- --- /multi_Numset Element
// --- --- --- --- --- --- --- --- --- --- --- --- NumCycle Element
/*
typedef struct {
	int *incumbency;
	int increment, lo, hi; // lo and hi are inclusive
	bool mouse_over, pressed;
	clock_t ct, step;
	SDL_Color colors [3];
	TX_label label;
} numcycle_data;
*/
void UI_build_numcycle( UI_Set *S, int c, int r, int *incumbency, int increment, int low, int high, 
						SDL_Color lo, SDL_Color mo, SDL_Color hi ){
	int I = S->build_index;
	S->set[ I ].rect = calc_rect( S, c, r );//calc_rect( S, c, r ); //(SDL_FRect){ calc_cell_x( S, c ), calc_cell_y( S, r ), calc_cell_w( S ), calc_cell_h( S ) };
	numcycle_data *data = malloc( sizeof(numcycle_data) );
	*data = (numcycle_data){ incumbency, increment, low, high, 0, 0, 0, 0, 
							 {lo, mo, hi}, {NULL, NULL, 0, 0} };
	S->set[ I ].data = (void*)data;
	S->set[ I ].IEHFs[mouse_motion] = numcycle_mouse_motion;
	S->set[ I ].IEHFs[ mouse_down ] = numcycle_mouse_down;
	S->set[ I ].IEHFs[  mouse_up  ] = numcycle_mouse_up;
	S->set[ I ].IEHFs[ mouse_wheel] = empty_IEHF;
	S->set[ I ].IEHFs[  key_down  ] = empty_IEHF;
	S->set[ I ].IEHFs[   key_up   ] = empty_IEHF;
	S->set[ I ].IEHFs[ text_input ] = empty_IEHF;
	S->set[ I ].IEHFs[text_editing] = empty_IEHF;
	S->set[ I ].display_function = numcycle_display;
	S->set[ I ].destroy_function = numcycle_destroy;
	log_indices( S, c, r );
	S->build_index ++;
}

UI_Interactive new_numcycle( SDL_Rect R, int *incumbency, int increment, int low, int high, SDL_Color lo, SDL_Color mo, SDL_Color hi ){
	UI_Interactive O = (UI_Interactive){ R, malloc( sizeof(numcycle_data) ), 
										 { numcycle_mouse_motion, empty_IEHF, 
										   numcycle_mouse_up, 
										   empty_IEHF, empty_IEHF, empty_IEHF },
										 numcycle_display, numcycle_destroy };
	numcycle_data *data = (numcycle_data*) O.data;
	data->incumbency = incumbency;
	data->increment = increment;
	data->lo = low;
	data->hi = high;
	data->mouse_over = 0;
	data->pressed = 0;
	data->colors[0] = lo;
	data->colors[1] = mo;
	data->colors[2] = hi;
	data->label = (TX_label){NULL, NULL, 0, 0};
	return O;
}
void numcycle_set_label( UI_Interactive *E, TX_Font *font, char *str, char pos[2] ){
	numcycle_data *data = (numcycle_data*) E->data;
	SDL_FRect dst = (SDL_FRect){ 0 };
	TX_SizeText( font, str, &(dst.w), &(dst.h) );
	float lh = (font->scale * (font->ascent - font->descent));
	int pad = 2;
		 if( pos[0] == 'T' ) dst.y = pad;
	else if( pos[0] == 'B' ) dst.y = E->rect.h - lh - pad;
	else                     dst.y = 0.5*(E->rect.h - lh);
		 if( pos[1] == 'L' ) dst.x = pad;
	else if( pos[1] == 'R' ) dst.x = E->rect.w - dst.w - pad;
	else                     dst.x = 0.5*(E->rect.w - dst.w);
	dst.y -= font->scale * font->descent;
	data->label = (TX_label){ font, NULL, E->rect.x + dst.x, E->rect.y + dst.y };
	data->label.content = malloc( strlen(str)+1 );
	strcpy( data->label.content, str );
}

char numcycle_mouse_motion( UI_Interactive *E, SDL_Event *event, bool IN  ){
	((numcycle_data*) E->data)->mouse_over = IN;
	return 0;
}

char numcycle_mouse_down( UI_Interactive *E, SDL_Event *event, bool IN  ){
	numcycle_data *data = (numcycle_data*) E->data;
	data->pressed = IN;
	if( IN ){
		*(data->incumbency) += data->increment;
			 if( *(data->incumbency) > data->hi ) *(data->incumbency) = data->lo;
		else if( *(data->incumbency) < data->lo ) *(data->incumbency) = data->hi;
		data->ct = clock();
		data->step = 0.6 * CLOCKS_PER_SEC;
	}
	return IN;
}

char numcycle_mouse_up( UI_Interactive *E, SDL_Event *event, bool IN  ){
	if( IN ){ //if( inside( E, x, y ) ){
		numcycle_data *data = (numcycle_data*) E->data;
		clock_t since = clock() - data->ct;
		if( since > data->step ){
			*(data->incumbency) += data->increment;
				 if( *(data->incumbency) > data->hi ) *(data->incumbency) = data->lo;
			else if( *(data->incumbency) < data->lo ) *(data->incumbency) = data->hi;
		}
		data->pressed = 0;
	}
	return IN;
}

void numcycle_display( SDL_Renderer *R, UI_Interactive *E ){
	numcycle_data *data = (numcycle_data*) E->data;
	int Q = 0;
	if( data->pressed ){
		clock_t since = clock() - data->ct;
		if( since > data->step ){
			*(data->incumbency) += data->increment;
				 if( *(data->incumbency) > data->hi ) *(data->incumbency) = data->lo;
			else if( *(data->incumbency) < data->lo ) *(data->incumbency) = data->hi;

			data->step *= 0.75;
			if( data->step < 0.166 * CLOCKS_PER_SEC ) data->step = 0.166 * CLOCKS_PER_SEC;
			data->ct = clock();
		}
		Q = 2;
	} 
	else if( data->mouse_over ) Q = 1;
	SDL_SetRenderDraw_SDL_Color( R, data->colors + Q );
	SDL_RenderFillRect( R, &(E->rect) );
	SDL_SetRenderDraw_SDL_Color( R, data->colors + 2 );
	SDL_RenderDrawRect( R, &(E->rect) );
	display_TX_label( R, &(data->label) );
}

void numcycle_destroy( UI_Interactive *E ){
	numcycle_data *data = (numcycle_data*) E->data;
	//for (int i = 0; i < 3; ++i)  SDL_DestroyTexture( data->textures[i] );
	free( data->label.content );
	free( E->data );
}

// --- --- --- --- --- --- --- --- --- --- --- --- /NumCycle Element
// --- --- --- --- --- --- --- --- --- --- --- ---  PlusMinus Element




void UI_build_plusminus( UI_Set *S, int c, int r, int *incumbency, int increment ){
	int I = S->build_index;
	S->set[ I ].rect = calc_rect( S, c, r );//calc_rect( S, c, r ); //(SDL_FRect){ calc_cell_x( S, c ), calc_cell_y( S, r ), calc_cell_w( S ), calc_cell_h( S ) };
	plusminus_data *data = malloc( sizeof(plusminus_data) );
	*data = (plusminus_data){ incumbency, increment, S->set[ I ].rect.h, 0, 0, 0, 0.9, 0.5, 0, 
							{ (SDL_Rect){ S->set[I].rect.x,                                       S->set[I].rect.y, S->set[I].rect.h, S->set[I].rect.h },  
							  (SDL_Rect){ S->set[I].rect.x + S->set[I].rect.w - S->set[I].rect.h, S->set[I].rect.y, S->set[I].rect.h, S->set[I].rect.h } },
							{NULL, NULL, NULL, NULL, NULL, NULL, NULL}, NULL };
	S->set[ I ].data = (void*)data;
	S->set[ I ].IEHFs[mouse_motion] = plusminus_mouse_motion;
	S->set[ I ].IEHFs[ mouse_down ] = plusminus_mouse_down;
	S->set[ I ].IEHFs[  mouse_up  ] = plusminus_mouse_up;
	S->set[ I ].IEHFs[ mouse_wheel] = empty_IEHF;
	S->set[ I ].IEHFs[  key_down  ] = empty_IEHF;
	S->set[ I ].IEHFs[   key_up   ] = empty_IEHF;
	S->set[ I ].IEHFs[ text_input ] = empty_IEHF;
	S->set[ I ].IEHFs[text_editing] = empty_IEHF;
	S->set[ I ].display_function = plusminus_display;
	S->set[ I ].destroy_function = plusminus_destroy;
	log_indices( S, c, r );
	S->build_index++;
}

char plusminus_mouse_motion( UI_Interactive *E, SDL_Event *event, bool IN  ){
	plusminus_data *data = (plusminus_data*) E->data;
	if( IN ){
		if( event->motion.x < E->rect.x + data->button_width ){
			data->lb = 1;
			data->rb = 0;
		}
		else if( event->motion.x > E->rect.x + E->rect.w - data->button_width ){
			data->rb = 1;
			data->lb = 0;
		}
		else goto neither;
	}
	else {
		neither:
		data->lb = 0;
		data->rb = 0;
	}
	return 0;
}

char plusminus_mouse_down( UI_Interactive *E, SDL_Event *event, bool IN  ){
	plusminus_data *data = (plusminus_data*) E->data;
	if( IN ){
		if( event->motion.x < E->rect.x + data->button_width ){
			data->lb = 2;
			data->t = clock();
			data->held = 0;
			*(data->incumbency) -= data->increment;
		}
		else if( event->motion.x > E->rect.x + E->rect.w - data->button_width ){
			data->rb = 2;
			data->t = clock();
			data->held = 0;
			*(data->incumbency) += data->increment;
		}
	}
	return IN;
}

char plusminus_mouse_up( UI_Interactive *E, SDL_Event *event, bool IN  ){
	plusminus_data *data = (plusminus_data*) E->data;
	if( data->lb == 2 ){
		if( IN && event->motion.x < E->rect.x + data->button_width ) data->lb = 1;
		else data->lb = 0;
	}
	else if( data->rb == 2 ){
		if( IN && event->motion.x > E->rect.x + E->rect.w - data->button_width ) data->rb = 1;
		else data->rb = 0;
	}
	return IN;
}

void plusminus_display( SDL_Renderer *R, UI_Interactive *E ){
	plusminus_data *data = (plusminus_data*) E->data;
	//====== TICK
	if( data->lb == 2 ){
		clock_t n = clock() - data->t;
		float s = ((float)n)/CLOCKS_PER_SEC;
		if( data->held ){
			if( s >= data->freq ){
				*(data->incumbency) -= data->increment;
				data->t = clock();
			}
		}
		else{
			if( s >= data->hold ){
				data->held = 1;
				*(data->incumbency) -= data->increment;
				data->t = clock();
			}
		}
	}
	else if( data->rb == 2 ){
		clock_t n = clock() - data->t;
		float s = ((float)n)/CLOCKS_PER_SEC;
		if( data->held ){
			if( s >= data->freq ){
				*(data->incumbency) += data->increment;
				data->t = clock();
			}
		}
		else{
			if( s >= data->hold ){
				data->held = 1;
				*(data->incumbency) += data->increment;
				data->t = clock();
			}
		}
	}

	//===========
	SDL_RenderCopy( R, data->textures[0], NULL, &(E->rect) );
	SDL_RenderCopy( R, data->textures[data->lb + 1], NULL, data->brects );
	SDL_RenderCopy( R, data->textures[data->rb + 4], NULL, data->brects + 1 );

	char str [32];
	sprintf( str, "%d", *(data->incumbency) );
	int w = 0, h = 0;
	TX_SizeText( data->font, str, &w, &h );
	h += data->font->scale * data->font->descent;
	TX_render_string( R, data->font, str, E->rect.x + 0.5*(E->rect.w - w), E->rect.y + 0.5*(E->rect.h - h) );
}

void plusminus_destroy( UI_Interactive *E ){
	plusminus_data *data = (plusminus_data*) E->data;
	for (int i = 0; i < 7; ++i)  SDL_DestroyTexture( data->textures[i] );
	free( E->data );
}




// --- --- --- --- --- --- --- --- --- --- --- --- /PlusMinus Element
// --- --- --- --- --- --- --- --- --- --- --- --- Slider Element


/*
typedef struct {
	float *incumbency;
	float min, max;
	bool dragging;
	SDL_Texture *bg;
	SDL_Texture *knob;
	SDL_Rect knob_dst;
} slider_data;*/

void UI_build_slider( UI_Set *S, int c, int r, float *incumbency, float min, float max ){
	int I = S->build_index;
	S->set[ I ].rect = calc_rect( S, c, r );
	slider_data *data = malloc( sizeof(slider_data) );
	*data = (slider_data){ incumbency, min, max, 0, NULL, NULL, (SDL_Rect){0,0,0,0} };
	S->set[ I ].data = (void*)data;
	S->set[ I ].IEHFs[mouse_motion] = slider_mouse_motion;//empty_IEHF;
	S->set[ I ].IEHFs[ mouse_down ] = slider_mouse_down;  //empty_IEHF;
	S->set[ I ].IEHFs[  mouse_up  ] = slider_mouse_up;    //empty_IEHF;
	S->set[ I ].IEHFs[ mouse_wheel] = empty_IEHF;
	S->set[ I ].IEHFs[  key_down  ] = empty_IEHF;
	S->set[ I ].IEHFs[   key_up   ] = empty_IEHF;
	S->set[ I ].IEHFs[ text_input ] = empty_IEHF;
	S->set[ I ].IEHFs[text_editing] = empty_IEHF;
	S->set[ I ].display_function = slider_display;
	S->set[ I ].destroy_function = slider_destroy;
	log_indices( S, c, r );
	S->build_index ++;
}

void slider_set_incumbency( UI_Interactive *E, float *incumbency ){
	slider_data *data = (slider_data*) E->data;
	data->incumbency = incumbency;
	data->knob_dst.x = map( *(data->incumbency), data->min, data->max, E->rect.x, E->rect.x + E->rect.w-(data->knob_dst.w/2) );
}

char slider_mouse_down( UI_Interactive *E, SDL_Event *event, bool IN ){
	if( IN ){
		slider_data *data = (slider_data*) E->data;
		data->dragging = 1;
		int right = E->rect.x + E->rect.w;
		int hkw = data->knob_dst.w / 2;
		*(data->incumbency) = constrainF( map( event->button.x, E->rect.x + hkw, right-hkw, data->min, data->max ), data->min, data->max );
		data->knob_dst.x = constrain( event->button.x - hkw, E->rect.x, right - data->knob_dst.w);
	}
	return IN;
}

char slider_mouse_motion( UI_Interactive *E, SDL_Event *event, bool IN ){
	slider_data *data = (slider_data*) E->data;
	if( data->dragging ){
		int right = E->rect.x + E->rect.w;
		int hkw = data->knob_dst.w / 2;
		*(data->incumbency) = constrainF( map( event->button.x, E->rect.x + hkw, right-hkw, data->min, data->max ), data->min, data->max );
		data->knob_dst.x = constrain( event->button.x - hkw, E->rect.x, right - data->knob_dst.w);
	}
	return data->dragging;
}

char slider_mouse_up( UI_Interactive *E, SDL_Event *event, bool IN ){
	slider_data *data = (slider_data*) E->data;
	data->dragging = 0;
	return 0;
}

void slider_display( SDL_Renderer *R, UI_Interactive *E ){
	slider_data *data = (slider_data*) E->data;
	SDL_RenderCopy( R, data->bg, NULL, &(E->rect) );
	SDL_RenderCopy( R, data->knob, NULL, &(data->knob_dst) );
}

void slider_destroy( UI_Interactive *E ){
	free( E->data );
}





// --- --- --- --- --- --- --- --- --- --- --- --- /Slider Element
// --- --- --- --- --- --- --- --- --- --- --- ---  Dialog Element





typedef struct String_chainlink {
	char *string;
	float y;
	int h;
	String_chainlink *next;
} String_chainlink;

String_chainlink* get_link( String_chainlink* chainstart, int A ){
	String_chainlink* out = chainstart;
	for (int i = 0; i < A; ++i){
		out = out->next;
		if( out == NULL ) return chainstart;
	}
	return out;
}

String_chainlink* get_last_link( String_chainlink* chainstart ){
	String_chainlink* out = chainstart;
	while( out->next != NULL ){
		out = out->next;
	}
	return out;
}

int chain_size( String_chainlink* chainstart ){
	String_chainlink* out = chainstart;
	int len = 0;
	while( out != NULL ){
		out = out->next;
		++len;
	}
	return len;
}

String_chainlink** new_link( String_chainlink** chainstart ){
	String_chainlink** out = chainstart;
	while( *out != NULL ){
		out = &((*out)->next);
	}
	return out;
}

String_chainlink** new_link_and_id( String_chainlink** chainstart, int *id ){
	String_chainlink** out = chainstart;
	*id = 0;
	while( *out != NULL ){
		out = &((*out)->next);
		*id += 1;
	}
	return out;
}

void chain_backspace( String_chainlink** chainstart ){

	if( chainstart == NULL ) return;
	if( *chainstart == NULL ) return;

	if( (*chainstart)->next == NULL ){
		free( (*chainstart)->string );
		free( *chainstart );
		*chainstart = NULL;
		return;
	}

	String_chainlink *link = *chainstart;	
	while( link->next != NULL ){
		if( link->next->next == NULL ){
			//printf("unsaying %s\n", link->next->string );
			free( link->next->string );
			free( link->next );
			link->next = NULL;
			return;
		}
		link = link->next;
	}
}

void destroy_chain( String_chainlink** chainstart ){

	if( chainstart == NULL ) return;
	if( *chainstart == NULL ) return;

	String_chainlink* link = *chainstart;
	while( link != NULL ){
		String_chainlink *next = link->next;
		free( link->string );
		free( link );
		link = next;
	}
	*chainstart = NULL;
}

/*
typedef struct {

	SDL_Texture *background;
	int *incumbency;

	TX_Font *font;

	int margin;
	String_chainlink *log;
	SDL_Texture *log_texture;
	SDL_Rect log_rect;
	SDL_Rect *hyperlinks;
	int *hyperlink_indices;
	int hyperlinks_length;

	int scroll, scroll_width, current_height;
	UI_Interactive scrollbar;
	SDL_Texture *arrows;
	SDL_Rect arrow_srcs[3];
	SDL_Rect arrow_dsts[2];
	bool scrolling_up, scrolling_down, over_up, over_down;
	int scrolling_wheel;

	bool animate;
	Typist typist;
	int typist_index;
	
} dialog_data;
*/
UI_Interactive * UI_build_dialog( UI_Set *S, int c, int r, int *incumbency, int sbw, TX_Font *font, float font_scale ){
	//puts( "UI_build_dialog" );
	int I = S->build_index;
	S->set[ I ].rect = calc_rect( S, c, r );

	dialog_data *data = malloc( sizeof(dialog_data) );

	*data = (dialog_data){ incumbency, 
						   font, font_scale,
						   S->margin,
						   NULL, NULL, 
						   S->set[ I ].rect, 
						   NULL, NULL, 0,
						   0, sbw, 0,
						   {0},
						   //NULL,
						   //{ (SDL_Rect){-1,-1,-1,-1}, (SDL_Rect){-1,-1,-1,-1}, (SDL_Rect){-1,-1,-1,-1} },
						   { (SDL_Rect){9999,0,0,0}, (SDL_Rect){9999,0,0,0} },
						   0, 0, 0, 0, 
						   0, 
						   0,
						   (Typist){ NULL, NULL, 0, 0, 0, 0, NULL },
						   0 };
	data->log_rect.x += 0.07 * S->set[ I ].rect.w;
	data->log_rect.y += 0.07 * S->set[ I ].rect.h;
	data->log_rect.w = 0.86 * S->set[ I ].rect.w;
	data->log_rect.h = 0.86 * S->set[ I ].rect.h;
	data->scrollbar = new_vertical_scrollbar( data->log_rect, sbw, &(data->scroll) );
	//data->arrow_dsts[0].x = S->set[I].rect.x + S->set[I].rect.w -sbw;
	//data->arrow_dsts[1].x = data->arrow_dsts[0].x;
	//data->arrow_dsts[0].y = S->set[I].rect.y;
	//data->arrow_dsts[1].y = S->set[I].rect.y;
	//data->arrow_dsts[0].w = sbw; data->arrow_dsts[1].w = sbw;

	S->set[ I ].data = (void*)data;
	S->set[ I ].IEHFs[mouse_motion] = dialog_mouse_motion;
	S->set[ I ].IEHFs[ mouse_down ] = dialog_mouse_down;
	S->set[ I ].IEHFs[  mouse_up  ] = dialog_mouse_up;
	S->set[ I ].IEHFs[ mouse_wheel] = dialog_mouse_wheel;
	S->set[ I ].IEHFs[  key_down  ] = empty_IEHF;
	S->set[ I ].IEHFs[   key_up   ] = empty_IEHF;
	S->set[ I ].IEHFs[ text_input ] = empty_IEHF;
	S->set[ I ].IEHFs[text_editing] = empty_IEHF;
	S->set[ I ].display_function = dialog_display;
	S->set[ I ].destroy_function = dialog_destroy;
	log_indices( S, c, r );
	S->build_index ++;
	return S->set + I;
}


void refresh_dialog_texture( SDL_Renderer *R, UI_Interactive *E ){
	//puts( "refresh_dialog_texture" );
	dialog_data *data = (dialog_data*) E->data;

	if( data->log_texture == NULL ){
		data->log_texture = SDL_CreateTexture( R, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 
											   data->log_rect.w, data->log_rect.h );
	}

	float top = -(data->scroll);
	float bottom = top + data->log_rect.h;
	bool flip = 0;
	String_chainlink *link = data->log;

	SDL_SetRenderTarget( R, data->log_texture );
	SDL_SetRenderDrawColor( R, 0, 0, 0, 0 );
	SDL_RenderClear( R );
	data->font->scale = data->font_scale;
	while( link != NULL ){
		if( (link->y > top || link->y + link->h > top ) && link->y < bottom ){
			TX_render_string_wrapped( R, data->font, link->string, 0, link->y + data->scroll, data->log_rect.w );
			flip = 1;
		}
		else if( flip ) break;
		link = link->next;
	}
	for (int i = 0; i < data->hyperlinks_N; ++i){
		int y = data->hyperlinks[i].y + data->hyperlinks[i].h -8;
		if( y > top && y < bottom ){
			SDL_SetRenderDrawColor( R, 255, 255, 255, 255 );
			SDL_RenderDrawLine( R, data->hyperlinks[i].x + 2,                     y + data->scroll, 
										  data->hyperlinks[i].x + data->hyperlinks[i].w, y + data->scroll );

			//SDL_RenderDrawRect( R, &(SDL_Rect){ data->hyperlinks[i].x,
			//                                           data->hyperlinks[i].y + data->scroll,
			//                                           data->hyperlinks[i].w,
			//                                           data->hyperlinks[i].h } );
		}
	}
	SDL_SetRenderTarget( R, NULL );
}

void refresh_dialog_until( SDL_Renderer *R, UI_Interactive *E, int until ){
	//printf( "refresh_dialog_until %d\n", until );
	if( until < 0 ) return;
	dialog_data *data = (dialog_data*) E->data;

	float top = -(data->scroll);
	float bottom = top + data->log_rect.h;
	bool flip = 0;
	String_chainlink *link = data->log;

	SDL_SetRenderTarget( R, data->log_texture );
	SDL_SetRenderDrawColor( R, 0, 0, 0, 0 );
	SDL_RenderClear( R );
	int i = 0;
	while( link != NULL ){
		if( (link->y > top || link->y + link->h > top ) && link->y < bottom ){
			TX_render_string_wrapped( R, data->font, link->string, 0, link->y + data->scroll, data->log_rect.w );
			flip = 1;
		}
		else if( flip ) break;
		link = link->next;
		i++;
		if( i > until ) break;
	}
	SDL_SetRenderTarget( R, NULL );
}

char dialog_mouse_motion( UI_Interactive *E, SDL_Event *event, bool IN ){
	dialog_data *data = (dialog_data*) E->data;
	data->over_up = 0;
	data->over_down = 0;
	
	if( IN ){
		if( vertical_scrollbar_mouse_motion( &(data->scrollbar), event, event->motion.x > data->scrollbar.rect.x ) ){//E->rect.x + E->rect.w - data->scroll_width
			return 1;
		}
		if( event->motion.x > data->arrow_dsts[0].x ){
			if( event->motion.y < data->arrow_dsts[0].y + data->arrow_dsts[0].h ){
				data->over_up = 1;
				return 1;
			}
			else if( event->motion.y > data->arrow_dsts[1].y ){
				data->over_down = 1;
				return 1;
			}
		}
	}
	return 0;
}

char dialog_mouse_down( UI_Interactive *E, SDL_Event *event, bool IN ){
	//puts( "dialog_mouse_down" );
	if( IN ){
		dialog_data *data = (dialog_data*) E->data;
		if( vertical_scrollbar_mouse_down( &(data->scrollbar), event, event->button.x > data->scrollbar.rect.x ) ){//E->rect.x + E->rect.w - data->scroll_width
			return 1;
		}
		if( data->over_up ){
			data->scrolling_up = 1;
			return 1;
		}
		else if( data->over_down ){
			data->scrolling_down = 1;
			return 1;
		}
	}
	return 0;
}

char dialog_mouse_up( UI_Interactive *E, SDL_Event *event, bool IN ){
	//printf( "dialog_mouse_up, IN:%d\n", IN );
	dialog_data *data = (dialog_data*) E->data;
	data->scrolling_up = 0;
	data->scrolling_down = 0;
	//printf( "%d, %f, %d, %d\n", event->button.x - E->rect.x, event->button.y - E->rect.y + data->scroll, data->hyperlinks[0].x, data->hyperlinks[0].y );
	if( IN ){
		char captured = vertical_scrollbar_mouse_up( &(data->scrollbar), event, IN && (event->button.x > data->scrollbar.rect.x) );
		if( !captured ){
			for (int i = 0; i < data->hyperlinks_N; ++i){
				if( coordinates_in_Rect( event->button.x - data->log_rect.x, event->button.y - data->log_rect.y - data->scroll, (data->hyperlinks) + i ) ){
					//printf("click on hyperlink number %d, indices: %p\n", i, data->hyperlink_indices );
					if( data->hyperlink_indices != NULL ){
						//for(int j = 0; j < data->hyperlinks_N; j++){	printf("%d, ", data->hyperlink_indices[ j ] );}
						*(data->incumbency) = data->hyperlink_indices[ i ];
						free( data->hyperlink_indices );
						data->hyperlink_indices = NULL;
					}
					else{
						*(data->incumbency) = i;
					}
					data->hyperlinks_N = 0;
					free( data->hyperlinks );
					data->hyperlinks = NULL;
					return 1;
				}
			}
		}
	}
	return 0;
}

char dialog_mouse_wheel( UI_Interactive *E, SDL_Event *event, bool IN ){
	//puts("dialog_mouse_wheel");
	if( IN ){
		((dialog_data*) E->data)->scrolling_wheel = event->wheel.y;	
	}
	return IN;
}

void dialog_say( SDL_Renderer *R, UI_Interactive *E, char *text ){

	if( text == NULL ) return;
	//printf("dialog_saying \"%s\"\n", text );

	dialog_data *data = (dialog_data*) E->data;
	data->font->scale = data->font_scale;
	int id;
	String_chainlink **neo = new_link_and_id( &(data->log), &id );
	*neo = malloc( sizeof(String_chainlink) );
	//printf("%d : %d\n", &((*neo)->y), &(data->log->y) );
	//(*neo)->surf = TTF_RenderUTF8_Blended_Wrapped( font, text, data->fg, data->log_rect.w );
	int len = strlen( text );
	(*neo)->string = malloc( len + 1 );
	strcpy( (*neo)->string, text );
	(*neo)->y = data->current_height;
	(*neo)->h = TX_wrapped_string_height( data->font, text, data->log_rect.w ) + lrint(data->font->line_skip * data->font->scale * 0.8);
	(*neo)->next = NULL;
	data->current_height += (*neo)->h;// + data->margin;

	vertical_scrollbar_update( R, &(data->scrollbar), data->current_height );
	data->scroll = fminf( data->log_rect.h - data->current_height, 0 );
	vertical_scrollbar_external_scroll( &(data->scrollbar), 0 );

	if( data->animate ){
		if( typing_done(&(data->typist)) ){
			reinit_typist( &(data->typist), (*neo)->string, 1 );
			data->typist_index = id;
		}
		refresh_dialog_until( R, E, data->typist_index-1 );
	}
	else{
		//puts("say");
		refresh_dialog_texture( R , E );
	}
}

void dialog_ask( SDL_Renderer *R, UI_Interactive *E, char *text ){

	if( text == NULL ) return;
	//printf("dialog_asking \"%s\"\n", text);
	dialog_data *data = (dialog_data*) E->data;
	data->font->scale = data->font_scale;

	data->hyperlinks_N = strcchr( text, '#' );
	if( data->hyperlinks_N <= 0 ){
		dialog_say( R, E, text );
		return;
	}
	//printf("#s: %d\n", data->hyperlinks_N );//debug

	if( data->hyperlinks != NULL ){
		free( data->hyperlinks );
		//data->hyperlinks = NULL;
	}
	if( data->hyperlink_indices != NULL ){
		free( data->hyperlink_indices );
		data->hyperlink_indices = NULL;
	}

	data->hyperlinks = malloc( data->hyperlinks_N * sizeof(SDL_Rect) );
	int link_index = 0;

	int textlen = strlen( text );

	int id;
	String_chainlink **neo = new_link_and_id( &(data->log), &id );
	*neo = malloc( sizeof(String_chainlink) );
	(*neo)->string = malloc( textlen + 1 );//strcpy( (*neo)->string, text );
	int neo_index = 0;
	(*neo)->y = data->current_height;
	//(*neo)->h = TX_wrapped_string_height( data->font, text, data->log_rect.w );
	(*neo)->next = NULL;
	//puts("chain is good.");//debug

	int cx = 0;//cursor
	int cy = (*neo)->y;
	int line_end = data->log_rect.w;
	int space = data->font->scale * data->font->space;
	int line_height = data->font->scale * data->font->line_skip;
	bool parenthesizing = 0;
	int pcx = 0, pcy = 0;
	int paren_depth = 0;
	bool close_paren = 0;

	for ( int i = 0; i < textlen; ){

		//printf("i=%d\n",i );//debug

		bool is_link = 0;
		int ww = 0;//word width
		int we = textlen;//word end
		for ( int j = i; j <= textlen; ++j ){
			if( j == textlen ){
				we = j;
				break;
			}
			else if( j == i && text[j] == '#' ){
				is_link = 1;
				i++;
				if( text[j+1] == '(' ){
					j += 1;
					i++;
					parenthesizing = 1;
					paren_depth = 1;
					close_paren = 0;
					pcx = cx;
					pcy = cy;
					is_link = 0;
					//puts("begin parenthesizing");//debug
				}
				continue;
			}
			else if( parenthesizing && text[j] == '(' ){
				paren_depth += 1;
			}
			else if( parenthesizing && text[j] == ')' ){
				paren_depth -= 1;
				if( paren_depth <= 0 ){
					close_paren = 1;
					is_link = 1;
					we = j;
					break;
				}
			}
			else if( text[j] < '!' || text[j] > '~' ){
				we = j;
				break;
			}
			//else if ( punctuation_or_symbol( text[j] ) ){
			//	we = j+1;
			//	break;
			//}
			//ww += data->font->scale * data->font->adv[ text[j] - '!' ];
			int bytes = 0;
			ww += TX_glyph_width( data->font, text + j, &bytes );
			if( ww > line_end ){
				ww -= TX_glyph_width( data->font, text + j, &bytes );
				we = j-1;
				break;
			}
			j += bytes-1;
		}
		//printf("we=%d, ww=%d\n", we, ww );//debug
		if( cx + ww > line_end ){
			cx = 0;
			cy += line_height;
		}
		//printf("is_link=%d, parenthesizing=%d, close_paren=%d\n", is_link, parenthesizing, close_paren );//debug
		if( is_link ){
			if( parenthesizing ){
				if( close_paren ){
					if( cy > pcy ){

						int lns = lrint( (cy - pcy) / (float)line_height );

						//printf("multiline, lns = %d, link_index: %d\n", lns, link_index );//debug

						data->hyperlinks_N += lns;
						data->hyperlinks = realloc( data->hyperlinks, data->hyperlinks_N * sizeof(SDL_Rect) );
						bool first_multi = (data->hyperlink_indices == NULL);
						data->hyperlink_indices = realloc( data->hyperlink_indices, data->hyperlinks_N * sizeof(int) );

						if( first_multi ){
							for(int hi = 0; hi < link_index; hi++){
								data->hyperlink_indices[ hi ] = hi;
							}
						}

						int oli = link_index;
						if( link_index > 0 ){
							oli = 1 + data->hyperlink_indices[ link_index-1 ];
						}

						data->hyperlinks[ link_index ] = (SDL_Rect){ pcx, pcy, line_end-pcx, line_height };
						data->hyperlink_indices[ link_index ] = oli;
						link_index++;

						for(int ly = 1; ly <= lns; ly++){
							int tcy = pcy + (ly * line_height);
							if( ly == lns ){
								data->hyperlinks[ link_index ] = (SDL_Rect){ 0, tcy, cx + ww, line_height };
							}
							else{
								data->hyperlinks[ link_index ] = (SDL_Rect){ 0, tcy, line_end, line_height };
							}
							data->hyperlink_indices[ link_index ] = oli;
							link_index++;
						}
					}
					else{
						if( data->hyperlink_indices != NULL ){
							data->hyperlink_indices[ link_index ] = 1 + data->hyperlink_indices[ link_index-1 ];
						}
						data->hyperlinks[ link_index++ ] = (SDL_Rect){ pcx, pcy, (cx+ww)-pcx, line_height };
					}

					parenthesizing = 0;
				}
			}
			else {
				if( data->hyperlink_indices != NULL ){
					data->hyperlink_indices[ link_index ] = 1 + data->hyperlink_indices[ link_index-1 ];
				}
				data->hyperlinks[ link_index++ ] = (SDL_Rect){ cx, cy, ww, line_height };
			}
		}

		while( i < we ){
			(*neo)->string[ neo_index++ ] = text[i];
			++i;
		}
		cx += ww;
		//printf("w:%d\n", i );//debug

		if( close_paren ){
			++i;// skip the close paren;
			if( i >= textlen ) break;
			close_paren = 0;
		}

		//printf("up next... %c\n", text[i] );//debug

		while( !( isalnum( text[i] ) || text[i] == '#' ) ){
			
			if( text[i] == ' ' ){
				if( cx + space > line_end ){
					cx = 0;
					cy += line_height;
				}
				else cx += space;
			}
			else if( text[i] == '\t' ){
				if( cx + (4*space) > line_end ){
					cx = 0;
					cy += line_height;
				}
				else cx += (4*space);
			}
			else if( text[i] == '\n' ){
				cx = 0;
				cy += line_height;
			}
			else if( text[i] < '!' || text[i] > '~' ){
				if( cx + space > line_end ){
					cx = space;
					cy += line_height;
				}
				else cx += space;
			}
			else{
				int bytes = 0;
				float cw = TX_glyph_width( data->font, text + i, &bytes );//data->font->scale * data->font->adv[I];
				if( cx + cw > line_end ){
					cx = 0;
					cy += line_height;
				}
				cx += cw;
			}
			(*neo)->string[ neo_index++ ] = text[i];
			++i;
			if( i >= textlen ) break;
		}
	}
	(*neo)->string[ neo_index ] = '\0';

	(*neo)->h = cy + line_height - (*neo)->y + lrint(data->font->line_skip * data->font->scale * 0.8);


	data->current_height += (*neo)->h;// + data->margin;
	vertical_scrollbar_update( R, &(data->scrollbar), data->current_height );
	data->scroll = fminf( data->log_rect.h - data->current_height, 0 );
	vertical_scrollbar_external_scroll( &(data->scrollbar), 0 );
	
	if( data->animate ){
		if( typing_done(&(data->typist)) ){
			reinit_typist( &(data->typist), (*neo)->string, 0 );
			data->typist_index = id;	
		}
		refresh_dialog_until( R, E, data->typist_index-1 );
	}
	else{
		//puts("ask");
		refresh_dialog_texture( R , E );
	}

	*(data->incumbency) = -1;
}

void dialog_unsay( SDL_Renderer *R, UI_Interactive *E ){
	dialog_data *data = (dialog_data*) E->data;
	String_chainlink *last = get_last_link( data->log );
	data->current_height -= last->h;
	chain_backspace( &(data->log) );
	if( data->hyperlinks_N > 0 ){
		data->hyperlinks_N = 0;
		free( data->hyperlinks );
		data->hyperlinks = NULL;
	}
	if( data->hyperlink_indices != NULL ){
		free( data->hyperlink_indices );
		data->hyperlink_indices = NULL;
	}
	vertical_scrollbar_update( R, &(data->scrollbar), data->current_height );

	if( data->animate && !typing_done( &(data->typist) ) ){
		refresh_dialog_until( R, E, data->typist_index-1 );
	}
	else refresh_dialog_texture( R , E );
}

void dialog_clear( SDL_Renderer *R, UI_Interactive *E ){
	dialog_data *data = (dialog_data*) E->data;
	destroy_chain( &(data->log) );
	if( data->hyperlinks_N > 0 ){
		data->hyperlinks_N = 0;
		free( data->hyperlinks );
		data->hyperlinks = NULL;
	}
	if( data->hyperlink_indices != NULL ){
		free( data->hyperlink_indices );
		data->hyperlink_indices = NULL;
	}
	data->current_height = 0;
	refresh_dialog_texture( R , E );
	vertical_scrollbar_update( R, &(data->scrollbar), data->current_height );
}

void dialog_display( SDL_Renderer *R, UI_Interactive *E ){
	//puts( "dialog_display" );
	dialog_data *data = (dialog_data*) E->data;
	data->font->scale = data->font_scale;

	//SDL_RenderCopy( R, data->background, NULL, &(E->rect) );

	vertical_scrollbar_data *vsbd = (vertical_scrollbar_data *)(data->scrollbar.data);
	//if( data->scrolling_up || data->scrolling_down || data->scrolling_wheel != 0 || vsbd->dragging || vsbd->holding ){
		//data->scroll += ( data->scrolling_down * 1.2 ) - ( data->scrolling_up * 1.2 ) - ( data->scrolling_wheel * 36 );
		//printf("(%d)\n", data->scrolling_up );
		//
	if( vsbd->dragging || vsbd->holding ){
		refresh_dialog_texture( R, E );
	}
	if( data->scrolling_wheel != 0  ){//data->scrolling_up || data->scrolling_down || 
		//data->scroll += (data->scrolling_up * 2.5) - (data->scrolling_down * 2.5) + ( data->scrolling_wheel * 36 );
		//data->scroll = constrainF( data->scroll, -(data->current_height-data->log_rect.h), 0 ); 
		//data->scrolling_wheel = 0;
		vertical_scrollbar_external_scroll( &(data->scrollbar), ( data->scrolling_wheel * 36 ) );//( data->scrolling_down * 0.03125 ) - ( data->scrolling_up * 0.03125 ) + 
		//data->scroll = constrainF( data->scroll, 0, fmaxf( data->current_height - E->rect.h, 0 ) );
		//puts("display");

		if( data->animate && !typing_done( &(data->typist) ) ){
			refresh_dialog_until( R, E, data->typist_index-1 );
		}
		else refresh_dialog_texture( R , E );
	}

	if( data->animate && !typing_done( &(data->typist) ) ){
		String_chainlink* link = get_link( data->log, data->typist_index );

		SDL_RenderSetClipRect( R, &(data->log_rect) );
		render_typist( R, data->font, &(data->typist), data->log_rect.x, data->log_rect.y + link->y + data->scroll, data->log_rect.w, TX_ALIGN_LEFT );
		SDL_RenderSetClipRect( R, NULL );

		if( typing_done( &(data->typist) ) ){
			
			if( chain_size( data->log )-1 > data->typist_index ){
				refresh_dialog_until( R, E, data->typist_index );
				data->typist_index += 1;
				link = get_link( data->log, data->typist_index );
				reinit_typist( &(data->typist), link->string, 1 );
			}
			else{
				puts("typing done");
				refresh_dialog_texture( R, E );
			}
		}
	}

	SDL_RenderCopy( R, data->log_texture, NULL, &(data->log_rect) );

	vertical_scrollbar_display( R, &(data->scrollbar) );

	/*int A = 0;
	if( data->scrolling_up ) A = 2;
	else if( data->over_up ) A = 1;
	SDL_RenderCopy( R, data->arrows, data->arrow_srcs + A, data->arrow_dsts + 0 );
	A = 0;
	if( data->scrolling_down ) A = 2;
	else if( data->over_down ) A = 1;
	SDL_RenderCopyEx( R, data->arrows, data->arrow_srcs + A, data->arrow_dsts + 1, 0, NULL, SDL_FLIP_VERTICAL );
	*/
}

void dialog_destroy( UI_Interactive *E ){
	//puts("destroying dialog...");
	dialog_data *data = (dialog_data*) E->data;
	//SDL_DestroyTexture( data->background );
	SDL_DestroyTexture( data->log_texture );
	//SDL_DestroyTexture( data->arrows ); we're assuming this is an extenal object, like a spritesheet, data->arrows is just a referrence to it
	destroy_chain( &(data->log) );
	vertical_scrollbar_destroy( &(data->scrollbar) );
	if( data->hyperlinks != NULL ){
		free( data->hyperlinks );
	}
	if( data->hyperlink_indices != NULL ){
		free( data->hyperlink_indices );
	}
	free( E->data );
	//puts("destroyed dialog.");
}






// --- --- --- --- --- --- --- --- --- --- --- --- /Dialog Element
// --- --- --- --- --- --- --- --- --- --- --- ---  text line input Element

/*
typedef struct {
	char **incumbency;
	int size, len, cursor, cursorX, offset;
	bool cursor_on, changes;
	int xmargin, ymargin;
	TX_Font *font;
	SDL_Color colors [3];

} text_line_input_data;*/


bool *UI_build_text_line_input( UI_Set *S, int c, int r, char *incumbency, int size, TX_Font *font, 
								SDL_Color lo, SDL_Color mo, SDL_Color hi ){
	int I = S->build_index;
	S->set[ I ].rect = calc_rect( S, c, r );
	text_line_input_data *data = malloc( sizeof(text_line_input_data) );
	float ym = (0.5 * (S->set[ I ].rect.h - (font->scale * (font->ascent - font->descent)))) - (font->scale * font->descent);
	*data = (text_line_input_data){ incumbency, 
									size, strlen(incumbency), 0, S->set[ I ].rect.x + 4, 0, 
									0, 0, 8, ym, 
									font, {lo, mo, hi} };
	S->set[ I ].data = (void*)data;
	S->set[ I ].IEHFs[mouse_motion] = empty_IEHF;
	S->set[ I ].IEHFs[ mouse_down ] = text_line_input_mouse_down;
	S->set[ I ].IEHFs[  mouse_up  ] = text_line_input_mouse_up;
	S->set[ I ].IEHFs[ mouse_wheel] = empty_IEHF;
	S->set[ I ].IEHFs[  key_down  ] = text_line_input_key_down;
	S->set[ I ].IEHFs[   key_up   ] = text_line_input_key_up;
	S->set[ I ].IEHFs[ text_input ] = text_line_input_text_input;
	S->set[ I ].IEHFs[text_editing] = text_line_input_text_editing;
	S->set[ I ].display_function = text_line_input_display;
	S->set[ I ].destroy_function = text_line_input_destroy;
	log_indices( S, c, r );
	S->build_index ++;
	return &(data->changes);
}

char text_line_input_mouse_down( UI_Interactive *E, SDL_Event *event, bool IN ){
	if( !IN ){
		text_line_input_data *data = (text_line_input_data*) E->data;
		if( data->cursor_on ){
			data->cursor_on = 0;
			SDL_StopTextInput();
		}
	}
	return IN;
}

char text_line_input_mouse_up( UI_Interactive *E, SDL_Event *event, bool IN ){
	text_line_input_data *data = (text_line_input_data*) E->data; //data->incumbency = ! data->incumbency;
	if( IN ){
		data->len = strlen( data->incumbency );

		float w = 0;
		for (int i = 0; i < data->len; ++i){
			int bytes = 0;
			w += TX_glyph_width( data->font, data->incumbency + i, &bytes );
			
			if( w > event->button.x - E->rect.x - data->xmargin ){
				data->cursor = i;
				data->cursor_on = 1;
				TX_SizeTextUntil( data->font, data->incumbency, data->cursor, &(data->cursorX), NULL );
				data->cursorX += E->rect.x + data->xmargin;
				break;
			}
			i += bytes-1;
		}
		if( !data->cursor_on ){
			data->len = strlen( data->incumbency );
			data->cursor = data->len;
			data->cursor_on = 1;
			TX_SizeText( data->font, data->incumbency, &w, NULL );
			data->cursorX = w + E->rect.x + data->xmargin;
		}
		if( ! SDL_IsTextInputActive() ){
			SDL_StartTextInput();
		}
	}
	return IN;
}

static void text_line_input_calc_cursor( UI_Interactive *E ){
	text_line_input_data *data = (text_line_input_data*) E->data;
	float w = 0;
	TX_SizeTextUntil( data->font, data->incumbency, data->cursor, &w, NULL );
	int L = E->rect.x + data->xmargin;
	int R = E->rect.x + E->rect.w - data->xmargin;

	data->cursorX = w + L + data->offset;

	if( data->cursorX < L ){
		data->offset = -w;
		data->cursorX = L; 
	}
	else if( data->cursorX > R ){
		data->offset = E->rect.w -(2*data->xmargin) -w;
		data->cursorX = R;
	}
	data->offset = constrain( data->offset, -1000000, 0 );
}

char text_line_input_key_down( UI_Interactive *E, SDL_Event *event, bool IN ){
	text_line_input_data *data = (text_line_input_data*) E->data;
	if( data->cursor_on ){
		data->len = strlen( data->incumbency );

		if( data->cursor < 0 || data->cursor > data->len || data->cursor >= data->size-1 ) data->cursor = data->len;
		int pcursor = data->cursor;

		//printf( "%c (%d)\n", event->key.keysym.sym, event->key.keysym.sym );

		if( event->key.keysym.sym == SDLK_LEFT ) data->cursor = constrain( data->cursor - 1, 0, data->len );
		else if( event->key.keysym.sym == SDLK_RIGHT ) data->cursor = constrain( data->cursor + 1, 0, data->len );
		else if( event->key.keysym.sym == SDLK_BACKSPACE && data->cursor > 0 ){
			str_delete_char( data->incumbency, data->cursor-1, data->len );
			data->len -= 1;
			data->cursor -= 1;
			data->changes = 1;
		}
		else if( event->key.keysym.sym == SDLK_DELETE && data->cursor < data->len ){
			str_delete_char( data->incumbency, data->cursor, data->len );
			data->len -= 1;
			data->changes = 1;
		}
		else if( event->key.keysym.sym == SDLK_HOME ) data->cursor = 0;
		else if( event->key.keysym.sym == SDLK_END ) data->cursor = data->len;
		else{
			const Uint8 *state = SDL_GetKeyboardState(NULL);
			if( state[SDL_SCANCODE_LCTRL] || state[SDL_SCANCODE_RCTRL] ){
				if( event->key.keysym.sym == 'c' ){
					SDL_SetClipboardText( data->incumbency );
				}
				else if( event->key.keysym.sym == 'v' ){
					char *cb =  SDL_GetClipboardText();
					int tl = strlen( cb );
					if( data->len + tl < data->size - 1 ){
						str_insert_str( data->incumbency, cb, data->cursor );
						data->cursor += tl;
						data->len += tl;
					}
					SDL_free( cb );
				}
			}
		}
		/* We handle this in text_input now
		else if( ascii_text( event->key.keysym.sym ) ){
			const Uint8 *state = SDL_GetKeyboardState(NULL);
			char key = event->key.keysym.sym;
			if( state[SDL_SCANCODE_LSHIFT] || state[SDL_SCANCODE_RSHIFT] ){
				key = shifted_keys( key );
			}
			if( data->len < data->size-1 ){
				//data->size *= 2;
				//data->incumbency = realloc( data->incumbency, data->size * sizeof(char) );
				//for (int i = data->len; i < data->size; ++i) data->incumbency[i] = 0;
				str_insert_char( data->incumbency, key, data->cursor, data->size );
				data->cursor += 1;
				data->len += 1;
				data->changes = 1;
			}			
		}*/

		if( pcursor != data->cursor ){
			text_line_input_calc_cursor( E );
		}
		//data->render_flag = 1;
		return 1;
	}
	return 0;
}

char text_line_input_key_up( UI_Interactive *E, SDL_Event *event, bool IN ){
	text_line_input_data *data = (text_line_input_data*) E->data;
	return data->cursor_on;
}

char text_line_input_text_input( UI_Interactive *E, SDL_Event *event, bool IN ){
	text_line_input_data *data = (text_line_input_data*) E->data;
	if( data->cursor_on ){
		if( data->len < data->size-1 ){
			//data->size *= 2;
			//data->incumbency = realloc( data->incumbency, data->size * sizeof(char) );
			//for (int i = data->len; i < data->size; ++i) data->incumbency[i] = 0;
			//str_insert_char( data->incumbency, key, data->cursor, data->size );
			str_insert_str( data->incumbency, event->text.text, data->cursor );
			data->cursor += 1;
			data->len += 1;
			data->changes = 1;

			text_line_input_calc_cursor( E );
		}
		return 1;
	}
	return 0;
}

char text_line_input_text_editing( UI_Interactive *E, SDL_Event *event, bool IN ){
	text_line_input_data *data = (text_line_input_data*) E->data;
	if( data->cursor_on ){
		//composition = event.edit.text;
	    data->cursor = event->edit.start;
	    //selection_len = event.edit.length;
		return 1;
	}
	return 0;
}


void text_line_input_display( SDL_Renderer *R, UI_Interactive *E ){
	text_line_input_data *data = (text_line_input_data*) E->data;

	//SDL_RenderCopy( R, data->background, NULL, &(E->rect) );
	SDL_SetRenderDraw_SDL_Color( R, data->colors + 0 );
	SDL_RenderFillRect( R, &(E->rect) );
	
	SDL_RenderSetClipRect( R, &(E->rect) );
	TX_render_string( R, data->font, data->incumbency, E->rect.x + data->xmargin + data->offset, E->rect.y + data->ymargin );
	SDL_RenderSetClipRect( R, NULL );

	SDL_SetRenderDraw_SDL_Color( R, data->colors + 2 );
	SDL_RenderDrawRect( R, &(E->rect) );

	if( data->cursor_on ){
		SDL_SetRenderDraw_SDL_Color( R, data->colors + 2 );
		int Y = E->rect.y + 2;
		int X = data->cursorX + 1;
		SDL_RenderDrawLine( R, X, Y, X, Y + E->rect.h - 4 ); X += 1;
		SDL_RenderDrawLine( R, X, Y, X, Y + E->rect.h - 4 );
		//SDL_SetRenderDrawColor( R, 255, 255, 255, 255 );
		//SDL_RenderDrawLine( R, data->cursorX, E->rect.y + data->ymargin, data->cursorX, E->rect.y + E->rect.h - data->ymargin );
	}
}

void text_line_input_destroy( UI_Interactive *E ){
	//text_line_input_data *data = (text_line_input_data*) E->data;
	//SDL_DestroyTexture( data->background );
	free( E->data );
}



// --- --- --- --- --- --- --- --- --- --- --- --- Key Select input Element
/*
typedef struct {
	SDL_Keycode *incumbency;
	bool mouse_over, awaiting_input;
	char preamble [32];
	TX_label label;
	SDL_Texture *textures[3];

} key_select_data;*/

static void sprintf_key_select_label( UI_Interactive *E ){
	key_select_data *data = (key_select_data*) E->data;
	if( *(data->incumbency) > 0 ){
		sprintf( data->label.content, "%s%s", data->preamble, SDL_GetKeyName( *(data->incumbency) ) );
	}
	else{
		sprintf( data->label.content, "%s...", data->preamble );
	}
	float w = 0;
	float h = 0;
	TX_SizeText( data->label.font, data->label.content, &w, &h );
	data->label.tx = E->rect.x + ((E->rect.w - w) / 2);
	data->label.ty = E->rect.y + ((E->rect.h - h) / 2);
}

bool *UI_build_key_select( UI_Set *S, int c, int r, SDL_Keycode *incumbency, char *preamble, TX_Font *font ){
	int I = S->build_index;
	S->set[ I ].rect = calc_rect( S, c, r );
	key_select_data *data = malloc( sizeof(key_select_data) );
	*data = (key_select_data){ incumbency, 0, 0, "", {font, NULL, 0, 0}, {NULL, NULL, NULL} };
	strcpy( data->preamble, preamble );
	data->label.content = malloc( 48 );
	S->set[ I ].data = (void*)data;
	sprintf_key_select_label( S->set + I );
	S->set[ I ].IEHFs[mouse_motion] = key_select_mouse_motion;
	S->set[ I ].IEHFs[ mouse_down ] = empty_IEHF;
	S->set[ I ].IEHFs[  mouse_up  ] = key_select_mouse_up;
	S->set[ I ].IEHFs[ mouse_wheel] = empty_IEHF;
	S->set[ I ].IEHFs[  key_down  ] = key_select_key_down;
	S->set[ I ].IEHFs[   key_up   ] = key_select_key_up;
	S->set[ I ].IEHFs[ text_input ] = empty_IEHF;
	S->set[ I ].IEHFs[text_editing] = empty_IEHF;
	S->set[ I ].display_function = key_select_display;
	S->set[ I ].destroy_function = key_select_destroy;
	log_indices( S, c, r );
	S->build_index ++;
}

char key_select_mouse_motion( UI_Interactive *E, SDL_Event *event, bool IN ){
	key_select_data *data = (key_select_data*) E->data;
	data->mouse_over = IN;
	return 0;
}

char key_select_mouse_up( UI_Interactive *E, SDL_Event *event, bool IN ){
	key_select_data *data = (key_select_data*) E->data;
	if( IN ){
		data->awaiting_input = 1;
	}
	return IN;
}

char key_select_key_down( UI_Interactive *E, SDL_Event *event, bool IN ){
	key_select_data *data = (key_select_data*) E->data;
	if( data->awaiting_input ) return 1;
	return 0;
}

char key_select_key_up( UI_Interactive *E, SDL_Event *event, bool IN ){
	key_select_data *data = (key_select_data*) E->data;
	if( data->awaiting_input ){
		*(data->incumbency) = event->key.keysym.sym;
		data->awaiting_input = 0;
		sprintf_key_select_label( E );
		return 1;
	}
	return 0;
}


void key_select_display( SDL_Renderer *R, UI_Interactive *E ){
	key_select_data *data = (key_select_data*) E->data;
	int Q = 0;
	if( data->awaiting_input ) Q = 2;
	else if( data->mouse_over ) Q = 1;
	SDL_RenderCopy( R, data->textures[Q], NULL, &(E->rect) );
	display_TX_label( R, &(data->label) );
}

void key_select_destroy( UI_Interactive *E ){
	key_select_data *data = (key_select_data*) E->data;
	free( data->label.content );
	free( E->data );
}


// --- --- --- --- --- --- --- --- --- --- --- ---  /text line input Element
// --- --- --- --- --- --- --- --- --- --- --- ---  number input Element

/*
typedef struct {
	void *incumbency;
	char str[16];
	char fmt[8];
	char_validity_check valid_char;
	int len, cursor, cursorX, offset;
	bool cursor_on, changes;
	int xmargin, ymargin;
	TX_Font *font;
	SDL_Color colors [3];

} number_input_data;
*/

void number_input_sprintf_incumbency( number_input_data *data ){
	setlocale(LC_ALL, "C");
	int flen = strlen(data->fmt);
	switch( data->fmt[flen-1] ){
		case 'i':
		case 'd':
		case 'x': case 'X':;
			snprintf( data->str, 16, data->fmt, *((int*)(data->incumbency)) );
			break;
		case 'g':
		case 'f':;
			if( data->fmt[flen-2] == 'l' ){
				snprintf( data->str, 16, data->fmt, *((double*)(data->incumbency)) );//printf( "<%lf>\n", *dfv );
			}
			else{
				snprintf( data->str, 16, data->fmt, *((float*)(data->incumbency)) );
			}
			break;
	}
	data->len = strlen( data->str );
}
void number_input_scanf_incumbency( number_input_data *data ){
	int flen = strlen(data->fmt);
	switch( data->fmt[flen-1] ){
		case 'i':
		case 'd':
		case 'x': case 'X':;
			int iv = 0;
			sscanf( data->str, data->fmt, &iv );
			*((int*)(data->incumbency)) = iv;
			break;
		case 'g':
		case 'f':;
			if( data->fmt[flen-2] == 'l' ){
				double dfv = 0;
				sscanf( data->str, data->fmt, &dfv );
				*((double*)(data->incumbency)) = dfv;
			}
			else{
				float fv = 0;
				sscanf( data->str, data->fmt, &fv );
				*((float*)(data->incumbency)) = fv;
			}
			break;
	}
}


bool *UI_build_number_input( UI_Set *S, int c, int r, void *incumbency, char fmt[8], TX_Font *font, 
							 SDL_Color lo, SDL_Color mo, SDL_Color hi ){
	int I = S->build_index;
	S->set[ I ].rect = calc_rect( S, c, r );
	number_input_data *data = malloc( sizeof(number_input_data) );
	*data = (number_input_data){ incumbency, "", "", NULL,
									0, 0, S->set[ I ].rect.x + 4, 0, 
									0, 0,
									8, 3, 
									font, {lo, mo, hi} };
	//printf("%p, %s\n", incumbency, fmt );
	strcpy( data->fmt, fmt );

	if( incumbency == NULL ){
		data->str[0] = '\0';
		data->len = 0;
	}
	else{
		number_input_sprintf_incumbency(data);
		//printf("str: \"%s\" (%d)\n", data->str, data->len );
	}
	int flen = strlen(data->fmt);
	switch( fmt[flen-1] ){
		case 'i':;
			data->valid_char = i_check;
			break;
		case 'd':;
			data->valid_char = d_check;
			break;
		case 'x': case 'X':;
			data->valid_char = x_check;
			break;
		case 'g':
		case 'f':;
			data->valid_char = f_check;
			break;
	}
	
	S->set[ I ].data = (void*)data;
	S->set[ I ].IEHFs[mouse_motion] = empty_IEHF;
	S->set[ I ].IEHFs[ mouse_down ] = empty_IEHF;
	S->set[ I ].IEHFs[  mouse_up  ] = number_input_mouse_up;
	S->set[ I ].IEHFs[ mouse_wheel] = empty_IEHF;
	S->set[ I ].IEHFs[  key_down  ] = number_input_key_down;
	S->set[ I ].IEHFs[   key_up   ] = number_input_key_up;
	S->set[ I ].IEHFs[ text_input ] = number_input_text_input;
	S->set[ I ].IEHFs[text_editing] = number_input_text_editing;
	S->set[ I ].display_function = number_input_display;
	S->set[ I ].destroy_function = number_input_destroy;
	log_indices( S, c, r );
	S->build_index ++;
	return &(data->changes);
}

void number_input_set_incumbency( UI_Interactive *E, void *incumbency ){
	number_input_data *data = (number_input_data*) E->data;
	data->incumbency = incumbency;
	number_input_sprintf_incumbency(data);
	//printf("we set a new incumbency (%p)\n", incumbency );
}

void number_input_update( UI_Interactive *E ){
	number_input_sprintf_incumbency( (number_input_data*) E->data );
}

char number_input_mouse_up( UI_Interactive *E, SDL_Event *event, bool IN ){
	number_input_data *data = (number_input_data*) E->data;
	if( IN ){
		float w = 0;
		for (int i = 0; i < 16; ++i){

			int bytes = 0;
			w += TX_glyph_width( data->font, data->str + i, &bytes );

			if( data->str[i] == '\0' || w > event->button.x - E->rect.x - data->xmargin ){
				data->cursor = i;
				data->cursor_on = 1;
				TX_SizeTextUntil( data->font, data->str, data->cursor, &(data->cursorX), NULL );
				data->cursorX += E->rect.x + data->xmargin;
				break;
			}

			i += bytes-1;
		}

		//refresh the str while we're here
		number_input_sprintf_incumbency(data);
	}
	else data->cursor_on = 0;
	return IN;
}

void number_input_calc_cursor( UI_Interactive *E ){
	number_input_data *data = (number_input_data*) E->data;
	float w = 0, h = 0;
	TX_SizeTextUntil( data->font, data->str, data->cursor, &w, &h );
	int L = E->rect.x + data->xmargin;
	int R = E->rect.x + E->rect.w - data->xmargin;

	data->cursorX = w + L + data->offset;

	if( data->cursorX < L ){
		data->offset = -w;
		data->cursorX = L; 
	}
	else if( data->cursorX > R ){
		data->offset = E->rect.w -(2*data->xmargin) -w;
		data->cursorX = R;
	}
	data->offset = constrain( data->offset, -1000000, 0 );
}

char number_input_key_down( UI_Interactive *E, SDL_Event *event, bool IN ){
	number_input_data *data = (number_input_data*) E->data;
	if( data->cursor_on ){
		data->len = strlen( data->str );

		data->changes = 0;
		if( data->cursor < 0 || data->cursor > data->len || data->cursor >= 15 ) data->cursor = data->len;
		int pcursor = data->cursor;

		//printf( "%c (%d)\n", event->key.keysym.sym, event->key.keysym.sym );

		if( event->key.keysym.sym == SDLK_LEFT ) data->cursor = constrain( data->cursor - 1, 0, data->len );
		else if( event->key.keysym.sym == SDLK_RIGHT ) data->cursor = constrain( data->cursor + 1, 0, data->len );
		else if( event->key.keysym.sym == SDLK_BACKSPACE && data->cursor > 0 ){
			str_delete_char( data->str, data->cursor-1, data->len );
			data->len -= 1;
			data->cursor -= 1;
			data->changes = 1;
		}
		else if( event->key.keysym.sym == SDLK_DELETE && data->cursor < data->len ){
			str_delete_char( data->str, data->cursor, data->len );
			data->len -= 1;
			data->changes = 1;
		}
		else if( event->key.keysym.sym == SDLK_HOME ) data->cursor = 0;
		else if( event->key.keysym.sym == SDLK_END ) data->cursor = data->len;

		if( data->changes ){
			number_input_scanf_incumbency(data);
		}

		/*
		else if( data->valid_char( event->key.keysym.sym ) ){
			const Uint8 *state = SDL_GetKeyboardState(NULL);
			char key = event->key.keysym.sym;
			if( state[SDL_SCANCODE_LSHIFT] || state[SDL_SCANCODE_RSHIFT] ){
				key = shifted_keys( key );
			}
			if( data->len <= 15 ){
				//data->size *= 2;
				//data->str = realloc( data->str, data->size * sizeof(char) );
				//for (int i = data->len; i < data->size; ++i) data->str[i] = 0;
				str_insert_char( data->str, key, data->cursor, 16 );
				data->cursor += 1;
				data->len += 1;
				data->changes = 1;
			}
		}*/

		//printf( data->fmt, data->incumbency );puts(",");

		//*(data->incumbency) = atof( data->str );
		//sscanf( data->str, data->fmt, data->incumbency );

		//printf( "len:%d, cursor:%d\n", data->len, data->cursor );

		if( pcursor != data->cursor ){
			number_input_calc_cursor( E );
		}
		//data->render_flag = 1;
		return 1;
	}
	return 0;
}

char number_input_key_up( UI_Interactive *E, SDL_Event *event, bool IN ){
	number_input_data *data = (number_input_data*) E->data;
	return data->cursor_on;
}

char number_input_text_input( UI_Interactive *E, SDL_Event *event, bool IN ){
	number_input_data *data = (number_input_data*) E->data;
	if( data->cursor_on ){
		//printf("we're text inputting!!, {%s}, %p\n", event->text.text, data->incumbency );
		int tl = strlen( event->text.text );
		bool valid = 1;
		for (int i = 0; i < tl; ++i ){
			if( !(data->valid_char( event->text.text[i] )) ){
				valid = 0;
				break;
			}
		}
		if( valid && data->len < 15 ){
			str_insert_str( data->str, event->text.text, data->cursor );
			data->cursor += tl;
			data->len += tl;
			number_input_calc_cursor( E );
			number_input_scanf_incumbency(data);
		}
		return 1;
	}
	return 0;
}

char number_input_text_editing( UI_Interactive *E, SDL_Event *event, bool IN ){
	number_input_data *data = (number_input_data*) E->data;
	if( data->cursor_on ){
		//composition = event.edit.text;
	    //data->cursor = event->edit.start;
	    //selection_len = event.edit.length;
	    //printf("editing.... {%s}\n", event->edit.text );
		return 1;
	}
	return 0;
}

void number_input_display( SDL_Renderer *R, UI_Interactive *E ){
	number_input_data *data = (number_input_data*) E->data;

	//SDL_RenderCopy( R, data->background, NULL, &(E->rect) );
	SDL_SetRenderDraw_SDL_Color( R, data->colors + 0 );
	SDL_RenderFillRect( R, &(E->rect) );
	SDL_SetRenderDraw_SDL_Color( R, data->colors + 2 );
	SDL_RenderDrawRect( R, &(E->rect) );
	
	SDL_RenderSetClipRect( R, &(E->rect) );
	TX_render_string( R, data->font, data->str, E->rect.x + data->xmargin + data->offset, E->rect.y + data->ymargin );
	SDL_RenderSetClipRect( R, NULL );

	if( data->cursor_on ){
		SDL_SetRenderDraw_SDL_Color( R, data->colors + 2 );
		int Y = E->rect.y + data->ymargin;
		int X = data->cursorX + 1;
		SDL_RenderDrawLine( R, X, Y, X, Y + (data->font->h * data->font->scale) ); X += 1;
		SDL_RenderDrawLine( R, X, Y, X, Y + (data->font->h * data->font->scale) );
		//SDL_SetRenderDrawColor( R, 255, 255, 255, 255 );
		//SDL_RenderDrawLine( R, data->cursorX, E->rect.y + data->ymargin, data->cursorX, E->rect.y + E->rect.h - data->ymargin );
	}
}

void number_input_destroy( UI_Interactive *E ){
	//number_input_data *data = (number_input_data*) E->data;
	//SDL_DestroyTexture( data->background );
	free( E->data );
}



// --- --- --- --- --- --- --- --- --- --- --- ---  /number input Element
// --- --- --- --- --- --- --- --- --- --- --- --- text box input Element
/*
typedef struct {
	char **incumbency;
	int size, len;
	int cursor, cursorX, cursorY;
	bool cursor_on, render_flag;
	int xmargin, ymargin;
	TX_Font *font;
	float font_scale;
	//SDL_Texture *background;
	SDL_Color *bg;// background
	SDL_Color *sb;// scrollbar
	SDL_Color *fg;// foreground
	SDL_Texture *texture;
	SDL_Rect dst;

	int scroll, scroll_width, current_height, scrolling_value;
	UI_Interactive scrollbar;

} textbox_data;*/

void UI_build_textbox( UI_Set *S, int c, int r, TX_Font *font, float font_scale, int sbw, 
					   SDL_Renderer *R, SDL_Color *bg, SDL_Color *fg ){
	int I = S->build_index;
	S->set[ I ].rect = calc_rect( S, c, r );
	textbox_data *data = malloc( sizeof(textbox_data) );
	*data = (textbox_data){ NULL, 0, 0,
							0, 0, 0, 
							0, 0, 
							8, 8, 
							font, font_scale,
							bg, fg,
							NULL,
							(SDL_Rect){ S->set[ I ].rect.x,      S->set[ I ].rect.y, 
									    S->set[ I ].rect.w -sbw, S->set[ I ].rect.h },
							0, sbw, 0, 0,
							new_vertical_scrollbar( S->set[ I ].rect, sbw, &(data->scroll) ) };
	data->texture = SDL_CreateTexture( R, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, data->dst.w, data->dst.h );
	SDL_SetTextureBlendMode( data->texture, SDL_BLENDMODE_BLEND );
	
	S->set[ I ].data = (void*)data;
	S->set[ I ].IEHFs[mouse_motion] = textbox_mouse_motion;
	S->set[ I ].IEHFs[ mouse_down ] = textbox_mouse_down;
	S->set[ I ].IEHFs[  mouse_up  ] = textbox_mouse_up;
	S->set[ I ].IEHFs[ mouse_wheel] = textbox_mouse_wheel;
	S->set[ I ].IEHFs[  key_down  ] = textbox_key_down;
	S->set[ I ].IEHFs[   key_up   ] = textbox_key_up;
	S->set[ I ].IEHFs[ text_input ] = textbox_text_input;
	S->set[ I ].IEHFs[text_editing] = textbox_text_editing;
	S->set[ I ].display_function = textbox_display;
	S->set[ I ].destroy_function = textbox_destroy;
	log_indices( S, c, r );
	S->build_index ++;
}

void textbox_set_string( UI_Interactive *E, char *str ){

	textbox_data *data = (textbox_data*) E->data;
	data->len = strlen( str );
	
	//if( *(data->incumbency) != NULL ) free( *(data->incumbency) );
	if( data->len > 0 ){
		data->size = lrint( 1.5 * data->len );
		*(data->incumbency) = realloc( *(data->incumbency), data->size * sizeof(char) );
		strcpy( *(data->incumbency), str );
	}
	else{
		data->size = 8;
		*(data->incumbency) = realloc( *(data->incumbency), data->size * sizeof(char) );
		memset( *(data->incumbency), 0, 8 );
	}
	data->scrolling_value = -(data->scroll);
	data->render_flag = 1;
}

void textbox_set_incumbency( UI_Interactive *E, char **incumbency, int size ){

	textbox_data *data = (textbox_data*) E->data;

	data->incumbency = incumbency;
	if( size <= 0 ){
		data->size = 8;
		*(data->incumbency) = realloc( *(data->incumbency), 8 * sizeof(char) );
		(*(data->incumbency))[0] = '\0';
		data->len = 0;
	}
	else{
		data->len = strlen( *incumbency );
		data->size = size;
	}
	
	data->scrolling_value = -(data->scroll);
	data->render_flag = 1;
	//printf("incumbency set!! size:%d, len:%d\n", data->size, data->len );
}


char textbox_mouse_motion( UI_Interactive *E, SDL_Event *event, bool IN ){
	
	//if( IN ){
	textbox_data *data = (textbox_data*) E->data;
	if( vertical_scrollbar_mouse_motion( &(data->scrollbar), event, (event->motion.x > E->rect.x + E->rect.w - data->scroll_width) ) ){
		return 1;
	}
	return 0;
}

char textbox_mouse_down( UI_Interactive *E, SDL_Event *event, bool IN ){
	textbox_data *data = (textbox_data*) E->data;
	//puts( "textbox_mouse_down" );
	if( IN ){
		vertical_scrollbar_mouse_down( &(data->scrollbar), event, (event->button.x > E->rect.x + E->rect.w - data->scroll_width) );
		return 1;
	}
	else if( data->cursor_on ){
		data->cursor_on = 0;
		SDL_StopTextInput();
	}
	return 0;
}

char textbox_mouse_up( UI_Interactive *E, SDL_Event *event, bool IN ){
	textbox_data *data = (textbox_data*) E->data;
	if( vertical_scrollbar_mouse_up( &(data->scrollbar), event, IN && (event->button.x > E->rect.x + E->rect.w - data->scroll_width) ) ){
		return 1;
	}
	if( IN ){
		int lines;
		int *wis = TX_wrapping_indices( data->font, *(data->incumbency), data->dst.w -2*data->xmargin, &lines );
		
		int mx = event->button.x - E->rect.x - data->xmargin;
		int line_height = data->font->line_skip * data->font->scale;
		int ml = (int)( (event->button.y - E->rect.y - data->ymargin - data->scroll) / ((float)line_height) );

		if( ml >= lines ){//clicked in empty space below the text
			ml = lines-1;
			mx += data->dst.w;//making sure the cursor goes to the end of the line
		}

		data->cursorY = E->rect.y + data->ymargin + ( ml * line_height );
		int pcon = data->cursor_on;
		data->cursor_on = 0;
		int space = data->font->space * data->font->scale;
		int end = ( ml == lines-1 )? data->len : wis[ ml+1 ];
		int w = 0;
		for (int i = wis[ml]; i < end; ){
			int dw = 0;
			int bytes = 1;
			if( (*(data->incumbency))[i] == '\n' ){
				data->cursor = i;
				data->cursor_on = 1;
				break;
			}
			else if( (*(data->incumbency))[i] == '\t' ) w += 4 * space;
			else if( (*(data->incumbency))[i] < '!' ){
				dw += space;
			}
			else dw += TX_glyph_width( data->font, (*(data->incumbency)) + i, &bytes );
			//data->font->adv[ (*(data->incumbency))[i]-'!' ] * data->font->scale;

			if( w+dw > mx ){
				data->cursor = i;
				data->cursor_on = 1;
				break;
			}
			w += dw;
			i += bytes;
		}
		if( !(data->cursor_on) ){
			data->cursor = end;
			data->cursor_on = 1;
		}

		if( !pcon || !SDL_IsTextInputActive() ){
			SDL_StartTextInput();
		}

		data->cursorX = w + E->rect.x + data->xmargin;
		
		free( wis );
	}
	else if( data->cursor_on ){
		data->cursor_on = 0;
		SDL_StopTextInput();
	}
	return IN;
}

char textbox_mouse_wheel( UI_Interactive *E, SDL_Event *event, bool IN ){
	//puts("textbox_mouse_wheel");
	if( IN ){
		((textbox_data*) E->data)->scrolling_value = event->wheel.y * 24;	
	}
	return IN;
}

static void textbox_calc_cursor( UI_Interactive *E ){
	textbox_data *data = (textbox_data*) E->data;
	int lines;
	int *wis = TX_wrapping_indices( data->font, *(data->incumbency), data->dst.w -2*data->xmargin, &lines );
	//for (int i = 0; i < lines; ++i) printf( "%d, ", wis[i] );

	int line_height = data->font->line_skip * data->font->scale;
	for (int i = 0; i < lines; ++i){
		if( data->cursor >= wis[i] && ( i == lines-1 || data->cursor < wis[i+1] ) ){
			int w = 0;
			for ( int j = wis[i]; j < data->cursor; ){
				int bytes = 1;
				w += TX_glyph_width( data->font, (*(data->incumbency)) + j, &bytes );
				j += bytes;
			}
			//printf("\n%d/%d, %c(%d), %d\n", i, lines, (*(data->incumbency))[wis[i]], wis[i], data->cursor );
			data->cursorX = E->rect.x + data->xmargin + w;
			data->cursorY = E->rect.y + data->ymargin + ( i * line_height );
			break;
		}
	}
	free( wis );

	int Y = data->cursorY + data->scroll;

	if( Y < E->rect.y + data->ymargin ){
		data->scrolling_value = (E->rect.y + data->ymargin) - Y;
	}

	if( Y + line_height > E->rect.y + E->rect.h + data->ymargin ){
		data->scrolling_value = (E->rect.y + E->rect.h) -(Y + line_height);
	}
}

char textbox_key_down( UI_Interactive *E, SDL_Event *event, bool IN ){
	textbox_data *data = (textbox_data*) E->data;
	if( data->cursor_on ){
		data->len = strlen(*(data->incumbency));

		if( data->cursor < 0 || data->cursor > data->len || data->cursor >= data->size-1 ) data->cursor = data->len;
		int pcursor = data->cursor;

		int prev_bytes = 0;
		if( data->cursor > 0 ) prev_bytes = retrobytes_in_a_utf_codepoint( *(data->incumbency) + data->cursor -1 );
		int curr_bytes = 0;
		if( data->cursor < data->len ) curr_bytes = bytes_in_a_utf_codepoint( *(data->incumbency) + data->cursor    );

		//printf( "%c (%d)\n", event->key.keysym.sym, event->key.keysym.sym );
		char key = 0;

			 if( event->key.keysym.sym == SDLK_LEFT  ) data->cursor = constrain( data->cursor - prev_bytes, 0, data->len );
		else if( event->key.keysym.sym == SDLK_RIGHT ) data->cursor = constrain( data->cursor + curr_bytes, 0, data->len );
		else if( event->key.keysym.sym == SDLK_UP   || event->key.keysym.sym == SDLK_DOWN ||
				 event->key.keysym.sym == SDLK_HOME || event->key.keysym.sym == SDLK_END  ){
			
			int lines;
			int *wis = TX_wrapping_indices( data->font, *(data->incumbency), data->dst.w -2*data->xmargin, &lines );

			for (int i = 0; i < lines; ++i){
				if( data->cursor >= wis[i] && ( i == lines-1 || data->cursor < wis[i+1] ) ){
					int cursorJ = i;
						 if( event->key.keysym.sym == SDLK_UP   ) cursorJ -= 1;
					else if( event->key.keysym.sym == SDLK_DOWN ) cursorJ += 1;
					else if( event->key.keysym.sym == SDLK_HOME ){
						data->cursor = wis[i];
						break;
					}
					else if( event->key.keysym.sym == SDLK_END ){
						data->cursor = (i == lines-1)? data->len : wis[ i+1 ]-1;
						break;
					}

						 if ( cursorJ < 0 )      data->cursor = 0;
					else if ( cursorJ >= lines ) data->cursor = data->len;
					else{
						int space = data->font->space * data->font->scale;
						//int line_height = data->font->line_skip * data->font->scale;
						int end = (cursorJ == lines-1)? data->len : wis[ cursorJ+1 ];
						int w = 0;
						for ( int j = wis[ cursorJ ]; j < end; ){
							int dx = 0;
							int bytes = 1;
							if( (*(data->incumbency))[j] == '\t' ) dx += 4 * space;
							else if( (*(data->incumbency))[j] < '!' ){
								dx += space;
							}
							else dx += TX_glyph_width( data->font, (*(data->incumbency)) + j, &bytes );
							//data->font->adv[ (*(data->incumbency))[j] -'!' ] * data->font->scale;

							int nc = E->rect.x + data->xmargin + w + dx;
							if( nc > data->cursorX ){
								if( abs( data->cursorX - nc ) < abs( data->cursorX - (E->rect.x + data->xmargin + w) ) ){
									 data->cursor = j+1;
								}
								else data->cursor = j;
								break;
							}
							w += dx;
							j += bytes;
						}
						if( pcursor == data->cursor ){
							data->cursor = end-1;
							if( cursorJ == lines-1 ) data->cursor += 1;
						}
					}
					break;
				}
			}
		}
		else if( event->key.keysym.sym == SDLK_BACKSPACE && data->cursor > 0 ){
			for (int c = 0; c < prev_bytes; ++c ){
				str_delete_char( *(data->incumbency), data->cursor-1, data->len );
				data->len -= 1;
				data->cursor -= 1;
			}
		}
		else if( event->key.keysym.sym == SDLK_DELETE && data->cursor < data->len-1 ){
			for (int c = 0; c < curr_bytes; ++c ){
				str_delete_char( *(data->incumbency), data->cursor, data->len );
				data->len -= 1;
			}
		}
		else if( event->key.keysym.sym == SDLK_TAB    ) key = '\t';
		else if( event->key.keysym.sym == SDLK_RETURN ) key = '\n';
		else{
			const Uint8 *state = SDL_GetKeyboardState(NULL);
			if( state[SDL_SCANCODE_LCTRL] || state[SDL_SCANCODE_RCTRL] ){
				if( event->key.keysym.sym == 'c' ){
					SDL_SetClipboardText( *(data->incumbency) );
				}
				else if( event->key.keysym.sym == 'v' ){
					char *cb =  SDL_GetClipboardText();
					int tl = strlen( cb );
					while( data->len + tl >= data->size - 1 ){
						data->size *= 2;
						if( data->size == 0 ) data->size = 8;
						*(data->incumbency) = realloc( *(data->incumbency), data->size * sizeof(char) );
						for (int i = data->len; i < data->size; ++i) (*(data->incumbency))[i] = 0;
					}
					str_insert_str( *(data->incumbency), cb, data->cursor );
					data->cursor += tl;
					data->len += tl;
					//for (int i = 0; i < len; ++i ){
					//	str_insert_char( data->incumbency, cb[i], data->cursor, &(data->len), &(data->size) );
					//	data->cursor += 1;
					//}
					SDL_free( cb );
				}
			}
		}
		/* This is now handled in text_input
		else if( ascii_text( event->key.keysym.sym ) ){
			key = event->key.keysym.sym;
			const Uint8 *state = SDL_GetKeyboardState(NULL);
			if( state[SDL_SCANCODE_LSHIFT] || state[SDL_SCANCODE_RSHIFT] ){
				key = shifted_keys( key );
			}
		}
		//puts("handled input...");*/
		if( key ){
			if( data->len >= data->size-1 ){
				data->size *= 2;
				if( data->size == 0 ) data->size = 8;
				*(data->incumbency) = realloc( *(data->incumbency), data->size * sizeof(char) );
				for (int i = data->len; i < data->size; ++i) (*(data->incumbency))[i] = 0;
			}
			str_insert_char( *(data->incumbency), key, data->cursor, data->size );
			data->cursor += 1;
			data->len += 1;
		}

		if( pcursor != data->cursor ){
			textbox_calc_cursor( E );
		}

		data->render_flag = 1;
		return 1;
	}
	return 0;
}

char textbox_key_up( UI_Interactive *E, SDL_Event *event, bool IN ){
	textbox_data *data = (textbox_data*) E->data;
	if( data->cursor_on ){
		return 1;// this "captures" the key_up event, preventing it from being interpreted wrongly
	}
	return 0;
}

char textbox_text_input( UI_Interactive *E, SDL_Event *event, bool IN ){
	textbox_data *data = (textbox_data*) E->data;
	if( data->cursor_on ){
		int tl = strlen( event->text.text );
		while( data->len + tl >= data->size - 1 ){
			data->size *= 2;
			if( data->size == 0 ) data->size = 8;
			*(data->incumbency) = realloc( *(data->incumbency), data->size * sizeof(char) );
			for (int i = data->len; i < data->size; ++i) (*(data->incumbency))[i] = 0;
		}
		str_insert_str( *(data->incumbency), event->text.text, data->cursor );
		data->cursor += tl;
		data->len += tl;
		textbox_calc_cursor( E );
		data->render_flag = 1;
		return 1;
	}
	return 0;
}

char textbox_text_editing( UI_Interactive *E, SDL_Event *event, bool IN ){
	textbox_data *data = (textbox_data*) E->data;
	if( data->cursor_on ){
		//composition = event.edit.text;
	    //data->cursor = event->edit.start;
	    //selection_len = event.edit.length;
	    printf("editing.... {%s}\n", event->edit.text );
		return 1;
	}
	return 0;
}

void textbox_display( SDL_Renderer *R, UI_Interactive *E ){
	textbox_data *data = (textbox_data*) E->data;

	data->font->scale = data->font_scale;

	if( data->render_flag ){
		int nh = TX_wrapped_string_height( data->font, *(data->incumbency), data->dst.w -2*data->xmargin );
		if( nh > data->dst.h ) nh += data->ymargin;
		vertical_scrollbar_update( R, &(data->scrollbar), nh );
	}

	vertical_scrollbar_data *vsbd = (vertical_scrollbar_data *)(data->scrollbar.data);
	if( data->scrolling_value != 0 || vsbd->dragging || vsbd->holding ){
		vertical_scrollbar_external_scroll( &(data->scrollbar), data->scrolling_value );
		//printf("%d\n", data->scroll );
		data->scrolling_value = 0;
		data->render_flag = 1;
	}

	if( data->render_flag ){
		SDL_SetRenderTarget( R, data->texture );
		SDL_SetRenderDraw_SDL_Color( R, data->bg );
		SDL_RenderClear( R );
		TX_render_string_wrapped( R, data->font, *(data->incumbency), data->xmargin, 
								  data->ymargin + data->scroll, data->dst.w -2*data->xmargin );
		SDL_SetRenderTarget( R, NULL );
		data->render_flag = 0;
	}

	//SDL_RenderCopy( R, data->background, NULL, &(E->rect) );
	SDL_SetRenderDraw_SDL_Color( R, data->bg );
	SDL_RenderFillRect( R, &(E->rect) );
	SDL_RenderCopy( R, data->texture, NULL, &(data->dst) );

	
	if( data->cursor_on ){
		SDL_SetRenderDraw_SDL_Color( R, data->fg );
		int Y = data->cursorY + data->scroll;
		int X = data->cursorX + 1;
		SDL_RenderSetClipRect( R, &(E->rect) );
		SDL_RenderDrawLine( R, X, Y, X, Y + (data->font->h * data->font->scale) ); X += 1;
		SDL_RenderDrawLine( R, X, Y, X, Y + (data->font->h * data->font->scale) );
		SDL_RenderSetClipRect( R, NULL );
	}

	//printf("%p, %d, %d, %d, %d\n", vsbd->bar_texture, vsbd->bar_rect.x, vsbd->bar_rect.y, vsbd->bar_rect.w, vsbd->bar_rect.h );
	SDL_SetRenderDraw_SDL_Color( R, data->fg );
	vertical_scrollbar_display( R, &(data->scrollbar) );

	//outline
	//SDL_SetRenderDraw_SDL_Color( R, data->fg );
	//SDL_RenderDrawRect( R, &(E->rect) );
}

void textbox_destroy( UI_Interactive *E ){
	textbox_data *data = (textbox_data*) E->data;
	//SDL_DestroyTexture( data->background );
	SDL_DestroyTexture( data->texture );
	vertical_scrollbar_destroy( &(data->scrollbar) );
	free( E->data );
}







// --- --- --- --- --- --- --- --- --- --- --- --- /text box input Element
// --- --- --- --- --- --- --- --- --- --- --- --- 	Vertical Scrollbar







/*
typedef struct {
	float *incumbency;
	bool dragging, holding;
	int mouse_offset;
	int max_scroll;
	SDL_Rect bar_rect;
	SDL_Texture *bar_template;
	SDL_Rect template_rect;
	SDL_Texture *bar_texture;
} vertical_scrollbar_data;*/


UI_Interactive new_vertical_scrollbar( SDL_Rect R, int sbw, float *scroll ){
	UI_Interactive O = (UI_Interactive){ (SDL_Rect){ R.x + R.w -sbw, R.y, sbw, R.h }, 
										 malloc( sizeof(vertical_scrollbar_data) ), 
										 { vertical_scrollbar_mouse_motion, 
										   vertical_scrollbar_mouse_down, 
										   vertical_scrollbar_mouse_up, 
										   empty_IEHF, empty_IEHF, empty_IEHF },
										 vertical_scrollbar_display, vertical_scrollbar_destroy };
	vertical_scrollbar_data *data = (vertical_scrollbar_data*) O.data;
	data->incumbency = scroll;
	data->bar_rect = (SDL_Rect){R.x + R.w -sbw, R.y, sbw, R.h/2};
	data->dragging = 0;
	data->holding = 0;
	return O;
}


void UI_build_vertical_scrollbar( UI_Interactive *E, float *incumbency ){

	vertical_scrollbar_data *data = malloc( sizeof(vertical_scrollbar_data) );
	*data = (vertical_scrollbar_data){ incumbency, 0, 0, 0, 0, (SDL_Rect){E->rect.x, E->rect.y, E->rect.w, E->rect.h/2}, NULL, (SDL_Rect){0,0,0,0}, NULL };
	E->data = (void*)data;
	E->IEHFs[mouse_motion] = vertical_scrollbar_mouse_motion;
	E->IEHFs[ mouse_down ] = vertical_scrollbar_mouse_down;
	E->IEHFs[  mouse_up  ] = vertical_scrollbar_mouse_up;
	E->IEHFs[ mouse_wheel] = empty_IEHF;
	E->IEHFs[  key_down  ] = empty_IEHF;
	E->IEHFs[   key_up   ] = empty_IEHF;
	E->display_function = vertical_scrollbar_display;
	E->destroy_function = vertical_scrollbar_destroy;
}

void vertical_scrollbar_update( SDL_Renderer *R, UI_Interactive *E, int total_content_height ){

	vertical_scrollbar_data *data = (vertical_scrollbar_data*) E->data;

	int minH = data->bar_rect.w;
	data->bar_rect.h = constrain( lrint( E->rect.h * ( E->rect.h / (float)total_content_height ) ), minH, E->rect.h );
	//*(data->incumbency) = 0;
	//data->bar_rect.y = E->rect.y;
	//printf("%d :> %d\n", E->rect.h, data->bar_rect.h );
	if( total_content_height < E->rect.h ) data->max_scroll = 0;
	else data->max_scroll = E->rect.h - total_content_height;
	int theoretical_scroll = constrain( *(data->incumbency), data->max_scroll, 0 );
	int bottom = E->rect.y + E->rect.h - data->bar_rect.h;
	data->bar_rect.y = map( theoretical_scroll, data->max_scroll, 0, bottom, E->rect.y );
}

void vertical_scrollbar_external_scroll( UI_Interactive *E, float value ){
	vertical_scrollbar_data *data = (vertical_scrollbar_data*) E->data;

	if( data->max_scroll < 0 ){
		*(data->incumbency) = constrainF( *(data->incumbency) + value, data->max_scroll, 0 );
		int bottom = E->rect.y + E->rect.h - data->bar_rect.h;
		data->bar_rect.y = map( *(data->incumbency), data->max_scroll, 0, bottom, E->rect.y );
	}
	else{
		*(data->incumbency) = 0;
		data->bar_rect.y = E->rect.y;
	}
}

char vertical_scrollbar_mouse_motion( UI_Interactive *E, SDL_Event *event, bool IN ){

	vertical_scrollbar_data *data = (vertical_scrollbar_data*) E->data;
	if( data->dragging && data->max_scroll < 0 ){
		int bottom = E->rect.y + E->rect.h - data->bar_rect.h;
		data->bar_rect.y = constrain( event->motion.y - data->mouse_offset, E->rect.y, bottom );		*(data->incumbency) = lrint( map( data->bar_rect.y, E->rect.y, bottom, 0, data->max_scroll ) );
		return 1;
	}
	else if( data->holding ){
		data->mouse_offset = event->motion.y - E->rect.y;
		return 1;
	}
	return 0;
}

char vertical_scrollbar_mouse_down( UI_Interactive *E, SDL_Event *event, bool IN  ){

	if( IN ){
		vertical_scrollbar_data *data = (vertical_scrollbar_data*) E->data;
		if( event->button.y > data->bar_rect.y && event->button.y < data->bar_rect.y + data->bar_rect.h ){
			data->mouse_offset = event->button.y - data->bar_rect.y;
			data->dragging = 1;
		}
		else{
			data->mouse_offset = event->button.y;
			data->holding = 1;
		}
		return 1;
	}
	return 0;
}

char vertical_scrollbar_mouse_up( UI_Interactive *E, SDL_Event *event, bool IN ){

	vertical_scrollbar_data *data = (vertical_scrollbar_data*) E->data;
	data->dragging = 0;
	data->holding = 0;
	return IN;
}

void vertical_scrollbar_display( SDL_Renderer *R, UI_Interactive *E ){

	vertical_scrollbar_data *data = (vertical_scrollbar_data*) E->data;
	if( data->holding && data->max_scroll < 0 ){
		int bottom = E->rect.y + E->rect.h - data->bar_rect.h;
		data->bar_rect.y = constrain( lerp( data->bar_rect.y, data->mouse_offset - (data->bar_rect.h * 0.5), 0.125 ), E->rect.y, bottom );
		*(data->incumbency) = lrint( map( data->bar_rect.y, E->rect.y, bottom, 0, data->max_scroll ) );
		if( abs( data->mouse_offset - data->bar_rect.y ) < 4 ){
			data->mouse_offset = lrint( data->bar_rect.h * 0.5 );
			data->dragging = 1;
			data->holding = 0;
		}
	}

	//SDL_SetRenderDrawColor( R, 120, 120, 120, 255 );
	//gp_fill_roundedRect( R, &(data->bar_rect), 6 );
	float rad = 0.5 * data->bar_rect.w;
	float x = data->bar_rect.x + rad;
	gp_draw_roundedThickLine( R, x, data->bar_rect.y + rad, x, data->bar_rect.y + data->bar_rect.h - rad, rad );
	//SDL_RenderFillRect( R, &(data->bar_rect) );
}

void vertical_scrollbar_destroy( UI_Interactive *E ){
	//vertical_scrollbar_data *data = (vertical_scrollbar_data*) E->data;
	free( E->data );
}







// --- --- --- --- --- --- --- --- --- --- --- --- 	/Vertical Scrollbar
// --- --- --- --- --- --- --- --- --- --- --- ---  List Select







/*
typedef struct {
	char ***list;
	int *list_length;
	int *incumbency;
	int mouse_over;
	TX_Font *font;
	SDL_Color *bg;// background
	SDL_Color *mo;// mouseover
	SDL_Color *se;// selected
	//SDL_Texture *background;
	SDL_Texture *list_texture;
	SDL_Rect list_dst;
	int *list_heights;
	int total_height;
	int scroll;
	int sbw;
	UI_Interactive scrollbar;

} list_select_data;*/


UI_Interactive *UI_build_list_select( UI_Set *S, int c, int r, char ***list, int *list_length, int *incumbency,
									  TX_Font *font, float font_scale, SDL_Color *bg, SDL_Color *mo, SDL_Color *se, int sbw ){

	int I = S->build_index;
	S->set[ I ].rect = calc_rect( S, c, r );
	list_select_data *data = malloc( sizeof(list_select_data) );
	*data = (list_select_data){ list, list_length, 
								incumbency, 
								-1, 
								font, font_scale,
								bg, mo, se,
								NULL, 
								S->set[ I ].rect,
								NULL, 0, 0, sbw,
								new_vertical_scrollbar( S->set[ I ].rect, sbw, &(data->scroll) )
							  };
	data->list_dst.w -= sbw;

	S->set[ I ].data = (void*)data;
	S->set[ I ].IEHFs[mouse_motion] = list_select_mouse_motion;
	S->set[ I ].IEHFs[ mouse_down ] = list_select_mouse_down;  
	S->set[ I ].IEHFs[  mouse_up  ] = list_select_mouse_up;    
	S->set[ I ].IEHFs[ mouse_wheel] = list_select_mouse_wheel; 
	S->set[ I ].IEHFs[  key_down  ] = empty_IEHF;
	S->set[ I ].IEHFs[   key_up   ] = empty_IEHF;
	S->set[ I ].IEHFs[ text_input ] = empty_IEHF;
	S->set[ I ].IEHFs[text_editing] = empty_IEHF;
	S->set[ I ].display_function = list_select_display;
	S->set[ I ].destroy_function = list_select_destroy;
	log_indices( S, c, r );
	S->build_index ++;
	return S->set + I;
}

void list_select_update( SDL_Renderer *R, UI_Interactive *E ){
	
	list_select_data *data = (list_select_data*) E->data;
	if( *(data->list_length) < 0 || *(data->list_length) > 999 ){
		printf("Warning: you might have forgotten to initialize the length of the list for this list_select_update. (%p: %d)", data->list_length, *(data->list_length) );
		return;
	}
	data->font->scale = data->font_scale;

	data->total_height = 0;//*(data->list_length) * line_height + 2;
	data->list_heights = realloc( data->list_heights, (*(data->list_length)) * sizeof(int) );
	
	for (int i = 0; i < *(data->list_length); ++i){
		int h = TX_wrapped_string_height( data->font, (*(data->list))[i], data->list_dst.w-10 ) + ls_line_gap;
		data->total_height += h;
		data->list_heights[i] = h;
	}
	if( data->total_height < data->list_dst.h ) data->total_height = data->list_dst.h;

	vertical_scrollbar_update( R, &(data->scrollbar), data->total_height );
	
	if( data->list_texture != NULL ){
		int th = 0;
		SDL_QueryTexture( data->list_texture, NULL, NULL, NULL, &th);
		if( th != data->total_height ){
			SDL_DestroyTexture( data->list_texture );
			goto ls_make_new_texture;
		}
	}
	else{
		ls_make_new_texture:
		data->list_texture = SDL_CreateTexture( R, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, data->list_dst.w, data->total_height );
		SDL_SetTextureBlendMode( data->list_texture, SDL_BLENDMODE_BLEND );
	}
	SDL_SetRenderTarget( R, data->list_texture );
	SDL_SetRenderDrawColor( R, 0, 0, 0, 0 );
	SDL_RenderClear( R );

	int y = 0;
	for (int i = 0; i < *(data->list_length); ++i){
		SDL_SetRenderDraw_SDL_Color( R, data->mo );//SDL_SetRenderDrawColor( R, data->mo->r, data->mo->g, data->mo->b, data->mo->a );
		SDL_Rect rct = (SDL_Rect){ 0, y, data->list_dst.w, data->list_heights[ i ] };
		gp_draw_roundedRect( R, &rct, 12 );
		TX_render_string_wrapped( R, data->font, (*(data->list))[i], 8, y, data->list_dst.w-16 );
		y += data->list_heights[i];
	}
	//printf( "~list is %d pixels high\n", y );

	SDL_SetRenderTarget( R, NULL );
}

char list_select_mouse_motion( UI_Interactive *E, SDL_Event *event, bool IN ){
	list_select_data *data = (list_select_data*) E->data;
	data->mouse_over = -999;
	char cap = vertical_scrollbar_mouse_motion( &(data->scrollbar), event, IN );
	if( cap ) return 1;
	if( IN && (event->button.x < E->rect.x + E->rect.w - data->sbw) ){
		//float line_height = (data->font->line_skip * data->font->scale) + ls_line_gap;
		//data->mouse_over = (int)( ( event->motion.y - data->list_dst.y - data->scroll ) / line_height );
		int cy = event->motion.y - data->list_dst.y - data->scroll;
		int y = 0;
		for (int i = 0; i < *(data->list_length); ++i){
			y += data->list_heights[i];
			if( y > cy ){
				//printf("= %d, %d, %d\n", cy, y, i );
				data->mouse_over = i;
				break;
			}
		}
	}
	return 0;
}

char list_select_mouse_down( UI_Interactive *E, SDL_Event *event, bool IN ){
	if( IN ){
		list_select_data *data = (list_select_data*) E->data;
		vertical_scrollbar_mouse_down( &(data->scrollbar), event, (event->motion.x > data->scrollbar.rect.x) );
		return 1;
	}
	return 0;
}

char list_select_mouse_up( UI_Interactive *E, SDL_Event *event, bool IN ){
	list_select_data *data = (list_select_data*) E->data;
	if( IN ){
		if (event->button.x > data->scrollbar.rect.x ){//E->rect.x + E->rect.w - data->sbw){
			return vertical_scrollbar_mouse_up( &(data->scrollbar), event, 1 );		
		}
		else{
			if( data->mouse_over >= 0 && data->mouse_over < *(data->list_length) ){
				*(data->incumbency) = data->mouse_over;
			}
			vertical_scrollbar_mouse_up( &(data->scrollbar), event, 0 );
			return 1;
		}
	}
	vertical_scrollbar_mouse_up( &(data->scrollbar), event, 0 );
	return 0;
}

char list_select_mouse_wheel( UI_Interactive *E, SDL_Event *event, bool IN ){
	if( IN ){
		list_select_data *data = (list_select_data*) E->data;
		//vertical_scrollbar_data *vsdata = (vertical_scrollbar_data*) (data->scrollbar.data);
		//data->scroll = constrain( data->scroll - 8 * event->wheel.y, vsdata->max_scroll, 0 );
		vertical_scrollbar_external_scroll( &(data->scrollbar), 35 * event->wheel.y );
	}
	return IN;
}

void list_select_display( SDL_Renderer *R, UI_Interactive *E ){

	list_select_data *data = (list_select_data*) E->data;

	//SDL_RenderCopy( R, data->background, NULL, &(E->rect) );

	if( data->mouse_over >= 0 && data->mouse_over < *(data->list_length) ){
		int Y = data->list_dst.y + data->scroll;
		for (int i = 0; i < data->mouse_over; ++i){
			Y += data->list_heights[i];
		}
		SDL_RenderSetClipRect( R, &(data->list_dst) );
		SDL_SetRenderDraw_SDL_Color( R, data->mo );//SDL_SetRenderDrawColor( R, data->mo->r, data->mo->g, data->mo->b, data->mo->a );
		SDL_Rect rct = (SDL_Rect){ data->list_dst.x, Y, data->list_dst.w, data->list_heights[ data->mouse_over ] };
		gp_fill_roundedRect( R, &rct, 12 );
		SDL_RenderSetClipRect( R, NULL );
	}
	if( *(data->incumbency) >= 0 && *(data->incumbency) < *(data->list_length) ){
		int Y = data->list_dst.y + data->scroll;
		for (int i = 0; i < *(data->incumbency); ++i){
			Y += data->list_heights[i];
		}
		SDL_RenderSetClipRect( R, &(data->list_dst) );
		SDL_SetRenderDraw_SDL_Color( R, data->se );//SDL_SetRenderDrawColor( R, data->se->r, data->se->g, data->se->b, data->se->a );
		SDL_Rect rct = (SDL_Rect){ data->list_dst.x, Y, data->list_dst.w, data->list_heights[ *(data->incumbency) ] };
		gp_fill_roundedRect( R, &rct, 12 );
		SDL_RenderSetClipRect( R, NULL );
	}
	int lh = min( data->total_height + data->scroll, data->list_dst.h );
	SDL_Rect src = (SDL_Rect){ 0, -data->scroll, data->list_dst.w, lh };
	SDL_RenderCopy( R, data->list_texture, &src, &(data->list_dst) );

	SDL_SetRenderDraw_SDL_Color( R, data->se );
	data->scrollbar.display_function( R, &(data->scrollbar) );
}

void list_select_destroy( UI_Interactive *E ){

	list_select_data *data = (list_select_data*) E->data;
	data->scrollbar.destroy_function( &(data->scrollbar) );
	//SDL_DestroyTexture( data->background );
	SDL_DestroyTexture( data->list_texture );
	free( data->list_heights );
	free( E->data );
}







// --- --- --- --- --- --- --- --- --- --- --- --- 	/List Select
// --- --- --- --- --- --- --- --- --- --- --- ---  Column Select







/*
typedef struct {
	char ***list;//reference to list of strings
	int *list_length;
	int *incumbency;
	int mouse_over;
	TX_Font *font;
	SDL_Texture *background;
	SDL_Texture *list_texture;
	SDL_Rect list_dst;
	int xmargin, ymargin, padding;
	int list_height;
	int *row_heights;
	float column_width;
	int scroll, scrolling;
	SDL_Texture *arrows;
	SDL_Rect arrow_srcs[2];
	int arrow2x, arrow3x;
	SDL_Rect arrow_dsts[2];
	void* VG;//voided pointer to the GlobalGFX struct.

} columns_select_data;
*/


UI_Interactive *UI_build_columns_select( UI_Set *S, int c, int r, int *incumbency,
										 char ***list, int *list_length ){

	int I = S->build_index;
	S->set[ I ].rect = calc_rect( S, c, r );
	SDL_Rect *R = &(S->set[ I ].rect);
	columns_select_data *data = malloc( sizeof(columns_select_data) );
	*data = (columns_select_data){ list, list_length, 
								   incumbency, -1, 
								   NULL,
								   NULL, NULL, (SDL_Rect){0,0,0,0},
								   24, 24, 48,
								   0, NULL, 0, 
								   0, 0,
								   NULL, 
								   { (SDL_Rect){0,0,0,0}, (SDL_Rect){0,0,0,0} }, 0, 0, 
								   { (SDL_Rect){0,0,0,0}, (SDL_Rect){0,0,0,0} },
								   NULL };

	S->set[ I ].data = (void*)data;
	S->set[ I ].IEHFs[mouse_motion] = columns_select_mouse_motion;
	S->set[ I ].IEHFs[ mouse_down ] = columns_select_mouse_down;  
	S->set[ I ].IEHFs[  mouse_up  ] = columns_select_mouse_up;    
	S->set[ I ].IEHFs[ mouse_wheel] = columns_select_mouse_wheel; 
	S->set[ I ].IEHFs[  key_down  ] = empty_IEHF;
	S->set[ I ].IEHFs[   key_up   ] = empty_IEHF;
	S->set[ I ].IEHFs[ text_input ] = empty_IEHF;
	S->set[ I ].IEHFs[text_editing] = empty_IEHF;
	S->set[ I ].display_function = columns_select_display;
	S->set[ I ].destroy_function = columns_select_destroy;
	log_indices( S, c, r );
	S->build_index ++;
	return S->set + I;
}

void columns_select_update( SDL_Renderer *R, UI_Interactive *E ){

	columns_select_data *data = (columns_select_data*) E->data;

	if( *(data->list_length) < 0 || *(data->list_length) > 1000 ) printf("Warning: you might have forgotten to initialize the length of the list for this columns_select_update. (list length is %d)", *(data->list_length) );
	
	int COLS = lrint( data->list_dst.w / data->column_width);
	int ROWS = ceil( *(data->list_length) / (float)COLS );
	//printf("%d: %d x %d\n", *(data->list_length), COLS, ROWS );
	//data->list_height = ROWS * line_height;
	data->list_height = 0;
	float half_pad = 0.5 * data->padding;
	int cw = data->column_width - data->padding;
	data->row_heights = realloc( data->row_heights, ROWS * sizeof(int) );

	for(int j = 0; j < ROWS; j++){
		data->row_heights[j] = lrint( data->font->line_skip * data->font->scale );
		for(int i = 0; i < COLS; i++){
			int E = (j*COLS)+i;
			if( E >= *(data->list_length) ) break;
			int h = TX_wrapped_string_height( data->font, (*(data->list))[E], cw );
			if( h > data->row_heights[j] ) data->row_heights[j] = h;
		}
		data->row_heights[j] += lrint( data->font->line_skip * data->font->scale * 0.5 );
		data->list_height += data->row_heights[j];
	}
	if( data->list_height < data->list_dst.h ) data->list_height = data->list_dst.h;

	
	if( data->list_texture != NULL ) SDL_DestroyTexture( data->list_texture );
	data->list_texture = SDL_CreateTexture( R, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, data->list_dst.w, data->list_height );
	SDL_SetTextureBlendMode( data->list_texture, SDL_BLENDMODE_BLEND );
	SDL_SetRenderTarget( R, data->list_texture );


	int Y = 0;
	for(int j = 0; j < ROWS; j++){
		for(int i = 0; i < COLS; i++){
			int E = (j*COLS)+i;
			if( E >= *(data->list_length) ) break;
			int h = TX_wrapped_string_height( data->font, (*(data->list))[E], cw );
			int y = Y + ((data->row_heights[j] - h) * 0.5);
			TX_render_string_wrapped( R, data->font, (*(data->list))[E], (i * data->column_width) + half_pad, y, cw );
		}
		Y += data->row_heights[j];
	}

	SDL_SetRenderTarget( R, NULL );

	if( data->scroll > 0 ) data->scroll = 0;
	if( data->list_height + data->scroll < data->list_dst.h ) data->scroll = data->list_dst.h - data->list_height;
}

char columns_select_mouse_motion( UI_Interactive *E, SDL_Event *event, bool IN ){
	columns_select_data *data = (columns_select_data*) E->data;
	if( IN ){
		if( cursor_in_rect( event, &(data->list_dst) ) ){
			int COLS = lrint( data->list_dst.w / data->column_width);
			int ROWS = ceil( *(data->list_length) / (float)COLS );
			int Y = 0;
			for(int i = 0; i < ROWS; i++){
				if( event->motion.y - data->list_dst.y - data->scroll < Y + data->row_heights[i] ){
					Y = i;
					break;
				}
				Y += data->row_heights[i];
			}
			int X = (int)( (event->motion.x - data->list_dst.x ) / data->column_width );
			data->mouse_over = ( Y * COLS ) + X;
			if( data->mouse_over >= *(data->list_length) ) data->mouse_over = -1;
			return 0;
		}
		else if( cursor_in_rect( event, data->arrow_dsts + 0 ) ){
			data->mouse_over = -2;
		}
		else if( cursor_in_rect( event, data->arrow_dsts + 1 ) ){
			data->mouse_over = -3;
		}
		else data->mouse_over = -1;
	}
	else data->mouse_over = -1;

	return 0;
}

char columns_select_mouse_down( UI_Interactive *E, SDL_Event *event, bool IN ){
	if( IN ){
		columns_select_data *data = (columns_select_data*) E->data;
		if( data->mouse_over == -2 ){
			data->scrolling = 5;
			return 1;
		}
		else if( data->mouse_over == -3 ){
			data->scrolling = -5;
			return 1;
		}
		else if( data->mouse_over >= 0 ){
			*(data->incumbency) = data->mouse_over;
			return 1;
		}
		else return 0;
	}
	return 0;
}

char columns_select_mouse_up( UI_Interactive *E, SDL_Event *event, bool IN ){
	columns_select_data *data = (columns_select_data*) E->data;
	data->scrolling = 0;
	return 0;
}

char columns_select_mouse_wheel( UI_Interactive *E, SDL_Event *event, bool IN ){
	if( IN ){
		columns_select_data *data = (columns_select_data*) E->data;
		data->scroll += 20 * event->wheel.y;
		if( data->scroll > 0 ) data->scroll = 0;
		if( data->list_height + data->scroll < data->list_dst.h ) data->scroll = data->list_dst.h - data->list_height;
	}
	return IN;
}

void columns_select_display( SDL_Renderer *R, UI_Interactive *E ){

	columns_select_data *data = (columns_select_data*) E->data;

	SDL_RenderCopy( R, data->background, NULL, &(E->rect) );

	int COLS = lrint( data->list_dst.w / data->column_width);
	//int ROWS = ceil( *(data->list_length) / COLS );

	if( *(data->incumbency) >= 0 ){

		int Y = (int)( (*(data->incumbency)) / (float)(COLS) );
		int y = data->list_dst.y + data->scroll;
		for(int i = 0; i < Y; i++){
			y += data->row_heights[i];
		}
		int X = (*(data->incumbency)) % COLS;
		int x = data->list_dst.x + (X * data->column_width);
		SDL_FRect DST = (SDL_FRect){ x, y, lrint(data->column_width), data->row_heights[Y] };
		SDL_RenderSetClipRect( R, &(data->list_dst) );
		//SDL_SetRenderDrawColor( R, 198, 202, 192, 255 );
		//SDL_RenderFillRect( R, &DST );
		SDL_SetRenderDrawColor( R, 129, 133, 126, 255 );
		gp_fill_roundedRect( R, &DST, 12 );
		//SDLgp_roundedBox( R, DST.x, DST.y, DST.x+DST.w, DST.y+DST.h, 12 );
		SDL_RenderSetClipRect( R, NULL );
	}
	if( data->mouse_over >= 0 ){

		int Y = (int)( data->mouse_over / (float)(COLS) );
		int y = data->list_dst.y + data->scroll;
		for(int i = 0; i < Y; i++){
			y += data->row_heights[i];
		}
		int X = data->mouse_over % COLS;
		int x = data->list_dst.x + (X * data->column_width);
		SDL_RenderSetClipRect( R, &(data->list_dst) );
		SDL_Rect DST = (SDL_Rect){ x, y, lrint(data->column_width), data->row_heights[Y] };
		//draw_box( R, &DST, (int[4]){1,2,2,2}, data->VG );
		SDL_RenderSetClipRect( R, NULL );
	}

	//printf("%d\n", data->list_height + data->scroll );
	//int H = data->list_dst.h;
	//if( data->list_height + data->scroll < H ) H = data->list_height + data->scroll;
	SDL_Rect src = (SDL_Rect){ 0, -data->scroll, data->list_dst.w, data->list_dst.h };//min( data->list_dst.h, data->list_height + data->scroll )
	SDL_RenderCopy( R, data->list_texture, &src, &(data->list_dst) );

	src = data->arrow_srcs[0];
	if( data->scrolling > 0 ){
		src.x = data->arrow3x;
		data->scroll += data->scrolling;
		if( data->scroll > 0 ) data->scroll = 0;
	} 
	else if( data->mouse_over == -2 ) src.x = data->arrow2x;
	SDL_RenderCopy( R, data->arrows, &src, data->arrow_dsts + 0 );
	src = data->arrow_srcs[1];
	if( data->scrolling < 0 ){
		src.x = data->arrow3x;
		data->scroll += data->scrolling;
		if( data->list_height + data->scroll < data->list_dst.h ) data->scroll = data->list_dst.h - data->list_height;
	}
	else if( data->mouse_over == -3 ) src.x = data->arrow2x;
	SDL_RenderCopy( R, data->arrows, &src, data->arrow_dsts + 1 );
}

void columns_select_destroy( UI_Interactive *E ){

	columns_select_data *data = (columns_select_data*) E->data;
	SDL_DestroyTexture( data->background );
	SDL_DestroyTexture( data->list_texture );
	free( data->row_heights );
	free( E->data );
}






// --- --- --- --- --- --- --- --- --- --- --- --- /Columns_select
// --- --- --- --- --- --- --- --- --- --- --- ---  Image viewer







/*typedef struct {
	SDL_Texture *incumbency;
	int w, h;
	SDL_Rect *src
	SDL_Rect dst;
	double scale, scale_index;
	bool dragging;
	SDL_Color *stroke;
} image_viewer_data;*/

void UI_build_image_viewer( UI_Set *S, int c, int r, SDL_Texture *incumbency, SDL_Rect *src, SDL_Color *s ){
	int I = S->build_index;
	S->set[ I ].rect = calc_rect( S, c, r );
	image_viewer_data *data = malloc( sizeof(image_viewer_data) );
	int w = 0;
	int h = 0;
	SDL_QueryTexture( incumbency, NULL, NULL, &w, &h);
	*data = (image_viewer_data){ incumbency, w, h, src, (SDL_Rect){0,0,w,h}, 1, 0, 0, s };
	fit_rect( &(data->dst), &(S->set[ I ].rect) );
	data->scale = data->dst.w / (float) w;
	data->scale_index = logarithm( 1.1, data->scale );
	S->set[ I ].data = (void*)data;
	S->set[ I ].IEHFs[mouse_motion] = image_viewer_mouse_motion;//empty_IEHF;
	S->set[ I ].IEHFs[ mouse_down ] = image_viewer_mouse_down;  //empty_IEHF;
	S->set[ I ].IEHFs[  mouse_up  ] = image_viewer_mouse_up;    //empty_IEHF;
	S->set[ I ].IEHFs[ mouse_wheel] = image_viewer_mouse_wheel; //empty_IEHF;
	S->set[ I ].IEHFs[  key_down  ] = empty_IEHF;
	S->set[ I ].IEHFs[   key_up   ] = empty_IEHF;
	S->set[ I ].IEHFs[ text_input ] = empty_IEHF;
	S->set[ I ].IEHFs[text_editing] = empty_IEHF;
	S->set[ I ].display_function = image_viewer_display;
	S->set[ I ].destroy_function = image_viewer_destroy;
	log_indices( S, c, r );
	S->build_index ++;
}

void image_viewer_update( UI_Interactive *E, SDL_Texture *neo, SDL_Rect *src ){
	image_viewer_data *data = (image_viewer_data*) E->data;
	data->incumbency = neo;
	data->src = src;
	if( src == NULL ){
		if( SDL_QueryTexture( data->incumbency, NULL, NULL, &(data->w), &(data->h) ) == 0 ){
			data->dst.w = data->w;
			data->dst.h = data->h;
		}
		else{
			puts("you tried to update an image viewer with an invalid texture.");
			return;
		}
	}
	else{
		data->dst.w = data->w = src->w;
		data->dst.h = data->h = src->h;
	}
	fit_rect( &(data->dst), &(E->rect) );
	data->scale = data->dst.w / (float) data->w;
	data->scale_index = logarithm( 1.1, data->scale );
}

char image_viewer_mouse_motion( UI_Interactive *E, SDL_Event *event, bool IN ){
	image_viewer_data *data = (image_viewer_data*) E->data;
	if( data->dragging ){
		data->dst.x += event->motion.xrel;
		data->dst.y += event->motion.yrel;
		return 1;
	}
	return 0;
}

char image_viewer_mouse_down( UI_Interactive *E, SDL_Event *event, bool IN ){
	if( IN ){
		if( event->button.button != SDL_BUTTON_LEFT ){
			image_viewer_data *data = (image_viewer_data*) E->data;
			data->dragging = 1;
			return 1;
		}
	}
	return 0;
}

char image_viewer_mouse_up( UI_Interactive *E, SDL_Event *event, bool IN ){
	image_viewer_data *data = (image_viewer_data*) E->data;
	if( data->dragging ){
		data->dragging = 0;
		return 1;
	}
	return 0;
}

char image_viewer_mouse_wheel( UI_Interactive *E, SDL_Event *event, bool IN ){
	if( IN ){
		image_viewer_data *data = (image_viewer_data*) E->data;
		int mx = 0, my = 0;
		SDL_GetMouseState( &mx, &my );
		float xrd = (mx - data->dst.x) / data->scale;
		float yrd = (my - data->dst.y) / data->scale;
		data->scale_index -= event->wheel.y;
		data->scale = pow( 1.1, data->scale_index );
		data->dst.x = mx - xrd * data->scale;
		data->dst.y = my - yrd * data->scale;
		data->dst.w = data->w * data->scale;
		data->dst.h = data->h * data->scale;
	}
	return IN;
}

void image_viewer_display( SDL_Renderer *R, UI_Interactive *E ){
	image_viewer_data *data = (image_viewer_data*) E->data;
	SDL_RenderSetClipRect( R, &(E->rect) );
	SDL_RenderCopy( R, data->incumbency, data->src, &(data->dst) );
	SDL_RenderSetClipRect( R, NULL );
	SDL_SetRenderDrawColor( R, data->stroke->r, data->stroke->g, data->stroke->b, data->stroke->a );
	SDL_RenderDrawRect( R, &(E->rect) );
}

void image_viewer_destroy( UI_Interactive *E ){
	free( E->data );
}








// --- --- --- --- --- --- --- --- --- --- --- --- /Image viewer
// --- --- --- --- --- --- --- --- --- --- --- --- item grid Element

/*
typedef struct item_grid{
	
	int columns, rows;
	SDL_Texture ***list; // pointer to array of SDL_Texture pointers
	int *list_length;
	int *positions;
	int plen;

	SDL_Texture *background;
	int iw, ih;
	SDL_Rect cliprect;

	int dragging_id, dragging_pos;
	int mx, my, dox, doy;
	
	int margin, spacing;
	int scroll;
	int sbw;
	UI_Interactive scrollbar;

} item_grid_data;

--------------------------------------------------------------------------------------------------
	if you want to drag items to other containers you gotta catch the mouse_up event before the UI!
--------------------------------------------------------------------------------------------------	
*/

int *UI_build_item_grid( UI_Set *S, int c, int r, SDL_Texture ***list, int *list_length, int sbw, SDL_Rect item_size ){
	
	int I = S->build_index;
	S->set[ I ].rect = calc_rect( S, c, r );

	item_grid_data *data = malloc( sizeof(item_grid_data) );

	int spacing = 5;
	int columns = floor( (S->set[ I ].rect.w -sbw ) / (float)(item_size.w + spacing) );
	int margin = floor( 0.5 * ( S->set[ I ].rect.w -sbw -(columns * (item_size.w + spacing)) ) );
	int rows = ceil( (S->set[ I ].rect.h - 2*margin) / (float)(item_size.h + spacing) );

	int pl = columns * rows;
	if( *list_length > pl && *list_length < 99999 ){
		pl = columns * ceil( *list_length / (float)(columns) );
	}
	//printf(":%p\n", list );

	*data = (item_grid_data){ columns, rows,
							  list, list_length, 
							  calloc( pl, sizeof(int) ), pl,
							  NULL,
							  item_size.w, item_size.h,
							  (SDL_Rect){ S->set[ I ].rect.x + margin, S->set[ I ].rect.y + margin, S->set[ I ].rect.w - 2*margin, S->set[ I ].rect.h - 2*margin },
							  -1, -1, 0, 0, 0, 0,
							  margin, spacing,
							  0, sbw, new_vertical_scrollbar( S->set[ I ].rect, sbw, &(data->scroll) ) };
	
	S->set[ I ].data = (void*)data;
	S->set[ I ].IEHFs[mouse_motion] = item_grid_mouse_motion;
	S->set[ I ].IEHFs[ mouse_down ] = item_grid_mouse_down;  
	S->set[ I ].IEHFs[  mouse_up  ] = item_grid_mouse_up;    
	S->set[ I ].IEHFs[ mouse_wheel] = item_grid_mouse_wheel; 
	S->set[ I ].IEHFs[  key_down  ] = empty_IEHF;
	S->set[ I ].IEHFs[   key_up   ] = empty_IEHF;
	S->set[ I ].IEHFs[ text_input ] = empty_IEHF;
	S->set[ I ].IEHFs[text_editing] = empty_IEHF;
	S->set[ I ].display_function = item_grid_display;
	S->set[ I ].destroy_function = item_grid_destroy;
	log_indices( S, c, r );
	S->build_index ++;

	return &(data->dragging_id);
}

void item_grid_update( SDL_Renderer *R, UI_Interactive *E ){
	item_grid_data *data = (item_grid_data*) E->data;

	if( *(data->list_length) > data->plen ){
		data->plen = data->columns * ceil( *(data->list_length) / (float)(data->columns) );
		data->positions = realloc( data->positions, data->plen * sizeof(int) );
	}
	for (int i = 0; i < data->plen; ++i) data->positions[i] = -1;
	for (int i = 0; i < *(data->list_length); ++i){
		data->positions[i] = i;
	}
	UI_Interactive *igsb = &(data->scrollbar);
	int tch = ceil( ceil( *(data->list_length) / (float)(data->columns) ) * (data->ih + data->spacing) ) + (2 * data->margin);
	vertical_scrollbar_update( R, igsb, tch );
}

char item_grid_mouse_motion( UI_Interactive *E, SDL_Event *event, bool IN ){
	item_grid_data *data = (item_grid_data*) E->data;
	if( data->dragging_pos >= 0 ){
		data->mx = event->motion.x;
		data->my = event->motion.y;
	}
	char dragging = vertical_scrollbar_mouse_motion( &(data->scrollbar), event, (event->motion.x > E->rect.x + E->rect.w - data->sbw) );
	if( dragging ) return 1;
	return 0;
}

char item_grid_mouse_down( UI_Interactive *E, SDL_Event *event, bool IN ){
	if( IN ){
		item_grid_data *data = (item_grid_data*) E->data;
		char cap = vertical_scrollbar_mouse_down( &(data->scrollbar), event, cursor_in_rect(event, &(data->scrollbar.rect) ) );
		if( cap ) return 1;

		int I = floor( (event->button.x - E->rect.x - data->margin) / (float)(data->iw + data->spacing) );
		if( I < 0 || I >= data->columns ) return 0;
		int J = floor( (event->button.y - E->rect.y - data->margin - data->scroll) / (float)(data->ih + data->spacing) );
		if( J < 0 || event->button.y > E->rect.y + E->rect.h - data->margin ) return 0;
		data->dragging_pos = I + (J * data->columns);
		data->dragging_id = data->positions[ data->dragging_pos ];
		data->dox = event->button.x - E->rect.x - data->margin -(I * (data->iw + data->spacing));
		data->doy = event->button.y - E->rect.y - data->margin -(J * (data->ih + data->spacing)) - data->scroll;
		data->mx = event->button.x;
		data->my = event->button.y;
		return 1;
	}
	return 0;
}

char item_grid_mouse_up( UI_Interactive *E, SDL_Event *event, bool IN ){
	item_grid_data *data = (item_grid_data*) E->data;
	if( IN ){
		if( data->dragging_pos >= 0 ){
			int x = event->button.x - data->dox + round(data->iw * 0.5);
			int y = event->button.y - data->doy + round(data->ih * 0.5);
			int I = floor( (x - E->rect.x - data->margin               ) / (float)(data->iw + data->spacing) );
			int J = floor( (y - E->rect.y - data->margin - data->scroll) / (float)(data->ih + data->spacing) );
			if( I >= 0 && I < data->columns && J >= 0 && event->button.y < E->rect.y + E->rect.h - data->margin ){
				int ni = I + (J * data->columns);
				data->positions[ data->dragging_pos ] = data->positions[ ni ];
				data->positions[ ni ] = data->dragging_id;
			}
			data->dragging_id = -1;
			data->dragging_pos = -1;
		}
		else if (event->button.x > E->rect.x + E->rect.w - data->sbw){
			return vertical_scrollbar_mouse_up( &(data->scrollbar), event, 1 );		
		}
		else{
			vertical_scrollbar_mouse_up( &(data->scrollbar), event, 0 );
			return 1;		
		}
	}
	else if( data->dragging_pos >= 0 ){
		//data->positions[ data->dragging_pos ] = -1;
		data->dragging_id = -1;
		data->dragging_pos = -1;
	}
	vertical_scrollbar_mouse_up( &(data->scrollbar), event, 0 );
	return 0;
}

char item_grid_mouse_wheel( UI_Interactive *E, SDL_Event *event, bool IN ){
	if( IN ){
		item_grid_data *data = (item_grid_data*) E->data;
		vertical_scrollbar_external_scroll( &(data->scrollbar), 20 * event->wheel.y );
	}
	return IN;
}

void item_grid_display( SDL_Renderer *R, UI_Interactive *E ){

	item_grid_data *data = (item_grid_data*)(E->data);

	SDL_RenderCopy( R, data->background, NULL, &(E->rect) );

	int ty = floor(-data->scroll / (float)data->ih);
	int by = ty + data->rows +1;
	int ox = E->rect.x + data->margin;
	int oy = E->rect.y + data->margin + data->scroll;
	int dx = data->iw + data->spacing;
	int dy = data->ih + data->spacing;

	SDL_RenderSetClipRect( R, &(data->cliprect) );
	for (int i = 0; i < data->plen; ++i){
		if( data->positions[i] < 0 || data->dragging_pos == i ) continue;
		int Y = floor( i / (float)(data->columns) );
		if( Y >= ty && Y <= by ){
			int X = i % data->columns;
			SDL_RenderCopy( R, (*(data->list))[ data->positions[i] ], NULL, 
							&(SDL_Rect){ ox + X * dx, oy + Y * dy, data->iw, data->ih } );
		}
	}
	SDL_RenderSetClipRect( R, NULL );

	if( data->dragging_pos >= 0 ){

		int x = data->mx - data->dox;
		int y = data->my - data->doy;
		SDL_RenderCopy( R, (*(data->list))[ data->dragging_id ], NULL, 
						&(SDL_Rect){ data->mx - data->dox, data->my - data->doy, data->iw, data->ih } );

	}

	data->scrollbar.display_function( R, &(data->scrollbar) );
}

void item_grid_destroy( UI_Interactive *E ){
	//puts("destroying columns_select...");
	item_grid_data *data = (item_grid_data*) E->data;
	free( data->positions );
	data->scrollbar.destroy_function( &(data->scrollbar) );
	SDL_DestroyTexture( data->background );
	free( E->data );
	//puts("destroyed columns_select.");
}



// --- --- --- --- --- --- --- --- --- --- --- --- icon_select Element
/*
typedef struct {

	SDL_Texture *spritesheet;
	SDL_Rect *srcs;
	SDL_FRect dsts [3];
	UI_Interactive buttons[2];

} icon_select_data;*/

void UI_build_icon_select( UI_Set *S, int c, int r, int *incumbency, SDL_Texture *spritesheet, SDL_Rect *srcs, int count,
						   SDL_Color lo, SDL_Color mo, SDL_Color hi ){
	int I = S->build_index;
	S->set[ I ].rect = calc_rect( S, c, r );
	SDL_Rect *R = &(S->set[ I ].rect);
	icon_select_data *data = malloc( sizeof(icon_select_data) );
	int bw = ceil(R->w / 6.0);
	float ih = 0.8 * R->h;
	float iw = ih * ( srcs[0].w / (float)(srcs[0].h) );
	float cx = R->x + (R->w * 0.5);
	float cy = R->y + (R->h * 0.5);
	float liw = 0.6 * iw;
	float lih = 0.6 * ih;
	float magic = (0.375*((0.666*R->w)-iw));
	*data = (icon_select_data){ spritesheet, srcs, 
								{ (SDL_FRect){ R->x + bw + magic -(liw*0.5), cy - (lih * 0.5), liw, lih },
								  (SDL_FRect){ cx -(iw * 0.5), cy - (ih * 0.5), iw, ih },
								  (SDL_FRect){ R->x + R->w -bw -magic -(liw*0.5), cy - (lih * 0.5), liw, lih } },
								{ new_numcycle( (SDL_Rect){ R->x,         R->y, bw, R->h }, incumbency, -1, 0, count-1, lo, mo, hi ),
								  new_numcycle( (SDL_Rect){ R->x+R->w-bw, R->y, bw, R->h }, incumbency,  1, 0, count-1, lo, mo, hi ) } };
	S->set[ I ].data = (void*)data;
	S->set[ I ].IEHFs[mouse_motion] = icon_select_mouse_motion;//empty_IEHF;
	S->set[ I ].IEHFs[ mouse_down ] = icon_select_mouse_down;
	S->set[ I ].IEHFs[  mouse_up  ] = icon_select_mouse_up;    //empty_IEHF;
	S->set[ I ].IEHFs[ mouse_wheel] = empty_IEHF;
	S->set[ I ].IEHFs[  key_down  ] = empty_IEHF;
	S->set[ I ].IEHFs[   key_up   ] = empty_IEHF;
	S->set[ I ].IEHFs[ text_input ] = empty_IEHF;
	S->set[ I ].IEHFs[text_editing] = empty_IEHF;
	S->set[ I ].display_function = icon_select_display;
	S->set[ I ].destroy_function = icon_select_destroy;
	log_indices( S, c, r );
	S->build_index ++;	
}

void icon_select_set_labels( UI_Interactive *E, TX_Font *font, char *str, char pos[2] ){
	icon_select_data *data = (icon_select_data*) E->data;
	numcycle_set_label( data->buttons + 0, font, str,   pos );
	numcycle_set_label( data->buttons + 1, font, str+2, pos );
}


char icon_select_mouse_motion( UI_Interactive *E, SDL_Event *event, bool IN ){
	icon_select_data *data = (icon_select_data*) E->data;
	if( IN ){
		numcycle_mouse_motion( data->buttons + 0, event, event->motion.x < E->rect.x + data->buttons[0].rect.w );
		numcycle_mouse_motion( data->buttons + 1, event, event->motion.x > data->buttons[1].rect.x );
	}
	else{
		numcycle_mouse_motion( data->buttons + 0, event, 0 );
		numcycle_mouse_motion( data->buttons + 1, event, 0 );
	}
	return 0;
}

char icon_select_mouse_down( UI_Interactive *E, SDL_Event *event, bool IN ){
	if( IN ){
		icon_select_data *data = (icon_select_data*) E->data;
		char o = 0;
		o += numcycle_mouse_down( data->buttons + 0, event, event->motion.x < E->rect.x + data->buttons[0].rect.w );
		o += numcycle_mouse_down( data->buttons + 1, event, event->motion.x > data->buttons[1].rect.x );
		return o;
	}
	return 0;
}

char icon_select_mouse_up( UI_Interactive *E, SDL_Event *event, bool IN ){
	if( IN ){
		icon_select_data *data = (icon_select_data*) E->data;
		char o = 0;
		o += numcycle_mouse_up( data->buttons + 0, event, event->motion.x < E->rect.x + data->buttons[0].rect.w );
		o += numcycle_mouse_up( data->buttons + 1, event, event->motion.x > data->buttons[1].rect.x );
		return o;
	}
	return 0;
}

void icon_select_display( SDL_Renderer *R, UI_Interactive *E ){
	icon_select_data *data = (icon_select_data*) E->data;

	numcycle_data *bdata = (numcycle_data*)(data->buttons[0].data);
	int b = *(bdata->incumbency);
	int a = (b > 0        )? b - 1 : bdata->hi;
	int c = (b < bdata->hi)? b + 1 : 0;

	SDL_RenderCopyF( R, data->spritesheet, data->srcs + a, data->dsts + 0 );
	SDL_RenderCopyF( R, data->spritesheet, data->srcs + c, data->dsts + 2 );
	SDL_RenderCopyF( R, data->spritesheet, data->srcs + b, data->dsts + 1 );

	numcycle_display( R, data->buttons + 0 );	
	numcycle_display( R, data->buttons + 1 );
}

void icon_select_destroy( UI_Interactive *E ){
	icon_select_data *data = (icon_select_data*) E->data;
	data->buttons[0].destroy_function( data->buttons + 0 );
	data->buttons[1].destroy_function( data->buttons + 1 );
	free( E->data );
}






// --- --- --- --- --- --- --- --- --- --- --- ---  /icon_select




//=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=/=
/* BUILD TEMPLATE
void UI_build_( UI_Set *S, int c, int r, int *incumbency ){
	int I = S->build_index;
	S->set[ I ].rect = calc_rect( S, c, r );
	#_data *data = malloc( sizeof(#_data) );
	*data = (#_data){  };
	S->set[ I ].data = (void*)data;
	S->set[ I ].IEHFs[mouse_motion] = #_mouse_motion;//empty_IEHF;
	S->set[ I ].IEHFs[ mouse_down ] = #_mouse_down;  //empty_IEHF;
	S->set[ I ].IEHFs[  mouse_up  ] = #_mouse_up;    //empty_IEHF;
	S->set[ I ].IEHFs[ mouse_wheel] = #_mouse_wheel; //empty_IEHF;
	S->set[ I ].IEHFs[  key_down  ] = #_key_down;    //empty_IEHF;
	S->set[ I ].IEHFs[   key_up   ] = #_key_up;      //empty_IEHF;
	S->set[ I ].IEHFs[ text_input ] = #_text_input;  //empty_IEHF;
	S->set[ I ].IEHFs[text_editing] = #_text_editing;//empty_IEHF;
	S->set[ I ].display_function = #_display;
	S->set[ I ].destroy_function = #_destroy;
	log_indices( S, c, r );
	S->build_index ++;
}
*/