#ifndef MESHNAME_H
#define MESHNAME_H

#include "../oaddlg.h"
#include "buttons/filename.h"

class MeshName : public Filename
	{
public:
	MeshName::MeshName( typeDescriptor* td );
	virtual MeshName::~MeshName();

	virtual int make_dialog_gadgets( HWND );
	virtual void activate( HWND );
	virtual bool enable( bool bEnabled );

protected:
	bool validReference( const char* szMeshName ) const;
	void createMesh();

private:
	HWND _hwndCreateMesh;
	};

#endif	/* MESHNAME_H */
