#include "ok_lib.h"
#include "tools.h"



bool intersect( vec2d L0A, vec2d L0B, vec2d L1A, vec2d L1B ){
	double s1x = L0B.x - L0A.x;
	double s1y = L0B.y - L0A.y;
	double s2x = L1B.x - L1A.x;
	double s2y = L1B.y - L1A.y;
	double s = (-s1y * (L0A.x - L1A.x) + s1x * (L0A.y - L1A.y)) / (-s2x * s1y + s1x * s2y);
	double t = ( s2x * (L0A.y - L1A.y) - s2y * (L0A.x - L1A.x)) / (-s2x * s1y + s1x * s2y);
	//vec2d out = { NAN, NAN }
	if ( (s > 0 && s < 1) && (t > 0 && t < 1) ){
		//out.x = L0A.x + (t * s1x)
		//out.y = L0A.y + (t * s1y)
		return 1;
	}
	return 0;
}

bool nodes_connected( Nodemap *M, int i, int j ){
	if( j < i ){
		return nodes_connected( M, j, i );
	}
	for (int c = 1; c <= M->connections[i][0]; ++c ){
		if( M->connections[i][c] == j ) return 1;
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

void nodes_set_connection( Nodemap *M, int i, int j, bool value ){
	if( j < i ){
		nodes_set_connection( M, j, i, value );
		return;
	}
	if( value ){
		if( nodes_connected( M, i, j ) ) return;
		M->connections[i][0] += 1;
		M->connections[i][ M->connections[i][0] ] = j;
	}
	else{
		if( !nodes_connected( M, i, j ) ) return;
		int ji = 0;
		for (int c = 1; c <= M->connections[i][0]; ++c ){
			if( M->connections[i][c] == j ){
				ji = c;
				break;
			}
		}
		if( ji ){
			if( ji < M->connections[i][0] ){
				memmove( M->connections[i]+ji, M->connections[i]+ji+1, (M->connections[i][0]-ji) * sizeof(int16_t) );
			}
			M->connections[i][0] -= 1;
		}
	}
}

void connectify( Nodemap *M, int *selected, int selected_N, float distsq ){
	for (int i = 0; i < selected_N; ++i ){
		for (int j = i+1; j < selected_N; ++j ){
			if( v2d_distsq( M->points[ selected[i] ], M->points[ selected[j] ] ) <= distsq ){
				nodes_set_connection( M, selected[i], selected[j], 1 );
			}
		}
	}
}




void gen_nodemap( int N, Nodemap *M, float edge ){

	printf("genning graph! %d nodes.\n", N);//debug

	M->points = malloc( N * sizeof(vec2d) );
	M->icon_IDs = calloc( N, sizeof(int) );
	M->status = calloc( N, sizeof(int32_t) );
	M->deleted_N = 0;
	M->contents = calloc( N, sizeof(char*) );
	int *parent = malloc( N * sizeof(int) );
	int *children = malloc( N * sizeof(int) );
	vec2d *V = calloc( N, sizeof(vec2d) );
	vec2d *F = calloc( N, sizeof(vec2d) );

	M->points[0] = v2d(0,0);
	parent[0] = -1;
	children[0] = random(2,7);

	float edgesq = sq(edge);
	float range_factor = 3;
	double rangemax = range_factor * edge;
	double rangemax_sq = sq( rangemax );
	double force_lo = 2;
	double force_hi = -(range_factor-1) * force_lo;
	//double range = rangemax - edgesq;
	int len = 1;
	int grow_timer = 0;
	int generation = 1;

	puts("birthing and relaxing points...");//debug
	while(1){
		if( grow_timer < 0 ) break;
		if( grow_timer == 0 ){
			grow_timer--;
			int gen_cap = lrint(ceil(TWO_PI * generation));
			int gen_pop = 0;
			for (int i = len-1; i >= 0; --i){
				if( len >= N ) break;
				//printf("children[i]: %d\n", children[i] );
				if( children[i] > 0 ){
					for (int c = 0; c < children[i]; ++c){
						double a = random_angle();
						if( i > 0 ){
							a = atan2( M->points[i].y - M->points[0].y, M->points[i].x - M->points[0].x );
							a += random(-40,41)/80.0;
							if ( a != a ) a = random_angle();
						}
						vec2d offset = v2d_from_polar( edge + random(-3,4), a );
						M->points[ len ] = v2d_sum( M->points[i], offset );
						//printf( "%d, %d: %lf, %lf. %lf\n", i, len, M->points[ len ].x, M->points[ len ].y, a );
						parent[ len ] = i;
						children[ len ] = random_from_list( 10, 0, 0, 1, 1, 1, 2, 2, 2, 2, 3 );
						gen_pop += children[ len ];
						if( gen_pop > gen_cap ){
							children[ len ] += (gen_cap - gen_pop);
							gen_pop = gen_cap;
						}
						++len;
						if( len >= N ) break;
					}
					children[i] = 0;
				}
			}
			//printf("len:%d\n", len );
			check_again:
			if( gen_pop > 0 ) grow_timer = gen_pop * 8;
			else if ( len < N ){
				int left = N-len;
				if( left <= 3 ){
					children[ len-1 ] = left;
					gen_pop += left;
				}
				else{
					for (int i = len-1; i >= 0; --i){
						children[ i ] = random_from_list( 10, 0, 0, 1, 1, 1, 2, 2, 2, 2, 3 );
						gen_pop += children[ i ];
						left -= children[ i ];
						if( left < 0 ){
							children[ i ] += left;
							break;
						}
					}
				}
				goto check_again;
			}
			generation++;
		}
		else grow_timer--;//printf("%d ", grow_timer );

		//resetting forces
		for (int i = 0; i < len; ++i) F[i] = v2dzero;

		//Add up repelling/attracting forces
		/*for ( int i = 0; i < len; ++i ){
			for ( int j = i+1; j < len; ++j ){
				double d = v2d_distsq( M->points[i], M->points[j] );
				if( d < rangemax ){
					if( d == 0 ) d = 0.03125;
					vec2d diff = v2d_diff( M->points[i], M->points[j] );

					if ( d > edgesq &&  i == parent[j] ){
						double Q = map( d, rangemax, edgesq, 0.3, 0 );
						vec2d f = v2d_product( diff, Q );
						v2d_sub( F + i, f );
						v2d_add( F + j, f );
					}
					else{
						if( d < 10 ) d = 10;
						double Q = map( d, 0, rangemax, 40/d, 0 );
						vec2d f = v2d_product( diff, Q );
						v2d_add( F + i, f );
						v2d_sub( F + j, f );
					}
				}
			}
		}*/
		for ( int i = 0; i < len; ++i ){
			for ( int j = i+1; j < len; ++j ){
				double d = v2d_distsq( M->points[i], M->points[j] );
				if( d < rangemax_sq ){
					if( d == 0 ) d = 0.03125;
					vec2d f = v2d_normalize( v2d_diff( M->points[i], M->points[j] ) );
					d = sqrt(d);
					double mag = map( d, 0, rangemax, force_lo, force_hi );
					if( !( i != parent[j] && mag < 0 ) ){//never attract if we're not connected
						v2d_mult( &f, mag );
						v2d_add( F + i, f );
						v2d_sub( F + j, f );
					}
				}
			}
		}
		//effect motion
		for ( int i = len-1; i > 0; --i ){
			
			v2d_add( V + i, F[i] );
			v2d_add( M->points + i, V[i] );
			v2d_mult( V + i, 0.5 );
		}
	}

	puts("Done! now to form the connections");//debug

	M->connections = malloc( N * sizeof(int16_t*) );
	M->max_cnx = 16;

	int saturation = ceil( 2.5 * N );
	printf("total satch: %d\n", saturation );

	for (int i = 0; i < N; ++i){
		M->connections[i] = malloc( M->max_cnx * sizeof(int16_t) );
		int ic = 0;
		for (int j = i+1; j < N; ++j){
			if( (i == parent[j]) || (j == parent[i]) ){
				ic++;
				M->connections[i][ic] = j;
				if( ic == M->max_cnx ) break;
				saturation--;
			}
		}
		M->connections[i][0] = ic;
	}
	printf("satch: %d\n", saturation );//debug

	double max_dist = 1.25 * edge;

	int sattries = 0;

	begin_saturate:
	for (int o = N-1; o >= 0; o--){
		int i = random( 0, N );
		for (int j = 0; j < N; ++j){
			if( i == j ) continue;
			if( !nodes_connected( M, i, j ) && 
				v2d_dist( M->points[i], M->points[j] ) < max_dist ){
				nodes_set_connection( M, i, j, 1 );
				saturation--;
				if( saturation <= 0 ) goto done_baking;
			}
		}
	}
	if( saturation > 0 ){
		sattries += 1;
		if( sattries < 24 ){
			max_dist *= 1.5;
			goto begin_saturate;
		}
	}
	done_baking:;

	typedef struct ok_vec_of(index2d) vec_index2d_t;
	vec_index2d_t links = OK_VEC_INIT;
	for (int i = 0; i < N; ++i ){
		for (int ic = 1; ic <= M->connections[i][0]; ++ic ){
			index2d ind = (index2d){ i, M->connections[i][ic] };
			ok_vec_push( &links, ind );
		}
	}

	printf("cut intersections: ");
	for (int i = ok_vec_count(&links)-1; i > 0; --i ){
		index2d *li = ok_vec_get_ptr(&links, i);
		//printf( "%d, ", i );
		for (int j = i-1; j >= 0; --j ){
			index2d *lj = ok_vec_get_ptr(&links, j);
			if( intersect( M->points[li->i], M->points[li->j], M->points[lj->i], M->points[lj->j] ) ){
				double dli = v2d_dist( M->points[li->i], M->points[li->j] );
				double dlj = v2d_dist( M->points[lj->i], M->points[lj->j] );
				int l; index2d *L;
				if( dli > dlj ){
					l = i;
					L = li;
				}else{
					l = j;
					L = lj;
				}
				nodes_set_connection( M, L->i, L->j, 0 );
				ok_vec_remove_at( &links, l );
				if( l == i ) break;
			}
		}
	}
	//puts(".");

	puts("postbake 'tempering'");//debug
	for (int pb = 0; pb < 360; ++pb){
		//printf("%d, ", pb );

		for (int i = 0; i < len; ++i) F[i] = v2dzero;

		//Add up repelling/attracting forces
		for ( int i = 0; i < len; ++i ){
			for ( int j = i+1; j < len; ++j ){
				double d = v2d_distsq( M->points[i], M->points[j] );
				if( d < rangemax_sq ){
					if( d == 0 ) d = 0.03125;
					vec2d f = v2d_normalize( v2d_diff( M->points[i], M->points[j] ) );
					d = sqrt(d);
					double mag = map( d, 0, rangemax, force_lo, force_hi );
					if( !( !nodes_connected( M, i, j ) && mag < 0 ) ){//never attract if we're not connected
						v2d_mult( &f, mag );
						v2d_add( F + i, f );
						v2d_sub( F + j, f );
					}
				}
			}
		}
		//effect motion
		for ( int i = len-1; i > 0; --i ){
			
			v2d_add( V + i, F[i] );
			v2d_add( M->points + i, V[i] );
			v2d_mult( V + i, 0.5 );
		}
	}
	printf("done");//debug

	free(parent);
	free(children);
	free(V);
	free(F);
	ok_vec_deinit( &links );
	puts(" and temps all freed. Bye!");//debug
}

void destroy_nodemap( Nodemap *M ){

	for (int a = 0; a < M->N; ++a ){
		free( M->connections[a] );
		if( M->contents[a] != NULL ){
			free( M->contents[a] );
		}
	}
	free( M->points     );
	free( M->connections);
	free( M->icon_IDs   );
	free( M->status     );
	free( M->contents   );
}

void translate_nodemap( Nodemap *M, vec2d c ){

	vec2d Mc = v2d_medicenter( M->points, M->N );
	vec2d delta = v2d_diff( c, Mc );
	for (int i = 0; i < M->N; ++i ){
		v2d_add( M->points + i, delta );
	}
}

void merge_nodemaps( Nodemap *A, Nodemap *B ){
	puts("merging");
	int actual_AN = consolidate_nodemap( A );
	printf("actual_AN: %d\n", actual_AN );

	int NN = actual_AN + B->N;
		 A->points = realloc(      A->points,  NN * sizeof(vec2d)    );
	A->connections = realloc( A->connections,  NN * sizeof(int16_t*) );
	   A->icon_IDs = realloc(    A->icon_IDs,  NN * sizeof(int)      );
	     A->status = realloc(      A->status,  NN * sizeof(int32_t)  );
	   A->contents = realloc(    A->contents,  NN * sizeof(char*)    );
	puts("realloc'd");

	for (int b = 0; b < B->N; ++b ){
		for (int c = 1; c <= B->connections[b][0]; ++c ){
			B->connections[b][c] += actual_AN;
		}
	}
	puts("offeset conns");

	memcpy( A->points + actual_AN, B->points, B->N * sizeof(vec2d) );

	memcpy( A->connections + actual_AN, B->connections, B->N * sizeof(int16_t*) );

	if( B->status != NULL ){
		memcpy( A->status + actual_AN, B->status, B->N * sizeof(int32_t) );
	}else{
		memset( A->status + actual_AN, KNOWLEDGE_NONE, B->N * sizeof(int32_t) );
	}

	if( B->icon_IDs != NULL ){
		memcpy( A->icon_IDs + actual_AN, B->icon_IDs, B->N * sizeof(int) );
	}else{
		memset( A->icon_IDs + actual_AN, 0, B->N * sizeof(int) );
	}

	if( B->contents != NULL ){
		memcpy( A->contents + actual_AN, B->contents, B->N * sizeof(char*) );
	}else{
		memset( A->contents + actual_AN, 0, B->N * sizeof(char*) );
	}
	puts("memcpy'd / set");

	A->N = NN;

							  free( B->points      );
							  free( B->connections );
	if(   B->status != NULL ) free( B->status      );
	if( B->icon_IDs != NULL ) free( B->icon_IDs    );
	if( B->contents != NULL ) free( B->contents    );

	puts("freed n done.");
}

int create_node( Nodemap *M, vec2d p ){
	int I = -1;
	if( M->deleted_N > 0 ){
		for (int i = 0; i < M->N; ++i){
			if( M->status[i] & DELETED ){
				I = i;
				M->points[I] = p;
				M->icon_IDs[I] = 0;
				M->status[I] = KNOWLEDGE_NONE;
				M->deleted_N -= 1;
				break;
			}
		}
		if( I < 0 ) goto grow_map;
	}
	else{
		grow_map:
		I = M->N;
		M->N = (ceil( (M->N + 1) / 16.0 ) + 1) * 16;
		printf("M->N %d -> %d\n", I, M->N );
		M->deleted_N += (M->N - I)-1;
		M->points = realloc( M->points, M->N * sizeof(vec2d) );
		M->icon_IDs = realloc( M->icon_IDs, M->N * sizeof(int) );	
		M->status = realloc( M->status, M->N * sizeof(int32_t) );
		M->connections = realloc( M->connections, M->N * sizeof(int16_t*) );
		M->contents = realloc( M->contents, M->N * sizeof(char*) );
		for(int i = I; i < M->N; i++){
			M->icon_IDs[i] = 0;
			M->connections[i] = calloc( M->max_cnx, sizeof(int16_t) );
			M->contents[i] = NULL;
			M->status[i] = DELETED;
		}
		M->points[I] = p;
		M->status[I] = KNOWLEDGE_NONE;
	}
	//printf("fresh node: %d\n", I );
	return I;
}


SDL_Rect src_in_sheet( SDL_Rect layout, int ID, int margin ){
	SDL_Rect src = (SDL_Rect){ (ID % layout.x) * (layout.w + margin),
							   (ID / layout.x) * (layout.h + margin),
							   layout.w, layout.h };
	return src;
}



void record_action( action_data *AD, int ACTION, int *selected, int selected_N, vec2d cursor, vec2d offset, double baseline ){
	AD->ACTION = ACTION;
	if( selected_N > AD->selected_N || selected_N < AD->selected_N * 0.5 ){
		AD->selected = realloc( AD->selected, selected_N * sizeof(int) );
	}
	memcpy( AD->selected, selected, selected_N * sizeof(int) );
	AD->selected_N = selected_N;
	AD->cursor = cursor;
	AD->offset = offset;
	AD->baseline = baseline;
}
void undo_action( action_data *AD, vec2d *points ){

	switch( AD->ACTION ){

		case TRANSLATING:;
			vec2d delta = v2d_diff( AD->offset, AD->cursor );
			for (int s = 0; s < AD->selected_N; ++s ){
				points[AD->selected[s]] = v2d_sum( points[AD->selected[s]], delta );
			}
			break;

		case SCALING:;
			double S = AD->baseline / v2d_dist( AD->cursor, AD->offset );
			for (int s = 0; s < AD->selected_N; ++s ){
				int i = AD->selected[s];
				vec2d diff = v2d_diff( points[i], AD->offset );
				vec2d v = v2d_from_polar( S * v2d_mag(diff), v2d_heading(diff) );
				points[i] = v2d_sum( AD->offset, v );
			}
			break;

		case ROTATING:;
			double A = AD->baseline - v2d_heading( v2d_diff( AD->cursor, AD->offset ) );
			for (int s = 0; s < AD->selected_N; ++s ){
				int i = AD->selected[s];
				vec2d diff = v2d_diff( points[i], AD->offset );
				vec2d v = v2d_from_polar( v2d_mag(diff), A + v2d_heading(diff) );
				points[i] = v2d_sum( AD->offset, v );
			}
			break;

		default:
			printf("uhh, tried to undo ACTION = %d\n", AD->ACTION );
	}
}

void redo_action( action_data *AD, vec2d *points ){

	if( AD->ACTION == SCALING || AD->ACTION == ROTATING ){
		for (int s = 0; s < AD->selected_N; ++s ){
			vec2d diff = v2d_diff( points[ AD->selected[s] ], AD->offset );
			points[ AD->selected[s] ] = v2d_to_polar(diff);//v2d( v2d_mag(diff), v2d_heading(diff) );
		}
	}
	switch( AD->ACTION ){
		case TRANSLATING:;
			apply_translation( AD->selected, AD->selected_N, points, points, AD->cursor, AD->offset, 0 );
			break;
		case SCALING:;
			apply_scale( AD->selected, AD->selected_N, points, points, AD->cursor, AD->offset, AD->baseline );
			break;
		case ROTATING:;
			apply_rotation( AD->selected, AD->selected_N, points, points, AD->cursor, AD->offset, AD->baseline );
			break;
		default:
			printf("uhh, tried to redo ACTION = %d\n", AD->ACTION );
	}
}



void apply_translation( int *selected, int selected_N, vec2d *Psrc, vec2d *Pdst, vec2d cursor, vec2d offset, double baseline ){
	vec2d delta = v2d_diff( cursor, offset );
	for (int s = 0; s < selected_N; ++s ){
		Pdst[selected[s]] = v2d_sum( Psrc[selected[s]], delta );
	}
}
void apply_scale( int *selected, int selected_N, vec2d *Psrc, vec2d *Pdst, vec2d cursor, vec2d offset, double baseline ){
	double S = v2d_dist( cursor, offset ) / baseline;
	for (int s = 0; s < selected_N; ++s ){
		vec2d v = v2d_from_polar( S * Psrc[selected[s]].x, Psrc[selected[s]].y );
		Pdst[selected[s]] = v2d_sum( offset, v );
	}
}

void apply_rotation( int *selected, int selected_N, vec2d *Psrc, vec2d *Pdst, vec2d cursor, vec2d offset, double baseline ){
	double A = v2d_heading( v2d_diff( cursor, offset ) ) - baseline;
	for (int s = 0; s < selected_N; ++s ){
		vec2d v = v2d_from_polar( Psrc[selected[s]].x, A + Psrc[selected[s]].y );
		Pdst[selected[s]] = v2d_sum( offset, v );
	}
}


int mouse_on_node( vec2d rtm, Nodemap *M, Transform *T, int nirsq ){
	for (int i = 0; i < M->N; ++i){
		if( M->status[i] & DELETED ) continue;
		if( v2d_distsq( rtm, M->points[i] ) < nirsq ){
			return i;
		}
	}
	return -1;
}

int mouse_on_label( vec2d rtm, Maplabel **labels, int labels_N, Transform *T, int nirsq ){
	for (int i = 0; i < labels_N; ++i){
		if( v2d_distsq( rtm, labels[i]->pos ) < nirsq ){
			return i;
		}
	}
	return -1;
}
bool node_label_connected( Maplabel *label, int node_id ){
	for (int con = 0; con < label->connections_N; ++con ){
		if( label->connections[con] == node_id ){
			return 1;
		}
	}
	return 0;
}
void label_set_connection( Maplabel *label, int node_id, bool value ){
	if( value ){
		if( label->connections_N < 16 ){
			label->connections[ label->connections_N ] = node_id;
			label->connections_N += 1;
		}
	}
	else{
		int id_index = -1;
		for (int con = 0; con < label->connections_N; ++con ){
			if( label->connections[con] == node_id ){
				id_index = con;
				break;
			}
		}
		if( id_index >= 0 ){
			memmove( label->connections + id_index, label->connections + id_index + 1, (16 - id_index + 1) * sizeof(int) );
			label->connections_N -= 1;
		}
	}
}


int consolidate_nodemap( Nodemap *M ){
	int actual_N = M->N;
	printf("Consolidating:");
	for (int i = M->N -1; i >= 0 ; --i){
		if( M->status[i] & DELETED ){

			//packing in connection references
			for (int j = 0; j < actual_N; ++j ){ 
				if( j == i ) continue;
				for (int c = 1; c <= M->connections[j][0]; ++c ){
					if( M->connections[j][c] > i ){
						M->connections[j][c] -= 1;
					}
				}
			}

			if( i < actual_N - 1 ){
				free( M->connections[i] );
				memmove(      M->points + i,      M->points + i+1, (actual_N - (i+1)) * sizeof(vec2d)    );
				memmove( M->connections + i, M->connections + i+1, (actual_N - (i+1)) * sizeof(int16_t*) );
				memmove(      M->status + i,      M->status + i+1, (actual_N - (i+1)) * sizeof(int32_t)  );
				memmove(    M->icon_IDs + i,    M->icon_IDs + i+1, (actual_N - (i+1)) * sizeof(int)      );
				memmove(    M->contents + i,    M->contents + i+1, (actual_N - (i+1)) * sizeof(char*)    );
			}
			actual_N -= 1;
			putchar('<');
		}
		else putchar('.');
	}
	puts("");
	M->deleted_N = 0;
	return actual_N;
}


void export_nodemap( char *buf, Nodemap *M, Maplabel **labels, int labels_N ){
	//char *title = str_at_label( settings_content, "Title: ", "\n" );
					//sprintf( buf, "Stories/%s.txt", title );
	time_t rawtime;
	struct tm * timeinfo;
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	strftime( buf, 128, "creations/confab %Y.%m.%d %H-%M-%S.bin", timeinfo );

	printf("exporting \"%s\".\n", buf );
	setlocale(LC_ALL, "C");
	FILE *f = fopen( buf, "wb" );

	fprintf( f, "confab-V1 " );

	int actual_N = consolidate_nodemap( M );
	for (int i = actual_N; i < M->N; ++i){
		M->status[i] = DELETED;
		M->connections[i] = calloc( M->max_cnx, sizeof(int16_t) );
		M->contents[i] = NULL;
	}
	//M->N = actual_N;
	printf(" %d nodes.\n", actual_N );
	
	fwrite( &actual_N, sizeof(int), 1, f );

	fwrite(   M->points, sizeof(vec2d)  , actual_N, f );
	fwrite(   M->status, sizeof(int32_t), actual_N, f );
	fwrite( M->icon_IDs, sizeof(int)    , actual_N, f );

	for (int a = 0; a < actual_N; ++a ){
		fwrite( M->connections[a], sizeof(int16_t), M->connections[a][0]+1, f );
	}
	for (int a = 0; a < actual_N; ++a ){
		int len = 0;
		if( M->contents[a] == NULL ){
			fwrite( &len, sizeof(int), 1, f );
		}
		else{
			len = strlen( M->contents[a] ) + 1;
			fwrite( &len, sizeof(int), 1, f );
			fwrite( M->contents[a], sizeof(char), len, f );
		}
	}

	fwrite( &labels_N, sizeof(int), 1, f );

	for (int l = 0; l < labels_N; ++l ){
		fwrite( &(labels[l]->pos), sizeof(vec2d), 1, f );
		fwrite( &(labels[l]->scale), sizeof(double), 1, f );
		fwrite( &(labels[l]->angle), sizeof(double), 1, f );
		fwrite( &(labels[l]->curvature), sizeof(double), 1, f );
		fwrite( &(labels[l]->connections_N), sizeof(int), 1, f );
		if( labels[l]->connections_N > 0 ){
			fwrite( labels[l]->connections, sizeof(int), labels[l]->connections_N, f );
		}
		fwrite( &(labels[l]->strb.len), sizeof(int), 1, f );
		fwrite( labels[l]->strb.str, sizeof(char), labels[l]->strb.len, f );
	}

	fclose (f);



	setlocale(LC_ALL, "C");
	FILE *template = fopen( "Assets/template.c", "r" );
	//char *template = NULL;
	//load_file_as_str( "Assets/template.c", &template );

	int bl = strlen( buf );
	sprintf( buf+bl-3, "c\0" );
	printf("and now \"%s\".", buf );
	f = fopen( buf, "w" );

	char tag [] = "switch( LOC ){";
	int taglen = strlen(tag);
	int ti = 0;

	while(1){
		char c = getgc(template);
		if( feof(template) ) break;
		fputc( c, f );
		if( c == tag[ti] ){
			ti += 1;
			if( ti == taglen ){
				for (int i = 0; i < M->N; ++i ){
					if( M->contents[i] != NULL ){
						fprintf( f, "\n\t\t\tcase %d:\n\t\t\t\t%s\n\t\t\t\tbreak;\n\t\t\t\t", i, M->contents[i] );
					}else{
						fprintf( f, "\n\t\t\tcase %d: break;\n\t\t\t\t", i );
					}
				}
			}
		}
		else ti = 0;
	}
	fclose( template );
	fclose (f);
}


void import_nodemap( char *filename, Nodemap *M, Maplabel ***labels, int *labels_N ){

	int fnlen = strlen( filename );
	printf("importing filename: %s, len: %d\n", filename, fnlen );

	if( strcmp( filename + fnlen -3, "bin" ) == 0 ){
		puts("bin");

		setlocale(LC_ALL, "C");
		FILE *f = fopen( filename, "rb" );

		fseek( f, 8, SEEK_SET );
		int version = 0;
		fscanf( f, "%d", &version );
		printf("version: %d", version );
		if( fgetc(f) == ' ' ) puts("_");
		
		fread( &(M->N), sizeof(int), 1, f );
		printf("M->N: %d\n", M->N );

		     M->points = malloc( M->N * sizeof(vec2d) );
		M->connections = malloc( M->N * sizeof(int16_t*) );
		   M->icon_IDs = calloc( M->N,  sizeof(int) );
		     M->status = calloc( M->N,  sizeof(int32_t) );
		   M->contents = calloc( M->N,  sizeof(char*) );
		M->max_cnx = 16;
		
		fread(   M->points, sizeof(vec2d)  , M->N, f );
		fread(   M->status, sizeof(int32_t), M->N, f );
		fread( M->icon_IDs, sizeof(int)    , M->N, f );

		puts("connections:");
		for (int a = 0; a < M->N; ++a ){
			M->connections[a] = calloc( M->max_cnx, sizeof(int16_t) );
			fread( M->connections[a], sizeof(int16_t), 1, f );
			fread( M->connections[a]+1, sizeof(int16_t), M->connections[a][0], f );
		}
		puts("contents:");
		for (int a = 0; a < M->N; ++a ){
			int len = 0;
			fread( &len, sizeof(int), 1, f );
			if( len > 0 ){
				M->contents[a] = malloc( len * sizeof(char) );
				fread( M->contents[a], sizeof(char), len, f );
			}
		}

		fread( labels_N, sizeof(int), 1, f );

		if( !feof(f) ){

			*labels = malloc( (*labels_N) * sizeof(Maplabel*) );

			for (int l = 0; l < (*labels_N); ++l ){
				(*labels)[l] = malloc( sizeof(Maplabel) );
				fread( &((*labels)[l]->pos), sizeof(vec2d), 1, f );
				fread( &((*labels)[l]->scale), sizeof(double), 1, f );
				fread( &((*labels)[l]->angle), sizeof(double), 1, f );
				fread( &((*labels)[l]->curvature), sizeof(double), 1, f );
				fread( &((*labels)[l]->connections_N), sizeof(int), 1, f );
				if( (*labels)[l]->connections_N > 0 ){
					fread( (*labels)[l]->connections, sizeof(int), (*labels)[l]->connections_N, f );
				}
				fread( &((*labels)[l]->strb.len), sizeof(int), 1, f );
				//printf("(*labels)[l]->strb.len: %d\n", (*labels)[l]->strb.len );
				(*labels)[l]->strb.cap = 0;
				(*labels)[l]->strb.str = NULL;
				STRB_ensure( &((*labels)[l]->strb), (*labels)[l]->strb.len );
				fread( (*labels)[l]->strb.str, sizeof(char), (*labels)[l]->strb.len, f );
				//printf("(*labels)[l]->strb.str[0]: %c\n", (*labels)[l]->strb.str[0] );
				(*labels)[l]->strb.str[ (*labels)[l]->strb.len ] = '\0';
			}
		}
		else{
			*labels_N = 0;
		}

		fclose (f);
	}
}


#define     UP 1
#define  RIGHT 2
#define   DOWN 4
#define   LEFT 8
#define   DEAD 16
#define MASKED 32

static void dfs_mazer( int *M, int x, int y, int dir, int W, int H, int age ){

	int yW = y * W;

	if( M[x + yW] == 0 ) M[x + yW] = dir;
	else M[x + yW] |= dir;

	/*for (int t = 0; t < age; ++t ){
		if( rand() % 25 == 24 ) return;
	}*/

	switch( dir ){
		case    UP: y -= 1; break;
		case RIGHT: x += 1; break;
		case  DOWN: y += 1; break;
		case  LEFT: x -= 1; break;
	}

	yW = y * W;

	int options [4] = {0,0,0,0};
	int o_n;

	check_again:;

	o_n = 0;
	
	if( y > 0   && M[x + (yW-W)] == 0 ){ //UP
		options[0] = 1;
		o_n += 1;
	}
    if( x < W-1 && M[x+1 + yW] == 0 ){ //RIGHT
    	options[1] = 1;
    	o_n += 1;
    }
    if( y < H-1 && M[x + (yW+W)] == 0 ){ //DOWN
    	options[2] = 1;
    	o_n += 1;
    }
    if( x > 0    && M[x-1 + yW] == 0 ){ //LEFT
    	options[3] = 1;
    	o_n += 1;
    }
    //printf("x: %d, y: %d, dir: %hhd, age %d, o_n:%d\n", x, y, dir, age, o_n );

    if( o_n == 0 ){
    	if( M[x + yW] == 0 ) M[x + yW] = DEAD;
		else M[x + yW] |= DEAD;
		//printf("died at age %d\n", age );
	}
	else if ( o_n == 1 ){
		for(int i = 0; i < 4; i++){
			if( options[i] ){
				dfs_mazer( M, x, y, 1 << i, W, H, age + 1 );
				break;
			}
		}
	}
	else{
		int o = rand() % o_n;
		int c = 0;
		for(int i = 0; i < 4; i++){
			if( options[i] ){
				if( c == o ){
					dfs_mazer( M, x, y, 1 << i, W, H, age + 1 );
					goto check_again;
				}
				c++;
			}
		}
	}
}


Nodemap build_maze( float edge, SDL_Rect dest ){

	int W = dest.w / edge;
	int H = dest.h / edge;

	int *M = calloc( W * H, sizeof(int) );
	int sx = random(0, W);
	int sy = random(0, H);
	int dir = 1 << (rand() % 4);
	if( sx == 0   && dir == LEFT  ) dir = RIGHT;
	if( sx == W-1 && dir == RIGHT ) dir = LEFT;
	if( sy == 0   && dir == UP    ) dir = DOWN;
	if( sy == H-1 && dir == DOWN  ) dir = UP;
	//printf("mazin' %d x %d, %d\n", sx, sy, dir );
	dfs_mazer( M, sx, sy, dir, W, H, 0 );
	
	Nodemap N = {0};
	N.N = W * H;
	N.points = malloc( N.N * sizeof(vec2d) );
	N.connections = malloc( N.N * sizeof(int16_t*) );
	for (int i = 0; i < N.N; ++i ){
		N.connections[i] = calloc( 16, sizeof(int16_t) );
	}
	int o = 0;
	for (int j = 0; j < H; ++j ){
		for (int i = 0; i < W; ++i ){
			N.points[o] = v2d( dest.x + i * edge, dest.y + j * edge );
			if( j > 0   && (M[o] & UP) ){ //|| (M[o-W] & DOWN)
				nodes_set_connection( &N, o, o-W, 1 );
			}
			if( j < H-1 && (M[o] & DOWN) ){ //|| (M[o+W] & UP 
				nodes_set_connection( &N, o, o+W, 1 );
			}
			if( i > 0   && (M[o] & LEFT) ){ //|| (M[o-1] & RIGHT)
				nodes_set_connection( &N, o, o-1, 1 );
			}
			if( i < W-1 && (M[o] & RIGHT) ){ //|| (M[o+1] & LEFT)
				nodes_set_connection( &N, o, o+1, 1 );
			}
			o++;
		}
	}

	return N;
}
