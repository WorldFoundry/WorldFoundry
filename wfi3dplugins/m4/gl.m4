#
# CONFIGURE_OPENGL([ACTION-IF-FOUND [,ACTION-IF-NOT-FOUND]])
#
AC_DEFUN(CONFIGURE_OPENGL,
[
  GL_CFLAGS=""
  GL_LIBS=""
  gl_includes="/usr/X11R6/include" 
  gl_libraries="/usr/X11R6/lib" 

  AC_ARG_WITH(gl-prefix,    
    [  --with-gl-prefix=PFX     Prefix where OpenGL or Mesa is installed],
    [
      gl_includes="$withval/include"
      gl_libraries="$withval/lib"
    ])

  AC_ARG_WITH(gl-includes,    
    [  --with-gl-includes=DIR   where the OpenGL or Mesa includes are installed],
    [
      gl_includes="$withval"
    ])

  AC_ARG_WITH(gl-libraries,    
    [  --with-gl-libraries=DIR  where the OpenGL or Mesa libraries are installed],
    [
      gl_libraries="$withval"
    ])

  GL_CFLAGS="-I$gl_includes"
  GL_LIBS="-L$gl_libraries"

  saved_CFLAGS="$CFLAGS"
  saved_LIBS="$LIBS"
  AC_LANG_SAVE
  AC_LANG_C
  have_GL=no

  # test for standard OpenGL
  if test "$have_GL" = no; then
    AC_MSG_CHECKING([GL])
    CFLAGS="$saved_CFLAGS $GL_CFLAGS $X11_CFLAGS"
    LIBS="$saved_LIBS -lGLU -lGL $X11_LIBS"
    AC_TRY_LINK(,[ char glBegin(); glBegin(); ], have_GL=yes, have_GL=no)
    AC_MSG_RESULT($have_GL)
    if test "$have_GL" = yes; then
      GL_LIBS="-lGL -lGLU"
    fi
  fi

  # test for Mesa without threads
  if test "$have_GL" = no; then
    AC_MSG_CHECKING([Mesa])
    CFLAGS="$saved_CFLAGS $GL_CFLAGS $X11_CFLAGS"
    LIBS="$saved_LIBS -lMesaGLU -lMesaGL $X11_LIBS"
    AC_TRY_LINK(,[ char glBegin(); glBegin(); ], have_GL=yes, have_GL=no)
    AC_MSG_RESULT($have_GL)
    if test "$have_GL" = yes; then
      GL_LIBS="-lMesaGLU -lMesaGL"
    fi
  fi

  # test for Mesa with threads
  if test "$have_GL" = no; then
    AC_MSG_CHECKING([Mesa with pthreads])
    CFLAGS="$saved_CFLAGS $GL_CFLAGS $X11_CFLAGS"
    LIBS="$saved_LIBS -lMesaGLU -lMesaGL -lpthread $X11_LIBS"
    AC_TRY_LINK(,[ char glBegin(); glBegin(); ], have_GL=yes, have_GL=no)
    AC_MSG_RESULT($have_GL)
    if test "$have_GL" = yes; then
      GL_LIBS="-lMesaGLU -lMesaGL -lpthread"
    fi
  fi

  LIBS="$saved_LIBS"
  CFLAGS="$saved_CFLAGS"
  AC_LANG_RESTORE

  if test "$have_GL" = "yes"; then
     ifelse([$1], , :, [$1])     
  else
     GL_CFLAGS=""
     GL_LIBS=""
     ifelse([$2], , :, [$2])
  fi
  AC_SUBST(GL_CFLAGS)
  AC_SUBST(GL_LIBS)
])



