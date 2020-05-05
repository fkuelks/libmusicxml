/*
  MusicXML Library
  Copyright (C) Grame 2006-2013

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Grame Research Laboratory, 11, cours de Verdun Gensoul 69002 Lyon - France
  research@grame.fr
*/

#include <iomanip>      // setw, setprecision, ...

#include <regex>

#include "version.h"
#include "utilities.h"

#include "setTraceOahIfDesired.h"
#ifdef TRACE_OAH
  #include "traceOah.h"
#endif

#include "messagesHandling.h"

#include "generalOah.h"
#include "msr2BsrOah.h"


using namespace std;

namespace MusicXML2
{
//_______________________________________________________________________________

S_msr2BsrOah gMsr2BsrOah;
S_msr2BsrOah gMsr2BsrOahUserChoices;
S_msr2BsrOah gMsr2BsrOahWithDetailedTrace;

S_msr2BsrOah msr2BsrOah::create (
  S_oahHandler handlerUpLink)
{
  msr2BsrOah* o = new msr2BsrOah(
    handlerUpLink);
  assert(o!=0);
  return o;
}

msr2BsrOah::msr2BsrOah (
  S_oahHandler handlerUpLink)
  : oahGroup (
    "Msr2Bsr",
    "hmsr2bsr", "help-msr-to-bsr",
R"(These options control the way MSR data is translated to BSR.)",
    kElementVisibilityAlways,
    handlerUpLink)
{
/* JMI
  // sanity check
  msrAssert (
    handlerUpLink != nullptr,
    "handlerUpLink is null");
*/

  // append this options group to the options handler
  // if relevant
  if (handlerUpLink) {
    handlerUpLink->
      appendGroupToHandler (this);
  }

  // initialize it
  initializeMsr2BsrOah (false);
}

msr2BsrOah::~msr2BsrOah ()
{}

#ifdef TRACE_OAH
void msr2BsrOah::initializeMsr2BsrTraceOah (
  bool boolOptionsInitialValue)
{}
#endif

void msr2BsrOah::initializeMsr2BsrMiscellaneousOptions (
  bool boolOptionsInitialValue)
{
  S_oahSubGroup
    subGroup =
      oahSubGroup::create (
        "Miscellaneous",
        "hlpm", "help-miscellaneous",
R"()",
      kElementVisibilityAlways,
      this);

  appendSubGroupToGroup (subGroup);

  // braille music headings

  fNoBrailleMusicHeadings = boolOptionsInitialValue;

  subGroup->
    appendAtomToSubGroup (
      oahBooleanAtom::create (
        "nmh", "no-music-headings",
R"(Don't generate any music headings in the Braille code.)",
        "noBrailleMusicHeadings",
        fNoBrailleMusicHeadings));

  // braille tempos

  fNoTempos = boolOptionsInitialValue;

  subGroup->
    appendAtomToSubGroup (
      oahBooleanAtom::create (
        "nt", "no-tempos",
R"(Don't generate any tempos in the Braille code.)",
        "noTempos",
        fNoTempos));

  // braille page numbers

  fNoPageNumbers = boolOptionsInitialValue;

  subGroup->
    appendAtomToSubGroup (
      oahBooleanAtom::create (
        "npn", "no-page-numbers",
R"(Don't generate any page numbers in the Braille code.)",
        "noBraillePageNumbers",
        fNoPageNumbers));

  // braille line numbers

  fNoLineNumbers = boolOptionsInitialValue;

  subGroup->
    appendAtomToSubGroup (
      oahBooleanAtom::create (
        "nln", "no-line-numbers",
R"(Don't generate any line numbers in the Braille code.)",
        "noBrailleLineNumbers",
        fNoLineNumbers));

  // braille measure numbers

  fNoMeasureNumbers = boolOptionsInitialValue;

  subGroup->
    appendAtomToSubGroup (
      oahBooleanAtom::create (
        "nmn", "no-measure-numbers",
R"(Don't generate any measure numbers in the Braille code.)",
        "noMeasureNumbers",
        fNoMeasureNumbers));

  // braille lyrics

  fNoBrailleLyrics = boolOptionsInitialValue;

  subGroup->
    appendAtomToSubGroup (
      oahBooleanAtom::create (
        "nl", "no-lyrics",
R"(Don't generate any lyrics in the Braille code.)",
        "noBrailleLyrics",
        fNoBrailleLyrics));

  // braille compile date

  fBrailleCompileDate = boolOptionsInitialValue;

  subGroup->
    appendAtomToSubGroup (
      oahBooleanAtom::create (
        "lpcd", "braille-compile-date",
R"(Generate code to include the compilation date
when Braille creates the score.)",
        "brailleCompileDate",
        fBrailleCompileDate));

  // facsimile

  fFacSimileKind = kFacSimileNo;

  subGroup->
    appendAtomToSubGroup (
      bsrFacSimileKindAtom::create (
        "fs", "facsimile",
R"(Generate facsimile Braille nusic code.
By default, non-facsimile code is generated.)",
        "YES_OR_NO",
        "facSimileKind",
        fFacSimileKind));

  // clefs

  fIncludeClefs = boolOptionsInitialValue;

  subGroup->
    appendAtomToSubGroup (
      oahBooleanAtom::create (
        "clefs", "",
R"(Include clefs in BSR. By default, they are not.)",
        "includeClefs",
        fIncludeClefs));
}

