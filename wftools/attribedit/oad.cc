//=====================================================================
// oad.cc: 
//==============================================================================

#include "oad.hp"
#include "iffhelpers.hp"
#include <memory/memory.hp>
#include <math/scalar.hp>

#if defined(USEXML)
#include "xmlinput.h"
#include "xmloutput.h"
#include "xmlfile.h"
#endif


extern Memory* scratchMemory;

//==============================================================================

inline float
ScalarToFloat(const Scalar& s)
{
    float temp = s.AsLong();
    temp /= SCALAR_ONE_LS;
    return temp;
}


//==============================================================================

TypeBase::TypeBase()
{
    overridden = false;
    showAs = SHOW_AS_N_A;
}

//==============================================================================

void
TypeBase::ParseIFFChunk(IFFChunkIter& chunkIter)
{
    switch(chunkIter.GetChunkID().ID())
    {
        case IFFTAG('N','A','M','E'):
            {
                Name(IFFReadString(chunkIter));
                break;
            }
        case IFFTAG('D','S','N','M'):
            {
                displayName = IFFReadString(chunkIter);
                break;
            }
        case IFFTAG('H','E','L','P'):
            {
                help = IFFReadString(chunkIter);
                break;
            }
        case IFFTAG('E','N','B','L'):
            {
                enableExpression = IFFReadString(chunkIter);
                break;
            }

        case IFFTAG('D','I','S','P'):
            {
                showAs = (ShowAs)IFFReadInt32(chunkIter);
                break;
            }
        case IFFTAG('H','I','N','T'):
            {
                hint = IFFReadString(chunkIter);
                break;
            }
        default:
            {       
                std::cerr << "unknown chunk " << chunkIter.GetChunkID() << std::endl;
                assert(0);
            }
    }
}

//==============================================================================

void
TypeBase::Print(std::ostream& out) const
{
    out << "Name: " << name << std::endl;
    out << "NameNoSpaces: " << nameNoSpaces << std::endl;
    out << "DisplayName: " << displayName << std::endl;
    out << "Help: " << help << std::endl;
    out << "EnableExpression: " << enableExpression << std::endl;
    out << "ShowAs: " << showAs << std::endl;
    out << "Hint: " << hint << std::endl;
}

//==============================================================================

TypeInt32::TypeInt32(IFFChunkIter& chunkIter)
{
    assert(chunkIter.GetChunkID().ID() == IFFTAG('I','3','2','\0'));

    while(chunkIter.BytesLeft() > 0)
    {
        IFFChunkIter *childChunkIter = chunkIter.GetChunkIter(*scratchMemory);
        switch(childChunkIter->GetChunkID().ID())
        {
            case IFFTAG('R','A','N','G'):
                {
                    min = IFFReadInt32(*childChunkIter);
                    max = IFFReadInt32(*childChunkIter);
                    break;
                }
            case IFFTAG('D','A','T','A'):
                {
                    def = IFFReadInt32(*childChunkIter);
                    break;
                }
            case IFFTAG('E','N','V','L'):
                {
                    enumValues = IFFReadString(*childChunkIter);
                    break;
                }
            default:
                {       
                    ParseIFFChunk(*childChunkIter);
                }
        }
        MEMORY_DELETE(*scratchMemory,childChunkIter,IFFChunkIter);
    }
}

//==============================================================================

void
TypeInt32::Print(std::ostream& out) const
{
    TypeBase::Print(out);

    out << "Min: " << min << std::endl;
    out << "Max: " << max << std::endl;
    out << "Default: " << def << std::endl;
    out << "Enumerated Values: " <<  enumValues << std::endl;
}

//==============================================================================

