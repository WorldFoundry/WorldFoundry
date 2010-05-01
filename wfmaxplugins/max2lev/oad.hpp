// oad.hpp

#include "global.hpp"
#include "oas/pigtool.h"
#include "oas/oad.h"
//#include <stl/bstring.h>

class Oad
{
public:
	Oad( const char* );
	~Oad();

	typeDescriptor* startOfTypeDescriptors() const;
	int len() const;

	const string& className() const;
	const string& fileName() const;

protected:
	oadFile* _oadFileChunk;
	int _len;
	typeDescriptor* _td;

	string _szFileName;
	string _szClassName;
};


