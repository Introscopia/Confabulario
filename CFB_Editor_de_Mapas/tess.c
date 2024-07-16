#include "tess.h"
#include "ok_lib.h"


static inline bool v2d_in_rect( vec2d V, SDL_Rect R ){
	return ( V.x > R.x && V.x < R.x + R.w ) && (V. y > R.y && V.y < R.y + R.h );
}

Wcoord wc(int w0, int w1, int w2, int w3){ 
	Wcoord out;
	out.w[0] = w0; 
	out.w[1] = w1; 
	out.w[2] = w2; 
	out.w[3] = w3;
	return out;
}
Wcoord wc_sum( Wcoord A, Wcoord B ){
	Wcoord out;
	for (int i = 0; i < 4; i++) {
		out.w[i] = A.w[i] + B.w[i];
	}
	return out;
}
Wcoord wc_plus_warr( int *A, Wcoord B ){
	Wcoord out;
	for (int i = 0; i < 4; i++) {
		out.w[i] = A[i] + B.w[i];
	}
	return out;
}
Wcoord wc_scaled( int *A, int k ) {
	Wcoord out;
	for (int i = 0; i < 4; i++) {
		out.w[i] = A[i] * k;
	}
	return out;
}
vec2d wc_to_v2d( Wcoord A ){
	return v2d( A.w[0] + 0.5 * SQRT3 * A.w[1] + 0.5 * A.w[2], 
				0.5 * A.w[1] + 0.5 * SQRT3 * A.w[2] + A.w[3] );
}
vec2d warr_to_v2d( int *A ){
	return v2d( A[0] + 0.5 * SQRT3 * A[1] + 0.5 * A[2], 
				0.5 * A[1] + 0.5 * SQRT3 * A[2] + A[3] );
}
void sprint_wc( Wcoord A, char *buf ){
	sprintf( buf, "%d,%d,%d,%d", A.w[0], A.w[1], A.w[2], A.w[3] );
}



