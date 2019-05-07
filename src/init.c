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
 *                                      Anthony Mallet on Wed Oct 10 2001
 */
#include "elconfig.h"

static char copyright[] = " - Copyright (C) 2001-2012 LAAS-CNRS";
static char *version = ELTCLSH_VERSION;

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "eltclsh.h"

#if !HAVE_STRLCAT
extern size_t strlcat(char *, const char *, size_t);
#endif
#if !HAVE_STRLCPY
extern size_t strlcpy(char *, const char *, size_t);
#endif


/* private functions */
static const char *	elTclPrompt(EditLine *el);


/*
 * Eltclsh_Init ------------------------------------------------------------
 *
 * Initialization of eltclsh extension in an existing interpreter: start
 * libedit, source standard procedures and create commands.
 */
int
Eltclsh_Init(Tcl_Interp *interp)
{
   ElTclInterpInfo *iinfo;
   Tcl_Channel inChannel;
   Tcl_DString initFile;
   Tcl_Obj *obj;
   HistEvent ev;
#if TCL_MAJOR_VERSION >= 8 && TCL_MINOR_VERSION >= 4
   const
#endif
     char *eltclLibrary[2];

   /* initialize stubs (they appeared in 8.1) */
   if (Tcl_InitStubs(interp, "8.1", 0) == NULL) {
     return TCL_ERROR;
   }

   /* create main data structure */
   iinfo = calloc(1, sizeof(*iinfo));
   if (iinfo == NULL) {
      fputs("cannot alloc %d bytes\n", stderr);
      return TCL_ERROR;
   }
   iinfo->argv0 = "eltclsh";
   iinfo->interp = interp;
   if (elTclGetWindowSize(fileno(stdin), NULL, &iinfo->windowSize) < 0) {
     if (elTclGetWindowSize(fileno(stdout), NULL, &iinfo->windowSize) < 0)
       iinfo->windowSize = 80;
   }
   iinfo->completionQueryItems = 100;
   iinfo->prompt1Name = Tcl_NewStringObj("el::prompt1", -1);
   Tcl_IncrRefCount(iinfo->prompt1Name);
   iinfo->prompt2Name = Tcl_NewStringObj("el::prompt2", -1);
   Tcl_IncrRefCount(iinfo->prompt2Name);
   iinfo->matchesName = Tcl_NewStringObj("el::matches", -1);
   Tcl_IncrRefCount(iinfo->matchesName);
   iinfo->promptString = NULL;
   iinfo->preReadSz = 0;
   iinfo->gotPartial = 0;
   iinfo->command = NULL;
   iinfo->maxCols = 0; /* if >0, limit number of columns in completion output */

   iinfo->histSize = 800;
   obj = Tcl_NewStringObj("~/.eltclhistory", -1);
   Tcl_IncrRefCount(obj);
   iinfo->histFile = strdup(Tcl_FSGetNativePath(obj));
   Tcl_DecrRefCount(obj);

   if (elTclHandlersInit(iinfo) != TCL_OK) {
      fputs("warning: signal facility not created\n", stdout);
   }

   /* some useful commands */
   Tcl_CreateObjCommand(iinfo->interp, "exit", elTclExit, iinfo, NULL);
   Tcl_CreateObjCommand(iinfo->interp, "interactive", elTclInteractive, iinfo, NULL);
   Tcl_CreateObjCommand(iinfo->interp, "el::gets", elTclGets, iinfo, NULL);
   Tcl_CreateObjCommand(iinfo->interp, "el::getc", elTclGetc, iinfo, NULL);
   Tcl_CreateObjCommand(iinfo->interp, "el::history",
			elTclHistory, iinfo, NULL);
   Tcl_CreateObjCommand(iinfo->interp, "el::parse",
			elTclBreakCommandLine, iinfo, NULL);

#if 0
   Tcl_CreateObjCommand(iinfo->interp, "wrappedputs", rtclshWrappedPutsCmd, 0,
			NULL);
#endif


   /* and variables */
   Tcl_SetVar(iinfo->interp, "tcl_rcFileName", "~/.eltclshrc", TCL_GLOBAL_ONLY);

   obj = Tcl_NewStringObj("el::queryItems", -1);
   if (Tcl_LinkVar(iinfo->interp,
		   Tcl_GetStringFromObj(obj, NULL),
		   (char *)&iinfo->completionQueryItems,
		   TCL_LINK_INT) != TCL_OK) {
      return TCL_ERROR;
   }
   obj = Tcl_NewStringObj("el::maxCols", -1);
   Tcl_LinkVar(iinfo->interp, Tcl_GetStringFromObj(obj, NULL),
               (char *)&iinfo->maxCols, TCL_LINK_INT);

   Tcl_PkgProvide(iinfo->interp, "eltclsh", version);

   /* initialize libedit */
   iinfo->el = el_init(iinfo->argv0, stdin, stdout, stderr);
   if (iinfo->el == NULL) {
      Tcl_SetResult(iinfo->interp, "cannot initialize libedit", TCL_STATIC);
      return TCL_ERROR;
   }

   iinfo->history = history_init();
   history(iinfo->history, &ev, H_SETSIZE, iinfo->histSize);
   if (iinfo->histFile && iinfo->histFile[0])
     history(iinfo->history, &ev, H_LOAD, iinfo->histFile);

   iinfo->askaHistory = history_init();
   history(iinfo->askaHistory, &ev, H_SETSIZE, 100);

   el_set(iinfo->el, EL_CLIENTDATA, iinfo);
   el_set(iinfo->el, EL_HIST, history, iinfo->history);
   el_set(iinfo->el, EL_EDITOR, "emacs");
   el_set(iinfo->el, EL_PROMPT, elTclPrompt);
   el_source(iinfo->el, NULL);

   el_set(iinfo->el, EL_ADDFN,
	  "eltcl-complete", "Context sensitive argument completion",
	  elTclCompletion);
   el_set(iinfo->el, EL_BIND, "^I", "eltcl-complete", NULL);

   el_get(iinfo->el, EL_EDITMODE, &iinfo->editmode);

   /* set up the non-blocking read stuff */
   inChannel = Tcl_GetStdChannel(TCL_STDIN);
   if (inChannel) {
      Tcl_CreateChannelHandler(inChannel, TCL_READABLE, elTclRead, iinfo);
      el_set(iinfo->el, EL_GETCFN, elTclEventLoop);
   }

   /* configure standard path for packages */
   obj = Tcl_NewListObj(0, NULL);
   Tcl_ListObjAppendElement(iinfo->interp, obj,
			    Tcl_NewStringObj(ELTCLSH_LIBDIR, -1));
   Tcl_ListObjAppendElement(iinfo->interp, obj,
			    Tcl_NewStringObj(ELTCLSH_DATA "/..", -1));
   Tcl_SetVar(iinfo->interp,
	      "eltcl_pkgPath", Tcl_GetString(obj), TCL_GLOBAL_ONLY);

   /* source standard eltclsh libraries */
   eltclLibrary[0] = getenv("ELTCL_LIBRARY");
   if (eltclLibrary[0] == NULL) {
      eltclLibrary[0] = ELTCLSH_DATA;
   }
   eltclLibrary[1] = "init.tcl";
   Tcl_SetVar(iinfo->interp,
	      "eltcl_library", eltclLibrary[0], TCL_GLOBAL_ONLY);
   Tcl_DStringInit(&initFile);
   if (Tcl_EvalFile(iinfo->interp,
		    Tcl_JoinPath(2, eltclLibrary, &initFile)) != TCL_OK) {
      Tcl_AppendResult(iinfo->interp,
		       "\nThe directory ",
		       eltclLibrary[0],
		       " does not contain a valid ",
		       eltclLibrary[1],
		       " file.\nPlease check your installation.\n",
		       NULL);
      Tcl_DStringFree(&initFile);
      return TCL_ERROR;
   }
   Tcl_DStringFree(&initFile);

   return TCL_OK;
}


