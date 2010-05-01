// handle.cpp
// Copyright 1997 Recombinant Limited. All Rights Reserved.
// by William B. Norris IV

#include <global.hp>
#include "handle.h"
#include "id.hp"
#include "../lib/wf_id.hp"

ParamBlockDesc pdesc[] =
{
	{ TYPE_INT, NULL, FALSE },		// ID
	{ TYPE_INT, NULL, FALSE }		// selected vertex
};


class SelectData : public LocalModData
{
public:
	int _idxVertexSelected;
	ID _id;
	bool bHeld;

	SelectData()
	{
		bHeld = false;
		bValid = false;
		_idxVertexSelected = 0;
		_id = ID( "" );
	}
//	SelectData( Mesh* mesh )	{	bValid = true;	bHeld = false;	}
	void Invalidate()
	{
		bValid = false;
	}
	BOOL IsValid()
	{
		return bValid;
	}
	virtual LocalModData* Clone()
	{
		SelectData* pData = new SelectData();
		assert( pData );
		pData->_idxVertexSelected = _idxVertexSelected;
		pData->_id = _id;
		return pData;
	}

private:
	bool bValid;
};


class HandleModifier : public Modifier
{
public:
	static HWND hParams;
	static SelectModBoxCMode* selectMode;

	enum
	{
		SEL_OBJECT = 0,
		SEL_VERTEX
	};
	enum { selLevel = SEL_VERTEX };
	static IObjParam* ip;
	HandleModifier();
	~HandleModifier();
	static BOOL m_ignoreBackfaces;
	// From Animatable
	void DeleteThis()
	{
		delete this;
	}
	void GetClassName( TSTR& s )
	{
		s = Handle_ClassName;
	}
	virtual Class_ID ClassID()
	{
		return Handle_ClassID;
	}
	void BeginEditParams( IObjParam* ip, ULONG flags, Animatable* prev );
	void EndEditParams( IObjParam* ip, ULONG flags, Animatable* next );
	TCHAR* GetObjectName()
	{
		return szName.str();
	}
	CreateMouseCallBack* GetCreateMouseCallBack()
	{
		return NULL;
	}

	// From Modifier
	ChannelMask ChannelsUsed()
	{
		return OBJ_CHANNELS;
	}
	ChannelMask ChannelsChanged()
	{
		return SELECT_CHANNEL|SUBSEL_TYPE_CHANNEL|GEOM_CHANNEL;
	}
	Class_ID InputType()
	{
		return triObjectClassID;
	}
	void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
	virtual BOOL DependOnTopology(ModContext &mc)
	{
		return TRUE;
	}

	virtual IParamArray* GetParamBlock()
	{
		return pblock;
	}
	virtual int GetParamBlockIndex( int id )
	{
		return id;
	}

	int NumRefs()
	{
		return 0;
	}
	RefTargetHandle GetReference( int i )
	{
		return NULL;
	}
	void SetReference( int i, RefTargetHandle rtarg )
	{
	}

	int NumSubs()
	{
		return 0;
	}
	Animatable* SubAnim( int i )
	{
		return NULL;
	}
	TSTR SubAnimName( int )
	{
		return _T("");
	}

	RefTargetHandle Clone(RemapDir& remap = NoRemap());
	RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message){return REF_SUCCEED;}

	int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc);
	void ActivateSubobjSel(int level, XFormModes& modes);

	void NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc);
	virtual void SelectSubComponent(HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert=FALSE);
	void ClearSelection(int selLevel);
	void SelectAll(int selLevel);
	void InvertSelection(int selLevel);
	void RemoveRollupPages();
	void SetRollupPages();
	void Display();

	// Load and save the plug-in's private data
	IOResult Load(ILoad* iload);
	IOResult Save(ISave* isave);

	void Command( HWND hWnd, int button, int notifyCode, HandleModifier* mod );
	void UpdateData( INode*, ID&, int );

protected:
	IParamBlock* pblock;
	strstream szName;
};


class SubSelRestore : public RestoreObj
{
public:
	SelectData usel, rsel;
	HandleModifier* mod;
	SelectData* SelData;

	SubSelRestore(HandleModifier *m, SelectData *d);
	void Restore(int isUndo);
	void Redo();
	int Size()
	{
		return 1;
	}
	void EndHold()
	{
		SelData->bHeld = false;
	}
	TSTR Description()
	{
		return TSTR( _T( "SelectRestore" ) );
	}
};

