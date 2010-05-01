@*============================================================================
@* actor.inc: oas data for movement manager common blocks
@*============================================================================

	LEVELCONFLAGCOMMONBLOCK(movebloc)
	LEVELCONFLAGCOMMONBLOCK(toolset)
	LEVELCONFLAGCOMMONBLOCK(common)

@ifndef(DEFAULT_MESH_PROPSHEET)(@define DEFAULT_MESH_PROPSHEET 0)

	PROPERTY_SHEET_HEADER(Mesh,DEFAULT_MESH_PROPSHEET)
@*-------------------------------------------
@* kts must always have the meshname field so we can tell if the object has a mesh or not
@*	TYPEENTRYFILENAME(Mesh Name,,"Animation Master v4 Figure Files (*.fig)\0*.fig\0VRML Files (*.wrl)\0*.wrl\0""3DS MAX Meshes (*.max)\0*.max\0""3DS R4 Mesh Files (*.3ds)\0*.3ds\0" BITMAP_FILESPEC,".3DS mesh; 8/16-bit TGA; or 4/8-bit BMP","ModelType==1 || ModelType==2")
	TYPEENTRYFILENAME(Mesh Name,,"World Foundry IFF (*.iff)\0*.iff\0" BITMAP_FILESPEC,".iff mesh; 8/16/24-bit TGA; 4/8/24-bit BMP; 24-bit SGI","ModelType==1 || ModelType==2")
	LEVELCONFLAGCOMMONBLOCK(mesh)

	LEVELCONFLAGCOMMONBLOCK(shadow)
        @include flagbloc.inc

@*============================================================================