/*
 * elTclPrompt ----------------------------------------------------------
 *
 * Compute prompt, base on current context
 */

static const char *
elTclPrompt(EditLine *el)
{
   ElTclInterpInfo *iinfo;
   Tcl_Obj *promptCmdPtr;
   static char buf[32];
   const char *prompt;
   int code;

   /* get context */
   el_get(el, EL_CLIENTDATA, &iinfo);

   /* see if a static prompt is defined */
   if (iinfo->promptString != NULL) {
      return Tcl_GetStringFromObj(iinfo->promptString, NULL);
   }

   /* compute prompt */
   promptCmdPtr =
      Tcl_ObjGetVar2(iinfo->interp,
		     (iinfo->gotPartial?
		      iinfo->prompt2Name : iinfo->prompt1Name),
		     (Tcl_Obj *) NULL, TCL_GLOBAL_ONLY);
   if (promptCmdPtr == NULL) {
     defaultPrompt:
      if (!iinfo->gotPartial) {
	 strlcpy(buf, iinfo->argv0, sizeof(buf)-4);
	 strlcat(buf, " > ", sizeof(buf));
	 prompt = buf;
      } else
	 prompt = "» ";

   } else {
      code = Tcl_EvalObj(iinfo->interp, promptCmdPtr);

      if (code != TCL_OK) {
	 Tcl_Channel inChannel, outChannel, errChannel;

	 inChannel = Tcl_GetStdChannel(TCL_STDIN);
	 outChannel = Tcl_GetStdChannel(TCL_STDOUT);
	 errChannel = Tcl_GetStdChannel(TCL_STDERR);
	 if (errChannel) {
#if TCL_MAJOR_VERSION >= 8 && TCL_MINOR_VERSION >= 4
	    const char *bytes;
#else
	    char *bytes;
#endif /* TCL_VERSION */
	    bytes = Tcl_GetStringResult(iinfo->interp);
	    Tcl_Write(errChannel, bytes, strlen(bytes));
	    Tcl_Write(errChannel, "\n", 1);
	 }
	 Tcl_AddErrorInfo(iinfo->interp,
			  "\n    (script that generates prompt)");
	 goto defaultPrompt;
      }

      prompt = Tcl_GetStringResult(iinfo->interp);
   }

   return prompt;
}


