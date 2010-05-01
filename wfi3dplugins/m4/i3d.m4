dnl
dnl CONFIGURE_I3D([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl \author J Anderson <janderson@onelink.com>


AC_DEFUN(CONFIGURE_I3D,
[
  i3d_error="no"

  I3D_CFLAGS=""

  i3d_includes="/usr/local/include/i3d"
  i3d_dir="/usr/local"

  if test "$i3d_error" = "no"; then
    AC_MSG_CHECKING([for I3D environment variable I3DDIR])
    if test -z "$I3DDIR"; then
      AC_MSG_RESULT(no)
    else
      AC_MSG_RESULT(yes)
      i3d_includes="$I3DDIR/include"
    fi
  fi

  AC_ARG_WITH(i3d-prefix,
    [  --with-i3d-prefix=PFX    where the root of I3D is installed ],
    [  
      i3d_includes="$withval/include"
      i3d_dir="$withval"
    ]
  )

  AC_ARG_WITH(i3d-includes,
    [  --with-i3d-includes=DIR  where the I3D includes are installed ],
    [
      i3d_includes="$withval"
    ]
  )

  if test "$i3d_error" = "no"; then
    saved_CPPFLAGS="$CPPFLAGS"
    saved_LIBS="$LIBS"
    CPPFLAGS="$saved_CPPFLAGS -I$i3d_includes"

    AC_MSG_CHECKING([for I3D includes ($i3d_includes)])
    AC_CACHE_VAL(i3d_includes_found,
    [
      AC_TRY_CPP([#include <resource.h>],i3d_includes_found=yes,
        i3d_includes_found=no)
      if test "$i3d_includes_found" = "yes"; then
        I3D_CFLAGS="-I$i3d_includes"
        AC_MSG_RESULT(yes)
      else
        i3d_error=yes
        AC_MSG_RESULT(no)
      fi
    ])


    CPPFLAGS="$saved_CPPFLAGS"
    LIBS="$saved_LIBS"
  fi


  AC_SUBST(I3D_CFLAGS)
  AC_SUBST(i3d_dir)

  if test "$i3d_error" = "no"; then
     ifelse([$2], , :, [$2])     
  else
     ifelse([$3], , :, [$3])
  fi
])
