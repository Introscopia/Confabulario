#ifndef UI_H_INCLUDED
#define UI_H_INCLUDED


// #include <stdio.h>
// #include <stdlib.h>
// #include <math.h>
// #include <string.h>
#include "basics.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include "TX_Font.h"

//text padding
//#define pad 4
//throws "error: expected identifier or '(' before numeric constant"
//can't be bothered to track it down rn

typedef enum { mouse_motion, mouse_down, mouse_up, mouse_wheel, 
			  key_down, key_up, text_input, text_editing } event_type;

typedef struct UI_Interactive UI_Interactive;

typedef struct UI_Dynamic UI_Dynamic;

typedef char (*input_event_handling_function) ( UI_Interactive *E, SDL_Event *event, bool IN );

typedef void (*UI_Interactive_display_function) ( SDL_Renderer *renderer, UI_Interactive *E );

typedef void (*UI_Interactive_destroy_function) ( UI_Interactive *E );

typedef void (*UI_Dynamic_display_function) ( SDL_Renderer *renderer, UI_Dynamic *E );

typedef void (*UI_Dynamic_destroy_function) ( UI_Dynamic *E );

typedef struct {
	SDL_Rect rect;
	SDL_Texture *texture;
} UI_Static;// -o  -o  -o  -o  -o  -o  -o  -o  -o  -o  -o  -o  -o  -o  -o

typedef struct UI_Dynamic{
	SDL_Rect rect;
	void *data;
	UI_Dynamic_display_function display_function;
	UI_Dynamic_destroy_function destroy_function;
} UI_Dynamic;// -o  -o  -o  -o  -o  -o  -o  -o  -o  -o  -o  -o  -o  -o  -o

typedef struct UI_Interactive{
	SDL_Rect rect;
	void *data;
	input_event_handling_function IEHFs[8];
	UI_Interactive_display_function display_function;
	UI_Interactive_destroy_function destroy_function;
} UI_Interactive;// -o  -o  -o  -o  -o  -o  -o  -o  -o  -o  -o  -o  -o  -o  -o

typedef struct UI_Set_struct{
	
	UI_Interactive *set;
	int set_size, build_index;

	UI_Dynamic *dynamic_set;
	int dynamic_set_size, dynamic_build_index;

	UI_Static *static_set;
	int static_set_size, static_build_index;

	int **indices;
	int margin;
	float columns, rows;
	int column_width, row_height;
	float Hx, Vx;
	int mouseX, mouseY;// we need these because the mouse wheel events don't have the cursor position.
} UI_Set;// -o  -o  -o  -o  -o  -o  -o  -o  -o  -o  -o  -o  -o  -o  -o  -o  -o

bool inside( UI_Interactive *E, float x, float y );

float calc_cell_x( UI_Set *S, int c );
float calc_cell_y( UI_Set *S, int r );
float calc_cell_w( UI_Set *S );
float calc_cell_h( UI_Set *S );
SDL_Rect calc_rect( UI_Set *S, int c, int r );

// --- --- --- --- --- --- --- --- --- --- --- --- Set build

void build_UI_Set( UI_Set *S, int set_size, float margin, float columns, float rows, float Hx, float Vx, int width, int height );
void build_UI_Set2( UI_Set *S, int set_size, int column_width, int row_height, int width, int height );

void destroy_UI_Set( UI_Set *S );

void UI_init_dynamic_set( UI_Set *S, int size );

void UI_init_static_set( UI_Set *S, int size );



UI_Interactive *UI_last_element( UI_Set *S );

// --- --- --- --- --- --- --- --- --- --- --- --- Set display

void UI_display(  SDL_Renderer *renderer, UI_Set *S );

void UI_display_grid(  SDL_Renderer *renderer, UI_Set *S, int width, int height );

// --- --- --- --- --- --- --- --- --- --- --- --- Set input

char UI_event_handler( UI_Set *S, SDL_Event *event );

char UI_orphan_event_handler( UI_Interactive *E, SDL_Event *event );


