// eval.h

double eval(
	const char* szExpression,
	double (*fnSymbolLookup)( const char* szSymbolName )
	);
