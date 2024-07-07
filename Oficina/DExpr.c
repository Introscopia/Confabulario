#include "DExpr.h"
#include <ctype.h>

static int comp_ints (const void * a, const void * b){
	return ( *(int*)a - *(int*)b );
}

DExpr new_DExpr( int type ){
	DExpr out = {0};
	out.type = type;
	//out.parenthetical = 0;
	return out;
}


static void consume_token( DExpr **head, DE_vec *tokens, int *t, int tok_len ){

	DExpr *TOKEN = ok_vec_get_ptr(tokens, *t);
	//printf("%d, ", TOKEN->type );
	switch( TOKEN->type ){

		case TK_OPR:{
			*t += 1;
			DExpr *subhead = NULL;
			do{
				consume_token( &subhead, tokens, t, tok_len );
				*t += 1;
			} while( ok_vec_get_ptr(tokens, *t)->type != TK_CPR );

			subhead->parenthetical = 1;

			if( *head == NULL ){
				*head = subhead;
			}
			else{
				(*head)->dat.opr.R_operand = subhead;
			}

			}break;

		case DE_NUM:{

			if( (*t) < tok_len-2 && ok_vec_get_ptr(tokens, (*t)+1)->type == DE_DPL ){

				*head = malloc( sizeof(DExpr) );
				(*head)->type = DE_DPL;
				(*head)->parenthetical = 0;
				(*head)->dat.dpl.num = TOKEN->dat.lit.value;
				DExpr *STOK = ok_vec_get_ptr(tokens, (*t)+2);
				if( STOK->type != DE_NUM ){
					static const char errmsg [] = "Expected a number after D";
					(*head)->type = DE_NUL;
					(*head)->dat.err = errmsg;
				} else{
					(*head)->dat.dpl.sides = STOK->dat.lit.value;
				}
				*t += 2;

			} else{
				*head = malloc( sizeof(DExpr) );
				(*head)->type = DE_NUM;
				(*head)->parenthetical = 0;
				(*head)->dat.lit.value = TOKEN->dat.lit.value;
			}

			}break;

		// BINARY OPS
		case DE_EQL:
		case DE_NEQ:
		case DE_GTE:
		case DE_GTN:
		case DE_LTE:
		case DE_LTN:
		case DE_CFM:
		case DE_ADD:
		case DE_MUL:
		case DE_DIV:;
			binary_op:

			// NEW EXPR HAS PRECEDENCE
			if( *head == NULL || (*head)->type > TOKEN->type || (*head)->parenthetical ){
				DExpr *temp = *head;
				*head = malloc( sizeof(DExpr) );
				(*head)->type = TOKEN->type;
				(*head)->parenthetical = 0;
				(*head)->dat.opr.L_operand = temp;
				(*head)->dat.opr.R_operand = NULL;
				*t += 1;
				consume_token( &((*head)->dat.opr.R_operand), tokens, t, tok_len );
			}
			// HEAD HAS PRECEDENCE
			else{
				DExpr *temp = (*head)->dat.opr.R_operand;
				(*head)->dat.opr.R_operand = malloc( sizeof(DExpr) );
				(*head)->dat.opr.R_operand->type = TOKEN->type;
				(*head)->dat.opr.R_operand->parenthetical = 0;
				(*head)->dat.opr.R_operand->dat.opr.L_operand = temp;
				(*head)->dat.opr.R_operand->dat.opr.R_operand = NULL;
				*t += 1;
				consume_token( &((*head)->dat.opr.R_operand->dat.opr.R_operand), tokens, t, tok_len );
			}

			break;

		//UNARY OPS
		case DE_NEG:

			if( *t > 0 ){
				DExpr *PTOK = ok_vec_get_ptr( tokens, (*t)-1 );
				if( PTOK->type == DE_NUM ||
					PTOK->type == DE_DPL ||
					PTOK->type == TK_CPR ){

					TOKEN->type = DE_SUB;
					goto binary_op;
				}
			}
			// else fall through
		case DE_NOT:{

			*head = malloc( sizeof(DExpr) );
			(*head)->type = TOKEN->type;
			(*head)->parenthetical = 0;
			(*head)->dat.opr.L_operand = NULL;
			(*head)->dat.opr.R_operand = NULL;
			*t += 1;
			consume_token( &((*head)->dat.opr.R_operand), tokens, t, tok_len );

			} break;
	}
}


