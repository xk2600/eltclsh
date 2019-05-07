#
# Copyright (c) 2012 LAAS/CNRS
# All rights reserved.
#
# Permission to use, copy, modify, and distribute this software for any purpose
# with or without   fee is hereby granted, provided   that the above  copyright
# notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
# REGARD TO THIS  SOFTWARE INCLUDING ALL  IMPLIED WARRANTIES OF MERCHANTABILITY
# AND FITNESS. IN NO EVENT SHALL THE AUTHOR  BE LIABLE FOR ANY SPECIAL, DIRECT,
# INDIRECT, OR CONSEQUENTIAL DAMAGES OR  ANY DAMAGES WHATSOEVER RESULTING  FROM
# LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
# OTHER TORTIOUS ACTION,   ARISING OUT OF OR IN    CONNECTION WITH THE USE   OR
# PERFORMANCE OF THIS SOFTWARE.
#
#                                            Anthony Mallet on Thu Nov 22 2012
#
AM_CPPFLAGS=	${TCL_INCLUDE_SPEC}
AM_CPPFLAGS+=	-DELTCLSH_DATA='"${datadir}/eltcl"'
AM_CPPFLAGS+=	-DELTCLSH_LIBDIR='"${libdir}"'
AM_CPPFLAGS+=	-DELTCLSH_VERSION='"${PACKAGE_VERSION}"'

# interpreters
bin_PROGRAMS=		eltclsh elwish

eltclsh_LDFLAGS=	${RUNTIME_PATH}
eltclsh_LDADD=		libcommon.la ${TCL_LIB_SPEC}

elwish_CPPFLAGS=	${TK_INCLUDE_SPEC}
elwish_LDFLAGS=		${RUNTIME_PATH}
elwish_LDADD=		libcommon.la ${TK_LIB_SPEC} ${TCL_LIB_SPEC}

# package
lib_LTLIBRARIES=	libeltclsh.la

libeltclsh_la_SOURCES=\
	eltclsh.h	\
	parse.h		\
	el.c		\
	init.c		\
	parse.c		\
	signal.c	\
	complete.c

libeltclsh_la_LDFLAGS=	-avoid-version -rpath ${libdir} ${RUNTIMEPATH}
libeltclsh_la_LIBADD=	$(LTLIBOBJS) ${TCL_LIB_SPEC}

# common files
noinst_LTLIBRARIES= libcommon.la
libcommon_la_SOURCES=\
	loop.c


clean-local:
	-${LIBTOOL} --mode=clean rm -f $(LTLIBOBJS)