//--- ClassDescriptor and class vars ---------------------------------

HWND                   HandleModifier::hParams         = NULL;
SelectModBoxCMode     *HandleModifier::selectMode      = NULL;
IObjParam             *HandleModifier::ip              = NULL;
BOOL                   HandleModifier::m_ignoreBackfaces = FALSE;

class HandleModifierClassDesc:public ClassDesc
{
public:
	int 			IsPublic() { return 1; }
	void*			Create(BOOL loading = FALSE) { return new HandleModifier; }
	const TCHAR*	ClassName() { return Handle_ClassName; }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return Handle_ClassID; }
	const TCHAR* 	Category() { return WORLD_FOUNDRY; }
};

static HandleModifierClassDesc HandleModifierDesc;
extern ClassDesc* GetHandleModifierDesc() { return &HandleModifierDesc; }

static BOOL CALLBACK EditTriVertProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


static DLGPROC windProcs[] = { NULL, EditTriVertProc };
static int dlgIDs[] = { 0, IDD_SUBSELMOD_ATTRIB };
static int dlgTitles[] = { 0, IDS_RB_SELVERTEX };
const int hitLevel[] = { 0, SUBHIT_VERTS };				// For hit testing...

static TriObject* GetTriObject( TimeValue t, ObjectState& os, Interval& valid, BOOL& needsDel );

//--- Parameter map/block descriptors -------------------------------

//--- SelMod methods -------------------------------

HandleModifier::HandleModifier()
{
	//selLevel = SEL_VERTEX;
	pblock = CreateParameterBlock( pdesc, 2 );
	assert( pblock );

	szName << "Handle " /*<< "\'????\'"*/ << '\0';
}


HandleModifier::~HandleModifier()
{
	assert( pblock );
}


void
HandleModifier::BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev)
{
	this->ip = ip;

	// Set up User Interface
	SetRollupPages();

	// Add our sub object type
	TSTR type1( GetString( IDS_RB_VERTEX ) );
	const TCHAR *ptype[] = {type1};
	ip->RegisterSubObjectTypes( ptype, 1 );

	// Restore the selection level.
	ip->SetSubObjectLevel( selLevel );

	// Disable show end result.
	ip->EnableShowEndResult( FALSE );
}


void
HandleModifier::EndEditParams( IObjParam* ip, ULONG flags,Animatable* next )
{

	RemoveRollupPages();

	ip->DeleteMode( selectMode );

	if ( selectMode )
		delete selectMode, selectMode = NULL;

	// Enable show end result
	ip->EnableShowEndResult( TRUE );

	this->ip = NULL;
	hParams  = NULL;
}


void
HandleModifier::ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node)
{
	Interval valid = LocalValidity( t );
	DWORD meshSelLevel;

	assert( os->obj->IsSubClassOf( triObjectClassID ) );
	TriObject* tobj = (TriObject*)os->obj;

	SelectData* SelData  = (SelectData*)mc.localData;
	if ( !SelData )
		mc.localData = SelData = new SelectData;
	assert( SelData );
#if 0
	if( !SelData->IsValid() )
		SelData->Validate( &tobj->mesh );
#endif

	tobj->mesh.vertSel.Set( ((SelectData*)( mc.localData ))->_idxVertexSelected );
	tobj->mesh.SetDispFlag( DISP_VERTTICKS | DISP_SELVERTS );
	meshSelLevel = MESH_VERTEX;

	if ( tobj->mesh.selLevel != meshSelLevel )
		tobj->mesh.selLevel = meshSelLevel;

	tobj->UpdateValidity(SELECT_CHAN_NUM,valid);
	tobj->UpdateValidity(GEOM_CHAN_NUM,valid); // Have to do this to get it to evaluate
	tobj->UpdateValidity(TOPO_CHAN_NUM,valid); // Have to do this to get it to evaluate
	tobj->UpdateValidity(SUBSEL_TYPE_CHAN_NUM,FOREVER);

	Display();
}


RefTargetHandle
HandleModifier::Clone( RemapDir& )
{
	HandleModifier* mod = new HandleModifier();
	assert( mod );
	return mod;
}