DExpr* compile_DExpr( char *code ){

	DE_vec tokens;
	ok_vec_init( &tokens );

	int si = -1;
	int type = DE_NUL;

	for( int i = 0; code[i] != '\0'; i++ ){

		if( code[i] == '(' ){
			ok_vec_push( &tokens, new_DExpr( TK_OPR ) );
		}
		else if( code[i] == ')' ){
			ok_vec_push( &tokens, new_DExpr( TK_CPR ) );
		}
		if( isdigit(code[i]) ){
			char buf [32];
			int c = 0;
			do{
				buf[c] = code[i+c];
				c += 1;
			} while( isdigit(code[i+c]) );
			buf[c] = '\0';
			ok_vec_push( &tokens, new_DExpr( DE_NUM ) );
			ok_vec_last( &tokens )->dat.lit.value = atoi( buf );
			i += (c-1);
		}
		else if( code[i] == 'd' || code[i] == 'D' ){
			ok_vec_push( &tokens, new_DExpr( DE_DPL ) );
		}
		else if( code[i] == '=' ){
			if( code[i+1] == '=' ) i += 1;
			ok_vec_push( &tokens, new_DExpr( DE_EQL ) );
		}
		else if( code[i] == '>' ){
			if( code[i+1] == '=' ){
				ok_vec_push( &tokens, new_DExpr( DE_GTE ) );
				i += 1;
			} else{
				ok_vec_push( &tokens, new_DExpr( DE_GTN ) );
			}
		}
		else if( code[i] == '<' ){
			if( code[i+1] == '=' ){
				ok_vec_push( &tokens, new_DExpr( DE_LTE ) );
				i += 1;
			} else{
				ok_vec_push( &tokens, new_DExpr( DE_LTN ) );
			}
		}
		else if( code[i] == '@' ){
			ok_vec_push( &tokens, new_DExpr( DE_CFM ) );
		}
		else if( code[i] == '+' ){
			ok_vec_push( &tokens, new_DExpr( DE_ADD ) );
		}
		else if( code[i] == '-' ){
			ok_vec_push( &tokens, new_DExpr( DE_NEG ) );
		}
		else if( code[i] == '*' ){
			ok_vec_push( &tokens, new_DExpr( DE_MUL ) );
		}
		else if( code[i] == '/' ){
			ok_vec_push( &tokens, new_DExpr( DE_DIV ) );
		}
		else if( code[i] == '!' ){
			if( code[i+1] == '=' ){
				ok_vec_push( &tokens, new_DExpr( DE_NEQ ) );
				i += 1;
			}
			else{
				ok_vec_push( &tokens, new_DExpr( DE_NOT ) );
			}
		}
		else if( !isblank(code[i]) ){
			ok_vec_push( &tokens, new_DExpr( DE_NUL ) );
			static const char errmsg [] = "unrecognized token";
			ok_vec_last( &tokens )->dat.err = errmsg;
		}
	}

	DExpr *head = NULL;
	int tok_len = ok_vec_count( &tokens );
	bool expecting_r_operand = 0;
	int t = 0;
	do{
		consume_token( &head, &tokens, &t, tok_len );
		t += 1;
	} while( t < tok_len );

	return head;
}

