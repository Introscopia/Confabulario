#include "VFX.h"


SDL_Rect src_in_sheet( SDL_Rect layout, int ID, int margin ){
	SDL_Rect src = (SDL_Rect){ (ID % layout.x) * (layout.w + margin),
							   (ID / layout.x) * (layout.h + margin),
							   layout.w, layout.h };
	return src;
}

void render_animated_sprite( SDL_Renderer *R, Animated_Sprite_inst *ASi, float x, float y ){

	SDL_FRect dst = (SDL_FRect){ x + ASi->data->offset.x, y + ASi->data->offset.y,  
								 ASi->data->srcs[0].w * ASi->data->scale,
								 ASi->data->srcs[0].h * ASi->data->scale  };
	SDL_RenderCopyF( R, ASi->data->texture, ASi->data->srcs + ASi->frame, &dst );
	if( clock() > ASi->next_frame ){
		ASi->frame += 1;
		if( ASi->frame >= ASi->data->frames_N ){
			ASi->frame = 0;
		}
		ASi->next_frame = clock() + ASi->data->period;
	}
}

void build_Box_9Slice( Box_9Slice *B9S, Mundo *M, int id, SDL_Rect dst, int scale, int frames ){

	B9S->SI = id % M->borders_cols;
	B9S->SJ = id / M->borders_cols;
	B9S->dst = dst;
	B9S->scale = scale;
	B9S->c = 0;
	B9S->end = frames;
}
void render_Box_9Slice( SDL_Renderer *R, Box_9Slice *B9S, Mundo *M ){

	SDL_Rect celsrc = (SDL_Rect){ B9S->SI * (M->borders_cw + 1), B9S->SJ * (M->borders_cw + 1), M->borders_cw, M->borders_cw };
	SDL_FRect celdst;
	if( B9S->c < B9S->end ){
		celdst.w = ellipticalMap( B9S->c, B9S->end, 0, B9S->dst.w, 2 * B9S->scale * M->borders_cw );
		celdst.h = ellipticalMap( B9S->c, B9S->end, 0, B9S->dst.h, 2 * B9S->scale * M->borders_cw );
	}else{
		celdst.w = B9S->dst.w;
		celdst.h = B9S->dst.h;
	}
	celdst.x = (B9S->dst.x + (0.5 * B9S->dst.w)) - (0.5 * celdst.w);
	celdst.y = (B9S->dst.y + (0.5 * B9S->dst.h)) - (0.5 * celdst.h);
	int s_leftover = M->borders_cw - (2 * M->borders_slice);
	float sclo = B9S->scale * s_leftover;
	float scsli = B9S->scale * M->borders_slice;
	int d_leftoverw = ceil(celdst.w - (2 * scsli));
	int d_leftoverh = ceil(celdst.h - (2 * scsli));

	SDL_RenderFillRectF( R, &celdst );
	//top-left corner
	SDL_Rect src = (SDL_Rect){ celsrc.x, celsrc.y, M->borders_slice, M->borders_slice };	
	SDL_FRect dst = (SDL_FRect){ celdst.x, celdst.y, scsli, scsli };
	SDL_RenderCopyF( R, M->borders, &src, &dst );
	//top edge
	src = (SDL_Rect){ celsrc.x + M->borders_slice, celsrc.y, s_leftover, M->borders_slice };	
	dst = (SDL_FRect){ celdst.x + scsli, celdst.y, d_leftoverw, scsli };
	SDL_RenderCopyF( R, M->borders, &src, &dst );
	//top-right corner
	src = (SDL_Rect){ celsrc.x + M->borders_cw - M->borders_slice, celsrc.y, M->borders_slice, M->borders_slice };	
	dst = (SDL_FRect){ celdst.x + celdst.w - scsli, celdst.y, scsli, scsli };
	SDL_RenderCopyF( R, M->borders, &src, &dst );

	//left edge
	src = (SDL_Rect){ celsrc.x, celsrc.y + M->borders_slice, M->borders_slice, s_leftover };
	dst = (SDL_FRect){ celdst.x, celdst.y + scsli, scsli, d_leftoverh };
	SDL_RenderCopyF( R, M->borders, &src, &dst );
	//right edge
	src = (SDL_Rect){ celsrc.x + M->borders_cw - M->borders_slice, celsrc.y + M->borders_slice, M->borders_slice, s_leftover };
	dst = (SDL_FRect){ celdst.x + celdst.w - scsli, celdst.y + scsli, scsli, d_leftoverh };
	SDL_RenderCopyF( R, M->borders, &src, &dst );

	//bottom-left corner
	src = (SDL_Rect){ celsrc.x, celsrc.y + celsrc.h - M->borders_slice, M->borders_slice, M->borders_slice };	
	dst = (SDL_FRect){ celdst.x, celdst.y + celdst.h - scsli, scsli, scsli };
	SDL_RenderCopyF( R, M->borders, &src, &dst );
	//bottom edge
	src = (SDL_Rect){ celsrc.x + M->borders_slice, celsrc.y + celsrc.h - M->borders_slice, s_leftover, M->borders_slice };	
	dst = (SDL_FRect){ celdst.x + scsli, celdst.y + celdst.h - scsli, d_leftoverw, scsli };
	SDL_RenderCopyF( R, M->borders, &src, &dst );
	//bottom-right corner
	src = (SDL_Rect){ celsrc.x + M->borders_cw - M->borders_slice, celsrc.y + celsrc.h - M->borders_slice, M->borders_slice, M->borders_slice };	
	dst = (SDL_FRect){ celdst.x + celdst.w - scsli, celdst.y + celdst.h - scsli, scsli, scsli };
	SDL_RenderCopyF( R, M->borders, &src, &dst );

	if( B9S->c < B9S->end ) B9S->c += 1;
}





