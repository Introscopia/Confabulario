#include "TX_Font.h"




TX_Font render_TX_Font( SDL_Renderer *R, char *font_filename, int ptsize, SDL_Color fg ){

	TTF_Font *font = TTF_OpenFont( font_filename, ptsize );

	TX_Font out;

	int len = strlen( font_filename );
	/*
	for( int i = len-1; i >= 0; i-- ){
		if( font_filename[i] == '/' ){
			out.name = malloc( len - i );
			strcpy( out.name, font_filename + i + 1 );
			break;
		}
	}*/
	//printf("hello, I'm a font, my name is %s\n", out.name );

	//char block [] = "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
	//SDL_Surface *surf = TTF_RenderText_Solid( font, block, fg );
	
	out.ptsize = ptsize;
	//out.x = malloc( 94 * sizeof(int) );
	//out.bx = malloc( 94 * sizeof(int) );
	//out.w = malloc( 94 * sizeof(int) );
	//out.adv = malloc( 94 * sizeof(int) );
	out.scale = 1;

	int s = 0;
	TTF_GlyphMetrics32( font, ' ', NULL, NULL, NULL, NULL, &s );
	out.space = s;
	out.ascent = TTF_FontAscent( font );
	out.descent = TTF_FontDescent( font );
	if( out.descent < 0 ) out.descent *= -1;
	//printf("font: %s, ascent: %d, descent: %d\n", font_filename, out.ascent, out.descent );
	out.line_skip = TTF_FontLineSkip( font );
	out.widest_char = 0;

	for (char i = '!'; i <= '~'; ++i){

		int minx, maxx, miny, maxy, advance;
		TTF_GlyphMetrics32( font, i, &minx, &maxx, &miny, &maxy, &advance );
		
		int I = i - '!';
		if( I == 0 ) out.x[0] = 0;
		else out.x[I] = out.x[I-1] + out.adv[I-1];
		//out.bx[I] = minx;
		out.adv[I] = advance;
		if( minx < 0 ) out.adv[I] -= minx;

		if( out.adv[I] > out.widest_char ) out.widest_char = out.adv[I];

		//printf("%c - minx:%d, maxx:%d, advance:%d. maxx-minx:%d, advance-minx:%d\n", i, minx, maxx, advance, maxx-minx, advance-minx );
		//printf("%d\n", out.w[I] == advance ); CONFIRMED
		//printf("height of %c: %d\n", i, out.h ); all chars have the same height confirmed
	}

	char txt[2] = "A";
	TTF_SizeText( font, txt, NULL, &(out.h) );

	int totalw = out.x[93] + out.adv[93];

	out.texture = SDL_CreateTexture( R, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, totalw, out.h );
	SDL_SetTextureBlendMode( out.texture, SDL_BLENDMODE_NONE );
	SDL_SetRenderTarget( R, out.texture );
	SDL_SetRenderDrawColor( R, 0, 0, 0, 0 );
    SDL_RenderClear( R );
	
	for( char i = '!'; i <= '~'; ++i ){
		txt[0] = i;
		SDL_Surface *surf = TTF_RenderText_Blended( font, txt, fg );
		//SDL_Surface *surf = TTF_RenderGlyph_Blended( font, i, fg );// _Solid
		SDL_Texture *glyph = SDL_CreateTextureFromSurface( R, surf );
		SDL_SetTextureBlendMode( glyph, SDL_BLENDMODE_NONE );
		//printf("%c->w: %d, adv: %d\n", i, surf->w, out.adv[i - '!'] );
		SDL_RenderCopy( R, glyph, NULL, &(SDL_Rect){ out.x[i - '!'], 0, surf->w, out.h } );
		SDL_DestroyTexture( glyph );
		//if( i == 'J') IMG_SavePNG( surf, "J.png" );
		SDL_FreeSurface( surf );
	}

	SDL_SetRenderTarget( R, NULL );
	

	SDL_SetTextureBlendMode( out.texture, SDL_BLENDMODE_BLEND );

	out.ttf = font;
	out.ttf2 = NULL;
	out.fg = fg;


	ok_map_init(&(out.trans_ascii_map));
	out.trans_ascii_size = 0;
	out.trans_ascii_textures = NULL;
	out.trans_ascii_x = NULL;
	out.trans_ascii_adv = NULL;
	out.trans_ascii_h = NULL;

	return out;
}

void destroy_TX_Font( TX_Font *font ){

	//free( font->name );
	SDL_DestroyTexture( font->texture );
	TTF_CloseFont( font->ttf );
	if( font->trans_ascii_size > 0 ){
		for (int i = 0; i < font->trans_ascii_size; ++i ){
			SDL_DestroyTexture( font->trans_ascii_textures[i] );
		}
		free( font->trans_ascii_textures );
		free( font->trans_ascii_x );
		free( font->trans_ascii_adv );
		free( font->trans_ascii_h );
	}
	ok_map_deinit(&(font->trans_ascii_map));
	//free( font->x );
	//free( font->bx );
	//free( font->w );
	//free( font->adv );
}