void msr2BsrOah::initializeMsr2BsrOah (
  bool boolOptionsInitialValue)
{
#ifdef TRACE_OAH
  // trace
  // --------------------------------------
  initializeMsr2BsrTraceOah (
    boolOptionsInitialValue);
#endif

  // miscellaneous
  // --------------------------------------
  initializeMsr2BsrMiscellaneousOptions (
    boolOptionsInitialValue);
}

S_msr2BsrOah msr2BsrOah::createCloneWithDetailedTrace ()
{
  S_msr2BsrOah
    clone =
      msr2BsrOah::create (0);
      // 0 not to have it inserted twice in the option handler

  // set the options handler upLink
  clone->fHandlerUpLink =
    fHandlerUpLink;

  return clone;
}

//______________________________________________________________________________
void msr2BsrOah::setAllMusicXMLTraceOah (
  bool boolOptionsInitialValue)
{
#ifdef TRACE_OAH
  // specific trace
  // JMI
#endif
}

//______________________________________________________________________________
void msr2BsrOah::enforceQuietness ()
{}

//______________________________________________________________________________
void msr2BsrOah::checkOptionsConsistency ()
{}

//______________________________________________________________________________
void msr2BsrOah::acceptIn (basevisitor* v)
{
#ifdef TRACE_OAH
  if (gOahOah->fTraceOahVisitors) {
    gLogOstream <<
      ".\\\" ==> msr2BsrOah::acceptIn ()" <<
      endl;
  }
#endif

  if (visitor<S_msr2BsrOah>*
    p =
      dynamic_cast<visitor<S_msr2BsrOah>*> (v)) {
        S_msr2BsrOah elem = this;

#ifdef TRACE_OAH
        if (gOahOah->fTraceOahVisitors) {
          gLogOstream <<
            ".\\\" ==> Launching msr2BsrOah::visitStart ()" <<
            endl;
        }
#endif
        p->visitStart (elem);
  }
}

void msr2BsrOah::acceptOut (basevisitor* v)
{
#ifdef TRACE_OAH
  if (gOahOah->fTraceOahVisitors) {
    gLogOstream <<
      ".\\\" ==> msr2BsrOah::acceptOut ()" <<
      endl;
  }
#endif

  if (visitor<S_msr2BsrOah>*
    p =
      dynamic_cast<visitor<S_msr2BsrOah>*> (v)) {
        S_msr2BsrOah elem = this;

#ifdef TRACE_OAH
        if (gOahOah->fTraceOahVisitors) {
          gLogOstream <<
            ".\\\" ==> Launching msr2BsrOah::visitEnd ()" <<
            endl;
        }
#endif
        p->visitEnd (elem);
  }
}

void msr2BsrOah::browseData (basevisitor* v)
{
#ifdef TRACE_OAH
  if (gOahOah->fTraceOahVisitors) {
    gLogOstream <<
      ".\\\" ==> msr2BsrOah::browseData ()" <<
      endl;
  }
#endif
}

void msr2BsrOah::printMsr2BsrOahValues (int fieldWidth)
{
  gLogOstream <<
    "The MusicXML options are:" <<
    endl;

  gIndenter++;

  // miscellaneous
  // --------------------------------------

  gLogOstream <<
    "Miscellaneous:" <<
    endl;

  gIndenter++;

  gLogOstream << left <<
    setw (fieldWidth) << "noBrailleMusicHeadings" << " : " <<
      booleanAsString (fNoBrailleMusicHeadings) <<
      endl <<

    setw (fieldWidth) << "noTempos" << " : " <<
      booleanAsString (fNoTempos) <<
      endl <<

    setw (fieldWidth) << "noBraillePageNumbers" << " : " <<
      booleanAsString (fNoPageNumbers) <<
      endl <<
    setw (fieldWidth) << "noBrailleLineNumbers" << " : " <<
      booleanAsString (fNoLineNumbers) <<
      endl <<
    setw (fieldWidth) << "noMeasureNumbers" << " : " <<
      booleanAsString (fNoMeasureNumbers) <<
      endl <<

    setw (fieldWidth) << "noBrailleLyrics" << " : " <<
      booleanAsString (fNoBrailleLyrics) <<
      endl <<

    setw (fieldWidth) << "brailleCompileDate" << " : " <<
      booleanAsString (fBrailleCompileDate) <<
      endl <<

    setw (fieldWidth) << "facSimileKind" << " : " <<
      booleanAsString (fFacSimileKind) <<
      endl <<

    setw (fieldWidth) << "includeClefs" << " : " <<
      booleanAsString (fIncludeClefs) <<
      endl;

  gIndenter--;

  gIndenter--;
}

ostream& operator<< (ostream& os, const S_msr2BsrOah& elt)
{
  elt->print (os);
  return os;
}

//______________________________________________________________________________
void initializeMsr2BsrOahHandling (
  S_oahHandler handler)
{
#ifdef TRACE_OAH
  if (gTraceOah->fTraceOah && ! gGeneralOah->fQuiet) {
    gLogOstream <<
      "Initializing MusicXML options handling" <<
      endl;
  }
#endif

  // create the MusicXML options
  // ------------------------------------------------------

  gMsr2BsrOahUserChoices = msr2BsrOah::create (
    handler);
  assert(gMsr2BsrOahUserChoices != 0);

  gMsr2BsrOah =
    gMsr2BsrOahUserChoices;

  // prepare for measure detailed trace
  // ------------------------------------------------------

/* JMI
  gMsr2BsrOahWithDetailedTrace =
    gMsr2BsrOah->
      createCloneWithDetailedTrace ();
      */
}


}