void build_tela_abaulada( tela_abaulada *A, int cols, SDL_Rect *rect, float factor ){

	//float aspect_ratio = (float) rect->w / rect->h;
	//int rows = lrint( aspect_ratio * cols );
	float gc = 1.0 / cols;
	float colw = rect->w * gc;

	int rows = lrint( rect->h / colw );
	float gr = 1.0 / rows;
	float rowh = rect->h * gr;
	
	A->grid_len = (cols+1) * (rows+1);
	A->grid = malloc( A->grid_len * sizeof(SDL_Vertex) );
	A->ind_len = 6 * cols * rows;
	A->indices = malloc( A->ind_len * sizeof(int) );

	SDL_Color white = (SDL_Color){255,255,255,255};
	float hw = rect->w * 0.5;
	float hh = rect->h * 0.5;
	
	SDL_FPoint C = (SDL_FPoint){ rect->x + hw, rect->y + hh };
	double diag = hypot(C.x, C.y);
	A->fix = factor / sq(diag);
	A->sc = 1;//size correction factor
	if( rect->w >= rect->h ){
		double e = sq(hh) * A->fix;
		double y = lerp( rect->y, C.y, e );
		A->sc = -hh / (y-C.y);
	} else {
		double e = sq(hw) * A->fix;
		double x = lerp( rect->x, C.x, e );
		A->sc = -hw / (x-C.x);
	}
	//printf("A->sc: %lg\n", A->sc );

	float maxOD, maxFD;
	float midOD, midFD;
	float half_dif = 9999999;
	int iV = 0;

	int v = 0;
	for (int j = 0; j <= rows; ++j ){
		for (int i = 0; i <= cols; ++i ){
			SDL_FPoint P = (SDL_FPoint){ rect->x + i*colw, rect->y + j*rowh };
			//((1+factor)*(fabs(P.x-C.x)/diag)) fun hyperbolic effect
			P.x = C.x + A->sc * (P.x - C.x);
			P.y = C.y + A->sc * (P.y - C.y);
			double d = hypot(C.x - P.x, C.y - P.y);
			double e = sq(d) * A->fix;
			//double e = (sq(C.x - P.x) + sq(C.y - P.y)) * A->fix; 
			P.x = lerp( P.x, C.x, e );
			P.y = lerp( P.y, C.y, e );
			A->grid[v++] = (SDL_Vertex){ P, white, {i*gc, j*gr} };

			if( i == 0 && j == 0 ){
				maxOD = d;
				maxFD = hypot(C.x - P.x, C.y - P.y);
			}
			else{
				if( fabs( d - (maxOD * 0.5) ) < half_dif ){
					half_dif = fabs( d - (maxOD * 0.5) );
					midOD = d;
					midFD = hypot(C.x - P.x, C.y - P.y);
				}
			}
			/*if( i > j ){
				double fd = hypot(C.x - P.x, C.y - P.y);
				printf("i: %d, j: %d, od: %lg, fd: %lg, ratio: %lg\n", i, j, d, fd, fd/d );
			}*/
		}
	}

	// the line we use to estimate the original distance in the abaulado_reverso() func.
	//slope is based on the corner datapoint
	A->m = maxOD / maxFD;
	//y-offset is half of that of the mid-distance point.
	A->b = (midOD - A->m * midFD) * 0.5;
	A->C = C;

	int hgc = cols / 2;
	int hgr = rows / 2;
	int o = 0;
	int pitch = cols+1;
	for (int j = 0; j < rows; ++j ){
		for (int i = 0; i < cols; ++i ){
			if( (i < hgc) == (j < hgr) ){
				A->indices[o++] = i   + ( j    * pitch);
				A->indices[o++] = i   + ((j+1) * pitch);
				A->indices[o++] = i+1 + ( j    * pitch);
				A->indices[o++] = i   + ((j+1) * pitch);
				A->indices[o++] = i+1 + ( j    * pitch);
				A->indices[o++] = i+1 + ((j+1) * pitch);
			}
			else {
				A->indices[o++] = i   + ( j    * pitch);
				A->indices[o++] = i   + ((j+1) * pitch);
				A->indices[o++] = i+1 + ((j+1) * pitch);
				A->indices[o++] = i   + ( j    * pitch);
				A->indices[o++] = i+1 + ( j    * pitch);
				A->indices[o++] = i+1 + ((j+1) * pitch);
			}
		}
	}
}

