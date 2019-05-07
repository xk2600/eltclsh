/*
 * Copyright (c) 2001-2004,2008-2010,2012 LAAS/CNRS
 * All rights reserved.
 *
 * Redistribution and use  in source  and binary  forms,  with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *   1. Redistributions of  source  code must retain the  above copyright
 *      notice, this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice,  this list of  conditions and the following disclaimer in
 *      the  documentation  and/or  other   materials provided  with  the
 *      distribution.
 *
 * THIS  SOFTWARE IS PROVIDED BY  THE  COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND  ANY  EXPRESS OR IMPLIED  WARRANTIES,  INCLUDING,  BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES  OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR  PURPOSE ARE DISCLAIMED. IN  NO EVENT SHALL THE COPYRIGHT
 * HOLDERS OR      CONTRIBUTORS  BE LIABLE FOR   ANY    DIRECT, INDIRECT,
 * INCIDENTAL,  SPECIAL,  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE   OF THIS SOFTWARE, EVEN   IF ADVISED OF   THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 *                                      Anthony Mallet on Tue Oct 16 2001
 */
#include "elconfig.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>

#include "eltclsh.h"


/*
 * elTclshLoop ----------------------------------------------------------
 *
 * Main loop: it reads commands and execute them
 */

#if TCL_MAJOR_VERSION >= 8 && TCL_MINOR_VERSION >= 4
void
elTclshLoop(int argc, const char **argv, ElTclAppInitProc appInitProc)
#else
void
elTclshLoop(int argc, char **argv, ElTclAppInitProc appInitProc)
#endif /* TCL_VERSION */
{
   Tcl_Interp *interp;
   Tcl_Obj *resultPtr, *obj;
#if TCL_MAJOR_VERSION >= 8 && TCL_MINOR_VERSION >= 4
   const
#endif
     char *fileName, *args;
   char buffer[1000], *bytes;
   int code, tty, length;
   int exitCode = 0;
   Tcl_Channel errChannel;

   /* initialize interpreter */
   interp = Tcl_CreateInterp();
   if (interp == NULL) {
      fputs("cannot create tcl interpreter\n", stderr);
      return;
   }

   /*
    * Make command-line arguments available in the Tcl variables "argc"
    * and "argv".  If the first argument doesn't start with a "-" then
    * strip it off and use it as the name of a script file to process.
    */

   fileName = NULL;
   if ((argc > 1) && (argv[1][0] != '-')) {
      fileName = argv[1];
      argc--; argv++;
   }
   args = Tcl_Merge(argc-1, argv+1);
   Tcl_SetVar(interp, "argv", args, TCL_GLOBAL_ONLY);
   Tcl_Free((char *)args);
   snprintf(buffer, sizeof(buffer), "%d", argc-1);
   Tcl_SetVar(interp, "argc", buffer, TCL_GLOBAL_ONLY);
   args = (fileName != NULL) ? fileName : argv[0];
   Tcl_SetVar(interp, "argv0", args, TCL_GLOBAL_ONLY);


   /* Set the "tcl_interactive" variable. */
   tty = isatty(0);
   Tcl_SetVar(interp, "tcl_interactive",
	      ((fileName == NULL) && tty) ? "1" : "0", TCL_GLOBAL_ONLY);

   /* Invoke application-specific initialization. */
   if (Tcl_Init(interp) == TCL_ERROR) {
      Tcl_SetResult(interp, "cannot initialize tcl", TCL_STATIC);
      return;
   }

   /* configure standard path for packages */
   obj = Tcl_GetVar2Ex(interp, "auto_path", NULL, TCL_GLOBAL_ONLY);
   if (!obj) obj = Tcl_NewListObj(0, NULL);

   Tcl_ListObjAppendElement(interp, obj, Tcl_NewStringObj(ELTCLSH_DATA, -1));
   Tcl_SetVar2Ex(interp, "auto_path", NULL, obj, TCL_GLOBAL_ONLY);


   /* require eltclsh extension. In case this fails (typically during install,
    * before pkgIndex.tcl is built), print the error message but don't give up
    * (chicken-egg pb for pkg_mkIndex). */
   if (!Tcl_PkgRequire(interp, "eltclsh", ELTCLSH_VERSION, 1)) {
      errChannel = Tcl_GetStdChannel(TCL_STDERR);
      if (errChannel) {
	 resultPtr = Tcl_GetObjResult(interp);
	 bytes = Tcl_GetStringFromObj(resultPtr, &length);
	 Tcl_Write(errChannel, bytes, length);
	 Tcl_Write(errChannel, "...continuing anyway\n", 21);
      }
   }

   if (appInitProc && (*appInitProc)(interp) != TCL_OK) {
      errChannel = Tcl_GetStdChannel(TCL_STDERR);
      if (errChannel) {
#if TCL_MAJOR_VERSION >= 8 && TCL_MINOR_VERSION >= 4
	 const char *msg;
#else
	 char *msg;
#endif /* TCL_VERSION */

	 msg = Tcl_GetVar(interp, "errorInfo", TCL_GLOBAL_ONLY);
	 if (msg != NULL) {
	    Tcl_Write(errChannel, msg, strlen(msg));
	    Tcl_Write(errChannel, "\n", 1);
	 }
	 resultPtr = Tcl_GetObjResult(interp);
	 bytes = Tcl_GetStringFromObj(resultPtr, &length);
	 Tcl_Write(errChannel, bytes, length);
	 Tcl_Write(errChannel, "\n", 1);
      }

      exitCode = 2;
      goto done;
   }

   (void)Tcl_SourceRCFile(interp);
   Tcl_Flush(Tcl_GetStdChannel(TCL_STDERR));

   /* If a script file was specified then just source that file and
    * quit. */
   if (fileName != NULL) {
      code = Tcl_EvalFile(interp, fileName);
      if (code != TCL_OK) {
	 errChannel = Tcl_GetStdChannel(TCL_STDERR);
	 if (errChannel) {
	    /* The following statement guarantees that the errorInfo
	     * variable is set properly. */
	    Tcl_AddErrorInfo(interp, "");
	    Tcl_Write(errChannel,
		      Tcl_GetVar(interp, "errorInfo", TCL_GLOBAL_ONLY),
		      -1);
	    Tcl_Write(errChannel, "\n", 1);
	 }

	 exitCode = 1;
      }

      goto done;
   }

   /* Process commands from stdin until there's an end-of-file. */
   Tcl_Eval(interp, "interactive");

   /*
    * Rather than calling exit, invoke the "exit" command so that
    * users can replace "exit" with some other command to do additional
    * cleanup on exit.  The Tcl_Eval call should never return.
    */

 done:
   snprintf(buffer, sizeof(buffer), "exit %d", exitCode);
   Tcl_Eval(interp, buffer);
}