//    []o[]o[]o[]o[]o[]o[]o[]o[]o[]o[]o[]o[]o[]o   S T A T I C   o[]o[]o[]o[]o[]o[]o[]o[]o[]o[]o[]o[]o[]o[]o


char empty_IEHF( UI_Interactive *E, SDL_Event *event, bool IN );

void UI_build_label( UI_Set *S, int c, int r, SDL_Texture *txtr );

void UI_build_static_label( UI_Set *S, int c, int r, SDL_Renderer * renderer, char *str, char pos[2], TX_Font *font );


//    []o[]o[]o[]o[]o[]o[]o[]o[]o[]o[]o[]o[]o[]o    D Y N A M I C   o[]o[]o[]o[]o[]o[]o[]o[]o[]o[]o[]o[]o[]o[]o


// --- --- --- --- --- --- --- --- --- --- --- --- dynamic_int_label

typedef struct {
	int *content;
	char *fmt;
	TX_Font *font;
	char pos [2];
} dynamic_int_label;

//returns a pointer to the int *pointer so you can change what it points to!
int **UI_build_dynamic_int_label( UI_Set *S, int c, int r, char *fmt, char *pos, int *content, TX_Font *font );

void dynamic_int_label_display( SDL_Renderer *renderer, UI_Dynamic *E );

void dynamic_int_label_destroy( UI_Dynamic *E );




// --- --- --- --- --- --- --- --- --- --- --- dynamic_string_label

typedef struct {
	char *content;
	TX_Font *font;
	char pos [2];
} dynamic_string_label;

//returns a pointer to the char *pointer so you can change what it points to!
char **UI_build_dynamic_string_label( UI_Set *S, int c, int r, char pos[2], TX_Font *font, char *content );

void dynamic_string_label_display( SDL_Renderer *renderer, UI_Dynamic *E );

void dynamic_string_label_destroy( UI_Dynamic *E );



// --- --- --- --- --- --- --- --- --- --- --- TX_Label

typedef struct {
	TX_Font *font;
	char *content;
	float tx, ty;
} TX_label;

void display_TX_label( SDL_Renderer *R, TX_label *L );
void display_TX_label_alt( SDL_Renderer *R, TX_label *L, TX_Font *alt );


// []o[]o[]o[]o[]o[]o[]o[]o[]o[]o[]o[]o[]o[]o    I N T E R A C T I V E   o[]o[]o[]o[]o[]o[]o[]o[]o[]o[]o[]o[]o[]o[]o





// --- --- --- --- --- --- --- --- --- --- --- --- Toggle Element

typedef struct {
	bool *incumbency;
	bool mouse_over;
	SDL_Color colors [3];
	TX_label label;
	TX_Font *antifont;
} toggle_data;

void UI_build_toggle( UI_Set *S, int c, int r, bool *incumbency, SDL_Color lo, SDL_Color mo, SDL_Color hi );
UI_Interactive build_orphan_toggle( int x, int y, int w, int h, bool *incumbency, SDL_Color lo, SDL_Color mo, SDL_Color hi );

void toggle_set_label( UI_Interactive *E, TX_Font *font, TX_Font *antifont, char *str, char pos[2] );

char toggle_mouse_motion( UI_Interactive *E, SDL_Event *event, bool IN );

char toggle_mouse_up( UI_Interactive *E, SDL_Event *event, bool IN );

void toggle_display( SDL_Renderer *renderer, UI_Interactive *E );
void tickbox_display( SDL_Renderer *renderer, UI_Interactive *E );

void toggle_destroy( UI_Interactive *E );

void destroy_toggle_and_textures( UI_Interactive *E );

// --- --- --- --- --- --- --- --- --- --- --- --- NumSet Element

typedef struct {
	int *incumbency;
	int set, unset;
	bool mouse_over;
	SDL_Color colors [3];
	TX_label label;
	TX_Font *antifont;
} numset_data;

void UI_build_numset( UI_Set *S, int c, int r, int *incumbency, int set, int unset, SDL_Color lo, SDL_Color mo, SDL_Color hi );

