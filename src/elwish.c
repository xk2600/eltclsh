/*
 * Copyright (c) 2001,2010-2012 LAAS/CNRS
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

#include "eltclsh.h"

#include <tk.h>

/* local functions */
static int	elWishAppInit(Tcl_Interp *interp);


/*
 * main -----------------------------------------------------------------
 *
 * elwish main
 */

#if TCL_MAJOR_VERSION >= 8 && TCL_MINOR_VERSION >= 4
int
main(int argc, const char *argv[])
#else
int
main(int argc, char *argv[])
#endif /* TCL_VERSION */
{
   elTclshLoop(argc, argv, elWishAppInit);
   return 0;
}


/*
 * elWishAppInit --------------------------------------------------------
 *
 * elwish application init. We just call elTclAppInit and initialize Tk
 */

static int
elWishAppInit(Tcl_Interp *interp)
{
   if (Tk_Init(interp) == TCL_ERROR) {
      return TCL_ERROR;
   }

   /* change the rc file */
   Tcl_SetVar(interp, "tcl_rcFileName", "~/.elwishrc", TCL_GLOBAL_ONLY);

   /* I hate that stupid empty window you get after Tk_Init() */
   Tcl_Eval(interp, "wm withdraw .");

   return TCL_OK;
}
