#include "threedee.h"
#include "ok_lib.h"
#include <locale.h>

static inline float v3d_dot( vec3d v1, vec3d v2 ){
	return (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z);
}
void v3d_normalize( vec3d *v ){
	float vdotv = v3d_dot( *v, *v );
	if( vdotv > FLT_EPSILON ){
		float l = 1.0 / sqrt( vdotv );
		v->x *= l;
		v->y *= l;
		v->z *= l;
	}
}
static inline vec3d v3d_diff( vec3d v1, vec3d v2 ){
	return (vec3d){ v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
}
static inline vec3d v3d_prod( vec3d v1, vec3d v2 ){
	return (vec3d){ v1.x * v2.x, v1.y * v2.y, v1.z * v2.z };
}
static inline vec3d v3d_scalar_mult( vec3d v1, float s ){
	return (vec3d){ v1.x * s, v1.y * s, v1.z * s };
}
static inline double v3d_mag( vec3d v ){
	return  sqrt( sq(v.x) + sq(v.y) + sq(v.z) );
}

vec3d face_normal( vec3d vecs [3] ){
	// Use Cross-Product to get surface normal
	vec3d normal, line1, line2;
	line1.x = vecs[1].x - vecs[0].x;
	line1.y = vecs[1].y - vecs[0].y;
	line1.z = vecs[1].z - vecs[0].z;

	line2.x = vecs[2].x - vecs[0].x;
	line2.y = vecs[2].y - vecs[0].y;
	line2.z = vecs[2].z - vecs[0].z;

	normal.x = line1.y * line2.z - line1.z * line2.y;
	normal.y = line1.z * line2.x - line1.x * line2.z;
	normal.z = line1.x * line2.y - line1.y * line2.x;

	// It's normally normal to normalise the normal
	v3d_normalize(&normal);

	return normal;
}

void reset_Transform( Transform_3d *T ){
	T->basis_x = (vec3d){ 1, 0, 0 };
	T->basis_y = (vec3d){ 0, 1, 0 };
	T->basis_z = (vec3d){ 0, 0, 1 };
}

void printT( Transform_3d *T ){
	printf("{% 1.1g, % 1.1g, % 1.1g}\n", T->basis_x.x, T->basis_x.y, T->basis_x.z );
	printf("{% 1.1g, % 1.1g, % 1.1g}\n", T->basis_y.x, T->basis_y.y, T->basis_y.z );
	printf("{% 1.1g, % 1.1g, % 1.1g}\n", T->basis_z.x, T->basis_z.y, T->basis_z.z );
}


void MultiplyMatrixVector(vec3d *i, vec3d *o, mat4x4 *m){
	   o->x = i->x * m->m[0][0] + i->y * m->m[1][0] + i->z * m->m[2][0] + m->m[3][0];
	   o->y = i->x * m->m[0][1] + i->y * m->m[1][1] + i->z * m->m[2][1] + m->m[3][1];
	   o->z = i->x * m->m[0][2] + i->y * m->m[1][2] + i->z * m->m[2][2] + m->m[3][2];
	float w = i->x * m->m[0][3] + i->y * m->m[1][3] + i->z * m->m[2][3] + m->m[3][3];
	if (w != 0.0){
		o->x /= w;
		o->y /= w;
		o->z /= w;
	}
}

static inline vec3d xform_v3d( Transform_3d T, vec3d v ){
	return (vec3d){ v3d_dot( T.basis_x, v ) + T.origin.x,
					v3d_dot( T.basis_y, v ) + T.origin.y,
					v3d_dot( T.basis_z, v ) + T.origin.z };
}

void build_rotation_matrix( mat4x4 *M, int axis, float angle ){

	memset( M->m, 0, sizeof(float[4][4]) );
	switch( axis ){
		case 0:// X
			M->m[0][0] = 1;
			M->m[1][1] = cos(angle);
			M->m[1][2] = sin(angle);
			M->m[2][1] = -M->m[1][2];
			M->m[2][2] =  M->m[1][1];
			M->m[3][3] = 1;
			break;
		case 1:// Y
			M->m[0][0] = cos(angle);
			M->m[2][0] = sin(angle);
			M->m[1][1] = 1;
			M->m[0][2] = -M->m[2][0];
			M->m[2][2] =  M->m[0][0];
			M->m[3][3] = 1;
			break;
		case 2:// Z
			M->m[0][0] = cos(angle);
			M->m[0][1] = sin(angle);
			M->m[1][0] = -M->m[0][1];
			M->m[1][1] =  M->m[0][0];
			M->m[2][2] = 1;
			M->m[3][3] = 1;
			break;
	}
}

void orthonormalize( Transform_3d *T ) {
	// Gram-Schmidt Process

	vec3d x = (vec3d){ T->basis_x.x, T->basis_y.x, T->basis_z.x };
	vec3d y = (vec3d){ T->basis_x.y, T->basis_y.y, T->basis_z.y };
	vec3d z = (vec3d){ T->basis_x.z, T->basis_y.z, T->basis_z.z };

	v3d_normalize( &x );
	vec3d xsxdy = v3d_scalar_mult( x, v3d_dot(x, y) );
	y = v3d_diff( y, xsxdy );
	v3d_normalize( &y );
	vec3d xsxdz = v3d_scalar_mult( x, v3d_dot(x, z) );
	vec3d ysydz = v3d_scalar_mult( y, v3d_dot(y, z) );
	z = v3d_diff( v3d_diff( z, xsxdz ), ysydz );
	v3d_normalize( &z );

	T->basis_x.x = x.x;
	T->basis_y.x = x.y;
	T->basis_z.x = x.z;

	T->basis_x.y = y.x;
	T->basis_y.y = y.y;
	T->basis_z.y = y.z;

	T->basis_x.z = z.x;
	T->basis_y.z = z.y;
	T->basis_z.z = z.z;
}

void rotate_transform( Transform_3d *T, vec3d rots ){

	mat4x4 matRotX, matRotY, matRotZ;

	build_rotation_matrix( &matRotX, 0, rots.x );
	build_rotation_matrix( &matRotY, 1, rots.y );
	build_rotation_matrix( &matRotZ, 2, rots.z );

	MultiplyMatrixVector( &(T->basis_x), &(T->basis_x), &matRotX );
	MultiplyMatrixVector( &(T->basis_x), &(T->basis_x), &matRotY );
	MultiplyMatrixVector( &(T->basis_x), &(T->basis_x), &matRotZ );

	MultiplyMatrixVector( &(T->basis_y), &(T->basis_y), &matRotX );
	MultiplyMatrixVector( &(T->basis_y), &(T->basis_y), &matRotY );
	MultiplyMatrixVector( &(T->basis_y), &(T->basis_y), &matRotZ );

	MultiplyMatrixVector( &(T->basis_z), &(T->basis_z), &matRotX );
	MultiplyMatrixVector( &(T->basis_z), &(T->basis_z), &matRotY );
	MultiplyMatrixVector( &(T->basis_z), &(T->basis_z), &matRotZ );

	orthonormalize( T );
}

void step_rotate_transform( Transform_3d *T, vec3d rots, float steps ){
	rots = v3d_scalar_mult( rots, 1.0/steps );
	for (int i = 0; i < steps; ++i ){
		rotate_transform( T, rots );
	}
}


double get_transform_angle( Transform_3d *T, vec3d *axis ){

	if (fabs(T->basis_x.y - T->basis_y.x) < DBL_EPSILON && 
		fabs(T->basis_x.z - T->basis_z.x) < DBL_EPSILON && 
		fabs(T->basis_y.z - T->basis_z.y) < DBL_EPSILON) {
		// Singularity found.
		// First check for identity matrix which must have +1 for all terms in leading diagonal and zero in other terms.
		if ( fabs(T->basis_x.x + T->basis_y.y + T->basis_z.z + T->basis_y.z + T->basis_z.y - 3) < 3 * DBL_EPSILON ) {
			// This singularity is identity matrix so angle = 0.
			return 0;
		}
		// Otherwise this singularity is angle = 180.
		float xx = (T->basis_x.x + 1) * 0.5;
		float yy = (T->basis_y.y + 1) * 0.5;
		float zz = (T->basis_z.z + 1) * 0.5;
		float xy = (T->basis_x.y + T->basis_y.x) * 0.25;
		float xz = (T->basis_x.z + T->basis_z.x) * 0.25;
		float yz = (T->basis_y.z + T->basis_z.y) * 0.25;

		if ((xx > yy) && (xx > zz)) { // T->basis_x.x is the largest diagonal term.
			if (xx < DBL_EPSILON) {
				axis->x = 0;
				axis->y = 0.7071067811865;
				axis->z = 0.7071067811865;
			} else {
				axis->x = sqrt(xx);
				axis->y = xy / axis->x;
				axis->z = xz / axis->x;
			}
		} else if (yy > zz) { // T->basis_y.y is the largest diagonal term.
			if (yy < DBL_EPSILON) {
				axis->x = 0.7071067811865;
				axis->y = 0;
				axis->z = 0.7071067811865;
			} else {
				axis->y = sqrt(yy);
				axis->x = xy / axis->y;
				axis->z = yz / axis->y;
			}
		} else { // T->basis_z.z is the largest diagonal term so base result on this.
			if (zz < DBL_EPSILON) {
				axis->x = 0.7071067811865;
				axis->y = 0.7071067811865;
				axis->z = 0;
			} else {
				axis->z = sqrt(zz);
				axis->x = xz / axis->z;
				axis->y = yz / axis->z;
			}
		}
		return PI;
	}
	// As we have reached here there are no singularities so we can handle normally.
	double s = sqrt( (T->basis_z.y - T->basis_y.z) * (T->basis_z.y - T->basis_y.z) + 
					 (T->basis_x.z - T->basis_z.x) * (T->basis_x.z - T->basis_z.x) + 
					 (T->basis_y.x - T->basis_x.y) * (T->basis_y.x - T->basis_x.y) ); // Used to normalize.

	if (fabs(s) < DBL_EPSILON) {
		// Prevent divide by zero, should not happen if matrix is orthogonal and should be caught by singularity test above.
		s = 1;
	}

	axis->x = (T->basis_z.y - T->basis_y.z) / s;
	axis->y = (T->basis_x.z - T->basis_z.x) / s;
	axis->z = (T->basis_y.x - T->basis_x.y) / s;

	// acos does clamping.
	double a = acos((T->basis_x.x + T->basis_y.y + T->basis_z.z - 1) * 0.5);
	//printf("a: %lg\n", a );
	return a;
}

/*
int compare_Tris_Z(const void* p1, const void* p2) {
	Tri* t1 = (Tri*)p1;
	Tri* t2 = (Tri*)p2;
	float z1 = (t1->p[0].z + t1->p[1].z + t1->p[2].z) / 3.0;
	float z2 = (t2->p[0].z + t2->p[1].z + t2->p[2].z) / 3.0;
	return z1 > z2;
}*/








void init_Cam( Camera_3d *Cam, float fov ){
	
	// Projection Matrix
	float fNear = 0.1;
	float fFar = 2200.0;
	float fAspectRatio = 1.0;// Cam->h / Cam->w;
	float fFovRad = 1.0 / tan( radians(fov * 0.5) );

	memset( Cam->matProj.m, 0, sizeof(float[4][4]) );
	Cam->matProj.m[0][0] = fAspectRatio * fFovRad;
	Cam->matProj.m[1][1] = fFovRad;
	Cam->matProj.m[2][2] = fFar / (fFar - fNear);
	Cam->matProj.m[3][2] = (-fFar * fNear) / (fFar - fNear);
	Cam->matProj.m[2][3] = 1.0;
	Cam->matProj.m[3][3] = 0.0;

	Cam->pos = (vec3d){0};

	Cam->light_direction = (vec3d){ 0.0, 0.0, -1.0 };
	//v3d_normalize(&(Cam->light_direction));
}


void print_Dice_3d( Dice_3d *D ){

	printf("Printing D: %p\n", D );
	printf( "   mesh: %p\n", D->mesh );
	printf( "   scale: %g\n   T:", D->scale );
	printT( &(D->T) );
	printf( "   rotvel: %g, %g, %g\n", D->rotvel.x, D->rotvel.y, D->rotvel.z );
	printf( "   inertia: %g\n", D->inertia );
	printf( "   rolling: %d\n", D->rolling );
	printf( "   face_values: %p\n", D->face_values );
	printf( "   value: %d\n", D->value );
	printf( "   tex: %p\n", D->tex );
	printf( "   roll: %p\n", D->roll );
	printf( "   read: %p\n\n", D->read );
}

Mesh load_obj( char *filename ){
	//printf("loading obj: \"%s\"\n", filename );

	Mesh m = (Mesh){ 0 };

	setlocale (LC_ALL, "C");
	FILE *f = fopen( filename, "rb" );
	if( f == NULL ){
		printf("load_obj: couldn't open file \"%s\"\n", filename );
		return m;
	}

	float *vtx = NULL;
	float *vty = NULL;
	int vtn = 0;

	do{
		char c0 = fgetc(f);
		char c1 = fgetc(f);
		if( c0 == 'v' ){
				 if( c1 == ' ' ) m.verts_N += 1;
			else if( c1 == 't' ) vtn += 1;
		}			
		else if( c0 == 'f' && c1 == ' ' ) m.tris_N += 1;
		fseek_lines( f, 1 );
	} while( !feof(f) );

	rewind(f);

	//printf("m.verts_N: %d, m.tris_N: %d, vtn: %d\n", m.verts_N, m.tris_N, vtn );

	m.verts = malloc( m.verts_N * sizeof( vec3d ) );
	m.tris = malloc( m.tris_N * sizeof( Tri ) );
	vtx = malloc( vtn * sizeof(float) );
	vty = malloc( vtn * sizeof(float) );

	char pc = getc(f);
	char c = getc(f);
	int vi = 0;
	int vti = 0, vni = 0, vpi = 0;
	int fi = 0;
	int mat_i = -1;
	while( c != EOF ){
		if( pc == '\n' && c == 'v' ){
			pc = c; c = getc(f);
			switch( c ){
				case ' '://VERTEX
					fscanf( f, "%f %f %f", &(m.verts[vi].x), &(m.verts[vi].y), &(m.verts[vi].z) );
					//printf( "v %f %f %f\n", m.verts[vi].x, m.verts[vi].y, m.verts[vi].z );
					vi++;
					break;
				case 't'://texture coordinates
					fscanf( f, "%f %f", vtx + vti, vty + vti );
					vti++;
					break;
				case 'n'://vertex normals
					break;
				case 'p'://Parameter space vertices
					break;
			}
		}
		else if( pc == '\n' && c == 'f' ){//FACE
			pc = c; c = getc(f);
			for (int v = 0; v < 3; ++v ){
				fscanf( f, "%d", m.tris[fi].vid + v );
				m.tris[fi].vid[v] -= 1;
				c = getc(f);
				if( c == '/' ){
					c = getc(f);
					if( c != '/' ){
						ungetc( c, f );
						int vt;
						fscanf( f, "%d", &vt );
						m.tris[fi].uv[v] = (SDL_FPoint){ vtx[vt-1], 1 - vty[vt-1] };
						c = getc(f);
						if( c == '/' ) goto scan_vn;
					}
					else{// == '/'
						scan_vn:;
						int vn;
						fscanf( f, "%d", &vn );
					}
					pc = c; c = getc(f);
				}
			}
			//printf("f %d %d %d (%d)\n", m.tris[fi].vid[0], m.tris[fi].vid[1], m.tris[fi].vid[2], m.face_mats[fi] );
			fi++;
		}
		else if( pc == '\n' && c == 'u' ){// usemtl
			/*
			str[0] = 'u';
			for (int i = 1; i <= 6; ++i ) str[i] = getc(f);
			str[7] = 0;
			if( strcmp( str, "usemtl " ) == 0 ){
				fgets( str, 64, f ); strtrim_trailingNL(str);
				mat_i = ok_map_get( &matmap, str )-1;
				//printf( "usemtl (%d)\n", mat_i );
				ungetc( '\n', f );
			}*/
		}
		pc = c;
		c = getc(f);
	}

	fclose(f);
	free( vtx );
	free( vty );

	return m;
}

void init_dice_3d( Dice_3d *D, char *obj_file, SDL_Texture *tex, float zdist, float diameter ){

	D->mesh = malloc( sizeof(Mesh) );
	*(D->mesh) = load_obj( obj_file );

	for (int i = 0; i < D->mesh->tris_N; ++i ){
		vec3d vecs [3];
		vecs[0] = D->mesh->verts[ D->mesh->tris[i].vid[0] ];
		vecs[1] = D->mesh->verts[ D->mesh->tris[i].vid[1] ];
		vecs[2] = D->mesh->verts[ D->mesh->tris[i].vid[2] ];
		D->mesh->tris[i].normal = face_normal( vecs );
	}

	D->face_values = malloc( D->mesh->tris_N * sizeof(int) );
	char str [256];
	strcpy( str, obj_file );
	int len = strlen( str );
	sprintf( str + len - 4, " faces.txt" );
	FILE *f = fopen( str, "rb" );
	if( f == NULL ){
		printf("\"%s\" not found, displaying face indices.\n", str );
		for (int i = 0; i < D->mesh->tris_N; ++i ){
			D->face_values[i] = i;
		}
	}
	else{
		//printf("\"%s\" found!", str );
		for ( int i = 0; i < D->mesh->tris_N; ++i ){
			D->face_values[i] = -1;
		}
		fseek_lines( f, 1 );
		for ( int i = 0; i < D->mesh->tris_N; ++i ){
			fscanf( f, "%d", D->face_values + i );
			if( feof(f) ){
				//printf(" it contained %d items.", i+1 );
				break;
			}
		}
		//puts("");
		fclose(f);
	}
	//puts("");

	// This magic number assumes fov=55, zdist=10.
	D->scale = 2.34375 * diameter * (1.0 / v3d_mag(D->mesh->verts[0]));
	D->tex = tex;

	D->rolling = 0;

	D->T.basis_x = (vec3d){ 1, 0, 0 };
	D->T.basis_y = (vec3d){ 0, 1, 0 };
	D->T.basis_z = (vec3d){ 0, 0, 1 };
	D->T.origin = (vec3d){ 0, 0, zdist };

	D->roll = roll_dice;
	D->read = read_dice;
	D->read( D );
}

int roll_dice( Dice_3d *D ){

	D->rotvel = (vec3d){0};
	int d = rand() % 3;
	switch(d){
		case 0:
			D->rotvel.x = random(900,1500) * 0.001;
			if( rand() % 2 == 1 ) D->rotvel.x *= -1;
			D->rotvel.y = random(900,1500) * 0.001;
			if( rand() % 2 == 1 ) D->rotvel.y *= -1;
			break;
		case 1:
			D->rotvel.y = random(900,1500) * 0.001;
			if( rand() % 2 == 1 ) D->rotvel.y *= -1;
			D->rotvel.z = random(900,1500) * 0.001;
			if( rand() % 2 == 1 ) D->rotvel.z *= -1;
			break;
		case 2:
			D->rotvel.z = random(900,1500) * 0.001;
			if( rand() % 2 == 1 ) D->rotvel.z *= -1;
			D->rotvel.x = random(900,1500) * 0.001;
			if( rand() % 2 == 1 ) D->rotvel.x *= -1;
			break;
	}

	/*int q = rand() % 3; switch( q ){ 	case 0: D->rotvel.x = 0; break; 	case 1: D->rotvel.y = 0; break; 	case 2: D->rotvel.z = 0; break; }*/
	//v3d_normalize( &(D->rotvel) );
	D->inertia = random(930, 980) * 0.001;
	D->rolling = 1;
	return 0;
}
int read_dice( Dice_3d *D ){

	int value = -1;
	static const vec3d FWD = (vec3d){ 0, 0, -1 };
	vec3d o = D->T.origin;
	D->T.origin = (vec3d){0};
	float max_alignment = -1;
	for (int v = 0; v < D->mesh->tris_N; ++v ){
		if( D->face_values[ v ] < 0 ) break;
		vec3d Tnormal = xform_v3d( D->T, D->mesh->tris[v].normal );
		float alignment = v3d_dot( Tnormal, FWD );
		if( alignment > max_alignment ){
			max_alignment = alignment;
			value = D->face_values[ v ];
		}
	}
	D->value = value;
	D->T.origin = o;
	return value;
}
void rotate_dice( Dice_3d *D, float dx, float dy ){
	//vec3d rots = (vec3d){ dy * 0.01, dx * 0.01, 0 };
	//rots = xform_v3d( D->T, rots );
	D->rotvel.x = dy * 0.1;//rots.x;
	D->rotvel.y = dx * 0.1;//rots.y;
	D->rotvel.z = 0;//rots.z;
	D->inertia = 0.25;
	D->rolling = 1;
}

void create_prismatic_dice_3d( SDL_Renderer *R, Dice_3d *D, int sides, TX_Font *font, 
	                           float zdist, float diameter, float width_factor     ){
	D->mesh = malloc(sizeof(Mesh));

	D->mesh->verts_N = 2 * sides;
	D->mesh->verts = malloc( D->mesh->verts_N * sizeof(vec3d) );
	float r = 0.5;//diameter *
	float wr = r * width_factor;
	float a = TWO_PI / sides;
	for (int i = 0; i < sides; ++i ){
		D->mesh->verts[  2*i    ] = (vec3d){ wr, r * sin(i*a), r * cos(i*a) };
		D->mesh->verts[ (2*i)+1 ] = D->mesh->verts[ 2*i ];
		D->mesh->verts[ (2*i)+1 ].x = -wr;
		//printf("D->mesh->verts[%d]: %g, %g, %g\n", 2*i, D->mesh->verts[ 2*i ].x, D->mesh->verts[ 2*i ].y, D->mesh->verts[ 2*i ].z  );
	}

	D->face_values = malloc( sides * sizeof(int) );
	for (int i = 0; i < sides; ++i ) D->face_values[i] = i+1;
	shuffle( D->face_values, sides );

	float fh = 2 * diameter * r * sin(PI/sides);//the height of a face
	float tscl = (font->h * font->scale * 1.15) / fh;
	fh *= tscl;
	int tw = tscl * diameter * width_factor;
	int th = lrint( sides * fh );
	float ym = (font->h * font->scale * 0.075);
	//printf("tw: %d, th: %d\n", tw, th );
	D->tex = SDL_CreateTexture( R, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, tw, th );
	SDL_SetRenderTarget( R, D->tex );
	SDL_SetRenderDrawColor( R, 255,255,255,255 );
	SDL_RenderClear( R );
	for (int i = 0; i < sides; ++i ){
		char str [8];
		sprintf(str, "%d", D->face_values[i] );
		TX_render_string_wrapped_aligned( R, font, str, 0, (fh * i) + ym, tw, TX_ALIGN_CENTER );
	}
	SDL_SetRenderTarget( R, NULL );
	//save_texture( "D9.png", R, D->tex );

	D->mesh->tris_N = 2 * sides;
	D->mesh->tris = malloc( D->mesh->tris_N * sizeof(Tri) );
	float oos = 1.0 / sides;
	for (int i = 0; i < sides; ++i ){
		int p =   2 *  i;                             /* p---q */
		int q =  (2 *  i)   +1;                       /* | _/| */
		int b =  (2 * (i+1))    % (D->mesh->verts_N); /* |/  | */
		int d = ((2 * (i+1))+1) % (D->mesh->verts_N); /* b---d */
		D->mesh->tris[ i       ].vid[0] = p;
		D->mesh->tris[ i       ].vid[1] = b;
		D->mesh->tris[ i       ].vid[2] = q;
		D->mesh->tris[ i+sides ].vid[0] = b;
		D->mesh->tris[ i+sides ].vid[1] = d;
		D->mesh->tris[ i+sides ].vid[2] = q;
		D->mesh->tris[ i       ].uv[0] = (SDL_FPoint){1, oos*(i+1)};
		D->mesh->tris[ i       ].uv[1] = (SDL_FPoint){1, oos*i};
		D->mesh->tris[ i       ].uv[2] = (SDL_FPoint){0, oos*(i+1)};
		D->mesh->tris[ i+sides ].uv[0] = (SDL_FPoint){1, oos*i};
		D->mesh->tris[ i+sides ].uv[1] = (SDL_FPoint){0, oos*i};
		D->mesh->tris[ i+sides ].uv[2] = (SDL_FPoint){0, oos*(i+1)};
		D->mesh->tris[ i ].normal = (vec3d){ 0, sin((i+0.5)*a), cos((i+0.5)*a) };
		D->mesh->tris[ i+sides ].normal = D->mesh->tris[ i ].normal;
	}
	
	D->scale = 2.34375 * diameter * (1.0 / v3d_mag(D->mesh->verts[0]));

	D->T.basis_x = (vec3d){ 1, 0, 0 };
	D->T.basis_y = (vec3d){ 0, 1, 0 };
	D->T.basis_z = (vec3d){ 0, 0, 1 };
	D->T.origin = (vec3d){ 0, 0, zdist };

	D->rolling = 0;

	D->roll = roll_prism;
	D->read = read_prism;
}

int roll_prism( Dice_3d *D ){
	D->rotvel = (vec3d){0};
	D->rotvel.x = random(1200,2000) * 0.0004;
	if( rand()%2 == 0 ) D->rotvel.x *= -1;
	D->inertia = random(950, 990) * 0.001;
	D->rolling = 1;
	//puts("roll~");printT( &(D->T) );
	//printf("D->rotvel: %g, %g, %g\n", D->rotvel.x, D->rotvel.y, D->rotvel.z );
	return 0;
}
int read_prism( Dice_3d *D ){

	vec3d axis = {0};
	double a = get_transform_angle( &(D->T), &axis );
	a = (axis.x > 0)? a+PI : PI-a;
	int sides = (D->mesh->tris_N / 2);
	float face = TWO_PI / sides;
	int i = ((int)(a / face)) % sides;
	D->value = D->face_values[i];
	//puts("read~");printT( &(D->T) );
	return D->value;
}

void create_coin_3d( SDL_Renderer *R, Dice_3d *D, int heads, int tails, int res, 
					 TX_Font *font, float zdist, float diameter, float thickness_factor ){

	D->mesh = malloc(sizeof(Mesh));

	D->mesh->verts_N = 2 + (2 * res);
	D->mesh->verts = malloc( D->mesh->verts_N * sizeof(vec3d) );
	float r = 0.5;
	float ht = thickness_factor * 0.5;
	float a = TWO_PI / res;
	for (int i = 0; i < res; ++i ){
		D->mesh->verts[ (2*i)   ] = (vec3d){ r * cos(i*a), r * sin(i*a), ht };
		D->mesh->verts[ (2*i)+1 ] = D->mesh->verts[ (2*i) ];
		D->mesh->verts[ (2*i)+1 ].z = -ht;
	}
	int cv1 = 2*res;
	int cv2 = cv1+1;
	D->mesh->verts[ cv1 ] = (vec3d){ 0, 0,  ht };
	D->mesh->verts[ cv2 ] = (vec3d){ 0, 0, -ht };

	D->face_values = malloc( 3 * sizeof(int) );
	D->face_values[0] = tails;
	D->face_values[1] = heads;
	D->face_values[2] = -1;

	float th = font->h * font->scale * 1.15;
	D->tex = SDL_CreateTexture( R, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, lrint(th), lrint(2*th) );
	SDL_SetRenderTarget( R, D->tex );
	SDL_SetRenderDrawColor( R, 255,255,255,255 );
	SDL_RenderClear( R );
	for (int i = 0; i < 2; ++i ){
		char str [8];
		sprintf(str, "%d", D->face_values[i] );
		TX_render_string_centered( R, font, str, th*0.5, th*(i+0.5) );
	}
	SDL_SetRenderTarget( R, NULL );

	D->mesh->tris_N = 4 * res;
	D->mesh->tris = malloc( D->mesh->tris_N * sizeof(Tri) );
	float pxl = 1.0 / (2*th);
	for (int i = 0; i < res; ++i ){
		int p =   2 *  i;                    /*      /p---q\      */
		int q =  (2 *  i)   +1;              /*  cv1/ | _/| \     */
		int b =  (2 * (i+1))    % (2 * res); /*     \ |/  | /cv2  */
		int d = ((2 * (i+1))+1) % (2 * res); /*      \b---d/      */

		D->mesh->tris[ (4*i)+0 ].vid[0] = b;
		D->mesh->tris[ (4*i)+0 ].vid[1] = p;
		D->mesh->tris[ (4*i)+0 ].vid[2] = cv1;
		D->mesh->tris[ (4*i)+0 ].uv[0] = (SDL_FPoint){ (D->mesh->verts[b].x + 0.5), 0.5 * (D->mesh->verts[b].y + 0.5) };
		D->mesh->tris[ (4*i)+0 ].uv[1] = (SDL_FPoint){ (D->mesh->verts[p].x + 0.5), 0.5 * (D->mesh->verts[p].y + 0.5) };
		D->mesh->tris[ (4*i)+0 ].uv[2] = (SDL_FPoint){0.5, 0.25};
		D->mesh->tris[ (4*i)+0 ].normal = (vec3d){ 0, 0, -1 };

		D->mesh->tris[ (4*i)+1 ].vid[0] = q;
		D->mesh->tris[ (4*i)+1 ].vid[1] = d;
		D->mesh->tris[ (4*i)+1 ].vid[2] = cv2;
		D->mesh->tris[ (4*i)+1 ].uv[0] = (SDL_FPoint){ (D->mesh->verts[q].x + 0.5), (0.5 * (0.5 - D->mesh->verts[q].y))+0.5 };
		D->mesh->tris[ (4*i)+1 ].uv[1] = (SDL_FPoint){ (D->mesh->verts[d].x + 0.5), (0.5 * (0.5 - D->mesh->verts[d].y))+0.5 };
		D->mesh->tris[ (4*i)+1 ].uv[2] = (SDL_FPoint){0.5, 0.75};
		D->mesh->tris[ (4*i)+1 ].normal = (vec3d){ 0, 0, 1 };

		D->mesh->tris[ (4*i)+2 ].vid[0] = p; // 2*i;
		D->mesh->tris[ (4*i)+2 ].vid[1] = b; //(2*(i+1)) % (D->mesh->verts_N);
		D->mesh->tris[ (4*i)+2 ].vid[2] = q; //(2*i)+1;
		D->mesh->tris[ (4*i)+2 ].uv[0] = (SDL_FPoint){0,   0  };
		D->mesh->tris[ (4*i)+2 ].uv[1] = (SDL_FPoint){0,   pxl};
		D->mesh->tris[ (4*i)+2 ].uv[2] = (SDL_FPoint){pxl, 0  };
		D->mesh->tris[ (4*i)+2 ].normal = (vec3d){ 0, sin((i+0.5)*a), cos((i+0.5)*a) };

		D->mesh->tris[ (4*i)+3 ].vid[0] = b; //(2*(i+1)) % (D->mesh->verts_N);
		D->mesh->tris[ (4*i)+3 ].vid[1] = d; //((2*(i+1))+1) % (D->mesh->verts_N);
		D->mesh->tris[ (4*i)+3 ].vid[2] = q; //(2*i)+1;
		D->mesh->tris[ (4*i)+3 ].uv[0] = (SDL_FPoint){0,   pxl};
		D->mesh->tris[ (4*i)+3 ].uv[1] = (SDL_FPoint){pxl, pxl};
		D->mesh->tris[ (4*i)+3 ].uv[2] = (SDL_FPoint){pxl, 0  };
		D->mesh->tris[ (4*i)+3 ].normal = D->mesh->tris[ i+2 ].normal;
	}
	
	D->scale = 2.34375 * diameter * (1.0 / v3d_mag(D->mesh->verts[0]));

	D->T.basis_x = (vec3d){ 1, 0, 0 };
	D->T.basis_y = (vec3d){ 0, 1, 0 };
	D->T.basis_z = (vec3d){ 0, 0, 1 };
	D->T.origin = (vec3d){ 0, 0, zdist };

	D->rolling = 0;

	D->roll = roll_prism;
	D->read = read_dice;
}

void scramble_dice( Dice_3d *D, int N ){
	for (int i = 0; i < N; ++i ){
		D->roll( D );
		while( D->rolling ){
			rotate_transform( &(D->T), D->rotvel );
			D->rotvel = v3d_scalar_mult( D->rotvel, D->inertia );
			if( v3d_dot( D->rotvel, D->rotvel ) < 0.00001 ) D->rolling = 0;
		}
	}
	D->read( D );
}

void render_dice_3d( SDL_Renderer *R, Dice_3d *D, Camera_3d *Cam, float tx, float ty ){

	if( D->rolling ){
		rotate_transform( &(D->T), D->rotvel );
		D->rotvel = v3d_scalar_mult( D->rotvel, D->inertia );
		if( v3d_dot( D->rotvel, D->rotvel ) < 0.00001 ) D->rolling = 0;
		D->read( D );
	}
	// Store triangles for rastering later
	vec3d *the_verts = NULL;
	int verts_N = 0;
	int *vids = malloc( D->mesh->verts_N * sizeof(int) );
	for (int v = 0; v < D->mesh->verts_N; ++v ){
		vids[v] = -1;
	}
	Tri *the_tris = NULL;
	int tris_N = 0;

	// Draw Tris
	for (int i = 0; i < D->mesh->tris_N; ++i ){
		//printf("%d, ", i );		

		//triTranslated, triRotX, triRotY, triRotZ;
		vec3d ttp [3];
		
		ttp[0] = xform_v3d( D->T, D->mesh->verts[ D->mesh->tris[i].vid[0] ] );
		ttp[1] = xform_v3d( D->T, D->mesh->verts[ D->mesh->tris[i].vid[1] ] );
		ttp[2] = xform_v3d( D->T, D->mesh->verts[ D->mesh->tris[i].vid[2] ] );
		
		vec3d normal = face_normal( ttp );

		if( normal.x * (ttp[0].x - Cam->pos.x) + 
		    normal.y * (ttp[0].y - Cam->pos.y) +
		    normal.z * (ttp[0].z - Cam->pos.z) < 0.0){

			vec3d triProjected [3]; 

			// Project triangles from 3D --> 2D
			MultiplyMatrixVector(ttp + 0, triProjected + 0, &(Cam->matProj));
			MultiplyMatrixVector(ttp + 1, triProjected + 1, &(Cam->matProj));
			MultiplyMatrixVector(ttp + 2, triProjected + 2, &(Cam->matProj));

			// Scale into view
			triProjected[0].x *= D->scale; triProjected[0].y *= D->scale;
			triProjected[1].x *= D->scale; triProjected[1].y *= D->scale;
			triProjected[2].x *= D->scale; triProjected[2].y *= D->scale;

			// How similar is normal to light direction
			float dp = v3d_dot( normal, Cam->light_direction );

			// Store triangle			
			int vp = 0;
			for (int j = 0; j < 3; ++j ){
				if( vids[ D->mesh->tris[i].vid[j] ] < 0 ){
					vp++;
				}
			}
			if( vp > 0 ) the_verts = realloc( the_verts, (verts_N + vp) * sizeof(vec3d) );
			the_tris = realloc( the_tris, (tris_N+1) * sizeof(Tri) );

			for (int j = 0; j < 3; ++j ){
				if( vids[ D->mesh->tris[i].vid[j] ] < 0 ){
					the_verts[ verts_N ] = triProjected[j];
					vids[ D->mesh->tris[i].vid[j] ] = verts_N;
					verts_N += 1;
				}
				the_tris[ tris_N ].vid[j] = vids[ D->mesh->tris[i].vid[j] ];
				the_tris[ tris_N ].uv[j] = D->mesh->tris[i].uv[j];
				//printf("uv[%d] = %f, %f\n", j, the_tris[ tris_N ].uv[j].x, the_tris[ tris_N ].uv[j].y );
			}
			int c = lrint(dp * 255);
			the_tris[ tris_N ].C = (SDL_Color){ c, c, c, 255 };
			tris_N++;
		}
	}

	free( vids );
	//printf("\ntris_N: %d\n", tris_N );

	// Sort triangles from back to front (we can get away with not doing this for convex shapes!)
	// qsort ( the_tris, tris_N, sizeof(Tri), compare_Tris_Z );

	//SDL_SetRenderDrawColor( R, 0,200,255,255 );
	for (int i = 0; i < tris_N; ++i ){

		SDL_Vertex verts [3];
		verts[0] = (SDL_Vertex){ { tx + the_verts[ the_tris[i].vid[0] ].x, 
								   ty + the_verts[ the_tris[i].vid[0] ].y }, the_tris[i].C, the_tris[i].uv[0] };
		verts[1] = (SDL_Vertex){ { tx + the_verts[ the_tris[i].vid[1] ].x, 
								   ty + the_verts[ the_tris[i].vid[1] ].y }, the_tris[i].C, the_tris[i].uv[1] };
		verts[2] = (SDL_Vertex){ { tx + the_verts[ the_tris[i].vid[2] ].x, 
								   ty + the_verts[ the_tris[i].vid[2] ].y }, the_tris[i].C, the_tris[i].uv[2] };

		if( SDL_RenderGeometry( R, D->tex, verts, 3, NULL, 0 ) < 0 ){
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_RenderGeometry error: %s", SDL_GetError());
		}
		//wireframe
		//SDL_RenderDrawLineF( R, verts[0].position.x, verts[0].position.y, verts[1].position.x, verts[1].position.y );
		//SDL_RenderDrawLineF( R, verts[1].position.x, verts[1].position.y, verts[2].position.x, verts[2].position.y );
		//SDL_RenderDrawLineF( R, verts[2].position.x, verts[2].position.y, verts[0].position.x, verts[0].position.y );
	}

	free( the_verts );
	free( the_tris );
}

void test_dice( Dice_3d *D, int faces, int N, bool reset ){

	printf( "testing %d-sided dice, %d throws.\nresetting after every throw: %c\n", faces, N, reset? 'Y' : 'N' );
	int *results = calloc( faces, sizeof(int) );
	int prev = -1;
	int repeats = 0;

	for (int n = 0; n < N; ++n ){
		if( reset ){
			reset_Transform( &(D->T) );
			//vec3d rots = (vec3d){ (rand()%4)*HALF_PI, (rand()%4)*HALF_PI, (rand()%4)*HALF_PI };
			//step_rotate_transform( &(D->T), rots, 4 );
			scramble_dice( D, 3 );
		}
		scramble_dice( D, 1 );
		int r = D->read( D );
		results[r-1] += 1;
		if( r == prev ) repeats += 1;
		prev = r;
	}
	float k = 100.0 / (float)N;
	float expected = N / (float)faces;
	for (int i = 0; i < faces; ++i ){
		printf("%d: %d (%.2f%%, %+.2f%%)\n", i+1, results[i], k * results[i], k * (results[i]-expected) );
	}
	printf("repeats: %d (%.2f%%, %+.2f%%)\n", repeats, k * repeats, k * (repeats-expected) );
	puts("");
	reset_Transform( &(D->T) );
}

int destroy_dice( Dice_3d *D ){
	free( D->mesh->verts );
	free( D->mesh->tris );
	free( D->mesh );
	free( D->face_values );
	SDL_DestroyTexture( D->tex );
	//free( D );
}