void numset_set_label( UI_Interactive *E, TX_Font *font, TX_Font *antifont, char *str, char pos[2] );

char numset_mouse_motion( UI_Interactive *E, SDL_Event *event, bool IN );

char numset_mouse_up( UI_Interactive *E, SDL_Event *event, bool IN );

void numset_display( SDL_Renderer *renderer, UI_Interactive *E );

void numset_destroy( UI_Interactive *E );


// --- --- --- --- --- --- --- --- --- --- --- --- Multi NumSet Element

typedef struct {
	int *incumbency;
	int N;
	int *set;
	int mouse_over;
	SDL_Texture *spritesheet;
	SDL_Rect *srcs;
	SDL_Rect *dsts;
} multi_numset_data;

void UI_build_multi_numset( UI_Set *S, int c, int r, int *incumbency, int N, int *set );

char multi_numset_mouse_motion( UI_Interactive *E, SDL_Event *event, bool IN );

char multi_numset_mouse_up( UI_Interactive *E, SDL_Event *event, bool IN );

void multi_numset_display( SDL_Renderer *renderer, UI_Interactive *E );

void multi_numset_destroy( UI_Interactive *E );



// --- --- --- --- --- --- --- --- --- --- --- --- NumCycle Element

typedef struct {
	int *incumbency;
	int increment, lo, hi; // lo and hi are inclusive
	bool mouse_over, pressed;
	clock_t ct, step;
	SDL_Color colors [3];
	TX_label label;
} numcycle_data;

UI_Interactive new_numcycle( SDL_Rect R, int *incumbency, int increment, int low, int high, SDL_Color lo, SDL_Color mo, SDL_Color hi );

void UI_build_numcycle( UI_Set *S, int c, int r, int *incumbency, int increment, int low, int high, SDL_Color lo, SDL_Color mo, SDL_Color hi );

void numcycle_set_label( UI_Interactive *E, TX_Font *font, char *str, char pos[2] );

char numcycle_mouse_motion( UI_Interactive *E, SDL_Event *event, bool IN );

char numcycle_mouse_down( UI_Interactive *E, SDL_Event *event, bool IN  );

char numcycle_mouse_up( UI_Interactive *E, SDL_Event *event, bool IN );

void numcycle_display( SDL_Renderer *renderer, UI_Interactive *E );

void numcycle_destroy( UI_Interactive *E );



// --- --- --- --- --- --- --- --- --- --- --- --- PlusMinus Element

typedef struct {
	int *incumbency;
	int increment;
	int button_width;
	char lb, rb;
	clock_t t;
	float hold, freq;
	bool held;
	SDL_Rect brects [2];
	SDL_Texture *textures[7];
	TX_Font *font;
} plusminus_data;

void UI_build_plusminus( UI_Set *S, int c, int r, int *incumbency, int increment );

char plusminus_mouse_motion( UI_Interactive *E, SDL_Event *event, bool IN );

char plusminus_mouse_down( UI_Interactive *E, SDL_Event *event, bool IN  );

char plusminus_mouse_up( UI_Interactive *E, SDL_Event *event, bool IN );

void plusminus_display( SDL_Renderer *renderer, UI_Interactive *E );

void plusminus_destroy( UI_Interactive *E );



// --- --- --- --- --- --- --- --- --- --- --- --- Slider Element


typedef struct {
	float *incumbency;
	float min, max;
	bool dragging;
	SDL_Texture *bg;
	SDL_Texture *knob;
	SDL_Rect knob_dst;
} slider_data;

void UI_build_slider( UI_Set *S, int c, int r, float *incumbency, float min, float max );

void slider_set_incumbency( UI_Interactive *E, float *incumbency );

char slider_mouse_down( UI_Interactive *E, SDL_Event *event, bool IN );

char slider_mouse_motion( UI_Interactive *E, SDL_Event *event, bool IN );

char slider_mouse_up( UI_Interactive *E, SDL_Event *event, bool IN );

void slider_display( SDL_Renderer *renderer, UI_Interactive *E );

void slider_destroy( UI_Interactive *E );


// --- --- --- --- --- --- --- --- --- --- --- --- Dialog Element