int
HandleModifier::HitTest( TimeValue t, INode* inode, int type, int crossing,
	int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc)
{
	Interval valid = FOREVER;
	int savedLimits;
	GraphicsWindow *gw = vpt->getGW();
	HitRegion hr;
	BOOL needsDel;

	// Setup GW
	MakeHitRegion( hr, type, crossing, 4, p );
	gw->setHitRegion( &hr );

	// Normally these methods expect world coordinates.
	// However if this matrix is set to an objects transformation matrix you can pass objects
	// space coordinates and they will be transformed into world space (and then put into
	// screen space when they are drawn).

	Matrix3 mat = inode->GetObjectTM( t );
	gw->setTransform( mat );

	gw->setRndLimits(((savedLimits = gw->getRndLimits()) | GW_PICK) & ~GW_ILLUM);

	if ( m_ignoreBackfaces )
		gw->setRndLimits( gw->getRndLimits() |  GW_BACKCULL );
	else
		gw->setRndLimits( gw->getRndLimits() & ~GW_BACKCULL );
	gw->clearHitCode();

	ObjectState os = inode->EvalWorldState(ip->GetTime());
	TriObject* tobj = GetTriObject( ip->GetTime(), os, valid, needsDel );
	assert( tobj );

	SubObjHitList hitList;
	int res = tobj->mesh.SubObjectHitTest( gw, gw->getMaterial(), &hr,
				flags | hitLevel[selLevel], hitList );

	for ( MeshSubHitRec* rec = hitList.First(); rec; rec = rec->Next() )
	{	// rec->index is the index of the verSel/faceSel/edgeSel Bitarray
		vpt->LogHit( inode, mc, rec->dist, rec->index, NULL );
	}

	gw->setRndLimits( savedLimits );

	if ( needsDel )
		tobj->DeleteThis();

	return res;
}


void
HandleModifier::NotifyInputChanged( Interval changeInt, PartID partID, RefMessage message, ModContext* mc )
{
	if ( message == REFMSG_CHANGE )
	{
		if ( mc->localData )
		{
			assert( mc->localData );
			((SelectData *)mc->localData)->Invalidate();
		}
	}
}


void
HandleModifier::SelectSubComponent( HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert )
{
	SelectData* SelData = NULL, *od = NULL;

	while ( hitRec )
	{
		SelData = (SelectData*)hitRec->modContext->localData;
		if (theHold.Holding() && !SelData->bHeld)
			theHold.Put(new SubSelRestore(this,SelData));
		theHold.Accept(_T("Select Vertex"));

		if ( all & invert )
		{
#if 0
			// hitRec->hitInfo is the MeshSubHitRec::index it was stored in the
			// HitTest method through LogHit
			if ( (*pArray)[hitRec->hitInfo] )
				pArray->Clear(hitRec->hitInfo);
			else
				pArray->Set(hitRec->hitInfo,selected);
#endif
		}
		else
		{
			SelData->_idxVertexSelected = hitRec->hitInfo;
		}

		if ( !all )
			break;

		hitRec = hitRec->Next();
	}

	NotifyDependents( FOREVER, PART_SELECT, REFMSG_CHANGE );
	Display();
}


void
HandleModifier::ActivateSubobjSel(int level, XFormModes& modes)
{
	// Save the current subobject level
	assert( ip );
	ip->SetSubObjectLevel( level );

	// Create sub object editing mode.
	if ( !selectMode )
		selectMode= new SelectModBoxCMode( this, ip );

	// Fill in modes with our sub-object modes
	if ( selLevel )
		modes = XFormModes(NULL,NULL,NULL,NULL,NULL,selectMode);

	// Setup rollup pages
	SetRollupPages();

	NotifyDependents( FOREVER, PART_SUBSEL_TYPE|PART_DISPLAY, REFMSG_CHANGE );
	ip->PipeSelLevelChanged();
	NotifyDependents( FOREVER, SELECT_CHANNEL|DISP_ATTRIB_CHANNEL|SUBSEL_TYPE_CHANNEL, REFMSG_CHANGE );
}


void
HandleModifier::ClearSelection( int selLevel )
{
	ModContextList list;
	INodeTab nodes;
	ip->GetModContexts(list,nodes);
	SelectData *SelData;

	for ( int i=0; i<list.Count(); ++i )
	{
		SelData = (SelectData*)list[i]->localData;
		if (theHold.Holding() && !SelData->bHeld)
			theHold.Put(new SubSelRestore(this,SelData));
	}

	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	Display();
}