/*
 * elTclInteractive --------------------------------------------------
 *
 * Process commands on stdin until end-of-file
 */

int
elTclInteractive(ClientData data,
		 Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
  ElTclInterpInfo *iinfo = data;
  HistEvent ev;
  Tcl_Obj *resultPtr, *command;
  Tcl_Channel inChannel, outChannel, errChannel;
  int code, tty, length;
  char *bytes;

  tty = isatty(0);

  /* Print the copyright message in interactive mode */
  if (tty) {
    length = (iinfo->windowSize -
	      (int)(strlen(version)+
		    strlen(copyright)+strlen(iinfo->argv0)))*3/4;
    if (length >= 0) {
      fputc('\n', stdout);
      for(;length;length--) fputc(' ', stdout);
      fputs(iinfo->argv0, stdout);
      fputs(version, stdout);
      fputs(copyright, stdout);
      fputs("\n\n", stdout);
    }
  }

  /*
   * Process commands from stdin until there's an end-of-file. Note
   * that we need to fetch the standard channels again after every
   * eval, since they may have been changed.
   */

  iinfo->command = Tcl_NewObj();
  Tcl_IncrRefCount(iinfo->command);

  inChannel = Tcl_GetStdChannel(TCL_STDIN);
  outChannel = Tcl_GetStdChannel(TCL_STDOUT);
  iinfo->gotPartial = 0;

  for(;/*eternity*/;)  {
    if (tty) {
      const char *line;

      line = el_gets(iinfo->el, &length);
      if (line == NULL) goto done;

      command = Tcl_NewStringObj(line, length);
      Tcl_AppendObjToObj(iinfo->command, command);

    } else {
      /* using libedit but not a tty - must use gets */
      if (!inChannel) goto done;

      length = Tcl_GetsObj(inChannel, iinfo->command);
      if (length < 0) goto done;
      if ((length == 0) &&
	  Tcl_Eof(inChannel) && (!iinfo->gotPartial)) goto done;

      /* Add the newline back to the string */
      Tcl_AppendToObj(iinfo->command, "\n", 1);
    }

    if (!Tcl_CommandComplete(
	  Tcl_GetStringFromObj(iinfo->command, &length))) {
      iinfo->gotPartial = 1; continue;
    }

    if (tty && length > 1) {
      /*  add the command line to history */
      history(iinfo->history, &ev, H_ENTER,
	      Tcl_GetStringFromObj(iinfo->command, NULL));
    }

    /* tricky: if the command calls el::get[sc], the completion engine
     * will think that iinfo->command is the beginning of an incomplete
     * command. Thus we must reset it before the Tcl_Eval call... */
    command = iinfo->command;

    iinfo->command = Tcl_NewObj();
    Tcl_IncrRefCount(iinfo->command);

    iinfo->gotPartial = 0;
#if TCL_MAJOR_VERSION >= 8 && TCL_MINOR_VERSION >= 4
    code = Tcl_RecordAndEvalObj(iinfo->interp, command, 0);
#else
    code = Tcl_EvalObj(iinfo->interp, command);
#endif /* TCL_VERSION */

    Tcl_DecrRefCount(command);

    inChannel = Tcl_GetStdChannel(TCL_STDIN);
    outChannel = Tcl_GetStdChannel(TCL_STDOUT);
    errChannel = Tcl_GetStdChannel(TCL_STDERR);
    if (code != TCL_OK) {
      if (errChannel) {
	resultPtr = Tcl_GetObjResult(iinfo->interp);
	bytes = Tcl_GetStringFromObj(resultPtr, &length);
	Tcl_Write(errChannel, bytes, length);
	Tcl_Write(errChannel, "\n", 1);
      }
    } else if (tty) {
      resultPtr = Tcl_GetObjResult(iinfo->interp);
      bytes = Tcl_GetStringFromObj(resultPtr, &length);

      if ((length > 0) && outChannel) {
	Tcl_Write(outChannel, bytes, length);
	Tcl_Write(outChannel, "\n", 1);
      }
    }
  }

done:
  /* the command does not return anything */
  Tcl_Write(outChannel, "\n", 1);
  Tcl_ResetResult(iinfo->interp);
  return TCL_OK;
}


/*
 * elTclExit ------------------------------------------------------------
 *
 * Destroy global info structure
 */

int
elTclExit(ClientData data,
	  Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
   ElTclInterpInfo *iinfo = data;
   HistEvent ev;
   int value;

   if ((objc != 1) && (objc != 2)) {
      Tcl_WrongNumArgs(interp, 1, objv, "?returnCode?");
      return TCL_ERROR;
   }

   if (objc == 1) {
      value = 0;
   } else if (Tcl_GetIntFromObj(interp, objv[1], &value) != TCL_OK) {
      return TCL_ERROR;
   }

   el_end(iinfo->el);

   if (iinfo->histFile && iinfo->histFile[0])
     history(iinfo->history, &ev, H_SAVE, iinfo->histFile);
   history_end(iinfo->history);
   history_end(iinfo->askaHistory);

   elTclHandlersExit(iinfo);
   Tcl_DecrRefCount(iinfo->prompt1Name);
   Tcl_DecrRefCount(iinfo->prompt2Name);
   Tcl_DecrRefCount(iinfo->matchesName);
   free(iinfo);

   fputs("\n", stdout);
   Tcl_Exit(value);
   return TCL_OK;
}