typedef struct String_chainlink String_chainlink;

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
	int hyperlinks_N;

	float scroll;
	int scroll_width, current_height;
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

UI_Interactive * UI_build_dialog( UI_Set *S, int c, int r, int *incumbency, int sbw );

char dialog_mouse_motion( UI_Interactive *E, SDL_Event *event, bool IN );

char dialog_mouse_down( UI_Interactive *E, SDL_Event *event, bool IN );

char dialog_mouse_up( UI_Interactive *E, SDL_Event *event, bool IN );

char dialog_mouse_wheel( UI_Interactive *E, SDL_Event *event, bool IN );

void dialog_say( SDL_Renderer * renderer, UI_Interactive *E, char *text );

void dialog_ask( SDL_Renderer * renderer, UI_Interactive *E, char *text );

void dialog_unsay( SDL_Renderer *renderer, UI_Interactive *E );

void dialog_clear( SDL_Renderer *renderer, UI_Interactive *E );

void dialog_display( SDL_Renderer *renderer, UI_Interactive *E );

void dialog_destroy( UI_Interactive *E );




// --- --- --- --- --- --- --- --- --- --- --- --- terminal input Element

typedef struct {
	bool *bang;
	char **content;
	int history, history_cursor, size, len, cursor, cursorX, offset;
	bool cursor_on, render_flag;
	int xmargin, ymargin;
	TX_Font *font;
	SDL_Texture *background;
	SDL_Texture *texture;

} terminal_input_data;

void UI_build_terminal_input( UI_Set *S, int c, int r, bool *bang, TX_Font *font );

void clear_terminal_input( UI_Interactive *E );

char terminal_input_mouse_up( UI_Interactive *E, SDL_Event *event, bool IN );

char terminal_input_key_down( UI_Interactive *E, SDL_Event *event, bool IN );

void terminal_input_display( SDL_Renderer *renderer, UI_Interactive *E );

void terminal_input_destroy( UI_Interactive *E );




// --- --- --- --- --- --- --- --- --- --- --- --- text line input Element

typedef struct {
	char *incumbency;
	int size, len, cursor, cursorX, offset;
	bool cursor_on, changes;
	int xmargin, ymargin;
	TX_Font *font;
	SDL_Color colors [3];
	//SDL_Texture *background;
} text_line_input_data;

bool *UI_build_text_line_input( UI_Set *S, int c, int r, char *incumbency, int size, TX_Font *font, 
								SDL_Color lo, SDL_Color mo, SDL_Color hi );

char text_line_input_mouse_down( UI_Interactive *E, SDL_Event *event, bool IN );

char text_line_input_mouse_up( UI_Interactive *E, SDL_Event *event, bool IN );

char text_line_input_key_down( UI_Interactive *E, SDL_Event *event, bool IN );
char text_line_input_key_up( UI_Interactive *E, SDL_Event *event, bool IN );

char text_line_input_text_input( UI_Interactive *E, SDL_Event *event, bool IN );
char text_line_input_text_editing( UI_Interactive *E, SDL_Event *event, bool IN );

void text_line_input_display( SDL_Renderer *renderer, UI_Interactive *E );

void text_line_input_destroy( UI_Interactive *E );



// --- --- --- --- --- --- --- --- --- --- --- --- Key Select input Element

typedef struct {
	SDL_Keycode *incumbency;
	bool mouse_over, awaiting_input;
	char preamble [32];
	TX_label label;
	SDL_Texture *textures[3];

} key_select_data;

bool *UI_build_key_select( UI_Set *S, int c, int r, SDL_Keycode *incumbency, char *preamble, TX_Font *font );

char key_select_mouse_motion( UI_Interactive *E, SDL_Event *event, bool IN );

char key_select_mouse_up( UI_Interactive *E, SDL_Event *event, bool IN );

char key_select_key_down( UI_Interactive *E, SDL_Event *event, bool IN );

char key_select_key_up( UI_Interactive *E, SDL_Event *event, bool IN );

void key_select_display( SDL_Renderer *renderer, UI_Interactive *E );

