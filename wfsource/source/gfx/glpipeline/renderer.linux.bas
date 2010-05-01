@*=============================================================================
@* Renderer.bas: source file for renderer.bat
@*=============================================================================

#==============================================================================
# renderer.mak : built from renderer.linux.bas DO NOT MODIFY
#==============================================================================

TARGETS = 

@define RENDERER_ENTRY(name, poly,polylower,filename,gouraudFlag, textureFlag, lightingFlag) TARGETS += filename@+.cc
@include renderer.s
@undef RENDERER_ENTRY

                                                                                                          
all : $(TARGETS)


@define RENDERER_ENTRY(name, poly,polylower,filename,gouraudFlag, textureFlag, lightingFlag) @-filename@+.cc : render.ccs renderer.mak @n	echo -DRENDER_TYPE=name -DPOLY_TYPE=poly -DPOLY_TYPE_LOWER=polylower -DGOURAUD_FLAG=gouraudFlag -DTEXTURE_FLAG=textureFlag -DLIGHTING_FLAG=lightingFlag -DFILENAME=filename >tmp.txt@n	../../../bin/prep @@tmp.txt render.ccs filename@+.cc
@include renderer.s
@undef RENDERER_ENTRY
                                                                                                          