int TX_rendercopy_char( SDL_Renderer *R, TX_Font *font, char *string, float *x, float y ){

	//if( string[0] >= '!' && string[0] <= '~' ) putchar('.');
	//else putchar('?');
	//putchar('>');

	uint8_t value = (uint8_t)(string[0]);

	if( value < '!' ) return 1;

	if( value > 0x7F ){
		//printf("trans ascii.");
		int bytes = 0;
		uint32_t key = UTF8_to_UINT32( string, &bytes, SDL_BIG_ENDIAN );
		int id = ok_map_get(&(font->trans_ascii_map), key) -1;
		//printf("..%08X [%d] ", key, id );
		if( id < 0 ){
			uint32_t bcp = binary_code_point( bytes, key );
			//printf("bcp: %08X\n", bcp );
			TTF_Font *ttfp = font->ttf;
			try_trans_ascii:
			if( TTF_GlyphIsProvided32( ttfp, bcp ) ){
				//puts("Glyph is provided");

				id = font->trans_ascii_size;
				//printf("putting new trans_ascii char: [%d], key: %08X, bcp: %X \n", id, key, bcp );
				ok_map_put( &(font->trans_ascii_map), key, id+1 );

				font->trans_ascii_size += 1;
				font->trans_ascii_textures = realloc( font->trans_ascii_textures, font->trans_ascii_size * sizeof(SDL_Texture*) );
				font->trans_ascii_x = realloc( font->trans_ascii_x, font->trans_ascii_size * sizeof(int) );
				font->trans_ascii_adv = realloc( font->trans_ascii_adv, font->trans_ascii_size * sizeof(int) );
				font->trans_ascii_h = realloc( font->trans_ascii_h, font->trans_ascii_size * sizeof(int) );

				char codepoint [5];
				memcpy( codepoint, string, bytes );
				codepoint[bytes] = '\0';				
				SDL_Surface *surf = TTF_RenderUTF8_Blended( ttfp, codepoint, font->fg );
				int minx, maxx, miny, maxy, advance;
				TTF_GlyphMetrics32( ttfp, bcp, &minx, &maxx, &miny, &maxy, &advance );
				//printf("minx: %d, maxx: %d, miny: %d, maxy: %d, advance: %d, surf->w: %d\n", minx, maxx, miny, maxy, advance, surf->w );
				font->trans_ascii_adv[id] = advance;//surf->w;
				if( minx < 0 ) font->trans_ascii_adv[id] -= minx;
				if( font->trans_ascii_adv[id] > font->widest_char ) font->widest_char = font->trans_ascii_adv[id];
				font->trans_ascii_x[id] = minx;
				font->trans_ascii_h[id] = surf->h;
				font->trans_ascii_textures[id] = SDL_CreateTextureFromSurface( R, surf );
				SDL_FreeSurface( surf );
				SDL_SetTextureBlendMode( font->trans_ascii_textures[id], SDL_BLENDMODE_BLEND );
			}
			else{
				if( font->ttf2 != NULL && ttfp != font->ttf2 ){
					ttfp = font->ttf2;
					goto try_trans_ascii;
				}
				//puts("Glyph not provided");
				int I = '?' - '!';
				SDL_Rect src = (SDL_Rect){ font->x[I], 0, font->adv[I], font->h };//+ (font->bx[I] * font->scale)
				SDL_FRect dst = (SDL_FRect){ *x, y, font->scale * src.w, font->scale * src.h };
				SDL_RenderCopyF( R, font->texture, &src, &dst );
				*x += dst.w;
				return bytes;
			}
		}
		//                         font->trans_ascii_x[id]
		SDL_Rect src = (SDL_Rect){ 0, 0, font->trans_ascii_adv[id], font->trans_ascii_h[id] };
		SDL_FRect dst = (SDL_FRect){ *x, y, //- font->scale * font->trans_ascii_x[id]
								   font->scale * font->trans_ascii_adv[id], 
								   font->scale * font->trans_ascii_h[id]   };
		SDL_RenderCopyF( R, font->trans_ascii_textures[id], &src, &dst );
		//printf("*x += %d, return %d\n", dst.w, bytes );
		*x += dst.w;
		return bytes;
	}
	else{
		int I = string[0] - '!';
		SDL_Rect src = (SDL_Rect){ font->x[I], 0, font->adv[I], font->h };//+ (font->bx[I] * font->scale)
		SDL_FRect dst = (SDL_FRect){ *x, y, font->scale * src.w, font->scale * src.h };
		SDL_RenderCopyF( R, font->texture, &src, &dst );
		*x += dst.w;
		return 1;
	}
}