void key_select_destroy( UI_Interactive *E );



// --- --- --- --- --- --- --- --- --- --- --- --- number input Element



typedef bool (*char_validity_check) ( char c );

#define num_str_size 24
typedef struct {
	void *incumbency;
	char str[num_str_size];
	char fmt[8];
	char_validity_check valid_char;
	int len, cursor, cursorX, offset;
	bool cursor_on, changes;
	int xmargin, ymargin;
	TX_Font *font;
	SDL_Color colors [3];
	//SDL_Texture *background;

} number_input_data;

bool *UI_build_number_input( UI_Set *S, int c, int r, void *incumbency, char fmt[8], TX_Font *font,
							 SDL_Color lo, SDL_Color mo, SDL_Color hi );

void number_input_sprintf_incumbency( number_input_data *data );

void number_input_update( UI_Interactive *E );

void number_input_set_incumbency( UI_Interactive *E, void *incumbency );

char number_input_mouse_up( UI_Interactive *E, SDL_Event *event, bool IN );

char number_input_key_down( UI_Interactive *E, SDL_Event *event, bool IN );

char number_input_key_up( UI_Interactive *E, SDL_Event *event, bool IN );

char number_input_text_input( UI_Interactive *E, SDL_Event *event, bool IN );

char number_input_text_editing( UI_Interactive *E, SDL_Event *event, bool IN );

void number_input_display( SDL_Renderer *renderer, UI_Interactive *E );

void number_input_destroy( UI_Interactive *E );



// --- --- --- --- --- --- --- --- --- --- --- --- text box input Element

typedef struct {
	STRB *incumbency;
	int cursor, cursorX, cursorY;
	bool cursor_on, render_flag;
	int xmargin, ymargin;
	TX_Font *font;
	//SDL_Texture *background;
	SDL_Color colors [3];
	SDL_Texture *texture;
	SDL_Rect dst;

	int scroll, scroll_width, current_height, scrolling_value;
	UI_Interactive scrollbar;

} textbox_data;

void UI_build_textbox( UI_Set *S, int c, int r, STRB *incumbency, TX_Font *font, int sbw, SDL_Renderer *R,
					   SDL_Color lo, SDL_Color mo, SDL_Color hi );

void textbox_set_string( UI_Interactive *E, char *str );
//void textbox_set_incumbency( UI_Interactive *E, char **incumbency, int size );

char textbox_mouse_motion( UI_Interactive *E, SDL_Event *event, bool IN );
char textbox_mouse_down( UI_Interactive *E, SDL_Event *event, bool IN );
char textbox_mouse_up( UI_Interactive *E, SDL_Event *event, bool IN );
char textbox_mouse_wheel( UI_Interactive *E, SDL_Event *event, bool IN );

char textbox_key_down( UI_Interactive *E, SDL_Event *event, bool IN );
char textbox_key_up( UI_Interactive *E, SDL_Event *event, bool IN );

char textbox_text_input( UI_Interactive *E, SDL_Event *event, bool IN );
char textbox_text_editing( UI_Interactive *E, SDL_Event *event, bool IN );

void textbox_display( SDL_Renderer *R, UI_Interactive *E );

void textbox_destroy( UI_Interactive *E );




// --- --- --- --- --- --- --- --- --- --- --- --- vertical scrollbar Element

typedef struct {
	int *incumbency;
	bool dragging, holding;
	int mouse_offset;
	int max_scroll;
	SDL_Rect bar_rect;
} vertical_scrollbar_data;

UI_Interactive new_vertical_scrollbar( SDL_Rect R, int sbw, int *scroll );

void UI_build_vertical_scrollbar( UI_Interactive *E, float *incumbency );

void vertical_scrollbar_update( SDL_Renderer *renderer, UI_Interactive *E, int total_content_height );

void vertical_scrollbar_external_scroll( UI_Interactive *E, float value );

char vertical_scrollbar_mouse_motion( UI_Interactive *E, SDL_Event *event, bool IN );

