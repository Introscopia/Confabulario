#ifndef DEXPR_H_INCLUDED
#define DEXPR_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include "ok_lib.h"

typedef struct dexpr_struct DExpr;

enum { DE_NUL, // NULL

	   TK_OPR, // Open Parenthesis
	   TK_CPR, // Close Parenthesis

	   DE_EQL, // Equals 
	   DE_NEQ, // Not Equals
	   DE_GTN, // Greater Than
	   DE_GTE, // Greater Than or Equal
	   DE_LTN, // Lesser Than
	   DE_LTE, // Lesser Than or Equal
	   DE_CFM, // Choose From (@)
	   DE_ADD, // Add
	   DE_SUB, // Subtract
	   DE_MUL, // Multiply
	   DE_DIV, // Divide

	   DE_NOT, // Not
	   DE_NEG, // unary Negative

	   DE_DPL, // Dice Pool
	   DE_NUM  // Number Literal
	 };

typedef struct dexpr_struct{

	int type;

	bool parenthetical;

	union{
		struct{
			DExpr *L_operand;
			DExpr *R_operand;
		} opr;

		struct{
			int num;
			int sides;
			int **values;//references to external RNG objects
		} dpl;

		struct{
			int value;
		} lit;

		const char *err;
	} dat;

} DExpr;// Dice Expression

typedef struct ok_vec_of(DExpr) DE_vec;

DExpr new_DExpr( int type );

DExpr* compile_DExpr( char *code );

int count_dpools( DExpr *DE );
DExpr* fetch_dpool( DExpr *DE, int id );

void destroy_DExpr( DExpr *DE );

void print_DExpr( DExpr *DE );
void print_tree_DExpr( DExpr *DE, int depth );

void sprint_DExpr( char *buf, DExpr *DE );
void sprint_DExpr_dpl_ids( char *buf, DExpr *DE );


typedef struct value_box{

	int total;
	int choices;
	bool critical;
	int N;
	int *rolls;

} DEVal;// Dice Expression value


/* 
EVALUATE:
	-comparisons return the Left-value's roll.
*/
DEVal evaluate_DExpr( DExpr *DE );

void destroy_DEVal( DEVal *dv );

#endif