float TX_glyph_width( TX_Font *font, char *string, int *bytes ){
	uint8_t value = (uint8_t)(string[0]);
	if( value > 0x7F ){
		uint32_t key = UTF8_to_UINT32( string, bytes, SDL_BIG_ENDIAN );
		int id = ok_map_get(&(font->trans_ascii_map), key) -1;
		if( id < 0 ){
			if( TTF_GlyphIsProvided32( font->ttf, key ) ){
				int minx, maxx, miny, maxy, advance;
				TTF_GlyphMetrics32( font->ttf, key, &minx, &maxx, &miny, &maxy, &advance );
				if( minx < 0 ) advance -= minx;
				return font->scale * advance;
			}
			else{
				return font->scale * font->adv[ '?' - '!' ];
			}
		}
		return font->scale * font->trans_ascii_adv[id];
	}
	else if( string[0] < '!' ){
		*bytes = 1;
		if( string[0] == '\t' ) return TAB_SIZE * (font->scale * font->space);
		else if( string[0] == ' '  ) return font->scale * font->space;
		return 0;
	}
	else{
		*bytes = 1;
		return font->scale * font->adv[ string[0] - '!' ];
	}
}


void TX_render_glyph( SDL_Renderer *R, TX_Font *font, char c, float x, float y, float s ){
	int I =  c - '!';
	SDL_Rect src = (SDL_Rect){ font->x[I], 0, font->adv[I], font->h };
	SDL_Rect dst = (SDL_Rect){ lrint(x - (0.5 * s * src.w)), lrint(y - (0.5 * s * src.h)), ceil(s * src.w), ceil(s * src.h) };
	SDL_RenderCopy( R, font->texture, &src, &dst );
}


void TX_render_string( SDL_Renderer *R, TX_Font *font, char *string, float x, float y ){
	int len = strlen( string );
	int space = font->scale * font->space;
	float ox = x;
	for( int i = 0; i < len; ){
		if( string[i] == '\n' ){
			y += font->line_skip * font->scale;
			x = ox;
			i++;
			continue;
		}
		else if( string[i] == '\t' ){
			x += (TAB_SIZE*space);
			i++;
			continue;
		}
		if( (uint8_t)(string[i]) < '!' ){ //|| string[i] > '~'
			x += space;
			i++;
			continue;
		}
		i += TX_rendercopy_char( R, font, string + i, &x, y );
	}
}


//printf("j: %d, %c(%2X), bytes: %d\n", j, string[j], string[j], bytes );
//!isalpha( string[j] )
// previous delimiter check:  
#define TOKENIZE()  for( j = i; j < len; ){                                  \
						if( (uint8_t)(string[j]) < '!' ){                    \
							break;                                           \
						}                                                    \
						int bytes = 0;                                       \
						float gw = TX_glyph_width( font, string+j, &bytes ); \
						ww += gw;                                            \
						if( ww > width ){                                    \
							ww -= gw;                                        \
							j -= bytes;                                      \
							break;                                           \
						}                                                    \
						j += bytes;                                          \
					}                                                        \


void TX_render_string_centered( SDL_Renderer *R, TX_Font *font, char *string, double x, double y ){
	float h;
	TX_SizeText( font, string, NULL, &h );
	//TX_render_string( R, font, string, x - ceil(w*0.5), y - ceil(h*0.5) );*/

	int len = strlen( string );
	int cy = y - rint(0.5*h);
	int line_height = font->line_skip * font->scale;
	int space = font->scale * font->space;
	int ls = 0;
	float lw = 0;
	
	for ( int i = 0; i < len; ++i ){

		if( isgraph( string[i] ) || (uint8_t)(string[i]) > 0x7F ){
			int bytes = 0;
			lw += TX_glyph_width( font, string+i, &bytes );
			i += (bytes-1);
		}
		else if( string[i] == '\t' ){
			lw += (TAB_SIZE*space);
		}
		else if( string[i] == '\n' ){
			TX_render_section( R, font, string, ls, i, x -(0.5 * lw), cy );
			lw = 0;
			ls = i+1;
			cy += line_height;
		}
		else if( (uint8_t)(string[i]) < '!' ){//string[i] == ' ' || 
			lw += space;
		}
		else{
			break;
		}
	}
	if( lw > 0 ){
		TX_render_section( R, font, string, ls, len, x -(0.5 * lw), cy );
	}
}


void TX_render_section( SDL_Renderer *R, TX_Font *font, char *string, int start, int end, float x, float y ){
	float ox = x;
	for( int i = start; i < end; ){
		if( string[i] == '\0' ) return;
		
		if( (uint8_t)(string[i]) < '!' ){
			if( string[i] == '\n' ){
				y += font->line_skip * font->scale;
				x = ox;
			}
			else if( string[i] == '\t' ){
				x += TAB_SIZE * font->scale * font->space;
			}
			else{
				x += font->scale * font->space;
			}
			i++;
		}
		else{
			i += TX_rendercopy_char( R, font, string + i, &x, y );
		}
	}
}

bool tightly_bound( char c ){
	if( isalpha(c) ) return true;          // latin alphabet
	if( (uint8_t)c > 0x7F ) return true;   // UTF8 
	const char binders [] = "\"'!?,.:;_";  // some hand-picked word-binders. 
	for (int i = 0; binders[i] != '\0'; ++i ){
		if( c == binders[i] ) return true;
	}
	return false;
}