char vertical_scrollbar_mouse_down( UI_Interactive *E, SDL_Event *event, bool IN  );

char vertical_scrollbar_mouse_up( UI_Interactive *E, SDL_Event *event, bool IN );

void vertical_scrollbar_display( SDL_Renderer *renderer, UI_Interactive *E );

void vertical_scrollbar_destroy( UI_Interactive *E );



// --- --- --- --- --- --- --- --- --- --- --- --- List Select Element

typedef struct {
	char ***list;
	int *list_length;
	int *incumbency;
	int mouse_over;
	TX_Font *font;
	TX_Font *antifont;
	SDL_Color colors [3];
	SDL_Texture *list_texture;
	SDL_Rect list_dst;
	int *list_heights;
	int total_height;
	int scroll;
	int sbw;
	UI_Interactive scrollbar;

} list_select_data;

#define ls_line_gap 2

void new_list_select( UI_Interactive *E, char ***list, int *list_length, int *incumbency,
					  TX_Font *font, TX_Font *antifont, SDL_Color lo, SDL_Color mo, SDL_Color hi, int sbw );

UI_Interactive *UI_build_list_select( UI_Set *S, int c, int r, char ***list, int *list_length, int *incumbency,
						   			  TX_Font *font, TX_Font *antifont, SDL_Color lo, SDL_Color mo, SDL_Color hi, int sbw );

void list_select_update( SDL_Renderer *renderer, UI_Interactive *E );

char list_select_mouse_motion( UI_Interactive *E, SDL_Event *event, bool IN );

char list_select_mouse_down( UI_Interactive *E, SDL_Event *event, bool IN );

char list_select_mouse_up( UI_Interactive *E, SDL_Event *event, bool IN );

char list_select_mouse_wheel( UI_Interactive *E, SDL_Event *event, bool IN );

void list_select_display( SDL_Renderer *renderer, UI_Interactive *E );

void list_select_destroy( UI_Interactive *E );


// --- --- --- --- --- --- --- --- --- --- --- --- Columns Select Element

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

UI_Interactive *UI_build_columns_select( UI_Set *S, int c, int r, int *incumbency,
										 char ***list, int *list_length );

void columns_select_update( SDL_Renderer *renderer, UI_Interactive *E );

char columns_select_mouse_motion( UI_Interactive *E, SDL_Event *event, bool IN );

char columns_select_mouse_down( UI_Interactive *E, SDL_Event *event, bool IN );

char columns_select_mouse_up( UI_Interactive *E, SDL_Event *event, bool IN );

char columns_select_mouse_wheel( UI_Interactive *E, SDL_Event *event, bool IN );

void columns_select_display( SDL_Renderer *renderer, UI_Interactive *E );

void columns_select_destroy( UI_Interactive *E );



// --- --- --- --- --- --- --- --- --- --- --- --- Image Viewer Element

typedef struct {
	SDL_Texture *incumbency;
	int w, h;
	SDL_Rect *src;
	SDL_Rect dst;
	double scale, scale_index;
	bool dragging;
	SDL_Color *stroke;
} image_viewer_data;

void UI_build_image_viewer( UI_Set *S, int c, int r, SDL_Texture *incumbency, SDL_Rect *src, SDL_Color *s );

void image_viewer_update( UI_Interactive *E, SDL_Texture *neo, SDL_Rect *src );

char image_viewer_mouse_motion( UI_Interactive *E, SDL_Event *event, bool IN );

char image_viewer_mouse_down( UI_Interactive *E, SDL_Event *event, bool IN );

char image_viewer_mouse_up( UI_Interactive *E, SDL_Event *event, bool IN );

char image_viewer_mouse_wheel( UI_Interactive *E, SDL_Event *event, bool IN );

void image_viewer_display( SDL_Renderer *renderer, UI_Interactive *E );

void image_viewer_destroy( UI_Interactive *E );




// --- --- --- --- --- --- --- --- --- --- --- --- item grid Element

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

//returns pointer to the dragging_ID
int *UI_build_item_grid( UI_Set *S, int c, int r, SDL_Texture ***list, int *list_length, int sbw, SDL_Rect item_size );