int
FindEnumStringIndex(const std::string& input, const std::string& value)
{
    int result = -1;

    char* origStrPtr = strdup(input.c_str());
    char* strPtr = origStrPtr;
    char* origStrEnd = origStrPtr+strlen(origStrPtr);

    int index=0;
    while(*strPtr)
    {
        char* cursor = strPtr;
        while(*cursor && *cursor != '|')
            cursor++;

        if(*cursor)
            *cursor++ = 0;

        if(std::string(strPtr) == value)
        {
            result = index;
            strPtr = NULL;
        }

        assert(cursor);
        assert(cursor <= origStrEnd);
        strPtr = cursor;
        index++;
    }
    free(origStrPtr);
    return result;
}


//==============================================================================

std::string
FindEnumString(const std::string& input, int value)
{
    std::string result;

    char* origStrPtr = strdup(input.c_str());
    char* strPtr = origStrPtr;
    char* origStrEnd = origStrPtr+strlen(origStrPtr);

    int index=0;
    while(*strPtr)
    {
        char* cursor = strPtr;
        while(*cursor && *cursor != '|')
            cursor++;

        if(*cursor)
            *cursor++ = 0;

        if(index == value)
        {
            result = std::string(strPtr);
            strPtr = NULL;
        }

        assert(cursor <= origStrEnd);
        strPtr = cursor;
        index++;
    }
    free(origStrPtr);
    return result;
}

//==============================================================================

#if defined(USEXML)
void 
TypeInt32::ReadOverride(OADInput& elem)
{
    XML::Char tmp[1024];
    size_t len = elem.ReadData(tmp, sizeof(tmp));
    tmp[len] = 0;
    AssertMsg(len, "0 length data for item " << DisplayName() << std::endl);

    std::cout << "string = " << tmp << std::endl;
    int32 value;

    if(GetShowAs() == TypeBase::SHOW_AS_DROPMENU)
    {
        value = FindEnumStringIndex(EnumValues(),tmp);
        assert(value != -1);
    }
    else
    {
        sscanf(tmp,"%d",&value);
    }

    CurrentValue(value);
    std::cout << "int override named " << DisplayName() << " set to " << CurrentValue() << std::endl;
}


void 
TypeInt32::WriteOverride(OADOutput& out) const
{
    out.BeginElement(NameNoSpaces().c_str(),XML::Output::terse);

    if(GetShowAs() == TypeBase::SHOW_AS_DROPMENU)
    {
        std::string result = FindEnumString(EnumValues(),CurrentValue());
        assert(result.size());
        out << result;
    }
    else
    {
#pragma message ("KTS " __FILE__ ": put eval back in")

               //*_iff << Fixed( theOptions.max2iffOptions.sizeReal, ::eval( oadEntry->GetString(), NULL ) );
               //*_iff << Fixed( theOptions.max2iffOptions.sizeReal,WF_SCALAR_TO_FLOAT(oadEntry->GetDef()));
       char buffer[100];
       snprintf(buffer,100,"%d",CurrentValue());
       out << std::string(buffer);
    }
    out.EndElement(XML::Output::terse);
}

#else
void 
TypeInt32::ReadOverride(OADInput& input)
{
    std::string data = IFFReadString(input);
    std::cout << "string = " << data << std::endl;
    int32 value;

    if(GetShowAs() == TypeBase::SHOW_AS_DROPMENU)
    {
        value = FindEnumStringIndex(EnumValues(),data);
        assert(value != -1);
    }
    else
    {
        sscanf(data.c_str(),"%d",&value);
    }

    CurrentValue(value);
    std::cout << "int override named " << DisplayName() << " set to " << CurrentValue() << std::endl;
}