int count_dpools( DExpr *DE ){

	switch( DE->type ){

		case DE_NUM:
			return 0;

		case DE_DPL:
			return 1;

		// BINARY
		case DE_EQL:
		case DE_NEQ:
		case DE_GTE:
		case DE_GTN:
		case DE_LTE:
		case DE_LTN:
		case DE_CFM:
		case DE_ADD:
		case DE_SUB:
		case DE_MUL:
		case DE_DIV:
			return count_dpools( DE->dat.opr.L_operand ) + count_dpools( DE->dat.opr.R_operand );

		//UNARY OPS
		case DE_NEG:
		case DE_NOT:
			return count_dpools( DE->dat.opr.R_operand );
	}

	return 0;
}

static DExpr* inner_fetch_dpool( DExpr *DE, int *id ){
	switch( DE->type ){

		case DE_DPL:
			*id -= 1;
			if( *id < 0 ){
				return DE;
			}
			break;

		// BINARY
		case DE_EQL:
		case DE_NEQ:
		case DE_GTE:
		case DE_GTN:
		case DE_LTE:
		case DE_LTN:
		case DE_CFM:
		case DE_ADD:
		case DE_SUB:
		case DE_MUL:
		case DE_DIV:{
			DExpr *db = inner_fetch_dpool( DE->dat.opr.L_operand, id );
			if( db != NULL ) return db;
			db = inner_fetch_dpool( DE->dat.opr.R_operand, id );
			if( db != NULL ) return db;
			}break;

		//UNARY OPS
		case DE_NEG:
		case DE_NOT:{
			DExpr *db = inner_fetch_dpool( DE->dat.opr.R_operand, id );
			if( db != NULL ) return db;
			}break;
	}
	return NULL;
}
DExpr* fetch_dpool( DExpr *DE, int id ){
	return inner_fetch_dpool( DE, &id );
}

static bool DE_has_L_opr( DExpr *DE ){
	if( DE->type >= DE_EQL && DE->type <= DE_DIV ) return 1;
	return 0;
}
static bool DE_has_R_opr( DExpr *DE ){
	if( DE->type >= DE_EQL && DE->type <= DE_NEG ) return 1;
	return 0;
}

void destroy_DExpr( DExpr *DE ){
	//printf("  X<%d>\n", DE->type );
	if( DE_has_L_opr( DE ) ){
		destroy_DExpr( DE->dat.opr.L_operand );
	}
	if( DE_has_R_opr( DE ) ){
		destroy_DExpr( DE->dat.opr.R_operand );
	}
	if( DE->type == DE_DPL && DE->dat.dpl.values != NULL ){
		free( DE->dat.dpl.values );
	}
	free( DE );
}

void print_DExpr( DExpr *DE ){

	if( DE->parenthetical ) putchar('(');

	switch( DE->type ){

		case DE_NUL:
			printf( "{Error:\"%s\"}", DE->dat.err );
			break;

		case DE_NUM:
			printf( "%d", DE->dat.lit.value );
			break;

		case DE_DPL:
			printf( "%dD%d", DE->dat.dpl.num, DE->dat.dpl.sides );
			break;

		// BINARY OPS
		case DE_EQL:
			print_DExpr( DE->dat.opr.L_operand );
			printf(" == ");
			print_DExpr( DE->dat.opr.R_operand );
			break;
		case DE_NEQ:
			print_DExpr( DE->dat.opr.L_operand );
			printf(" != ");
			print_DExpr( DE->dat.opr.R_operand );
			break;
		case DE_GTE:
			print_DExpr( DE->dat.opr.L_operand );
			printf(" >= ");
			print_DExpr( DE->dat.opr.R_operand );
			break;
		case DE_GTN:
			print_DExpr( DE->dat.opr.L_operand );
			printf(" > ");
			print_DExpr( DE->dat.opr.R_operand );
			break;
		case DE_LTE:
			print_DExpr( DE->dat.opr.L_operand );
			printf(" <= ");
			print_DExpr( DE->dat.opr.R_operand );
			break;
		case DE_LTN:
			print_DExpr( DE->dat.opr.L_operand );
			printf(" < ");
			print_DExpr( DE->dat.opr.R_operand );
			break;
		case DE_CFM:
			print_DExpr( DE->dat.opr.L_operand );
			printf(" @ ");
			print_DExpr( DE->dat.opr.R_operand );
			break;
		case DE_ADD:
			print_DExpr( DE->dat.opr.L_operand );
			printf(" + ");
			print_DExpr( DE->dat.opr.R_operand );
			break;
		case DE_SUB:
			print_DExpr( DE->dat.opr.L_operand );
			printf(" - ");
			print_DExpr( DE->dat.opr.R_operand );
			break;
		case DE_MUL:
			print_DExpr( DE->dat.opr.L_operand );
			printf(" * ");
			print_DExpr( DE->dat.opr.R_operand );
			break;
		case DE_DIV:
			print_DExpr( DE->dat.opr.L_operand );
			printf(" / ");
			print_DExpr( DE->dat.opr.R_operand );
			break;

		//UNARY OPS
		case DE_NEG:
			printf(" -");
			print_DExpr( DE->dat.opr.R_operand );
			break;

		case DE_NOT:printf(" !");
			print_DExpr( DE->dat.opr.R_operand );
			break;

	}

	if( DE->parenthetical ) putchar(')');
}

