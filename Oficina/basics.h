#ifndef BASICS_H_INCLUDED
#define BASICS_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <stdarg.h>

#include <dirent.h>
#include <errno.h>
#include <locale.h>

#include <SDL.h>
#include <SDL_image.h>


#define PHI           1.618033988749894848204586834365638117720309179805762862135
#define SQRT2         1.414213562373095048801688724209698078569671875376948073176
#define SQRT2O2       0.707106781186547524400844362104849039284835937688474036588
#define SQRT3         1.732050807568877293527446341505872366942805253810380628055
#define EULER		    2.718281828459045235360287471352662497757247093699959574966

#define TWO_PI        6.2831853071795864769252867665590057683943387987502116419
#define PI            3.1415926535897932384626433832795028841971693993751058209
#define TWO_THIRDS_PI 2.0943951023931954923084289221863352561314462662500705473
#define HALF_PI       1.5707963267948966192313216916397514420985846996875529104
#define THIRD_PI      1.0471975511965977461542144610931676280657231331250352736
#define QUARTER_PI    0.7853981633974483096156608458198757210492923498437764552
#define FIFTH_PI      0.6283185307179586476925286766559005768394338798750211641
#define SIXTH_PI      0.5235987755982988730771072305465838140328615665625176368
#define EIGTH_PI      0.3926990816987241548078304229099378605246461749218882276
#define TWELFTH_PI    0.2617993877991494365385536152732919070164307832812588184
#define ONE_OVER_PI   0.3183098861837906715377675267450287240689192914809128974

#define ONE_OVER_255  0.0039215686274509803921568627450980392156862745098039215

typedef uint32_t color;


#if SDL_BYTEORDER == SDL_BIG_ENDIAN
   #define rmask 0xff000000
   #define gmask 0x00ff0000
   #define bmask 0x0000ff00
   #define amask 0x000000ff
#else
   #define rmask 0x000000ff
   #define gmask 0x0000ff00
   #define bmask 0x00ff0000
   #define amask 0xff000000
#endif



// COLOR, DRAWING

Uint8   red( Uint32 color );
Uint8 green( Uint32 color );
Uint8  blue( Uint32 color );
Uint8 alpha( Uint32 color );

Uint8 brightness( Uint32 color );

Uint32 rgba_to_Uint32( Uint8 r, Uint8 g, Uint8 b, Uint8 a );
Uint32 SDL_Color_to_Uint32( SDL_Color C );
SDL_Color Uint32_to_SDL_Color( Uint32 C );

extern DECLSPEC int SDLCALL SDL_SetRenderDraw_SDL_Color( SDL_Renderer *renderer, SDL_Color *C );
extern DECLSPEC int SDLCALL SDL_SetRenderDraw_Uint32( SDL_Renderer *renderer, Uint32 C );
SDL_Color SDL_GetRender_SDL_Color( SDL_Renderer *R );

color lerp_color( color CA, color CB, float amt );
SDL_Color lerp_SDL_Color( SDL_Color CA, SDL_Color CB, float amt );

//format = %Y.%m.%d %H-%M-%S.png
void save_screenshot( SDL_Renderer *renderer, char *format, SDL_Rect *rct );
void save_texture(const char* file_name, SDL_Renderer* renderer, SDL_Texture* texture);


// NUMBERS, MATH ---------------------------------------------------------------------------


double sq( double a );
double logarithm( double base, double x );

#define SIGN(x) ((x > 0)-(x < 0))

// gets the smaller of each pair of divisors, excluding 1, and puts them in the list you provide, 
// returns the number of items it put on the list.
int get_divisors( int *list, int N );

// RANDOM: "array rules" min inclusive, max not inclusive
int random( int min, int max );
double random_angle();
int random_from_list( int n, ... );
double random_gaussian();
void shuffle( int *deck, int len );

double lerp(double start, double stop, double amt);

double /*linear*/map(double value, double source_lo, double source_hi, double dest_lo, double dest_hi);
double ellipticalMap(double value, double source_lo, double source_hi, double dest_lo, double dest_hi);
double    sigmoidMap(double value, double source_lo, double source_hi, double dest_lo, double dest_hi);
double advSigmoidMap(double value, double source_lo, double source_hi, double Slo, double Shi, double dest_lo, double dest_hi);

int cycle( int a, int min, int max );
int constrain( int a, int min, int max );
float constrainF( float a, float min, float max );
double constrainD( double a, double min, double max );

int count_set_bits( unsigned int v );

#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))

double minD( double a, double b );
double maxD( double a, double b );

double degrees( double radians );
double radians( double degrees );

double rectify_angle( double a );
double angle_diff( double a, double b );


// CHAR & STRING ---------------------------------------------------------------------------

// String split. eats the original string!
//void strspl( char *string, const char *delimiters, char ***list, int *size );
void split( char *string, char *separator, char ***list, int *list_len );
// String Count character
int strcchr( char *string, char C );
// String find character, returns id, or -1 if it doesn't find it.
int strfchr( char *string, char C );
// sub-string. allocates a new char*. start inclusive, stop not-inclusivve.
char *substr( char *string, int start, int stop );
// returns the index of the first char that does not match, or the length of the shortest string.
int str_match( char *A, char *B );
// insert char at position. returns whether it fit into the size or not
bool str_insert_char( char *string, char C, int pos, int size );
void str_insert_str( char *string, char *in, int pos );
void str_delete_char( char *string, int pos, int len );