int TX_wrap_line( TX_Font *font, char *string, int start, float width, float *line_width ){
	
	float lw = 0;// line width
	int i = start;
	while( string[i] != '\0' ){
		float tw = 0;// token width
		int j = i;
		if( tightly_bound( string[i] ) ){
			do{
				int bytes = 0;
				float gw = TX_glyph_width( font, string+j, &bytes );
				tw += gw;
				if( tw > width ){
					do{
						tw -= gw;
						do{ j -= 1; } while( j > start && (uint8_t)(string[j-1]) > 0x7F );
						gw = TX_glyph_width( font, string+j, &bytes );
					} while( lw + tw > width );
					lw += tw;
					if( line_width != NULL ) *line_width = lw;
					return j;
				}
				j += bytes;
			} while( isalpha(string[j]) || (uint8_t)(string[j]) > 0x7F );
		}
		else {
			int bytes = 0;
			tw = TX_glyph_width( font, string+i, &bytes );
			j += bytes;
		}

		if( lw + tw > width || string[i] == '\n' ){
			if( line_width != NULL ) *line_width = lw;
			//string[i] == ' ' || string[i] == '\t' || string[i] == '\n'
			if( (uint8_t)(string[i]) < '!' ) i += 1; //spaces and tabs disappear in the line breaks
			return i;
		}
		else lw += tw;

		i = j;
	}
	if( line_width != NULL ) *line_width = lw;
	return i;
}

void TX_render_string_wrapped( SDL_Renderer *R, TX_Font *font, char *string, float x, float y, float width ){

	int len = strlen( string );
	float cy = y;
	float line_height = font->line_skip * font->scale;

	if( width < font->scale * font->widest_char ){
		puts("TX_render_string_wrapped: umm.. that's a really narrow textbox you got there...");
		return;
	}

	int i = 0;
	do{
		int j = TX_wrap_line( font, string, i, width, NULL );
		if( i == j ) break;
		TX_render_section( R, font, string, i, j, x, cy );
		cy += line_height;
		i = j;
	} while( i < len );
}

float TX_whitespace( TX_Font *font, char *string, int start, int end ){
	float ws = 0;
	bool begun = 0;
	for (int i = end-1; i >= start; --i ){
		if( begun ){
			if( string[i] == '\t' ) ws += TAB_SIZE * font->scale * font->space;
			else if( string[i] == ' '  ) ws += font->scale * font->space;
		}
		else if( !isblank(string[i]) ){
			begun = 1;
		}
	}
	return ws;
}

void TX_render_string_wrapped_aligned( SDL_Renderer *R, TX_Font *font, char *string, int x, int y, int width, int alignment ){

	if( alignment == TX_ALIGN_LEFT ){
		return TX_render_string_wrapped( R, font, string, x, y, width );
	}

	int len = strlen( string );
	float cy = y;
	float line_height = font->line_skip * font->scale;

	if( width < font->scale * font->widest_char ){
		puts("TX_render_string_wrapped_aligned: umm.. that's a really narrow textbox you got there...");
		return;
	}

	int i = 0;
	do{
		float lw = 0;
		int j = TX_wrap_line( font, string, i, width, &lw );
		if( i == j ) break;

		switch( alignment ){
			case TX_ALIGN_CENTER:
				TX_render_section( R, font, string, i, j, round(x + (0.5 * (width - lw))), cy );
				break;
			case TX_ALIGN_RIGHT:
				TX_render_section( R, font, string, i, j, round(x + width - lw), cy );
				break;
			case TX_JUSTIFY:;
				float s = font->space;
				float ws = TX_whitespace( font, string, i, j );
				font->space = round( font->space * ((width -lw + ws) / ws) );
				TX_render_section( R, font, string, i, j, x, cy );
				font->space = s;
				break;
		}
		cy += line_height;
		i = j;
	} while( i < len );
}

void TX_render_wrapped_section( SDL_Renderer *R, TX_Font *font, char *string, int stop, int x, int y, int width, int alignment ){
	
	int len = strlen( string );
	float cy = y;
	float line_height = font->line_skip * font->scale;

	if( width < font->scale * font->widest_char ){
		puts("TX_render_string_wrapped_aligned: umm.. that's a really narrow textbox you got there...");
		return;
	}

	int i = 0;
	do{
		float lw = 0;
		int j = TX_wrap_line( font, string, i, width, &lw );
		if( i == j ) break;

		if( j > stop ) j = stop;

		switch( alignment ){
			case TX_ALIGN_LEFT:
				TX_render_section( R, font, string, i, j, x, cy );
				break;
			case TX_ALIGN_CENTER:
				TX_render_section( R, font, string, i, j, round(x + (0.5 * (width - lw))), cy );
				break;
			case TX_ALIGN_RIGHT:
				TX_render_section( R, font, string, i, j, round(x + width - lw), cy );
				break;
			case TX_JUSTIFY:;
				float s = font->space;
				float ws = TX_whitespace( font, string, i, j );
				font->space = round( font->space * ((width -lw + ws) / ws) );
				TX_render_section( R, font, string, i, j, x, cy );
				font->space = s;
				break;
		}

		if( j == stop ) break;

		cy += line_height;
		i = j;
	} while( i < len );
}