void sprint_DExpr( char *buf, DExpr *DE ){

	if( DE->parenthetical ) strcat(buf, "(");

	int l = strlen(buf);

	switch( DE->type ){

		case DE_NUL:
			sprintf( buf+l, "{Error:\"%s\"}", DE->dat.err );
			break;

		case DE_NUM:
			sprintf( buf+l, "%d", DE->dat.lit.value );
			break;

		case DE_DPL:
			sprintf( buf+l, "%dD%d", DE->dat.dpl.num, DE->dat.dpl.sides );
			break;

		// BINARY OPS
		case DE_EQL:
			sprint_DExpr( buf, DE->dat.opr.L_operand );
			strcat( buf," == ");
			sprint_DExpr( buf, DE->dat.opr.R_operand );
			break;
		case DE_NEQ:
			sprint_DExpr( buf, DE->dat.opr.L_operand );
			strcat( buf," != ");
			sprint_DExpr( buf, DE->dat.opr.R_operand );
			break;
		case DE_GTE:
			sprint_DExpr( buf, DE->dat.opr.L_operand );
			strcat( buf," >= ");
			sprint_DExpr( buf, DE->dat.opr.R_operand );
			break;
		case DE_GTN:
			sprint_DExpr( buf, DE->dat.opr.L_operand );
			strcat( buf," > ");
			sprint_DExpr( buf, DE->dat.opr.R_operand );
			break;
		case DE_LTE:
			sprint_DExpr( buf, DE->dat.opr.L_operand );
			strcat( buf," <= ");
			sprint_DExpr( buf, DE->dat.opr.R_operand );
			break;
		case DE_LTN:
			sprint_DExpr( buf, DE->dat.opr.L_operand );
			strcat( buf," < ");
			sprint_DExpr( buf, DE->dat.opr.R_operand );
			break;
		case DE_CFM:
			sprint_DExpr( buf, DE->dat.opr.L_operand );
			strcat( buf," @ ");
			sprint_DExpr( buf, DE->dat.opr.R_operand );
			break;
		case DE_ADD:
			sprint_DExpr( buf, DE->dat.opr.L_operand );
			strcat( buf," + ");
			sprint_DExpr( buf, DE->dat.opr.R_operand );
			break;
		case DE_SUB:
			sprint_DExpr( buf, DE->dat.opr.L_operand );
			strcat( buf," - ");
			sprint_DExpr( buf, DE->dat.opr.R_operand );
			break;
		case DE_MUL:
			sprint_DExpr( buf, DE->dat.opr.L_operand );
			strcat( buf," * ");
			sprint_DExpr( buf, DE->dat.opr.R_operand );
			break;
		case DE_DIV:
			sprint_DExpr( buf, DE->dat.opr.L_operand );
			strcat( buf," / ");
			sprint_DExpr( buf, DE->dat.opr.R_operand );
			break;

		//UNARY OPS
		case DE_NEG:
			strcat( buf," -");
			sprint_DExpr( buf, DE->dat.opr.R_operand );
			break;

		case DE_NOT:
			strcat( buf," !");
			sprint_DExpr( buf, DE->dat.opr.R_operand );
			break;

	}

	if( DE->parenthetical ) strcat(buf, ")");
}