bool load_tess_as_nodemap( char *code, vec2d **points, int16_t ***connections, int *N, int max_cnx, 
						   SDL_Rect dest, float edge ){

	FILE *f = fopen("Assets/tesselations.yaml", "rb");
	if( f != NULL ){
		char buf [128];
		sprintf( buf, "- name: %s\n", code );
		if( fseek_string( f, buf ) ){

			Tess TT;
			fseek_string( f, "T1: [" );
			for (int i = 0; i < 4; ++i ){
				fscanf( f, "%d, ", TT.T1 + i );
			}
			fseek_string( f, "T2: [" );
			for (int i = 0; i < 4; ++i ){
				fscanf( f, "%d, ", TT.T2 + i );
			}
			fseek_string( f, "  seed:\n" );
			long int seed_pos = ftell( f );
			TT.seed_count = 0;
			while(1){
				char c = getgc( f );
				if( c == '-' ) break;
				TT.seed_count += 1;
				fseek_lines( f, 1 );
			}
			fseek( f, seed_pos, SEEK_SET );
			TT.seed = malloc( TT.seed_count * sizeof(int*) );
			for (int i = 0; i < TT.seed_count; ++i ){
				TT.seed[i] = malloc(4 * sizeof(int));
				fseek_string( f, "[" );
				//for (int j = 0; j < 4; ++j ) fscanf( f, "%d", TT.seed[i] + j );
				fscanf( f, "%d, %d, %d, %d]", TT.seed[i] + 0, TT.seed[i] + 1, TT.seed[i] + 2, TT.seed[i] + 3 );
			}
			
			/*
			printf("name: %s\n", code);
			printf("T1: %d, %d, %d, %d\n", TT.T1[0], TT.T1[1], TT.T1[2], TT.T1[3] );
			printf("T2: %d, %d, %d, %d\n", TT.T2[0], TT.T2[1], TT.T2[2], TT.T2[3] );
			printf("TT.seed_count: %d\n", TT.seed_count );
			for (int i = 0; i < TT.seed_count; ++i ){
				printf("seed[%d]: %d, %d, %d, %d\n", i, TT.seed[i][0], TT.seed[i][1], TT.seed[i][2], TT.seed[i][3] );
			}*/
			
			fclose(f);

			const Wcoord dir12 [12] = {
				wc(1, 0, 0, 0),
				wc(0, 1, 0, 0),
				wc(0, 0, 1, 0),
				wc(0, 0, 0, 1),
				wc(-1, 0, 1, 0),
				wc(0, -1, 0, 1),
				wc(-1, 0, 0, 0),
				wc(0, -1, 0, 0),
				wc(0, 0, -1, 0),
				wc(0, 0, 0, -1),
				wc(1, 0, -1, 0),
				wc(0, 1, 0, -1)
			};

			//                                2  3  4   5   
			const int polytype [] = { -1, -1, 3, 4, 6, 12 };

			/*map_str_int hash;
			ok_map_init( &hash );
			str_vec coord_codes;
			ok_vec_init(&coord_codes);
			*/

			int size = 8;
			*N = 0;
			*points = malloc( size * sizeof(vec2d) );
			vec2d dc = v2d( lerp(dest.x, dest.x+dest.w, 0.5), lerp(dest.y, dest.y+dest.h, 0.5) );

			int X = 0;
			int Y = 0;
			int step = 0;
			int leg = 0;
			int ring = 0;
			int hits = 0;
			while( 1 ){

				//printf( "%+d x %+d\n", X, Y );

				Wcoord trans = wc_sum( wc_scaled( TT.T1, X ), wc_scaled( TT.T2, Y ) );
				for (int s = 0; s < TT.seed_count; s++){
					Wcoord C = wc_plus_warr( TT.seed[s], trans );
					vec2d V = wc_to_v2d( C );
					v2d_mult( &V, edge );
					v2d_add( &V, dc );
					if( v2d_in_rect( V, dest ) ){
						/*
						sprint_wc( C, buf );
						char *str = malloc( strlen(buf)+1 );
						strcpy( str, buf );
						ok_vec_push(&coord_codes, str);
						ok_map_put( &hash, *ok_vec_last(&coord_codes), (s+1) );
						*/
						int I = *N;
						if( I >= size ){
							size *= 2;
							*points = realloc( *points, size * sizeof(vec2d) );
						}
						(*points)[I] = V;
						*N += 1;
						//printf( "hit: point #%d\n", I );

						hits++;
					}
				}

				if( ring > 0 ){
					step += 1;
					if( step >= 2 * ring ){
						step = 0;
						switch( leg ){
							case 0:
								leg = 1;
								Y += 1;
								break;
							case 1: 
								leg = 2;
								X -= 1;
								break;
							case 2: 
								leg = 3;
								Y -= 1;
								break;
							case 3:
								if( hits <= 0 ){
									goto done_tesselating_nm;
								}
								hits = 0;

								ring += 1;
								leg = 0;
								Y -= 1;
								break;
						}
					}
					else{
						switch( leg ){
							case 0: X += 1; break;
							case 1: Y += 1; break;
							case 2: X -= 1; break;
							case 3: Y -= 1; break;
						}
					}
				}
				else{
					X = 0;
					Y = -1;
					step = 0;
					leg = 0;
					ring = 1;
				}
			}
			done_tesselating_nm:

			printf("\n:: *N: %d\n", *N );

			*connections = malloc( (*N) * sizeof(int16_t*) );
			for (int i = 0; i < *N; ++i ){
				(*connections)[i] = calloc( max_cnx, sizeof(int16_t) );
			}

			for (int i = 0; i < TT.seed_count; ++i ){
				free( TT.seed[i] );
			}
			free( TT.seed );

			//printf("ok_vec_count(&coord_codes): %d\n", ok_vec_count(&coord_codes) );

			/*
			ok_map_deinit(&hash);
			ok_vec_foreach(&coord_codes, char *str){
				free(str);
			}
			ok_vec_deinit(&coord_codes);
			*/

			return 1;
		}
		else{
			printf("tess <%s> not found!\n", buf );
			fclose(f);
			return 0;
		}
	}
}