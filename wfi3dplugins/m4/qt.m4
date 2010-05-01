dnl
dnl CONFIGURE_QT([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl \author J.E. Hoffmann <je-h@gmx.net>
dnl
AC_DEFUN(CONFIGURE_QT,
[
  qt_error="no"

  QT_CFLAGS=""
  QT_LIBS=""
  MOC="moc"
  UIC="uic"
  qt_includes="/usr/include/qt" 
  qt_libraries="/usr/lib" 
  qt_binaries="/usr/bin/"

  if test "$qt_error" = "no"; then
    AC_MSG_CHECKING([for QT environment variable QTDIR])
    if test -z "$QTDIR"; then
      AC_MSG_RESULT(no)
    else
      AC_MSG_RESULT(yes)
      qt_includes="$QTDIR/include" 
      qt_libraries="$QTDIR/lib" 
      qt_binaries="$QTDIR/bin"
      MOC="$qt_binaries/moc";
      UIC="$qt_binaries/uic";
    fi
  fi

  AC_ARG_WITH(qt-prefix,
    [  --with-qt-prefix=PFX    where the root of Qt is installed ],
    [  
      qt_includes="$withval/include"
      qt_libraries="$withval/lib"
      qt_binaries="$withval/bin"
      MOC="$qt_binaries/moc";
      UIC="$qt_binaries/uic";
    ])

  AC_ARG_WITH(qt-includes,
    [  --with-qt-includes=DIR  where the Qt includes are installed ],
    [
      qt_includes="$withval"
    ])

  AC_ARG_WITH(qt-libraries,
    [  --with-qt-libraries=DIR where the Qt libraries are installed.],
    [  
      qt_libraries="$withval"
    ])

  AC_ARG_WITH(qt-binaries,
    [  --with-qt-binaries=DIR  where the Qt binaries are installed.],
    [  
      qt_binaries="$withval"
      MOC="$qt_binaries/moc";
      UIC="$qt_binaries/uic";
    ])

  AC_ARG_WITH(qt-moc,
    [  --with-qt-moc=PROG      where the Qt meta object compiler is installed.],
    [  
      MOC="$withval"
    ])

  AC_ARG_WITH(qt-uic,
    [  --with-qt-uic=PROG      where the Qt user interface compiler is installed.],
    [  
      UIC="$withval"
    ])

  if test "$qt_error" = "no"; then
    saved_CPPFLAGS="$CPPFLAGS"
    saved_LIBS="$LIBS"
    CPPFLAGS="$saved_CPPFLAGS -I$qt_includes"


    gl_qt_lib=""

    if test -e "$qt_libraries/libqt-gl.so"; then
    	gl_qt_lib="qt-gl"
    else
    	if test -e "$qt_libraries/libqt2.so"; then
    		gl_qt_lib="qt2"
    	else
    		gl_qt_lib="qt-mt"
	fi
    fi



    if test -n "$qt_libraries"; then
      LIBS="$saved_LIBS -L$qt_libraries -l$gl_qt_lib"
    else
      LIBS="$saved_LIBS -lqt-mt -lm $X11_LIBS"
    fi
    AC_MSG_CHECKING([for QT includes ($qt_includes)])
    AC_CACHE_VAL(qt_includes_found,
    [
      AC_TRY_CPP([#include <qglobal.h>],qt_includes_found=yes,
        qt_includes_found=no)
      if test "$qt_includes_found" = "yes"; then
        QT_CFLAGS="-I$qt_includes"
        AC_MSG_RESULT(yes)
      else
        qt_error=yes
        AC_MSG_RESULT(no)
      fi
    ])

    AC_MSG_CHECKING([for QT libraries ($qt_libraries)])
    AC_CACHE_VAL(qt_libraries_found,
    [
      AC_LANG_SAVE
      AC_LANG_CPLUSPLUS
      saved_CXXFLAGS="$CXXFLAGS"
      saved_LIBS="$LIBS"
      CXXFLAGS="$QT_CFLAGS $LIBS"
      LIBS="$QT_LIBS $LIBS"

      AC_TRY_RUN([
          #include <qglobal.h>
          int main(int argc, char **argv)
          {
          }
        ],[
          AC_MSG_RESULT(yes)
        ],[
          AC_MSG_RESULT(no)
          qt_error="no"
        ],
        AC_MSG_ERROR([cross compiling unsupported])
      )

      LIBS="$saved_LIBS"
      CXXFLAGS="$saved_CXXFLAGS"
      AC_LANG_RESTORE
    ])dnl



    AC_MSG_CHECKING([for QT moc ($MOC)])
    output=`eval "$MOC --help 2>&1 | sed -e '1q' | grep Qt"`
    if test -z "$output"; then
      AC_MSG_RESULT(no)
      qt_error="yes"
    else
      AC_MSG_RESULT(yes)
    fi

    AC_MSG_CHECKING([for QT uic ($UIC)])
    output=`eval "$UIC --help 2>&1 | sed -e '1q' | grep Qt"`
    if test -z "$output"; then
      AC_MSG_RESULT(no)
      qt_error="yes"
    else
      AC_MSG_RESULT(yes)
    fi
    CPPFLAGS="$saved_CPPFLAGS"
    LIBS="$saved_LIBS"
  fi

  if test "$qt_error" = "no"; then
    AC_MSG_CHECKING([for QT version >= $1])
    qt_major_version=`echo $1 | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    qt_minor_version=`echo $1 | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    qt_micro_version=`echo $1 | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
    qt_version="$qt_major_version$qt_minor_version$qt_micro_version"

    AC_LANG_SAVE
    AC_LANG_CPLUSPLUS
    saved_CXXFLAGS="$CXXFLAGS"
    saved_LIBS="$LIBS"
    CXXFLAGS="$QT_CFLAGS $LIBS"
    LIBS="$QT_LIBS $LIBS"

    AC_TRY_RUN([
        #include <qglobal.h>
        int main()
        {
          if (QT_VERSION < $qt_version) return(1);
          return(0);
        }
      ],[
        AC_MSG_RESULT(yes)
      ],[
        AC_MSG_RESULT(no)
        qt_error="yes"
      ],
      AC_MSG_ERROR([cross compiling unsupported])
    )

    LIBS="$saved_LIBS"
    CXXFLAGS="$saved_CXXFLAGS"
    AC_LANG_RESTORE
  fi

 dnl gl test here.


  AC_SUBST(QT_CFLAGS)
  AC_SUBST(QT_LIBS)
  AC_SUBST(MOC)
  AC_SUBST(UIC)
  if test "$qt_error" = "no"; then
     ifelse([$2], , :, [$2])     
  else
     ifelse([$3], , :, [$3])
  fi
])