typedef struct stringbuilder{

   char *str;
   int len;
   int cap;

} STRB;

void STRB_init( STRB *S, int sz );
void STRB_ensure( STRB *S, int len );
void STRB_reset( STRB *S, int sz );
void STRB_justify( STRB *S );
void STRB_copy( STRB *S, char *str );

void STRB_append_char( STRB *S, char c );
void STRB_append_utf8( STRB *S, uint32_t c, int endianness );
void STRB_append_str( STRB *S, char *str );

void STRB_insert_char( STRB *S, char c, int pos );
void STRB_insert_utf8( STRB *S, uint32_t c, int endianness, int pos );
void STRB_insert_str( STRB *S, char *str, int pos );

void STRB_delete( STRB *S, int pos );
void STRB_delete_range( STRB *S, int start, int stop );

char STRB_event_handler( STRB *S, int *cursor, SDL_Event *event );


void insert_sorted( int* list, int *len, int N );
void delete_repack( int* list, int *len, int i );

void reverse_list(int* list, int len);

void Lshift_str( char *str, int n );
void strtrim( char *string );
void strtrim_fgetsd_str( char *string );

int getgc( FILE *f );
int lines_in_a_file( FILE* f );
bool fseek_lines( FILE* f, int N );
bool fseek_string( FILE *f, char *str );
bool fseek_string_before( FILE *f, char *str, char *terminator );
void fseek_category( FILE *f, int(*cateorize)(int c) );
void fskip_spaces( FILE *f );
int fnext_nonspace( FILE *f );
void fscan_str_until( FILE *f, char *dest, char *terminator, int size );

//fscan a comma-separated list of strings (or any other separator char)
//length of output list in placed into n (optional, just send NULL elsewise)
//must free output[0] (which contains all the chars), and then output itself (which is the list)
char **fscan_cslist( FILE *f, int *n, char separator );

void fgets_but_good( FILE *f, char *dest, int size );

void get_filenames( char *directory, char ***list, int *length );

// just replaces the last '/' with a '\0'
void up_one_folder( char *path );

Uint16 *ascii_to_unicode( char *str );

bool cursor_in_rect( SDL_Event *event, SDL_Rect *R );
bool coordinates_in_Rect( float x, float y, SDL_Rect *R );
bool coordinates_in_FRect( float x, float y, SDL_FRect *R );
bool SDL_FRect_overlap( SDL_FRect *A, SDL_FRect *B );
bool rect_overlap( int Ax, int Ay, int Aw, int Ah, int Bx, int By, int Bw, int Bh );
bool intersecting_or_touching( SDL_Rect *A, SDL_Rect *B);
SDL_Rect add_rects( SDL_Rect *A, SDL_Rect *B);
//scale and translate A to fit inside B, centralized
void fit_rect( SDL_Rect *A, SDL_Rect *B );


Uint32 char4_to_int( char str [4] );
void int_to_char4( Uint32 N, char str [4] );

int count_digits( int n );

bool signed_numeral( char c );
bool ascii_text( int c );
bool i_check( char c );
bool d_check( char c );
bool x_check( char c );
bool f_check( char c );

int bytes_in_a_utf_codepoint( uint8_t ch );
int retrobytes_in_a_utf_codepoint( const char *str );
uint32_t binary_code_point( int bytes, uint32_t key );
size_t utf8_strlen(const char *s);
uint32_t UTF8_to_UINT32( char *str, int *bytes, int endianness );
int UINT32_to_UTF8( char *str, uint32_t num, int endianness );

bool str_contains( char *str, bool(*cateorize)(char c) );
bool str_contains_only( char *str, bool(*cateorize)(char c) );

bool list_contains( int *list, int len, int x );
int find_in_list( int *list, int len, int x );

char shifted_keys( char c );

void print_string_with_escape_chars_visible( char *str, int N );
void hard_print( char *str, int N );
void hard_print_f( FILE *f, int N );
//printf("{%d, %d, %d, %d}\n", rect.x, rect.y, rect.w, rect.h );

//return elapsed time
int SDL_framerateDelay( int frame_period );

typedef struct {
    int i, j;
} index2d;


int rect_area( SDL_Rect *r );

typedef struct{
   int len, size;
   SDL_Rect *rcts;
   SDL_Rect original;
} rectCluster;

void rectCluster_init( rectCluster *rC, int x, int y, int w, int h );

void clip_rectCluster( rectCluster *rC, SDL_Rect cut );

int rectCluster_area( rectCluster *rC );


#endif

/*
☺☻♥♦♣
♫☼►◄↕‼¶§▬↨↑↓→←∟↔▲▼ !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~⌂ÇüéâäàåçêëèïîìÄÅÉæÆôöòûùÿÖÜø£Ø×ƒáíóúñÑªº¿®¬½¼¡«»░▒▓│┤ÁÂÀ©╣║╗╝¢¥┐└┴┬├─┼ãÃ╚╔╩╦╠═╬¤ðÐÊËÈıÍÎÏ┘┌█▄¦Ì▀ÓßÔÒõÕµþÞÚÛÙýÝ¯´­±‗¾¶§÷¸°¨·¹³²
*/