static bool all_leaves_are( DExpr *DE, int type ){
	bool out = 0;
	if( DE_has_L_opr( DE ) ){
		out |= all_leaves_are( DE->dat.opr.L_operand, type );
	}
	if( DE_has_R_opr( DE ) ){
		out |= all_leaves_are( DE->dat.opr.R_operand, type );
	}
	if( DE->type < DE_DPL ) return out;
	else{
		return DE->type == type;
	}
}

static void inner_sprint_DExpr_dpl_ids( char *buf, DExpr *DE, int *id ){
	if( DE->parenthetical ) strcat(buf, "(");

	int l = strlen(buf);

	switch( DE->type ){

		case DE_NUL:
			sprintf( buf+l, "{Error:\"%s\"}", DE->dat.err );
			break;

		case DE_NUM:
			sprintf( buf+l, "%d", DE->dat.lit.value );
			break;

		case DE_DPL:
			sprintf( buf+l, "$%d", *id );
			*id += 1;
			break;

		// BINARY OPS
		case DE_EQL:
			inner_sprint_DExpr_dpl_ids( buf, DE->dat.opr.L_operand, id );
			strcat( buf," = ");
			inner_sprint_DExpr_dpl_ids( buf, DE->dat.opr.R_operand, id );
			break;
		case DE_NEQ:
			inner_sprint_DExpr_dpl_ids( buf, DE->dat.opr.L_operand, id );
			strcat( buf," ≠ ");
			inner_sprint_DExpr_dpl_ids( buf, DE->dat.opr.R_operand, id );
			break;
		case DE_GTE:
			inner_sprint_DExpr_dpl_ids( buf, DE->dat.opr.L_operand, id );
			strcat( buf," ≥ ");
			inner_sprint_DExpr_dpl_ids( buf, DE->dat.opr.R_operand, id );
			break;
		case DE_GTN:
			inner_sprint_DExpr_dpl_ids( buf, DE->dat.opr.L_operand, id );
			strcat( buf," > ");
			inner_sprint_DExpr_dpl_ids( buf, DE->dat.opr.R_operand, id );
			break;
		case DE_LTE:
			inner_sprint_DExpr_dpl_ids( buf, DE->dat.opr.L_operand, id );
			strcat( buf," ≤ ");
			inner_sprint_DExpr_dpl_ids( buf, DE->dat.opr.R_operand, id );
			break;
		case DE_LTN:
			inner_sprint_DExpr_dpl_ids( buf, DE->dat.opr.L_operand, id );
			strcat( buf," < ");
			inner_sprint_DExpr_dpl_ids( buf, DE->dat.opr.R_operand, id );
			break;
		case DE_CFM:
			inner_sprint_DExpr_dpl_ids( buf, DE->dat.opr.L_operand, id );
			strcat( buf," @ ");
			inner_sprint_DExpr_dpl_ids( buf, DE->dat.opr.R_operand, id );
			break;
		case DE_ADD:
			//strcat( buf,"a");
			inner_sprint_DExpr_dpl_ids( buf, DE->dat.opr.L_operand, id );//strcat( buf,"b");
			// omit the " + " when both operands are dice pools.
			if( !( all_leaves_are( DE->dat.opr.L_operand, DE_DPL) && 
				   all_leaves_are( DE->dat.opr.R_operand, DE_DPL) ) ){
				strcat( buf," + ");//strcat( buf,"c");
			}
			inner_sprint_DExpr_dpl_ids( buf, DE->dat.opr.R_operand, id );//strcat( buf,"d");
			break;
		case DE_SUB:
			inner_sprint_DExpr_dpl_ids( buf, DE->dat.opr.L_operand, id );
			strcat( buf," - ");
			inner_sprint_DExpr_dpl_ids( buf, DE->dat.opr.R_operand, id );
			break;
		case DE_MUL:
			inner_sprint_DExpr_dpl_ids( buf, DE->dat.opr.L_operand, id );
			strcat( buf," * ");
			inner_sprint_DExpr_dpl_ids( buf, DE->dat.opr.R_operand, id );
			break;
		case DE_DIV:
			inner_sprint_DExpr_dpl_ids( buf, DE->dat.opr.L_operand, id );
			strcat( buf," / ");
			inner_sprint_DExpr_dpl_ids( buf, DE->dat.opr.R_operand, id );
			break;

		//UNARY OPS
		case DE_NEG:
			strcat( buf," -");
			inner_sprint_DExpr_dpl_ids( buf, DE->dat.opr.R_operand, id );
			break;

		case DE_NOT:
			strcat( buf," !");
			inner_sprint_DExpr_dpl_ids( buf, DE->dat.opr.R_operand, id );
			break;
	}

	if( DE->parenthetical ) strcat(buf, ")");
}
void sprint_DExpr_dpl_ids( char *buf, DExpr *DE ){

	int id = 0;
	inner_sprint_DExpr_dpl_ids( buf, DE, &id );
}

