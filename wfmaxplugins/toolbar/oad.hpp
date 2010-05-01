// oad.hpp

#include "source/oas/pigtool.h"
#include "source/oas/oad.h"

class Oad
{
public:
	Oad( const char* );
	~Oad();

	typeDescriptor* startOfTypeDescriptors() const;
	int len() const;
	char* header() const;

protected:
	oadFile* _oadFileChunk;
	int _len;
	typeDescriptor* _td;

};