void 
TypeInt32::WriteOverride(OADOutput& iffOut) const
{
    iffOut.enterChunk( ID( "I32" ) );

    iffOut.enterChunk( ID( "NAME" ) );
    iffOut << Name(); 
    iffOut.exitChunk();


    if(GetShowAs() == TypeBase::SHOW_AS_DROPMENU)
    {
        iffOut.enterChunk( ID( "STR" ) );
        iffOut << FindEnumString(EnumValues(),CurrentValue());
        iffOut.exitChunk();
    }
    else
    {
       iffOut.enterChunk( ID( "DATA" ) );
#pragma message ("KTS " __FILE__ ": put eval back in")

               //*_iff << Fixed( theOptions.max2iffOptions.sizeReal, ::eval( oadEntry->GetString(), NULL ) );
               //*_iff << Fixed( theOptions.max2iffOptions.sizeReal,WF_SCALAR_TO_FLOAT(oadEntry->GetDef()));
       iffOut << CurrentValue();
       iffOut.exitChunk();

       iffOut.enterChunk( ID( "STR" ) );
       char buffer[100];
       snprintf(buffer,100,"%d",CurrentValue());
       iffOut << std::string(buffer);
       iffOut.exitChunk();

    }
    iffOut.exitChunk();
}
#endif

//==============================================================================

TypeFixed32::TypeFixed32(IFFChunkIter& chunkIter)
{
    assert(chunkIter.GetChunkID().ID() == IFFTAG('F','3','2','\0'));

    while(chunkIter.BytesLeft() > 0)
    {
        IFFChunkIter *childChunkIter = chunkIter.GetChunkIter(*scratchMemory);
        switch(childChunkIter->GetChunkID().ID())
        {
            case IFFTAG('R','A','N','G'):
                {
                    min = ScalarToFloat(IFFReadFixed32(*childChunkIter));
                    max = ScalarToFloat(IFFReadFixed32(*childChunkIter));
                    break;
                }
            case IFFTAG('D','A','T','A'):
                {
                    def = ScalarToFloat(IFFReadFixed32(*childChunkIter));
                    break;
                }
            default:
                {       
                    ParseIFFChunk(*childChunkIter);
                }
        }
        MEMORY_DELETE(*scratchMemory,childChunkIter,IFFChunkIter);
    }
}

//==============================================================================

void
TypeFixed32::Print(std::ostream& out) const
{
    TypeBase::Print(out);

    out << "Min: " << min << std::endl;
    out << "Max: " << max << std::endl;
    out << "Default: " << def << std::endl;
}


//==============================================================================

#if defined(USEXML)
void 
TypeFixed32::ReadOverride(OADInput& elem)
{
     XML::Char tmp[1024];
     size_t len = elem.ReadData(tmp, sizeof(tmp));
     tmp[len] = 0;
     assert(len);

    float temp;
    sscanf(tmp,"%f",&temp);
    CurrentValue(temp);
    //std::cout << "fixed override  named " << DisplayName() << "set to " << temp << std::endl;
}

//==============================================================================

void 
TypeFixed32::WriteOverride(OADOutput& out) const
{
    out.BeginElement(NameNoSpaces().c_str(),XML::Output::terse);

#pragma message ("KTS " __FILE__ ": put eval back in")
            //*_iff << Fixed( theOptions.max2iffOptions.sizeReal, ::eval( oadEntry->GetString(), NULL ) );
            //*_iff << Fixed( theOptions.max2iffOptions.sizeReal,WF_SCALAR_TO_FLOAT(oadEntry->GetDef()));
        
        char buffer[100];
        snprintf(buffer,100,"%f",CurrentValue());
        out << std::string(buffer);
    out.EndElement(XML::Output::terse);
}
#else
void 
TypeFixed32::ReadOverride(OADInput& chunkIter)
{
    std::string data = IFFReadString(chunkIter);
    //std::cout << "string = " << data << std::endl;

    float temp;
    sscanf(data.c_str(),"%f",&temp);
    CurrentValue(temp);
    //std::cout << "fixed override  named " << DisplayName() << "set to " << temp << std::endl;
}

//==============================================================================