void print_tree_DExpr( DExpr *DE, int depth ){

	for (int i = 0; i < depth; ++i ){
		putchar(' ');
	}

	//printf("<%d>", DE->type );

	switch( DE->type ){

		case DE_NUL:
			printf( "NUL: %s\n", DE->dat.err );
			break;

		case DE_NUM:
			printf( "NUM: %d\n", DE->dat.lit.value );
			break;

		case DE_DPL:
			printf( "DPL: %dD%d\n", DE->dat.dpl.num, DE->dat.dpl.sides );
			break;

		// BINARY OPS
		case DE_EQL:
			printf("EQL:\n");
			print_tree_DExpr( DE->dat.opr.L_operand, depth+2 );
			print_tree_DExpr( DE->dat.opr.R_operand, depth+2 );
			break;
		case DE_NEQ:
			printf("NEQ:\n");
			print_tree_DExpr( DE->dat.opr.L_operand, depth+2 );
			print_tree_DExpr( DE->dat.opr.R_operand, depth+2 );
			break;
		case DE_GTE:
			printf("GTE:\n");
			print_tree_DExpr( DE->dat.opr.L_operand, depth+2 );
			print_tree_DExpr( DE->dat.opr.R_operand, depth+2 );
			break;
		case DE_GTN:
			printf("GTN:\n");
			print_tree_DExpr( DE->dat.opr.L_operand, depth+2 );
			print_tree_DExpr( DE->dat.opr.R_operand, depth+2 );
			break;
		case DE_LTE:
			printf("LTE:\n");
			print_tree_DExpr( DE->dat.opr.L_operand, depth+2 );
			print_tree_DExpr( DE->dat.opr.R_operand, depth+2 );
			break;
		case DE_LTN:
			printf("LTN:\n");
			print_tree_DExpr( DE->dat.opr.L_operand, depth+2 );
			print_tree_DExpr( DE->dat.opr.R_operand, depth+2 );
			break;
		case DE_CFM:
			printf("CFM:\n");
			print_tree_DExpr( DE->dat.opr.L_operand, depth+2 );
			print_tree_DExpr( DE->dat.opr.R_operand, depth+2 );
			break;
		case DE_ADD:
			printf("ADD:\n");
			print_tree_DExpr( DE->dat.opr.L_operand, depth+2 );
			print_tree_DExpr( DE->dat.opr.R_operand, depth+2 );
			break;
		case DE_SUB:
			printf("SUB:\n");
			print_tree_DExpr( DE->dat.opr.L_operand, depth+2 );
			print_tree_DExpr( DE->dat.opr.R_operand, depth+2 );
			break;
		case DE_MUL:
			printf("MUL:\n");
			print_tree_DExpr( DE->dat.opr.L_operand, depth+2 );
			print_tree_DExpr( DE->dat.opr.R_operand, depth+2 );
			break;
		case DE_DIV:
			printf("DIV:\n");
			print_tree_DExpr( DE->dat.opr.L_operand, depth+2 );
			print_tree_DExpr( DE->dat.opr.R_operand, depth+2 );
			break;

		//UNARY OPS
		case DE_NEG:
			printf("NEG:\n");
			print_tree_DExpr( DE->dat.opr.R_operand, depth+2 );
			break;

		case DE_NOT:
			printf("NOT:\n");
			print_tree_DExpr( DE->dat.opr.R_operand, depth+2 );
			break;
	}
}