void TX_cursor_after( TX_Font *font, char *string, int stop, float *x, float *y ){
	if( x != NULL ){
		*x = 0;
		float space = font->scale * font->space;
		for ( int i = 0; i < stop && string[i] != '\0'; ){
			int bytes = 1;
			if( string[i] == '\n' ){
				*x = 0;
			}
			else if( (uint8_t)(string[i]) < '!' ){//|| string[i] > '~' 
				*x += space;
			}
			else{
				*x += TX_glyph_width( font, string+i, &bytes );//font->scale * font->adv[ string[i] - '!' ];
			}
			i += bytes;
		}
	}
	if( y != NULL ){
		int lines = 0;
		for ( int i = 0; string[i] != '\0'; ++i ){
			if( string[i] == '\n' ){
				lines++;
			}
		}
		*y = lines * font->scale * font->line_skip;
	}
}

void TX_cursor_after_wrapped_aligned( TX_Font *font, char *string, int stop, int width, float *x, float *y, int alignment ){
	int len = strlen( string );
	if( stop > len ) stop = len;
	float cy = 0;
	float line_height = font->line_skip * font->scale;

	if( width < font->scale * font->widest_char ){
		puts("TX_cursor_after_wrapped_aligned: umm.. that's a really narrow textbox you got there...");
		return;
	}

	int i = 0;
	do{
		float lw = 0;
		int j = TX_wrap_line( font, string, i, width, &lw );
		
		if( j > stop ){

			if( x != NULL ){

				float sw = 0;
				if( alignment != TX_JUSTIFY ){
					TX_SizeTextUntil( font, string + i, stop-i, &sw, NULL );
				}

				switch( alignment ){
					case TX_ALIGN_LEFT:
						*x = sw;
						break;
					case TX_ALIGN_CENTER:
						*x = sw + round(0.5 * (width - lw));
						break;
					case TX_ALIGN_RIGHT:
						*x = sw + round(width - lw);
						break;
					case TX_JUSTIFY:;
						float s = font->space;
						float ws = TX_whitespace( font, string, i, j );
						font->space = round( font->space * ((width -lw + ws) / ws) );
						TX_SizeTextUntil( font, string + i, stop-i, &sw, NULL );
						font->space = s;
						*x = sw;
						break;
				}
			}

			if( y != NULL ){
				*y = cy;
			}

			return;
		}

		cy += line_height;
		i = j;
	} while( i < len );	
}

// returns index of the longest line
int TX_SizeText( TX_Font *font, char *string, float *w, float *h ){
	int ll = 0;
	if( w != NULL ){
		*w = 0;
		float lw = 0;
		float maxw = 0;
		float space = font->scale * font->space;
		int l = 0;
		for ( int i = 0; string[i] != '\0'; ){
			int bytes = 1;
			if( (uint8_t)(string[i]) < '!' ){
				if( string[i] == '\n' ){
					if( lw > maxw ){
						maxw = lw;
						ll = l;
					}
					l++;
					lw = 0;
				}
				if( string[i] == '\t' ){
					lw += TAB_SIZE*space;
				}else{
					lw += space;
				}
			}
			else{
				lw += TX_glyph_width( font, string+i, &bytes );//font->scale * font->adv[ string[i] - '!' ];
			}
			i += bytes;//_in_a_utf_codepoint( (uint8_t)(string[i]) );
		}
		if( lw > maxw ){
			maxw = lw;
			ll = l;
		}
		*w = maxw;
	}
	if( h != NULL ){
		int lines = 1;
		for ( int i = 0; string[i] != '\0'; ++i ){
			if( string[i] == '\n' ){
				lines++;
			}
		}
		*h = lines * font->scale * font->line_skip;
	}
	return ll;
}

void TX_SizeTextUntil( TX_Font *font, char *string, int stop, float *w, float *h ){
	if( w != NULL ){
		*w = 0;
		float max = 0;
		float space = font->scale * font->space;
		for ( int i = 0; i < stop; ){
			int bytes = 1;
			if( string[i] == '\0' ) break;
			if( string[i] == '\n' ){
				if( *w > max ) max = *w;
				*w = 0;
			}
			else if( (uint8_t)(string[i]) < '!' ){// || string[i] > '~'
				*w += space;
			}
			else{
				*w += TX_glyph_width( font, string+i, &bytes );//font->scale * font->adv[ string[i] - '!' ];
			}
			i += bytes;
		}
		if( max > *w ) *w = max;
	}
	if( h != NULL ){
		int lines = 1;
		for ( int i = 0; i < stop; ++i ){
			if( string[i] == '\0' ) break;
			if( string[i] == '\n' ){
				lines++;
			}
		}
		*h = lines * font->scale * font->line_skip;
	}
}


float TX_wrapped_string_height( TX_Font *font, char *string, int width ){
	int len = strlen( string );
	int h = 0;

	if( width < font->scale * font->widest_char ){
		puts("TX_wrapped_string_height: umm.. that's a really narrow textbox you got there...");
		return;
	}

	int i = 0;
	do{
		float lw = 0;
		int j = TX_wrap_line( font, string, i, width, &lw );
		if( i == j ) break;
		h += 1;
		i = j;
	} while( i < len );
	
	return h * font->line_skip * font->scale;
}


