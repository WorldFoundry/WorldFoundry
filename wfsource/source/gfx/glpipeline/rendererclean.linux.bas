@*=============================================================================
@* Rendererclean.linux.bas: source file for rendererclean
@*=============================================================================

@define RENDERER_ENTRY(name, poly,polylower,filename,gouraudFlag, textureFlag, lightingFlag) rm filename@+.cc
@include renderer.s
@undef RENDERER_ENTRY