vec2d abaulado_reverso( tela_abaulada *A, vec2d v ){

	double d = hypot(A->C.x - v.x, A->C.y - v.y);
	//d = 0.00107 * sq(d) + 0.5 * d + 50; //my hand-made perf quad regression....
	d = A->m * d + A->b;
	double e = sq(d) * A->fix; 

	v.x = (v.x/(1-e))-(A->C.x/((1.0/e)-1));
	v.y = (v.y/(1-e))-(A->C.y/((1.0/e)-1));

	v.x = ( (v.x - A->C.x) / A->sc ) + A->C.x;
	v.y = ( (v.y - A->C.y) / A->sc ) + A->C.y;

	return v;
}


void render_tela_abaulada( SDL_Renderer *R, SDL_Texture *T, tela_abaulada *A ){
	/* render lines
		SDL_SetRenderDraw_SDL_Color( rend, &pal_hi );
		for (int j = 0; j <= grows; ++j ){
			int J = j * (gcols+1);
			for (int i = 0; i <= gcols; ++i ){
				float X = grid[i+J].position.x + map_frame.x;
				float Y = grid[i+J].position.y + map_frame.y;
				//printf("i: %d, j: %d, i+J: %d, gcols: %d\n", i, j, i+J, gcols );
				if( j < grows ){
					SDL_RenderDrawLineF(rend, X, Y, grid[i+J+gcols+1].position.x + map_frame.x, 
						                            grid[i+J+gcols+1].position.y + map_frame.y );
				}
				if( i < gcols ){
					SDL_RenderDrawLineF(rend, X, Y, grid[i+1+J].position.x + map_frame.x, 
													grid[i+1+J].position.y + map_frame.y );
				}
			}
		}
	*/
	/*
		for (int j = 0; j < grows; ++j ){
			int J = j * (gcols+1);
			for (int i = 0; i < gcols; ++i ){
				if( i < hgc == j < hgr ){
					int indices [6] = { i+J, i+J+gcols+1, i+1+J, i+J+gcols+1, i+1+J, i+J+gcols+2 };
					SDL_RenderGeometry( rend, AAtexture, grid, gridN, indices, 6 );
				}
				else {
					int indices [6] = { i+J, i+J+gcols+1, i+J+gcols+2, i+J, i+J+1, i+J+gcols+2 };
					SDL_RenderGeometry( rend, AAtexture, grid, gridN, indices, 6 );
				}
			}
		}
	*/
	SDL_RenderGeometry( R, T, A->grid, A->grid_len, A->indices, A->ind_len );
}

void destroy_tela_abaulada( tela_abaulada *A ){
	free( A->grid );
	free( A->indices );
	free( A );
}