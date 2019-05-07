dnl
dnl configure.in --
dnl	process this file with autoconf to produce a configure script.
dnl
AC_INIT([eltclsh],[1.14],[openrobots@laas.fr])
AC_PREREQ(2.59)

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([m4])
AC_CONFIG_HEADER([m4/elconfig.h])
AC_CONFIG_LIBOBJ_DIR([compat])
AM_INIT_AUTOMAKE([foreign no-define subdir-objects])


dnl --- checks for programs ---------------------------------------------
dnl
AC_DISABLE_STATIC
LT_INIT
AC_PROG_CC


dnl Checks for headers --------------------------------------------------
dnl
AC_TYPE_SIGNAL
AC_REPLACE_FUNCS(strlcat strlcpy)


dnl --- checks for libraries --------------------------------------------
dnl
AC_SEARCH_LIBS([tputs], [termcap ncurses])
AC_SEARCH_LIBS([ceil], [m])

dnl libedit
AC_LANG_C

AC_PREPROC_IFELSE(
  [AC_LANG_PROGRAM([[#include <histedit.h>]],[[int a = EL_GETCFN]])],
  [AC_MSG_RESULT([checking for libedit headers... yes])],
  [AC_MSG_FAILURE([checking for libedit headers... no])])
AC_SEARCH_LIBS([el_set],[edit],
  [AC_MSG_RESULT([checking for libedit library... yes])],
  [AC_MSG_FAILURE([checking for libedit library... no])])


dnl tcl
SC_PATH_TCLCONFIG
SC_LOAD_TCLCONFIG

SC_PATH_TKCONFIG
SC_LOAD_TKCONFIG

dnl runtime path
rdir=
for d in $LDFLAGS $LIBS $TCL_LIB_SPEC $TK_LIB_SPEC; do
  case $next in dir) rdir=$rdir" "$d;; esac
  next=
  case $d in -L) next=dir;; -L*) rdir=$rdir" ${d#-L}";; esac
done
AC_SUBST(RUNTIME_PATH, [])
for d in $rdir; do
  case $d in
    /lib*|/usr/lib*) ;;
    *) RUNTIME_PATH=$RUNTIME_PATH" -R"$d ;;
  esac
done


dnl --- create the Makefiles --------------------------------------------
dnl
AC_CONFIG_FILES([
        eltclsh.pc
        Makefile
        src/Makefile
        tcl/Makefile
])

AC_OUTPUT
