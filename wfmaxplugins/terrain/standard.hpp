//=============================================================================
// standard.hpp:
//=============================================================================

#if !defined(_STANDARD_HPP)
#define _STANDARD_HPP

//=============================================================================

TCHAR* GetString(int id);
extern HINSTANCE hInstance;

//=============================================================================

class TypeEntryInvalid
{
};

//=============================================================================

class TypeEntry
{
	enum ParamType
	{
		TYPE_FLOAT,
		TYPE_INT,
		TYPE_RGBA,
		TYPE_POINT3,
		TYPE_INVALID
	};

public:
	TypeEntry(TypeEntryInvalid foo) 	 { _type = TYPE_INVALID; }
	TypeEntry(float*  value) { _type = TYPE_FLOAT  ; _float = value; }
	TypeEntry(int*    value) { _type = TYPE_INT    ; _int = value; }
	TypeEntry(Color*  value) { _type = TYPE_RGBA   ; _color = value; }
	TypeEntry(Point3* value) { _type = TYPE_POINT3 ; _point3 = value; }
//	TypeEntry(BOOL*   value) { _type = TYPE_BOOL   ; _bool = value; }

	void SetValue(IParamBlock* pblock,int index,TimeValue time)
	{
		switch ( _type)
		{
			case TYPE_FLOAT:
				pblock->SetValue(index, time, *_float);
				break;
			case TYPE_INT:
				pblock->SetValue(index, time, *_int);
				break;
			case TYPE_RGBA:
				pblock->SetValue(index, time, *_color);
				break;
			case TYPE_POINT3:
				pblock->SetValue(index, time, *_point3);
				break;
//			case TYPE_BOOL:
//				pblock->SetValue(index, time, *_bool);
//				break;
			case TYPE_INVALID:
				break;
			default:
				assert(0);
				break;
		}
	}

private:
	ParamType _type;
	union
	{
		float* _float;
		int* _int;
		Color* _color;
		Point3* _point3;
		BOOL* _bool;
	};
};

//=============================================================================

struct ParameterEntry
{
	char* _name;
	ParamDimension* _dimension;
	TypeEntry _entry;

	ParameterEntry(char* name, ParamDimension* dim, TypeEntry entry) : _entry(entry) { _name = name; _dimension = dim; }
};

//=============================================================================
#endif
//=============================================================================