static void append( int **list, int *len, int *size, int i ){
	*len += 1;
	if( *len >= *size ){
		*size *= 2;
		*list = realloc( *list, (*size) * sizeof(int) );
	}
	(*list)[ *len ] = i;
}
/*
	cx = 0;
	*lines += 1;
	if( *lines >= size ){
		size *= 2;
		indices = realloc( indices, size * sizeof(int) );
	}
	indices[ *lines ] = i;*/


int *TX_wrapping_indices( TX_Font *font, char *string, int width, int *lines ){
	int len = strlen( string );
	int cx = 0;//cursor
	*lines = 0;
	int space = font->scale * font->space;

	int size = ceil( 3 * ( (len * space) / ((float)width) ) );
	if( size <= 0 ) size = 1;
	int *indices = malloc( size * sizeof(int) );
	indices[0] = 0;

	for ( int i = 0; i < len; ){
		float ww = 0;//word width int we = 0;//word end
		int j;
		TOKENIZE();
		if( cx + ww > width ){
			cx = 0;
			append( &indices, lines, &size, i );
		}

		cx += ww;
		i = j;

		while( !( isalnum( string[i] ) || (uint8_t)(string[i]) > 0x7F ) ){
			if( string[i] == ' ' ){
				if( cx + space > width ){
					cx = 0;
					append( &indices, lines, &size, i+1 );
				}
				else cx += space;
			}
			else if( string[i] == '\t' ){
				if( cx + (TAB_SIZE*space) > width ){
					cx = 0;
					append( &indices, lines, &size, i+1 );
				}
				else cx += (TAB_SIZE*space);
			}
			else if( string[i] == '\n' ){
				cx = 0;
				append( &indices, lines, &size, i+1 );
			}
			else if( (uint8_t)(string[i]) < '!' ){// || string[i] > '~'
				if( cx + space > width ){
					cx = space;
					append( &indices, lines, &size, i );
				}
				else cx += space;
			}
			else{
				break;
			}
			++i;
			if( i >= len ) break;
		}
	}

	*lines += 1;
	indices = realloc( indices, (*lines) * sizeof(int) );
	return indices;
}




int TX_rendercopyExF_char( SDL_Renderer *R, TX_Font *font, char *string, double x, double y, double angle ){

	//printf("<%c>", string[0] );
	//if( string[0] >= '!' && string[0] <= '~' ) putchar('.');
	//else putchar('?');
	//putchar('>');

	uint8_t value = (uint8_t)(string[0]);

	if( value < '!' ) return 1;

	if( value > 0x7F ){
		//printf("trans ascii.");
		int bytes = 0;
		uint32_t key = UTF8_to_UINT32( string, &bytes, SDL_BIG_ENDIAN );
		int id = ok_map_get(&(font->trans_ascii_map), key) -1;
		//printf("..%08X [%d] ", key, id );
		if( id < 0 ){
			uint32_t bcp = binary_code_point( bytes, key );
			//printf("bcp: %08X\n", bcp );
			TTF_Font *ttfp = font->ttf;
			try_trans_ascii:
			if( TTF_GlyphIsProvided32( ttfp, bcp ) ){
				//puts("Glyph is provided");

				id = font->trans_ascii_size;
				//printf("putting new trans_ascii char: [%d] = %08X\n", id, key );
				ok_map_put( &(font->trans_ascii_map), key, id+1 );

				font->trans_ascii_size += 1;
				font->trans_ascii_textures = realloc( font->trans_ascii_textures, font->trans_ascii_size * sizeof(SDL_Texture*) );
				font->trans_ascii_x = realloc( font->trans_ascii_x, font->trans_ascii_size * sizeof(int) );
				font->trans_ascii_adv = realloc( font->trans_ascii_adv, font->trans_ascii_size * sizeof(int) );
				font->trans_ascii_h = realloc( font->trans_ascii_h, font->trans_ascii_size * sizeof(int) );

				char codepoint [5];
				memcpy( codepoint, string, bytes );
				codepoint[bytes] = '\0';				
				SDL_Surface *surf = TTF_RenderUTF8_Blended( ttfp, codepoint, font->fg );
				int minx, maxx, miny, maxy, advance;
				TTF_GlyphMetrics32( ttfp, bcp, &minx, &maxx, &miny, &maxy, &advance );
				//printf("minx: %d, maxx: %d, miny: %d, maxy: %d, advance: %d\n", minx, maxx, miny, maxy, advance );
				font->trans_ascii_adv[id] = advance;//surf->w;
				if( minx < 0 ) font->trans_ascii_adv[id] -= minx;
				if( font->trans_ascii_adv[id] > font->widest_char ) font->widest_char = font->trans_ascii_adv[id];
				font->trans_ascii_x[id] = minx;
				font->trans_ascii_h[id] = surf->h;
				font->trans_ascii_textures[id] = SDL_CreateTextureFromSurface( R, surf );
				SDL_FreeSurface( surf );
				SDL_SetTextureBlendMode( font->trans_ascii_textures[id], SDL_BLENDMODE_BLEND );
			}
			else{
				if( font->ttf2 != NULL && ttfp != font->ttf2 ){
					ttfp = font->ttf2;
					goto try_trans_ascii;
				}
				//puts("Glyph not provided");
				int I = '?' - '!';
				SDL_Rect src = (SDL_Rect){ font->x[I], 0, font->adv[I], font->h };//+ (font->bx[I] * font->scale)
				SDL_FRect dst = (SDL_FRect){ x, y, font->scale * src.w, font->scale * src.h };
				//SDL_Point center = (SDL_Point){ rintf(dst.w * 0.5), rintf(dst.h) };
				dst.x -= dst.w * 0.5; dst.y -= dst.h * 0.5;
				SDL_RenderCopyExF( R, font->texture, &src, &dst, degrees(angle), NULL, SDL_FLIP_NONE );
				return bytes;
			}
		}
		//                         font->trans_ascii_x[id]
		SDL_Rect src = (SDL_Rect){ 0, 0, font->trans_ascii_adv[id], font->trans_ascii_h[id] };
		SDL_FRect dst = (SDL_FRect){ x, y, // - font->scale * font->trans_ascii_x[id]
									 font->scale * font->trans_ascii_adv[id], 
									 font->scale * font->trans_ascii_h[id]   };
		//SDL_Point center = (SDL_Point){ rintf(dst.w * 0.5), rintf(dst.h) };
		dst.x -= dst.w * 0.5; dst.y -= dst.h * 0.5;
		SDL_RenderCopyExF( R, font->trans_ascii_textures[id], &src, &dst, degrees(angle), NULL, SDL_FLIP_NONE );
		//printf("*x += %d, return %d\n", dst.w, bytes );
		return bytes;
	}
	else{
		int I = string[0] - '!';
		SDL_Rect src = (SDL_Rect){ font->x[I], 0, font->adv[I], font->h };//+ (font->bx[I] * font->scale)
		SDL_FRect dst = (SDL_FRect){ x, y, font->scale * src.w, font->scale * src.h };
		//SDL_Point center = (SDL_Point){ rintf(dst.w * 0.5), rintf(dst.h) };
		dst.x -= dst.w * 0.5; dst.y -= dst.h * 0.5;
		SDL_RenderCopyExF( R, font->texture, &src, &dst, degrees(angle), NULL, SDL_FLIP_NONE );
		return 1;
	}
}