//call this one after you build the i_g
void item_grid_update( SDL_Renderer *renderer, UI_Interactive *E );

char item_grid_mouse_motion( UI_Interactive *E, SDL_Event *event, bool IN );

char item_grid_mouse_down( UI_Interactive *E, SDL_Event *event, bool IN );

char item_grid_mouse_up( UI_Interactive *E, SDL_Event *event, bool IN );

char item_grid_mouse_wheel( UI_Interactive *E, SDL_Event *event, bool IN );

void item_grid_display( SDL_Renderer *renderer, UI_Interactive *E );

void item_grid_destroy( UI_Interactive *E );



// --- --- --- --- --- --- --- --- --- --- --- --- Sprite Picker Element

typedef struct {

	int *incumbency;
	bool *selection;
	int mouse_over;

	SDL_Texture *sheet;
	int w, h, columns, count;
	SDL_Rect dst;
	double scale, scale_index;
	bool dragging;
	//SDL_Color stroke;
	SDL_Color colors [3];

	int celw, celh, margin, spacing;

} sprite_picker_data;

void UI_build_sprite_picker( UI_Set *S, int c, int r, SDL_Texture *sheet, int celw, int celh, 
							 int margin, int spacing, int *incumbency, bool *selection,
							 SDL_Color lo, SDL_Color mo, SDL_Color hi );

char sprite_picker_mouse_motion( UI_Interactive *E, SDL_Event *event, bool IN );

char sprite_picker_mouse_down( UI_Interactive *E, SDL_Event *event, bool IN );

char sprite_picker_mouse_up( UI_Interactive *E, SDL_Event *event, bool IN );

char sprite_picker_mouse_wheel( UI_Interactive *E, SDL_Event *event, bool IN );

void sprite_picker_display( SDL_Renderer *renderer, UI_Interactive *E );

void sprite_picker_destroy( UI_Interactive *E );




// --- --- --- --- --- --- --- --- --- --- --- --- icon_select Element

typedef struct {

	SDL_Texture *spritesheet;
	SDL_Rect *srcs;
	SDL_FRect dsts [3];
	UI_Interactive buttons[2];

} icon_select_data;

void UI_build_icon_select( UI_Set *S, int c, int r, int *incumbency, SDL_Texture *spritesheet, SDL_Rect *srcs, int count, SDL_Color lo, SDL_Color mo, SDL_Color hi );

void icon_select_set_labels( UI_Interactive *E, TX_Font *font, char *str, char pos[2] );

char icon_select_mouse_motion( UI_Interactive *E, SDL_Event *event, bool IN );

char icon_select_mouse_down( UI_Interactive *E, SDL_Event *event, bool IN );

char icon_select_mouse_up( UI_Interactive *E, SDL_Event *event, bool IN );

void icon_select_display( SDL_Renderer *renderer, UI_Interactive *E );

void icon_select_destroy( UI_Interactive *E );




// --- --- --- --- --- --- --- --- --- --- --- --- file_browser Element

typedef struct {

	bool *bang;
	char *dest;

	char path [512];
	char **list;
	int list_length;
	int index;
	UI_Interactive LS;

	clock_t pmc;//previous mouse click
	bool update_flag;

} file_browser_data;

void UI_build_file_browser( UI_Set *S, int c, int r, TX_Font *font, TX_Font *antifont, 
							bool *bang, char *dest, SDL_Color lo, SDL_Color mo, SDL_Color hi );

void file_browser_set_path( UI_Interactive *E, char *newpath );

char file_browser_mouse_motion( UI_Interactive *E, SDL_Event *event, bool IN );

char file_browser_mouse_down( UI_Interactive *E, SDL_Event *event, bool IN );

char file_browser_mouse_up( UI_Interactive *E, SDL_Event *event, bool IN );

char file_browser_mouse_wheel( UI_Interactive *E, SDL_Event *event, bool IN );

void file_browser_display( SDL_Renderer *renderer, UI_Interactive *E );

void file_browser_destroy( UI_Interactive *E );




#endif