void 
TypeFixed32::WriteOverride(OADOutput& iffOut) const
{
    iffOut.enterChunk( ID( "FX32" ) );

    iffOut.enterChunk( ID( "NAME" ) );
    iffOut << Name(); 
    iffOut.exitChunk();

    iffOut.enterChunk( ID( "DATA" ) );
#pragma message ("KTS " __FILE__ ": put eval back in")

            //*_iff << Fixed( theOptions.max2iffOptions.sizeReal, ::eval( oadEntry->GetString(), NULL ) );

            //*_iff << Fixed( theOptions.max2iffOptions.sizeReal,WF_SCALAR_TO_FLOAT(oadEntry->GetDef()));
    iffOut << Fixed( fp_1_15_16,CurrentValue());
    iffOut.exitChunk();

    iffOut.enterChunk( ID( "STR" ) );
    char buffer[100];
    snprintf(buffer,100,"%f",CurrentValue());
    iffOut << std::string(buffer);
    iffOut.exitChunk();

    iffOut.exitChunk();
}
#endif

//==============================================================================

TypeString::TypeString(IFFChunkIter& chunkIter)
{
    assert(chunkIter.GetChunkID().ID() == IFFTAG('S','T','R','\0'));

    while(chunkIter.BytesLeft() > 0)
    {
        IFFChunkIter *childChunkIter = chunkIter.GetChunkIter(*scratchMemory);
        switch(childChunkIter->GetChunkID().ID())
        {
            case IFFTAG('D','A','T','A'):
                {
                    def = IFFReadString(*childChunkIter);
                    break;
                }
            case IFFTAG('B','O','O','L'):
                {               
                    while(childChunkIter->BytesLeft() > 0)
                    {
                        IFFChunkIter *boolChunkIter = childChunkIter->GetChunkIter(*scratchMemory);
                        switch(boolChunkIter->GetChunkID().ID())
                        {
                            case IFFTAG('N','A','M','E'):
                                {
                                    std::string temp = IFFReadString(*boolChunkIter);
                                    assert(temp == "Required");
                                    break;
                                }
                            case IFFTAG('D','A','T','A'):
                                {
                                    required = IFFReadBool(*boolChunkIter);
                                    break;
                                }
                            default:
                                {       
                                    
                                    assert(0);
                                }
                        }
                        MEMORY_DELETE(*scratchMemory,boolChunkIter,IFFChunkIter);

                    }
                    break;
                }
            default:
                {       
                    ParseIFFChunk(*childChunkIter);
                }
        }
        MEMORY_DELETE(*scratchMemory,childChunkIter,IFFChunkIter);
    }
}

//==============================================================================

void
TypeString::Print(std::ostream& out) const
{
    TypeBase::Print(out);

    out << "Default: " << def << std::endl;
    out << "String Type: " <<  stringType << std::endl;
    out << "Required: " <<  required << std::endl;
}

//==============================================================================

#if defined(USEXML)
void 
TypeString::WriteOverride(OADOutput& out) const
{
    out.BeginElement(NameNoSpaces().c_str(),XML::Output::terse);
        out << CurrentValue();
    out.EndElement(XML::Output::terse);
}

//==============================================================================

void 
TypeString::ReadOverride(OADInput& elem)
{
    XML::Char tmp[1024];
    size_t len = elem.ReadData(tmp, sizeof(tmp));
    tmp[len] = 0;
    //assert(len);          // it is ok to have a 0 length string

    std::string data = tmp;
   CurrentValue(data);
}
#else
void 
TypeString::WriteOverride(OADOutput& iffOut) const
{
    iffOut.enterChunk( ID( "STR" ) );

    iffOut.enterChunk( ID( "NAME" ) );
    iffOut << Name(); 
    iffOut.exitChunk();

    iffOut.enterChunk( ID( "DATA" ) );
    iffOut << CurrentValue();
    iffOut.exitChunk();
    iffOut.exitChunk();
}

//==============================================================================

void 
TypeString::ReadOverride(OADInput& input)
{
    std::string data = IFFReadString(input);
    //std::cout << "string = " << data << std::endl;

    CurrentValue(data);
    //std::cout << "string named " << DisplayName() << " override set to " << data << std::endl;

}
#endif

//==============================================================================