void TX_render_line_rotated( SDL_Renderer *R, TX_Font *font, char *string, double x, double y, 
						   int lw, int l, double angle, double cosa, double sina ){

	double cx = x - (cosa * lw * 0.5) + (l * font->scale * font->line_skip * cos( angle + HALF_PI ));
	double cy = y - (sina * lw * 0.5) + (l * font->scale * font->line_skip * sin( angle + HALF_PI ));

	//printf("ls: %d, i: %d\n", ls, i );
	for( int j = 0; string[j] != '\n' && string[j] != '\0'; ){//for (int j = ls; j < i; ){
		if( (uint8_t)(string[j]) < '!' ){
			if( string[j] == '\t' ){
				cx += cosa * TAB_SIZE * font->scale * font->space;
				cy += sina * TAB_SIZE * font->scale * font->space;
				j++;
			}
			else{
				cx += cosa * font->scale * font->space;
				cy += sina * font->scale * font->space;
				j++;
			}
		}
		else{
			int bytes = 0;
			double gw = TX_glyph_width( font, string + j, &bytes );
			gw *= 0.5;
			cx += cosa * gw;
			cy += sina * gw;
			j += TX_rendercopyExF_char( R, font, string + j, cx, cy, angle );
			//SDL_RenderFillRect( R, &(SDL_Rect){ cx, cy, 4, 4 } );
			//j += bytes;
			cx += cosa * gw;
			cy += sina * gw;
		}
	}
}

void TX_render_string_rotated( SDL_Renderer *R, TX_Font *font, char *string, double x, double y, double angle ){
	angle += HALF_PI;
	if( fabs( angle ) < FLT_EPSILON ){
		TX_render_string_centered( R, font, string, x, y );
	}
	int len = strlen( string );
	float space = font->scale * font->space;
	double cosa = cos( angle );
	double sina = sin( angle );
	int ls = 0;
	int lw = 0;
	int l = - floor( strcchr( string, '\n' ) * 0.5 );

	for( int i = 0; i < len; i++ ){
		//putchar(string[i]);
		if( isgraph( string[i] ) || (uint8_t)(string[i]) > 0x7F ){
			int bytes = 0;
			lw += TX_glyph_width( font, string+i, &bytes );
			i += (bytes-1);
		}
		else if( string[i] == '\n' ){

			TX_render_line_rotated( R, font, string + ls, x, y, lw, l, angle, cosa, sina );
			l++;
			lw = 0;
			ls = i+1;
		}
		else if( string[i] == '\t' ){
			lw += (TAB_SIZE*space);
		}
		else if( string[i] < '!' ){
			lw += space;
		}
		else{
			break;
		}
	}
	//puts("");
	if( lw > 0 ){
		TX_render_line_rotated( R, font, string + ls, x, y, lw, l, angle, cosa, sina );
	}
}


