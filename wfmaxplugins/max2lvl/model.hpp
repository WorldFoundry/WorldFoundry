//============================================================================
// model.hp:
// Copyright(c) 1995 Cave Logic Studios / PF.Magic
// By Kevin T. Seghetti
//============================================================================
/* Documentation:

	Abstract:
			in memory representation of level model data
	History:
			Created	05-04-95 11:09am Kevin T. Seghetti

	Class Hierarchy:
			none

	Dependancies:
		PIGS, BRender, STDIO
	Restrictions:

	Example:
*/
//============================================================================
// use only once insurance
#ifndef _mODEL_HP
#define _mODEL_HP

//============================================================================

#include "global.hpp"

#include "pigtool.h"
#include <iostream.h>
#include <stl/bstring.h>
//#include <pclib\boolean.hp>

//============================================================================
// for now, model struct consists of model name

class QModel
{
public:
	QModel() { good = false; }
	QModel(const string& name);
	~QModel();

	const string& GetName(void) { return name; }

	int operator==(const QModel& left) const { return(name == left.name); }

	friend ostream& operator<<(ostream& s, const QModel &model);

	void Save(ostream& lvl);
	enum { NAME_LEN=100};
private:
	bool good;
	string name;
//	char name[NAME_LEN+1];
};

//============================================================================
#endif
//============================================================================
