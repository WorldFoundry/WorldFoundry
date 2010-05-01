@*=============================================================================
@* Renderer.s: master control file for psx renderers
@*=============================================================================

@* be sure to change MAX_RENDERERS in rendobj3.hp

	@* Function Name extension, poly name upper case, poly name lower case, filename, gouroud flag, texture flag
	RENDERER_ENTRY(FlatColorLit,F3,f3,rendfcl,0,0,1)
	RENDERER_ENTRY(GouraudColorLit,G3,g3,rendgcl,1,0,1)
	RENDERER_ENTRY(FlatTextureLit,FT3,ft3,rendftl,0,1,1)
	RENDERER_ENTRY(GouraudTextureLit,GT3,gt3,rendgtl,1,1,1)

	RENDERER_ENTRY(FlatColorPreLit,F3,f3,rendfcp,0,0,0)
	RENDERER_ENTRY(GouraudColorPreLit,G3,g3,rendgcp,1,0,0)
	RENDERER_ENTRY(FlatTexturePreLit,FT3,ft3,rendftp,0,1,0)
	RENDERER_ENTRY(GouraudTexturePreLit,GT3,gt3,rendgtp,1,1,0)

@*=============================================================================
