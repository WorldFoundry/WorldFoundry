@*=============================================================================
@* Renderer.bas: source file for renderer.bat
@*=============================================================================

@define RENDERER_ENTRY(name, poly,polylower,filename,gouraudFlag, textureFlag, prelitFlag) &RenderObject3D::RenderPoly3D@+name,
@include renderer.s
@undef RENDERER_ENTRY