void
HandleModifier::SelectAll( int selLevel )
{
#if 1
	ClearSelection( selLevel );
#else
	ModContextList list;
	INodeTab nodes;
	ip->GetModContexts( list, nodes );
	SelectData* SelData;
	for (int i=0; i<list.Count(); i++)
	{
		SelData = (SelectData*)list[i]->localData;
		if (theHold.Holding() && !SelData->bHeld)
			theHold.Put(new SubSelRestore(this,SelData));
	}

	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	Display();
	theHold.Accept(_T("Select All"));
#endif
}


void
HandleModifier::InvertSelection( int selLevel )
{
#if 1
	ClearSelection( selLevel );
#else
	ModContextList list;
	INodeTab nodes;
	ip->GetModContexts(list,nodes);
	SelectData *SelData;
	for (int i=0; i<list.Count(); i++)
	{
		SelData = (SelectData*)list[i]->localData;
		if (theHold.Holding() && !SelData->bHeld)
			theHold.Put(new SubSelRestore(this,SelData));
	}

	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	Display();
	theHold.Accept(_T("Invert Selection"));
#endif
}


void
HandleModifier::RemoveRollupPages()
{
	if ( hParams )
	{
		ip->DeleteRollupPage( hParams );
	}
	hParams = NULL;
}


void
HandleModifier::SetRollupPages()
{
	RemoveRollupPages();
	hParams = ip->AddRollupPage(
		hInstance,
		MAKEINTRESOURCE(dlgIDs[selLevel]),
		windProcs[selLevel],
		GetString(dlgTitles[selLevel]),
		(LPARAM)this);
	assert( hParams );

	ModContextList mcList;
	INodeTab nodes;

	ip->GetModContexts(mcList,nodes);
	assert( mcList.Count() == 1 );
	SelectData* SelData = (SelectData*)mcList[0]->localData;
	assert( SelData );

	Interval dummyInterval = FOREVER;
	int handleID;
	pblock->GetValue( PB_INDEX_ID, TimeValue(0), handleID, dummyInterval );
	SelData->_id = handleID;
	pblock->GetValue( PB_INDEX_SELECTED_VERTEX, TimeValue(0), SelData->_idxVertexSelected, dummyInterval );

	Display();
}

void
HandleModifier::UpdateData( INode* pNode, ID& id, int idxVertexSelected )
{
	assert( pblock );
	pblock->SetValue( PB_INDEX_ID, TimeValue( 0 ), int( id() ) );
	pblock->SetValue( PB_INDEX_SELECTED_VERTEX, TimeValue( 0 ), idxVertexSelected );
}


void
HandleModifier::Display()
{
	TSTR buf;

	if ( !hParams )
		return;

	ModContextList mcList;
	INodeTab nodes;

	ip->GetModContexts(mcList,nodes);
	assert( mcList.Count() == 1 );
	SelectData* SelData = (SelectData*)mcList[0]->localData;

	buf.printf( "Vertex #%d", SelData->_idxVertexSelected );
	SetDlgItemText( hParams, IDC_VERTEX_NUM, buf );

	strstream szId;
	szId << SelData->_id << '\0';

	HWND hwndID = GetDlgItem( hParams, IDC_ID );
	if ( hwndID )
	{	// This RollUp has the Handle ID edit box
		Edit_SetText( hwndID, szId.str() );
	}

	INode* pNode = nodes[ 0 ];
	assert( pNode );
//	UpdateData( pNode, SelData->_id, SelData->_idxVertexSelected );
}


// Load and Save methods

enum 
{
	HANDLE_ID_CHUNK_TYPE =		1000,
	VERTEX_NUMBER_CHUNK_TYPE =	1010
};

IOResult
HandleModifier::Save( ISave* isave )
{
	Interval dummyInterval;
	ULONG bytesWritten(0);

	isave->BeginChunk( HANDLE_ID_CHUNK_TYPE );
	int handleID;
	pblock->GetValue( PB_INDEX_ID, TimeValue(0), handleID, dummyInterval );
	isave->Write( &handleID, sizeof( handleID ), &bytesWritten );
	if ( bytesWritten != sizeof( handleID ) )
		return IO_ERROR;
	isave->EndChunk();

	isave->BeginChunk( VERTEX_NUMBER_CHUNK_TYPE );
	int vertexNumber;
	pblock->GetValue( PB_INDEX_SELECTED_VERTEX, TimeValue(0), vertexNumber, dummyInterval );
	isave->Write( &vertexNumber, sizeof( vertexNumber ), &bytesWritten );
	if ( bytesWritten != sizeof( vertexNumber ) )
		return IO_ERROR;
	isave->EndChunk();
	return IO_OK;
}


