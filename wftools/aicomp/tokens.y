%{
#include <math/scalar.hp>
%}

%union
{
	struct
	{
		//float f;
		char* name;
		Scalar val;
		unsigned long l;
		unsigned long opcode;
	};
}


%token BASE_TOKEN_VAL

%token <opcode> LT LTE GT GTE EQ NE
%token <opcode> MULT DIVIDE PLUS MINUS QUOTIENT REMAINDER NEGATE
%token <opcode> LAND LOR LNOT INC DEC LSHIFT RSHIFT BAND BOR BNOT XOR
%token <opcode> IF COND ELSE
%token <opcode> ZERO_Q POSITIVE_Q NEGATIVE_Q ROUND TRUNCATE FLOOR CEILING OP_ABS
%token <opcode> OP_MIN OP_MAX
%token <opcode> NEWLINE WRITE WRITELN
%token <opcode> READ_MAILBOX WRITE_TO_MAILBOX SEND_MESSAGE RANDOM
%token <name> ACTOR MAILBOX_TYPE
%token <name> STRING
%token <val> NUMBER
%token <opcode> DEFINE LAMBDA FIND_CLASS CREATE_OBJECT SELF SLEEP SET
%token <opcode> SIN COS ASIN ACOS ATAN2 ATAN2QUICK TAN
%token <opcode> BEGINP EULER_TO_VECTOR VECTOR_TO_EULER EXIT
%token <opcode> CHAR_LITERAL MEMBER EVENT MAILBOX
%token <opcode> BRANCH			/* internal code only */
%token <opcode> PTR_DEREF

%%

//%type <val> expr

dummy : '0'
	;

%%