static int comp( int type, int L_value, int R_value ){
	switch( type ){
		case DE_EQL:
			//printf( "%d == %d; ", L_value, R_value );
			return L_value == R_value;
		case DE_NEQ:
			//printf( "%d != %d; ", L_value, R_value );
			return L_value != R_value;
		case DE_GTE:
			//printf( "%d >= %d; ", L_value, R_value );
			return L_value >= R_value;
		case DE_GTN:
			//printf( "%d > %d; ", L_value, R_value );
			return L_value > R_value;
		case DE_LTE:
			//printf( "%d <= %d; ", L_value, R_value );
			return L_value <= R_value;
		case DE_LTN:
			//printf( "%d < %d; ", L_value, R_value );
			return L_value < R_value;
	}
	return 0;
}
static int arith( int type, int L_value, int R_value ){
	switch( type ){
		case DE_ADD:
			return L_value + R_value;
		case DE_SUB:
			return L_value - R_value;
		case DE_MUL:
			return L_value * R_value;
		case DE_DIV:
			return lrint( L_value / ((float)R_value) );
	}
}

bool DETree_contains( DExpr *DE, int type ){

	if( DE->type == type ) return 1;
	if( DE_has_L_opr( DE ) ){
		if( DETree_contains( DE->dat.opr.L_operand, type ) ) return 1;
	}
	if( DE_has_R_opr( DE ) ){
		if( DETree_contains( DE->dat.opr.R_operand, type ) ) return 1;
	}
	return 0;
}