IOResult
HandleModifier::Load( ILoad* iload )
{
	ULONG bytesRead(0);
	IOResult result;

	assert( pblock );

	while (IO_OK==(result=iload->OpenChunk()))
	{
		switch(iload->CurChunkID())
		{
			case HANDLE_ID_CHUNK_TYPE:
			{
				int handleID;
				result = iload->Read(&handleID, sizeof(int), &bytesRead);
				pblock->SetValue( PB_INDEX_ID, TimeValue(0), handleID );
				break;
			}
			case VERTEX_NUMBER_CHUNK_TYPE:
			{
				int vertexNumber;
				result = iload->Read(&vertexNumber, sizeof(int), &bytesRead);
				pblock->SetValue( PB_INDEX_SELECTED_VERTEX, TimeValue(0), vertexNumber );
				break;
			}
		}
		iload->CloseChunk();
		if (result != IO_OK)
			return result;
	}

	return IO_OK;
}


// Useful Procs ------------------------------------------------------

static TriObject*
GetTriObject( TimeValue t, ObjectState& os, Interval& valid, BOOL& needsDel )
{
	needsDel = FALSE;
	valid &= os.Validity( t );

	if ( os.obj->IsSubClassOf( triObjectClassID ) )
	{
		return (TriObject*)os.obj;
	}
	else
	{
		if ( os.obj->CanConvertToType( triObjectClassID ) )
		{
			Object* oldObj = os.obj;
			TriObject* tobj = (TriObject*)os.obj->ConvertToType(t,triObjectClassID);
			needsDel = ( tobj != oldObj );
			return tobj;
		}
	}

	return NULL;
}

// Window Procs ------------------------------------------------------

void
HandleModifier::Command( HWND hWnd, int button, int notifyCode, HandleModifier* mod )
{
	assert( mod );

	HWND hwndButton = GetDlgItem( hWnd, button );
	// CAUTION: hwndButton may be NULL -- assert before using

	switch ( button )
	{
		case IDC_ID:
		{
			if ( notifyCode == EN_SETFOCUS )
				DisableAccelerators();
			else if ( notifyCode == EN_KILLFOCUS )
				EnableAccelerators();
			else if ( notifyCode == EN_UPDATE )
			{
				char szId[ 100 ];
				HWND _hwndId = GetDlgItem( hWnd, IDC_ID );
				assert( _hwndId );

				Edit_GetText( _hwndId, szId, sizeof( szId ) );
				if ( strlen( szId ) > 4 )
					*( szId + 4 ) = '\0';

				ModContextList mcList;
				INodeTab nodes;
				ip->GetModContexts( mcList, nodes );
				assert( mcList.Count() == 1 );
				SelectData* SelData = (SelectData*)mcList[0]->localData;

				SelData->_id = ID( szId );

				UpdateData( nodes[0], SelData->_id, SelData->_idxVertexSelected );

				// force truncated version back into box
				Edit_SetText( _hwndId, szId );
			}
			break;
		}

		case 1:		// MAX sends this when RETURN is pressed in the ID field
		{
			break;
		}

		default:
		{
			assert( 0 );
			break;
		}
	}
}


static BOOL CALLBACK
EditTriVertProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	HandleModifier* mod = (HandleModifier*)GetWindowLong( hWnd, GWL_USERDATA );
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			mod = (HandleModifier*)lParam;
			assert( mod );
			SetWindowLong(hWnd,GWL_USERDATA,lParam);
			mod->hParams = hWnd;
			break;
		}

		case WM_COMMAND:
		{
			assert( mod );
			mod->Command( hWnd, LOWORD( wParam ), HIWORD( wParam ), mod );
			break;
		}

		default:
			return FALSE;
	}
	return TRUE;
}

// SubSelRestore --------------------------------------------------

SubSelRestore::SubSelRestore(HandleModifier *m, SelectData *data)
{
	mod			  = m;
	SelData       = data;
	SelData->bHeld = true;
	usel = *SelData;
}


void
SubSelRestore::Restore(int isUndo)
{
	if (isUndo)
		rsel = *SelData;

	*SelData = usel;

	assert( mod );
	mod->NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	mod->Display();
}


void
SubSelRestore::Redo()
{
	*SelData = rsel;

	assert( mod );
	mod->NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	mod->Display();
}
