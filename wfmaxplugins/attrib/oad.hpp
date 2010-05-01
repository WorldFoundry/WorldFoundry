// oad.hpp

#include "oas/pigtool.h"
#include "oas/oad.h"
#include <string>

class Oad
{
public:
	Oad( const char* );
	~Oad();

	typeDescriptor* startOfTypeDescriptors() const;
	int len() const;

	const std::string& className() const;
	const std::string& fileName() const;

protected:
	oadFile* _oadFileChunk;
	int _len;
	typeDescriptor* _td;

	std::string _szFileName;
	std::string _szClassName;
};


