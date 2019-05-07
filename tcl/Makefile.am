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

ECHO=   echo

sysdir=	${datadir}/eltcl
dist_sys_DATA=\
	tools.tcl	\
	complete.tcl	\
	init.tcl

nodist_sys_DATA=\
	version.tcl	\
	pkgIndex.tcl

BUILT_SOURCES=  version.tcl pkgIndex.tcl
CLEANFILES=     ${BUILT_SOURCES}

version.tcl: Makefile
	exec >$@;						\
	$(ECHO) '# $(PACKAGE_STRING) system files';		\
	$(ECHO) 'package provide $(PACKAGE_NAME) $(PACKAGE_VERSION)';

pkgIndex.tcl: Makefile
	l='[list load [file join ${libdir} libeltclsh.so]]';	\
	l="$$l"'|[list source [file join $$dir version.tcl]]';	\
	for s in $(dist_sys_DATA); do				\
	  l="$$l"'|[list source [file join $$dir '"$$s"']]';	\
	done;							\
	$(ECHO) "package ifneeded $(PACKAGE_NAME)"		\
	  "$(PACKAGE_VERSION) $$l" | sed 's/|/\\n/g' >$@
