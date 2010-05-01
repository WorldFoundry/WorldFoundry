// oadentry.hpp

#include "global.hpp"
#include <oas/oad.h>
#include "../lib/iffwrite.hp"

class OadEntry
{
public:
	OadEntry();		// for STL only
	OadEntry( char* & xdata );
	OadEntry( const typeDescriptor& td );
	OadEntry( const OadEntry& oe );
	~OadEntry();
	bool operator==( const OadEntry& oadEntry ) const;

	const char* GetName() const;
	int32 GetDef() const;
	int32 GetMin() const;
	int32 GetMax() const;
	buttonType GetButtonType() const;
	const char* GetDisplayName() const;
	const char* GetEnableExpression() const;
	const char* GetString() const;
	visualRepresentation GetVisualRepresentation() const;
	const ID GetID() const;
	friend ostream& operator << ( ostream& s, const OadEntry& oadEntry );

	const char* buttonTypeToName() const;

	typeDescriptor* GetTypeDescriptor();
	const typeDescriptor* GetTypeDescriptor() const;
	const char* GetEnumeratedValues() const		{ assert( _szEnumeratedValues ); return _szEnumeratedValues; }

	const char* GetXData() const;
	void SetXData(const char* xdata);

	void SetDef(int32);
	void SetString(const char* string);


	bool DefaultOverriden() const;

// for now
protected:
	typeDescriptor _td;
	char* _xdata;
	ID _id;
	char* _szEnumeratedValues;

	ID buttonTypeToID() const;
	bool _hasBeenWrittenTo;
};

