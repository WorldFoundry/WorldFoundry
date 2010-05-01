// oad.hpp

#include "oas/pigtool.h"
#include "oas/oad.h"

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


