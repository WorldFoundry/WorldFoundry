@*=============================================================================
@* Renderer.bas: source file for renderer.bat
@*=============================================================================

@define RENDERER_ENTRY(name, poly,polylower,filename,gouraudFlag, textureFlag, lightingFlag) echo -DRENDER_TYPE=name -DPOLY_TYPE=poly -DPOLY_TYPE_LOWER=polylower -DGOURAUD_FLAG=gouraudFlag -DTEXTURE_FLAG=textureFlag -DLIGHTING_FLAG=lightingFlag >tmp.txt@n..\..\..\bin\prep @@tmp.txt render.ccs filename@+.cc
@include renderer.s
@undef RENDERER_ENTRY

