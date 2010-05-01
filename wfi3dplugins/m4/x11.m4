dnl
dnl CONFIGURE_X11([[ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl \author J.E. Hoffmann <je-h@gmx.net>
dnl
AC_DEFUN(CONFIGURE_X11,
[
  x11_error="no"
  X11_CFLAGS=""
  X11_LIBS=""

  AC_LANG_SAVE
  AC_LANG_C
  AC_PATH_XTRA
  X11_CFLAGS="$X_CFLAGS"
  X11_LIBS="$X_LIBS"
  if test -n "$X_PRE_LIBS"; then
    X11_LIBS="$X_PRE_LIBS $X11_LIBS"
  fi
  if test -n "$X_EXTRA_LIBS"; then
    X11_LIBS="$X_EXTRA_LIBS $X11_LIBS"
  fi
  X11_LIBS="$X11_LIBS -lXmu -lX11"

  AC_MSG_CHECKING(for X)
  if test "$no_x" = "yes"; then
    AC_MSG_RESULT(failed)
    x11_error="yes"
  else
    AC_MSG_RESULT(ok)
  fi
  AC_LANG_RESTORE

  if test "$x11_error" = "no"; then
     ifelse([$1], , :, [$1])     
  else
     ifelse([$2], , :, [$2])
  fi
  AC_SUBST(X11_CFLAGS)
  AC_SUBST(X11_LIBS)
])