void TX_render_line_curved( SDL_Renderer *R, TX_Font *font, char *string, int lw, 
							double cx, double cy, double angle, double radius ){

	double oorad = 1.0 / radius;
	double ca = angle - ((lw * 0.5) * oorad);//angular cursor
	
	for( int j = 0; string[j] != '\n' && string[j] != '\0'; ){//for( int j = 0; j < len; ){
		if( (uint8_t)(string[j]) < '!' ){
			if( string[j] == '\t' ){
				ca +=  TAB_SIZE * font->scale * font->space * oorad;
				j++;
			}
			else{
				ca += font->scale * font->space * oorad;
				j++;
			}
		}
		else{
			int bytes = 0;
			double gw = TX_glyph_width( font, string + j, &bytes );
			gw *= 0.5 * oorad;
			ca += gw;
			j += TX_rendercopyExF_char( R, font, string + j, 
										cx + radius * cos(ca), 
										cy + radius * sin(ca), 
										ca + HALF_PI );
			ca += gw;
		}
	}
}

void TX_render_string_curved( SDL_Renderer *R, TX_Font *font, char *string, double x, double y, 
							  double angle, double curvature ){
	
	if( fabs(curvature) < FLT_EPSILON ){
		if( fabs( angle + HALF_PI ) < FLT_EPSILON ){
			TX_render_string_centered( R, font, string, x, y );
		}
		else TX_render_string_rotated( R, font, string, x, y, angle );

		return;
	}
	//angle += HALF_PI;

	int len = strlen( string );
	float space = font->scale * font->space;
	int ln = strcchr( string, '\n' ) + 1;
	float tw, th;
	int ll = TX_SizeText( font, string, &tw, &th );
	double radius = (tw / curvature) +((ll+0.5) * font->scale * font->line_skip) - (0.5 * th);
	//printf("tw: %d, th: %d, ll: %d, rad: %lg\n", tw, th, ll, radius );
	double cx = x + radius * cos( angle + PI );
	double cy = y + radius * sin( angle + PI );
	radius = (tw / curvature) + (ll* font->scale * font->line_skip);
	int ls = 0;
	int lw = 0;
	int l = 0;

	for( int i = 0; i < len; i++ ){
		//putchar(string[i]);
		if( isgraph( string[i] ) || (uint8_t)(string[i]) > 0x7F ){
			int bytes = 0;
			lw += TX_glyph_width( font, string+i, &bytes );
			i += (bytes-1);
		}
		else if( string[i] == '\n' ){

			TX_render_line_curved( R, font, string + ls, lw, cx, cy, angle, radius );
			l++;
			radius -= (font->scale * font->line_skip);
			lw = 0;
			ls = i+1;
		}
		else if( string[i] == '\t' ){
			lw += (TAB_SIZE*space);
		}
		else if( string[i] < '!' ){
			lw += space;
		}
		else{
			break;
		}
	}
	//puts("");
	if( lw > 0 ){
		TX_render_line_curved( R, font, string + ls, lw, cx, cy, angle, radius );
	}
}



void init_typist( Typist *T, char *string, int period, SDL_Color *c ){

	T->str = string;
	T->len = strlen( string );
	T->I = 0;
	T->P = period-1;
	T->fc = T->P;
	T->cursor_color = c;
}

void reinit_typist( Typist *T, char *string, int period ){

	T->str = string;
	T->len = strlen( string );
	T->I = 0;
	T->P = period;
	T->fc = period;
}

void clear_typist( Typist *T ){
	T->str = NULL;
	T->len = 0;
	T->I = 0;	
}

bool typing_done( Typist *T ){
	return T->I >= T->len;
}

void typist_step( Typist *T ){
	if( T->fc <= 0 ){
		do{
			int bytes = bytes_in_a_utf_codepoint( T->str[ T->I ] );
			T->I += bytes;			
			if( T->I >= T->len ){
				T->I = T->len;
				break;
			}
		} while( T->str[ T->I ] == '\n' );

		T->fc = T->P;
	}
	else T->fc -= 1;
}

void render_typist( SDL_Renderer *R, TX_Font *font, Typist *T, float X, float Y, int w, int alignment ){

	if( w <= 0 ){
		TX_render_section(  R, font, T->str, 0, T->I, X, Y );
	}else{
		TX_render_wrapped_section( R, font, T->str, T->I, X, Y, w, alignment );
	}

	if( T->I < T->len ){
		float x = 0, y = 0;
		if( w <= 0 ){
			TX_cursor_after( font, T->str, T->I, &x, &y );
		}else{
			TX_cursor_after_wrapped_aligned( font, T->str, T->I, w, &x, &y, alignment );
		}
		int bytes  = 0;
		float adv = TX_glyph_width( font, T->str + T->I, &bytes );
		if( adv > 0 ){
			SDL_SetRenderDraw_SDL_Color( R, T->cursor_color );
			SDL_RenderFillRectF( R, &(SDL_FRect){ X+x, Y+y, adv, font->scale * font->h } );
		}
	}

	typist_step( T );
}