DEVal evaluate_DExpr( DExpr *DE ){
	
	switch( DE->type ){

		case DE_NUM:
			return (DEVal){ DE->dat.lit.value, 0, 0, 0, NULL };

		case DE_DPL:{
			DEVal OV = {0};
			OV.N = DE->dat.dpl.num;
			OV.rolls = malloc( OV.N * sizeof(int) );
			if(  DE->dat.dpl.values != NULL ){
				for (int i = 0; i < OV.N; ++i ){
					OV.rolls[i] = *(DE->dat.dpl.values[i]);
					OV.total += OV.rolls[i];
				}
			}
			else{
				for (int i = 0; i < OV.N; ++i ){
					OV.rolls[i] = ( rand() % (DE->dat.dpl.sides) ) + 1;
					OV.total += OV.rolls[i];
				}
			}
			if( OV.total == (DE->dat.dpl.num * DE->dat.dpl.sides) ){
				OV.critical = 1;
			}
			return OV;
		}

		// COMPS
		case DE_EQL:
		case DE_NEQ:
		case DE_GTE:
		case DE_GTN:
		case DE_LTE:
		case DE_LTN:{
			bool Lcfm = DETree_contains( DE->dat.opr.L_operand, DE_CFM );
			bool Rcfm = DETree_contains( DE->dat.opr.R_operand, DE_CFM );
			if( Lcfm || Rcfm ){
				DEVal LV = evaluate_DExpr( DE->dat.opr.L_operand );
				DEVal RV = evaluate_DExpr( DE->dat.opr.R_operand );
				int threshhold = 0;
				DEVal OV = {0};
				if( Lcfm && !Rcfm ){
					OV.choices = LV.choices;
					OV.N = LV.N; LV.N = 0;
					OV.rolls = LV.rolls;
					threshhold = RV.total;
				} else if( !Lcfm && Rcfm ){
					OV.choices = RV.choices;
					OV.N = RV.N; RV.N = 0;
					OV.rolls = RV.rolls;
					threshhold = LV.total;
				} else if( Lcfm && Rcfm ){
					/// comparing two choosefroms??????????????????????
					return OV;
				}
				int hits = 0;
				for (int i = 0; i < OV.N; ++i ){
					if( comp( DE->type, OV.rolls[i], threshhold ) ){
						hits += 1;
					}
				}
				//puts("");
				OV.total = (hits >= OV.choices);
				OV.critical = (hits == OV.N);
				destroy_DEVal( &LV ); destroy_DEVal( &RV );
				return OV;
			}
			else{
				DEVal LV = evaluate_DExpr( DE->dat.opr.L_operand );
				DEVal RV = evaluate_DExpr( DE->dat.opr.R_operand );
				int res = comp( DE->type, LV.total, RV.total );
				LV.total = res;
				destroy_DEVal( &RV );
				return LV;
			}
		}

		case DE_CFM:{
			DEVal LV = evaluate_DExpr( DE->dat.opr.L_operand );
			DEVal RV = evaluate_DExpr( DE->dat.opr.R_operand );
			RV.choices = LV.total;
			destroy_DEVal( &LV );
			return RV;
		}

		//ARITHMETIC
		case DE_ADD:
		case DE_SUB:
		case DE_MUL:
		case DE_DIV:{
			DEVal LV = evaluate_DExpr( DE->dat.opr.L_operand );
			DEVal RV = evaluate_DExpr( DE->dat.opr.R_operand );
			DEVal OV;
			OV.total = arith( DE->type, LV.total, RV.total );

			int lvn0 = LV.N > 0;
			int rvn0 = RV.N > 0;
			// One dice one number: <arith> each roll and the number
			if( lvn0 + rvn0 == 1 ){
				DEVal *dice = lvn0? &LV : &RV;
				DEVal *scalar = lvn0? &RV : &LV;
				OV.N = dice->N;
				OV.choices = dice->choices;
				OV.critical = dice->critical;
				OV.rolls = malloc( OV.N * sizeof(int) );
				for (int i = 0; i < dice->N; ++i ){
					OV.rolls[i] =  arith( DE->type, dice->rolls[i], scalar->total );
				}
			}
			// two dice pools? concatenate them.
			else if( lvn0 && rvn0 ){
				// (what could SUB, MUL and DIV do instead?)
				OV.N = LV.N + RV.N;
				OV.choices = LV.choices + RV.choices;
				OV.critical = LV.critical && RV.critical;
				OV.rolls = malloc( OV.N * sizeof(int) );
				for (int i = 0; i < OV.N; ++i ){
					if( i < LV.N ){
						OV.rolls[i] = LV.rolls[i];
					}else{
						OV.rolls[i] = RV.rolls[i-LV.N];
					}
				}
			}
			destroy_DEVal( &LV ); destroy_DEVal( &RV );
			return OV;
		}

		//UNARY OPS
		case DE_NEG:{
			DEVal RV = evaluate_DExpr( DE->dat.opr.R_operand );
			RV.total *= -1;
			return RV;
		}
		case DE_NOT:{
			DEVal RV = evaluate_DExpr( DE->dat.opr.R_operand );
			RV.total = !(RV.total);
			return RV;
		}
	}

	return (DEVal){ 0 };
}

void destroy_DEVal( DEVal *dv ){
	if( dv->N > 0 ){
		free( dv->rolls );
	}
}