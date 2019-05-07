#	$LAAS$

#
#  Copyright (c) 2001-2003,2010,2012 LAAS/CNRS             --  Sat Oct  6 2001
#  All rights reserved.                                    Anthony Mallet
#
#
# Redistribution  and  use in source   and binary forms,  with or without
# modification, are permitted provided that  the following conditions are
# met:
#
#   1. Redistributions  of  source code must  retain  the above copyright
#      notice, this list of conditions and the following disclaimer.
#   2. Redistributions in binary form must  reproduce the above copyright
#      notice,  this list of  conditions and  the following disclaimer in
#      the  documentation   and/or  other  materials   provided with  the
#      distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE  AUTHOR AND CONTRIBUTORS ``AS IS'' AND
# ANY  EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES   OF MERCHANTABILITY AND  FITNESS  FOR  A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO  EVENT SHALL THE AUTHOR OR  CONTRIBUTORS
# BE LIABLE FOR ANY DIRECT, INDIRECT,  INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING,  BUT  NOT LIMITED TO, PROCUREMENT  OF
# SUBSTITUTE  GOODS OR SERVICES;  LOSS   OF  USE,  DATA, OR PROFITS;   OR
# BUSINESS  INTERRUPTION) HOWEVER CAUSED AND  ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE  USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

# Make packages in that directory available
eval lappend auto_path $eltcl_pkgPath
if { [info exists env(ELTCLLIBPATH)] } {
    eval lappend auto_path $env(ELTCLLIBPATH)
}

# Install default signal handlers (if the signal command exists)
if { [info command signal] != "" } { namespace eval el {
    proc sighandler { signal severity } {
	switch $severity {
	    "fatal" {
		puts ""
		puts "*** Got signal SIG$signal"
		while { 1 } {
		    puts -nonewline "*** Choose 1: continue, 2: exit > "
		    flush stdout
		    set c [el::getc]
		    switch $c {
			1 { puts $c; break }
			2 { puts $c; exit }
			"\n" { }
			"\r" { }
			default { puts $c }
		    }
		}
	    }
	}
    }

    signal INT [namespace code "sighandler INT fatal"]
}}

# Preload packages
if {[info exists ::argv]} {
    while {[set i [lsearch -exact $::argv -package]] >= 0} {
	set pkgname [lindex $::argv [expr $i+1]]
	if {[catch {package require $pkgname} m]} {
	    puts "$m"
	    puts "cannot load $pkgname"
	} else {
	    puts "loaded $pkgname package"
	}
	set ::argv [lreplace $::argv $i [expr $i+1]]
    }
    unset i
    catch { unset pkgname }
}
