// objname.cpp

#include <max.h>
#include "../lib/wf_id.hp"
#include "../lib/registry.h"
#include "../lib/levelnum.h"

const char*
CreateObjectFilename( INode* pNode )
{
	assert( pNode );

	static char szOutputFilename[ _MAX_PATH ];
	*szOutputFilename = '\0';

	sprintf( szOutputFilename, "%s%s.iff", CreateLevelDirName(), pNode->GetName() );

	return szOutputFilename;
}

