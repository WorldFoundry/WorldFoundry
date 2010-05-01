dnl
dnl CONFIGURE_NURBS([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl \author J Anderson <janderson@onelink.com>


AC_DEFUN(CONFIGURE_NURBS,
[
  nurbs_error="no"

  NURBS_CFLAGS=""
  NURBS_LDFLAGS=""

  nurbs_includes="/usr/local/include"
  nurbs_libs="/usr/local/lib"
  nurbs_dir="/usr/local"

  AC_ARG_WITH(nurbs-prefix,
    [  --with-nurbs-prefix=PFX    where the root of NURBS is installed ],
    [
      nurbs_includes="$withval/include"
      nurbs_dir="$withval"
     nurbs_lib="$withval/lib"
    ]
  )

  AC_ARG_WITH(nurbs-libs,
    [  --with-nurbs-libs=DIR  where the NURBS libraries are installed ],
    [
      nurbs_libs="$withval"
    ]
  )

  AC_ARG_WITH(nurbs-includes,
    [  --with-nurbs-includes=DIR  where the NURBS includes are installed ],
    [
      nurbs_includes="$withval"
    ]
  )

  if test "$nurbs_error" = "no"; then
    saved_CPPFLAGS="$CPPFLAGS"
    saved_LIBS="$LIBS"
    CPPFLAGS="$saved_CPPFLAGS -I$nurbs_includes"

    AC_MSG_CHECKING([for NURBS includes ($nurbs_includes)])
    AC_CACHE_VAL(nurbs_includes_found,
    [
     if test -e /usr/include/nurbs++/plib_config.h; then
				NURBS_CFLAGS=""
				AC_MSG_RESULT(yes)
     else
				if test -e "$nurbs_includes"/nurbs++/plib_config.h; then
	   			NURBS_CFLAGS="-I$nurbs_includes"
	   			AC_MSG_RESULT(yes)
       else
	   			nurbs_error=yes
	   			AC_MSG_RESULT(no)
        fi
     fi
    ])

    AC_MSG_CHECKING([for NURBS libraries ($nurbs_libs)])
    AC_CACHE_VAL(nurbs_libs_found,
    [
    	if test -e /usr/lib/libnurbsd.so; then
      	NURBS_LDFLAGS=""
        AC_MSG_RESULT(yes)
      else
      	if test -e "$nurbs_libs"/libnurbsd.so; then
					NURBS_LDFLAGS="-L$nurbs_libs"
					AC_MSG_RESULT(yes)
    		else
					nurbs_error=yes
					AC_MSG_RESULT(no)
      	fi
     fi

    ])

    CPPFLAGS="$saved_CPPFLAGS"
    LIBS="$saved_LIBS"
  fi

  if test "$nurbs_includes" = "/usr/include"; then
     	NURBS_CFLAGS=""
     fi
     if test "$nurbs_libraries" = "/usr/lib"; then
     	NURBS_LIBS=""
     fi


  AC_SUBST(NURBS_CFLAGS)
  AC_SUBST(NURBS_LDFLAGS)
  AC_SUBST(nurbs_dir)

  if test "$nurbs_error" = "no"; then
     ifelse([$2], , :, [$2])
  else
     ifelse([$3], , :, [$3])
  fi
])
