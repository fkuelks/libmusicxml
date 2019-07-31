/*
  MusicXML Library
  Copyright (C) Grame 2006-2013

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Grame Research Laboratory, 11, cours de Verdun Gensoul 69002 Lyon - France
  research@grame.fr
*/

#include <sstream>

#include <climits>      // INT_MIN, INT_MAX
#include <iomanip>      // setw, setprecision, ...

#include <regex>

#include "utilities.h"

#include "oahBasicTypes.h"
#include "executableOah.h"

#include "messagesHandling.h"

#include "setTraceOahIfDesired.h"
#ifdef TRACE_OAH
  #include "traceOah.h"
#endif


namespace MusicXML2
{

/*
Basics:
  - oah (Options And Help) is supposed to be pronouced something close to "whaaaah!"
    The intonation is left to the speaker, though...
    And as the saying goes: "OAH? oahy not!"

  - options handling is organized as a hierarchical, instrospective set of classes.
    Options and their corresponding help are grouped in a single object.

  - oahOption is the super-class of all options types, including groups and subgroups.
    It contains a short name and a long name, as well as a decription.
    Short and long names can be used and mixed at will in the command line,
    as well as '-' and '--'.
    The short name is mandatory, but the long name may be empty.

  - prefixes such '-t=' and -help=' allow for a contracted form of options.
    For example, -t=meas,notes is short for '-t-meas, -tnotes'.
    An oahPrefix contains the prefix name, the ersatz by which to replace it,
    and a description.

  - an oahHandler contains oahGroup's, each handled in a pair or .h/.cpp files,
    such as msrOah.h and msrOah.cpp, and a list of options prefixes.

  - an oahGroup contains oahSubGroup's and an upLink to the containing oahHandler.

  - an oahSubGroup contains oahAtom's and an upLink to the containing oahGroup.

  - each oahAtom is an atomic option to the executable and its corresponding help,
    and an upLink to the containing oahSubGroup.

Features:
  - partial help to be obtained, i.e. help about any group, subgroup or atom,
    showing the path in the hierarchy down to the corresponding option.

  - there are various subclasses of oahAtom such as oahIntegerAtom, oahBooleanAtom
    and oahRationalAtom to control options values of common types.

  - oahThreeBooleansAtom, for example, allows for three boolean settings
    to be controlled at once with a single option.

  - oahValuedAtom describes options for which a value is supplied in the command line.

  - a class such as optionsLpsrPitchesLanguageOption is used
    to supply a string value to be converted into an internal enumerated type.

  - oahCombinedBooleansAtom contains a list of atoms to manipulate several atoms as a single one,
    see the 'cubase' combined booleans atom in musicXMLOah.cpp.

  - storing options and the corresponding help in oahGroup's makes it easy to re-use them.
    For example, xml2ly and xml2lbr have their three first passes in common,
    (up to obtaining the MSR description of the score), as well as the corresponding options and help.

Handling:
  - each optionOption must have unique short and long names, for consistency.

  - an executable main() call decipherOptionsAndArguments(), in which:
    - handleOptionName() handles the option names
    - handleOptionValueOrArgument() handle the values that may follow an atom name
      and the arguments to the executable.

  - contracted forms are expanded in handleOptionName() before the resulting,
    uncontracted options are handled.

  - handleOptionName() fetches the oahOption corresponding to the name from the map,
    determines the type of the latter,
    and delegates the handling to the corresponding object.

  - handleOptionValueOrArgument() associatiates the value
    to the (preceding) fPendingValuedAtom if not null,
    or appends it fHandlerArgumentsVector to otherwise.

  - the printOptionsSummary() methods are used when there are errors in the options used.

  - the printHelp() methods perform the actual help print work

  - options deciphering it done by the handleAtom() methods defined:
      - in oahBasicTypes.h/.cpp for the predefined ones;
      - in the various options groups for those specific to the latter.

  - the value following the option name, if any, is taken care of
    by the handle*AtomValue() methods, using fPendingValuedAtom
    to hold the valuedAtom until the corresponding value is found.
*/

//______________________________________________________________________________
string optionVisibilityKindAsString (
  oahOptionVisibilityKind optionVisibilityKind)
{
  string result;

  switch (optionVisibilityKind) {
    case kElementVisibilityAlways:
      result = "elementVisibilityAlways";
      break;

    case kElementVisibilityHiddenByDefault:
      result = "elementVisibilityHiddenByDefault";
      break;
  } // switch

  return result;
}

//______________________________________________________________________________
S_oahOption oahOption::create (
  string                  shortName,
  string                  longName,
  string                  description,
  oahOptionVisibilityKind optionVisibilityKind)
{
  oahOption* o = new
    oahOption (
      shortName,
      longName,
      description,
      optionVisibilityKind);
  assert(o!=0);
  return o;
}

oahOption::oahOption (
  string                  shortName,
  string                  longName,
  string                  description,
  oahOptionVisibilityKind optionVisibilityKind)
{
  fShortName   = shortName;
  fLongName    = longName;
  fDescription = description;

  fOptionVisibilityKind = optionVisibilityKind;

  fIsHidden    = false;
}

oahOption::~oahOption ()
{}

S_oahOption oahOption::fetchOptionByName (
  string name)
{
  S_oahOption result;

  if (
    name == fShortName
     ||
    name == fLongName) {
    result = this;
  }

  return result;
}

string oahOption::fetchNames () const
{
  stringstream s;

  if (
    fShortName.size ()
        &&
    fLongName.size ()
  ) {
      s <<
        "-" << fShortName <<
        ", " <<
        "-" << fLongName;
  }

  else {
    if (fShortName.size ()) {
      s <<
      "-" << fShortName;
    }
    if (fLongName.size ()) {
      s <<
      "-" << fLongName;
    }
  }

  return s.str ();
}

string oahOption::fetchNamesInColumns (
  int subGroupsShortNameFieldWidth) const
{
  stringstream s;

  if (
    fShortName.size ()
        &&
    fLongName.size ()
    ) {
      s << left <<
        setw (subGroupsShortNameFieldWidth) <<
        "-" + fShortName <<
        ", " <<
        "-" << fLongName;
  }

  else {
    if (fShortName.size ()) {
      s << left <<
        setw (subGroupsShortNameFieldWidth) <<
          "-" + fShortName;
    }
    if (fLongName.size ()) {
      s <<
        "-" << fLongName;
    }
  }

  return s.str ();
}

string oahOption::fetchNamesBetweenParentheses () const
{
  stringstream s;

  s <<
    "(" <<
    fetchNames () <<
    ")";

  return s.str ();
}

string oahOption::fetchNamesInColumnsBetweenParentheses (
  int subGroupsShortNameFieldWidth) const
{
  stringstream s;

  s <<
    "(" <<
    fetchNamesInColumns (
      subGroupsShortNameFieldWidth) <<
    ")";

  return s.str ();
}

S_oahValuedAtom oahOption::handleOptionUnderName (
  string   optionName,
  ostream& os)
{
  stringstream s;

  s <<
    "### atom option name " << optionName <<
    " attached to '" <<
    this->asString () <<
    "' is not handled";

  msrInternalError (
    gExecutableOah->fInputSourceName,
    K_NO_INPUT_LINE_NUMBER,
    __FILE__, __LINE__,
    s.str ());

  // no option value is needed
  return nullptr;
}

string oahOption::asShortNamedOptionString () const
{
  return "-" + fShortName;
}

string oahOption::asLongNamedOptionString () const
{
  return "-" + fLongName;
}

string oahOption::asString () const
{
  stringstream s;

  s <<
    "'-" << fLongName << "'"; // JMI

  return s.str ();
}

void oahOption::printOptionHeader (ostream& os) const
{
  os <<
    "-" << fShortName <<
    endl <<
    "-" << fLongName <<
    endl;

  if (fDescription.size ()) {
    // indent a bit more for readability
    gIndenter.increment (K_OPTIONS_ELEMENTS_INDENTER_OFFSET);

    os <<
      gIndenter.indentMultiLineString (
        fDescription) <<
      endl;

    gIndenter.decrement (K_OPTIONS_ELEMENTS_INDENTER_OFFSET);
  }
}

void oahOption::printOptionEssentials (
  ostream& os,
  int      fieldWidth) const
{
  os << left <<
    setw (fieldWidth) <<
    "fShortName" << " : " <<
    fShortName <<
    endl <<
    setw (fieldWidth) <<
    "fLongName" << " : " <<
    fLongName <<
    endl <<
    setw (fieldWidth) <<
    "fDescription" << " : " <<
    fDescription <<
    endl <<
    setw (fieldWidth) <<
    "fIsHidden" << " : " <<
    booleanAsString (
      fIsHidden) <<
    endl;
}

void oahOption::print (ostream& os) const
{
  os <<
    "??? oahOption ???" <<
    endl;

  printOptionEssentials (os, 40); // JMI
}

void oahOption::printHelp (ostream& os)
{
  os <<
    fetchNames () <<
    endl;

  if (fDescription.size ()) {
    // indent a bit more for readability
    gIndenter.increment (K_OPTIONS_ELEMENTS_INDENTER_OFFSET);

    os <<
      gIndenter.indentMultiLineString (
        fDescription) <<
      endl;

    gIndenter.decrement (K_OPTIONS_ELEMENTS_INDENTER_OFFSET);
  }

  // register help print action in options handler upLink JMI ???
//  fHandlerUpLink->setOptionsHandlerFoundAHelpOption ();
}

ostream& operator<< (ostream& os, const S_oahOption& elt)
{
  elt->print (os);
  return os;
}

//______________________________________________________________________________
S_oahAtom oahAtom::create (
  string shortName,
  string longName,
  string description)
{
  oahAtom* o = new
    oahAtom (
      shortName,
      longName,
      description);
  assert(o!=0);
  return o;
}

oahAtom::oahAtom (
  string shortName,
  string longName,
  string description)
  : oahOption (
      shortName,
      longName,
      description,
      kElementVisibilityAlways)
{}

oahAtom::~oahAtom ()
{}

void oahAtom::setSubGroupUpLink (
  S_oahSubGroup subGroup)
{
  // sanity check
  msrAssert (
    subGroup != nullptr,
    "subGroup is null");

  // set the upLink
  fSubGroupUpLink = subGroup;
}

void oahAtom::registerAtomInHandler (
  S_oahHandler handler)
{
  handler->
    registerOptionInHandler (this);

  fHandlerUpLink = handler;
}

void oahAtom::print (ostream& os) const
{
  const int fieldWidth = 19;

  os <<
    "Atom ???:" <<
      endl;

  gIndenter++;

  oahOption::printOptionEssentials (
    os, fieldWidth);

  gIndenter--;
}

void oahAtom::printAtomOptionsValues (
  ostream& os,
  int      valueFieldWidth) const
{
  os <<
    "Atom values ???:" <<
    endl;
}

ostream& operator<< (ostream& os, const S_oahAtom& elt)
{
  elt->print (os);
  return os;
}

//______________________________________________________________________________
S_oahAtomSynonym oahAtomSynonym::create (
  string    shortName,
  string    longName,
  string    description,
  S_oahAtom originalOahAtom)
{
  oahAtomSynonym* o = new
    oahAtomSynonym (
      shortName,
      longName,
      description,
      originalOahAtom);
  assert(o!=0);
  return o;
}

oahAtomSynonym::oahAtomSynonym (
  string    shortName,
  string    longName,
  string    description,
  S_oahAtom originalOahAtom)
  : oahAtom (
      shortName,
      longName,
      description)
{
  // sanity check
  msrAssert (
    originalOahAtom != nullptr,
    "originalOahAtom is null");

  fOriginalOahAtom = originalOahAtom;
}

oahAtomSynonym::~oahAtomSynonym ()
{}

void oahAtomSynonym::print (ostream& os) const
{
  const int fieldWidth = K_OPTIONS_FIELD_WIDTH;

  os <<
    "AtomSynonym:" <<
    endl;

  gIndenter++;

  oahOption::printOptionEssentials (
    os, fieldWidth);

  gIndenter++;
  os <<
    gIndenter.indentMultiLineString (
      fDescription) <<
    endl;
  gIndenter--;

  gIndenter--;
}

void oahAtomSynonym::printAtomOptionsValues (
  ostream& os,
  int      valueFieldWidth) const
{
  // nothing to print here
}

ostream& operator<< (ostream& os, const S_oahAtomSynonym& elt)
{
  elt->print (os);
  return os;
}

//______________________________________________________________________________
S_oahOptionsUsageAtom oahOptionsUsageAtom::create (
  string shortName,
  string longName,
  string description)
{
  oahOptionsUsageAtom* o = new
    oahOptionsUsageAtom (
      shortName,
      longName,
      description);
  assert(o!=0);
  return o;
}

oahOptionsUsageAtom::oahOptionsUsageAtom (
  string shortName,
  string longName,
  string description)
  : oahAtom (
      shortName,
      longName,
      description)
{}

oahOptionsUsageAtom::~oahOptionsUsageAtom ()
{}

S_oahValuedAtom oahOptionsUsageAtom::handleOptionUnderName (
  string   optionName,
  ostream& os)
{
  // handle it at once
  os <<
    endl <<
    "Options usage" <<
    endl <<
    "-------------" <<
    endl <<
    endl;

  gIndenter++;

  os <<
    gIndenter.indentMultiLineString (
R"(As an argument, '-' represents standard input.

A number of options exist to fine tune the generated LilyPond code
and limit the need for manually editing the latter.
Most options have a short and a long name for commodity.

The options are organized in a group-subgroup-atom hierarchy.
Help can be obtained for groups or subgroups at will,
as well as for any option with the '-onh, option-name-help' option.

A subgroup can be hidden by default, in which case its description is printed
only when the corresponding short or long names are used.

Both '-' and '--' can be used to introduce options in the command line,
even though the help facility only shows them with '-'.

Command line options and arguments can be placed in any order,
provided atom values immediately follow the corresponding atoms.)") <<
    endl <<
    endl;

  gIndenter--;

  // no option value is needed
  return nullptr;
}

void oahOptionsUsageAtom::print (ostream& os) const
{
  const int fieldWidth = K_OPTIONS_FIELD_WIDTH;

  os <<
    "OptionsUsageAtom:" <<
    endl;

  gIndenter++;

  oahOption::printOptionEssentials (
    os, fieldWidth);

  gIndenter++;
  os <<
    gIndenter.indentMultiLineString (
      fDescription) <<
    endl;
  gIndenter--;

  gIndenter--;
}

/*
void oahOptionsUsageAtom::printOptionsUsage (ostream& os) const
{
  os <<
    endl <<
    "Options usage" <<
    endl <<
    "-------------" <<
    endl <<
    endl;

  gIndenter++;

  os <<
    gIndenter.indentMultiLineString (
R"(As an argument, '-' represents standard input.

A number of options exist to fine tune the generated LilyPond code
and limit the need for manually editing the latter.
Most options have a short and a long name for commodity.

The options are organized in a group-subgroup-atom hierarchy.
Help can be obtained for groups or subgroups at will,
as well as for any option with the '-onh, option-name-help' option.

A subgroup can be hidden by default, in which case its description is printed
only when the corresponding short or long names are used.

Both '-' and '--' can be used to introduce options in the command line,
even though the help facility only shows them with '-'.

Command line options and arguments can be placed in any order,
provided atom values immediately follow the corresponding atoms.)") <<
    endl <<
    endl;

  gIndenter--;
}
*/

void oahOptionsUsageAtom::printAtomOptionsValues (
  ostream& os,
  int      valueFieldWidth) const
{
  // nothing to print here
}

ostream& operator<< (ostream& os, const S_oahOptionsUsageAtom& elt)
{
  elt->print (os);
  return os;
}

//______________________________________________________________________________
S_oahOptionsSummaryAtom oahOptionsSummaryAtom::create (
  string shortName,
  string longName,
  string description)
{
  oahOptionsSummaryAtom* o = new
    oahOptionsSummaryAtom (
      shortName,
      longName,
      description);
  assert(o!=0);
  return o;
}

oahOptionsSummaryAtom::oahOptionsSummaryAtom (
  string shortName,
  string longName,
  string description)
  : oahAtom (
      shortName,
      longName,
      description)
{}

oahOptionsSummaryAtom::~oahOptionsSummaryAtom ()
{}

S_oahValuedAtom oahOptionsSummaryAtom::handleOptionUnderName (
  string   optionName,
  ostream& os)
{
  print (os); //JMI ???

  // no option value is needed
  return nullptr;
}

void oahOptionsSummaryAtom::print (ostream& os) const
{
  const int fieldWidth = K_OPTIONS_FIELD_WIDTH;

  os <<
    "OptionsSummaryAtom:" <<
    endl;

  gIndenter++;

  oahOption::printOptionEssentials (
    os, fieldWidth);

  gIndenter++;
  os <<
    gIndenter.indentMultiLineString (
      fDescription) <<
    endl;
  gIndenter--;

  gIndenter--;
}

void oahOptionsSummaryAtom::printOptionsSummary (ostream& os) const
{
  os <<
    gExecutableOah->fHandlerExecutableName <<
    endl;
}

void oahOptionsSummaryAtom::printAtomOptionsValues (
  ostream& os,
  int      valueFieldWidth) const
{
  // nothing to print here
}

ostream& operator<< (ostream& os, const S_oahOptionsSummaryAtom& elt)
{
  elt->print (os);
  return os;
}

//______________________________________________________________________________
S_oahAtomWithVariableName oahAtomWithVariableName::create (
  string shortName,
  string longName,
  string description,
  string variableName)
{
  oahAtomWithVariableName* o = new
    oahAtomWithVariableName (
      shortName,
      longName,
      description,
      variableName);
  assert(o!=0);
  return o;
}

oahAtomWithVariableName::oahAtomWithVariableName (
  string shortName,
  string longName,
  string description,
  string variableName)
  : oahAtom (
      shortName,
      longName,
      description),
    fVariableName (
      variableName)
{}

oahAtomWithVariableName::~oahAtomWithVariableName ()
{}

void oahAtomWithVariableName::print (ostream& os) const
{
  const int fieldWidth = K_OPTIONS_FIELD_WIDTH;

  os <<
    "AtomWithVariableName:" <<
    endl;

  gIndenter++;

  oahOption::printOptionEssentials (
    os, fieldWidth);

  gIndenter++;
  os <<
    gIndenter.indentMultiLineString (
      fDescription) <<
    endl;
  gIndenter--;

  os << left <<
    setw (fieldWidth) <<
    "fVariableName" << " : " <<
    fVariableName <<
    endl <<
    endl;

  gIndenter--;
}

void oahAtomWithVariableName::printAtomOptionsValues (
  ostream& os,
  int      valueFieldWidth) const
{
  os << left <<
    setw (valueFieldWidth) <<
    fVariableName <<
    " : " <<
    "FOO" <<
    endl;
}

ostream& operator<< (ostream& os, const S_oahAtomWithVariableName& elt)
{
  elt->print (os);
  return os;
}

//______________________________________________________________________________
S_oahBooleanAtom oahBooleanAtom::create (
  string shortName,
  string longName,
  string description,
  string variableName,
  bool&  booleanVariable)
{
  oahBooleanAtom* o = new
    oahBooleanAtom (
      shortName,
      longName,
      description,
      variableName,
      booleanVariable);
  assert(o!=0);
  return o;
}

oahBooleanAtom::oahBooleanAtom (
  string shortName,
  string longName,
  string description,
  string variableName,
  bool&  booleanVariable)
  : oahAtomWithVariableName (
      shortName,
      longName,
      description,
      variableName),
    fBooleanVariable (
      booleanVariable)
{}

oahBooleanAtom::~oahBooleanAtom ()
{}

S_oahValuedAtom oahBooleanAtom::handleOptionUnderName (
  string   optionName,
  ostream& os)
{
  // handle it at once
  setBooleanVariable (true);

  // no option value is needed
  return nullptr;
}

void oahBooleanAtom::print (ostream& os) const
{
  const int fieldWidth = K_OPTIONS_FIELD_WIDTH;

  os <<
    "BooleanAtom:" <<
    endl;

  gIndenter++;

  oahOption::printOptionEssentials (
    os, fieldWidth);

  gIndenter++;
  os <<
    gIndenter.indentMultiLineString (
      fDescription) <<
    endl;
  gIndenter--;

  os << left <<
    setw (fieldWidth) <<
    "fVariableName" << " : " <<
    fVariableName <<
    endl <<
    setw (fieldWidth) <<
    "fBooleanVariable" << " : " <<
    booleanAsString (
      fBooleanVariable) <<
    endl;

  gIndenter--;
}

void oahBooleanAtom::printAtomOptionsValues (
  ostream& os,
  int      valueFieldWidth) const
{
  os << left <<
    setw (valueFieldWidth) <<
    fVariableName <<
    " : " <<
    booleanAsString (
      fBooleanVariable) <<
    endl;
}

ostream& operator<< (ostream& os, const S_oahBooleanAtom& elt)
{
  elt->print (os);
  return os;
}

//______________________________________________________________________________
S_oahTwoBooleansAtom oahTwoBooleansAtom::create (
  string shortName,
  string longName,
  string description,
  string variableName,
  bool&  booleanVariable,
  bool&  booleanSecondaryVariable)
{
  oahTwoBooleansAtom* o = new
    oahTwoBooleansAtom (
      shortName,
      longName,
      description,
      variableName,
      booleanVariable,
      booleanSecondaryVariable);
  assert(o!=0);
  return o;
}

oahTwoBooleansAtom::oahTwoBooleansAtom (
  string shortName,
  string longName,
  string description,
  string variableName,
  bool&  booleanVariable,
  bool&  booleanSecondaryVariable)
  : oahAtomWithVariableName (
      shortName,
      longName,
      description,
      variableName),
    fBooleanVariable (
      booleanVariable),
    fBooleanSecondaryVariable (
      booleanSecondaryVariable)
{}

oahTwoBooleansAtom::~oahTwoBooleansAtom ()
{}

S_oahValuedAtom oahTwoBooleansAtom::handleOptionUnderName (
  string   optionName,
  ostream& os)
{
  // handle it at once
  setTwoBooleansVariables (true);

  // no option value is needed
  return nullptr;
}

void oahTwoBooleansAtom::print (ostream& os) const
{
  const int fieldWidth = K_OPTIONS_FIELD_WIDTH;

  os <<
    "TwoBooleansAtom:" <<
    endl;

  gIndenter++;

  oahOption::printOptionEssentials (
    os, fieldWidth);

  gIndenter++;
  os <<
    gIndenter.indentMultiLineString (
      fDescription);
  gIndenter--;

  os << left <<
    setw (fieldWidth) <<
    "fVariableName" << " : " <<
    fVariableName <<
    endl <<
    setw (fieldWidth) <<
    "fBooleanVariable" << " : " <<
    booleanAsString (
      fBooleanVariable) <<
    endl;

  gIndenter--;
}

void oahTwoBooleansAtom::printAtomOptionsValues (
  ostream& os,
  int      valueFieldWidth) const
{
  os << left <<
    setw (valueFieldWidth) <<
    fVariableName <<
    " : " <<
    booleanAsString (
      fBooleanVariable) <<
    endl;
}

ostream& operator<< (ostream& os, const S_oahTwoBooleansAtom& elt)
{
  elt->print (os);
  return os;
}

//______________________________________________________________________________
S_oahThreeBooleansAtom oahThreeBooleansAtom::create (
  string shortName,
  string longName,
  string description,
  string variableName,
  bool&  booleanVariable,
  bool&  booleanSecondaryVariable,
  bool&  booleanTertiaryVariable)
{
  oahThreeBooleansAtom* o = new
    oahThreeBooleansAtom (
      shortName,
      longName,
      description,
      variableName,
      booleanVariable,
      booleanSecondaryVariable,
      booleanTertiaryVariable);
  assert(o!=0);
  return o;
}

oahThreeBooleansAtom::oahThreeBooleansAtom (
  string shortName,
  string longName,
  string description,
  string variableName,
  bool&  booleanVariable,
  bool&  booleanSecondaryVariable,
  bool&  booleanTertiaryVariable)
  : oahAtomWithVariableName (
      shortName,
      longName,
      description,
      variableName),
    fBooleanVariable (
      booleanVariable),
    fBooleanSecondaryVariable (
      booleanSecondaryVariable),
    fBooleanTertiaryVariable (
      booleanTertiaryVariable)
{}

oahThreeBooleansAtom::~oahThreeBooleansAtom ()
{}

S_oahValuedAtom oahThreeBooleansAtom::handleOptionUnderName (
  string   optionName,
  ostream& os)
{
  // handle it at once
  setThreeBooleansVariables (true);

  // no option value is needed
  return nullptr;
}

void oahThreeBooleansAtom::print (ostream& os) const
{
  const int fieldWidth = K_OPTIONS_FIELD_WIDTH;

  os <<
    "ThreeBooleansAtom:" <<
    endl;

  gIndenter++;

  oahOption::printOptionEssentials (
    os, fieldWidth);

  gIndenter++;
  os <<
    gIndenter.indentMultiLineString (
      fDescription) <<
    endl;
  gIndenter--;

  os << left <<
    setw (fieldWidth) <<
    "fVariableName" << " : " <<
    fVariableName <<
    endl <<
    setw (fieldWidth) <<
    "fBooleanVariable" << " : " <<
    booleanAsString (
      fBooleanVariable) <<
    endl;

  gIndenter--;
}

void oahThreeBooleansAtom::printAtomOptionsValues (
  ostream& os,
  int      valueFieldWidth) const
{
  os << left <<
    setw (valueFieldWidth) <<
    fVariableName <<
    " : " <<
    booleanAsString (
      fBooleanVariable) <<
    endl;
}

ostream& operator<< (ostream& os, const S_oahThreeBooleansAtom& elt)
{
  elt->print (os);
  return os;
}

//______________________________________________________________________________
S_oahCombinedBooleansAtom oahCombinedBooleansAtom::create (
  string shortName,
  string longName,
  string description,
  string variableName,
  bool&  booleanVariable)
{
  oahCombinedBooleansAtom* o = new
    oahCombinedBooleansAtom (
      shortName,
      longName,
      description,
      variableName,
      booleanVariable);
  assert(o!=0);
  return o;
}

oahCombinedBooleansAtom::oahCombinedBooleansAtom (
  string shortName,
  string longName,
  string description,
  string variableName,
  bool&  booleanVariable)
  : oahAtomWithVariableName (
      shortName,
      longName,
      description,
      variableName),
    fBooleanVariable (
      booleanVariable)
{}

oahCombinedBooleansAtom::~oahCombinedBooleansAtom ()
{}

void oahCombinedBooleansAtom::addBooleanAtom (
  S_oahBooleanAtom booleanAtom)
{
  // sanity check
  msrAssert (
    booleanAtom != nullptr,
    "booleanAtom is null");

  fBooleanAtomsList.push_back (
    booleanAtom);
}

void oahCombinedBooleansAtom::addBooleanAtomByName (
  string name)
{
  // get the options handler
  S_oahHandler
    handler =
      getSubGroupUpLink ()->
        getGroupUpLink ()->
          getHandlerUpLink ();

  // sanity check
  msrAssert (
    handler != nullptr,
    "handler is null");

  // is name known in options map?
  S_oahOption
    option =
      handler->
        fetchOptionFromMap (
          name);

  if (! option) {
    // no, name is unknown in the map
    handler->
      printOptionsSummary ();

    stringstream s;

    s <<
      "INTERNAL ERROR: option name '" << name <<
      "' is unknown";

    optionError (s.str ());
  }

  else {
    // name is known, let's handle it

    if (
      // boolean atom?
      S_oahBooleanAtom
        atom =
          dynamic_cast<oahBooleanAtom*>(&(*option))
      ) {
      // handle the option atom
      fBooleanAtomsList.push_back (
        atom);
    }

    else {
      stringstream s;

      s <<
        "option name '" << name <<
        "' is not that of an atom";

      optionError (s.str ());

      exit (2);
    }
  }
}

void oahCombinedBooleansAtom::setCombinedBooleanVariables (
  bool value)
{
  // set the combined atoms atom variable
  fBooleanVariable = value;

  // set the value of the atoms in the list
  if (fBooleanAtomsList.size ()) {
    for (
      list<S_oahBooleanAtom>::const_iterator i =
        fBooleanAtomsList.begin ();
      i != fBooleanAtomsList.end ();
      i++
    ) {
      S_oahAtom atom = (*i);

      if (
        // boolean atom?
        S_oahBooleanAtom
          booleanAtom =
            dynamic_cast<oahBooleanAtom*>(&(*atom))
        ) {
        // set the boolean variable
        booleanAtom->
          setBooleanVariable (value);
      }
    } // for
  }
}

S_oahValuedAtom oahCombinedBooleansAtom::handleOptionUnderName (
  string   optionName,
  ostream& os)
{
  // handle it at once
  setCombinedBooleanVariables (true);

  // no option value is needed
  return nullptr;
}

void oahCombinedBooleansAtom::print (ostream& os) const
{
  const int fieldWidth = K_OPTIONS_FIELD_WIDTH;

  os <<
    "CombinedBooleansAtom:" <<
    endl;

  gIndenter++;

  printOptionEssentials (
    os, fieldWidth);

  os << left <<
    setw (fieldWidth) <<
    "fBooleanAtomsList" << " : ";

  if (! fBooleanAtomsList.size ()) {
    os <<
      "none";
  }

  else {
    os << endl;

    gIndenter++;

    os << "'";

    list<S_oahBooleanAtom>::const_iterator
      iBegin = fBooleanAtomsList.begin (),
      iEnd   = fBooleanAtomsList.end (),
      i      = iBegin;

    for ( ; ; ) {
      os << (*i);
      if (++i == iEnd) break;
      os << " ";
    } // for

    os << "'";

    gIndenter--;
  }

  gIndenter--;

  os << endl;
}

void oahCombinedBooleansAtom::printHelp (ostream& os)
{
  os <<
    fetchNames () <<
    endl;

  if (fDescription.size ()) {
    // indent a bit more for readability
    gIndenter.increment (K_OPTIONS_ELEMENTS_INDENTER_OFFSET);

    os <<
      gIndenter.indentMultiLineString (
        fDescription) <<
      endl;
  }

  os <<
    "This combined option is equivalent to: ";

  if (! fBooleanAtomsList.size ()) {
    os <<
      "none" <<
      endl;
  }

  else {
    os << endl;

    gIndenter++;

    list<S_oahBooleanAtom>::const_iterator
      iBegin = fBooleanAtomsList.begin (),
      iEnd   = fBooleanAtomsList.end (),
      i      = iBegin;

    for ( ; ; ) {
      os <<
        (*i)-> fetchNames ();
      if (++i == iEnd) break;
      os << endl;
    } // for

    os << endl;

    gIndenter--;
  }

  if (fDescription.size ()) { // ??? JMI
    gIndenter.decrement (K_OPTIONS_ELEMENTS_INDENTER_OFFSET);
  }

  // register help print action in options handler upLink
//  fHandlerUpLink->setOptionsHandlerFoundAHelpOption ();
}

void oahCombinedBooleansAtom::printAtomOptionsValues (
  ostream& os,
  int      valueFieldWidth) const
{
  os << left <<
    setw (valueFieldWidth) <<
    fVariableName <<
    " : " <<
    booleanAsString (
      fBooleanVariable) <<
    endl;

  int fieldWidth =
    valueFieldWidth - gIndenter.getIndent () + 1;

  gIndenter++; // only now

  if (! fBooleanAtomsList.size ()) {
    os <<
      "none" <<
      endl;
  }

  else {
    list<S_oahBooleanAtom>::const_iterator
      iBegin = fBooleanAtomsList.begin (),
      iEnd   = fBooleanAtomsList.end (),
      i      = iBegin;

    for ( ; ; ) {
      S_oahAtom
        atom = (*i);

      if (
        // boolean atom?
        S_oahBooleanAtom
          booleanAtom =
            dynamic_cast<oahBooleanAtom*>(&(*atom))
        ) {
        // print the boolean value
        booleanAtom->
          printAtomOptionsValues (
            os, fieldWidth);
      }

      if (++i == iEnd) break;

  // JMI    os << endl;
    } // for
  }

  gIndenter--;

}

ostream& operator<< (ostream& os, const S_oahCombinedBooleansAtom& elt)
{
  elt->print (os);
  return os;
}

//______________________________________________________________________________
/* pure virtual class
S_oahValuedAtom oahValuedAtom::create (
  string shortName,
  string longName,
  string description,
  string valueSpecification,
  string variableName)
{
  oahValuedAtom* o = new
    oahValuedAtom (
      shortName,
      longName,
      description,
      valueSpecification,
      variableName);
  assert(o!=0);
  return o;
}
*/

oahValuedAtom::oahValuedAtom (
  string shortName,
  string longName,
  string description,
  string valueSpecification,
  string variableName)
  : oahAtomWithVariableName (
      shortName,
      longName,
      description,
      variableName)
{
  fValueSpecification = valueSpecification;

  fValueIsOptional = false;
}

oahValuedAtom::~oahValuedAtom ()
{}

void oahValuedAtom::handleDefaultValue ()
{}

string oahValuedAtom::oahAtomKindAsString (
  oahValuedAtom::oahValuedAtomKind oahAtomKind)
{
  string result;

  switch (oahAtomKind) {
    case oahValuedAtom::kAtomHasNoArgument:
      result = "atomHasNoArgument";
      break;
    case oahValuedAtom::kAtomHasARequiredArgument:
      result = "atomHasARequiredArgument";
      break;
    case oahValuedAtom::kAtomHasAnOptionsArgument:
      result = "atomHasAnOptionsArgument";
      break;
  } // switch

  return result;
}

void oahValuedAtom::printValuedAtomEssentials (
  ostream& os,
  int      fieldWidth) const
{
  printOptionEssentials (
    os, fieldWidth);

  os << left <<
    setw (fieldWidth) <<
    "fValueSpecification" << " : " <<
    fValueSpecification <<
    endl <<
    setw (fieldWidth) <<
    "fValueIsOptional" << " : " <<
    booleanAsString (fValueIsOptional) <<
    endl;
}

void oahValuedAtom::print (ostream& os) const
{
  const int fieldWidth = 19;

  os <<
    "ValuedAtom ???:" <<
    endl;

  gIndenter++;

  printValuedAtomEssentials (
    os, fieldWidth);

  gIndenter--;
}

void oahValuedAtom::printHelp (ostream& os)
{
  os <<
    fetchNames () <<
    " " <<
    fValueSpecification <<
    endl;

  if (fDescription.size ()) {
    // indent a bit more for readability
    gIndenter.increment (K_OPTIONS_ELEMENTS_INDENTER_OFFSET);

    os <<
      gIndenter.indentMultiLineString (
        fDescription) <<
      endl;

    gIndenter.decrement (K_OPTIONS_ELEMENTS_INDENTER_OFFSET);
  }

/* superfluous JMI
  if (fValueIsOptional) {
    os <<
      "option value is optional" <<
      endl;
  }
*/

  // register help print action in options handler upLink // JMI
//  fHandlerUpLink->setOptionsHandlerFoundAHelpOption ();
}

void oahValuedAtom::printAtomOptionsValues (
  ostream& os,
  int      valueFieldWidth) const
{
  os <<
    "ValuedAtom values ???:" <<
    endl;
}

ostream& operator<< (ostream& os, const S_oahValuedAtom& elt)
{
  elt->print (os);
  return os;
}

//______________________________________________________________________________
S_oahIntegerAtom oahIntegerAtom::create (
  string shortName,
  string longName,
  string description,
  string valueSpecification,
  string variableName,
  int&   integerVariable)
{
  oahIntegerAtom* o = new
    oahIntegerAtom (
      shortName,
      longName,
      description,
      valueSpecification,
      variableName,
      integerVariable);
  assert(o!=0);
  return o;
}

oahIntegerAtom::oahIntegerAtom (
  string shortName,
  string longName,
  string description,
  string valueSpecification,
  string variableName,
  int&   integerVariable)
  : oahValuedAtom (
      shortName,
      longName,
      description,
      valueSpecification,
      variableName),
    fIntegerVariable (
      integerVariable)
{}

oahIntegerAtom::~oahIntegerAtom ()
{}

S_oahValuedAtom oahIntegerAtom::handleOptionUnderName (
  string   optionName,
  ostream& os)
{
  // an option value is needed
  return this;
}

void oahIntegerAtom::handleValue (
  string   theString,
  ostream& os)
{
  // theString contains the integer value

  // check whether it is well-formed
  string regularExpression (
    "([[:digit:]]+)");

  regex e (regularExpression);
  smatch sm;

  regex_match (theString, sm, e);

  unsigned smSize = sm.size ();

  if (smSize) {
#ifdef TRACE_OAH
    if (gExecutableOah->fTraceOah) {
      os <<
        "There are " << smSize << " matches" <<
        " for integer string '" << theString <<
        "' with regex '" << regularExpression <<
        "'" <<
        endl;

      for (unsigned i = 0; i < smSize; ++i) {
        os <<
          "[" << sm [i] << "] ";
      } // for

      os << endl;
    }
#endif

    // leave the low level details to the STL...
    int integerValue;
    {
      stringstream s;
      s << theString;
      s >> integerValue;
    }

    fIntegerVariable = integerValue;
  }

  else {
    stringstream s;

    s <<
      "integer value '" << theString <<
      "' for option '" << fetchNames () <<
      "' is ill-formed";

    optionError (s.str ());
    exit (4);
  }
}

string oahIntegerAtom::asShortNamedOptionString () const
{
  stringstream s;

  s <<
    "-"  << fShortName << " " << fIntegerVariable;

  return s.str ();
}

string oahIntegerAtom::asLongNamedOptionString () const
{
  stringstream s;

  s <<
    "-" << fLongName << " " << fIntegerVariable;

  return s.str ();
}

void oahIntegerAtom::print (ostream& os) const
{
  const int fieldWidth = K_OPTIONS_FIELD_WIDTH;

  os <<
    "IntegerAtom:" <<
    endl;

  gIndenter++;

  printValuedAtomEssentials (
    os, fieldWidth);

  os << left <<
    setw (fieldWidth) <<
    "fVariableName" << " : " <<
    fVariableName <<
    endl <<
    setw (fieldWidth) <<
    "fIntegerVariable" << " : " <<
    fIntegerVariable <<
    endl;

  gIndenter--;
}

void oahIntegerAtom::printAtomOptionsValues (
  ostream& os,
  int      valueFieldWidth) const
{
  os << left <<
    setw (valueFieldWidth) <<
    fVariableName <<
    " : " <<
    fIntegerVariable <<
    endl;
}

ostream& operator<< (ostream& os, const S_oahIntegerAtom& elt)
{
  elt->print (os);
  return os;
}

//______________________________________________________________________________
S_oahFloatAtom oahFloatAtom::create (
  string shortName,
  string longName,
  string description,
  string valueSpecification,
  string variableName,
  float& floatVariable)
{
  oahFloatAtom* o = new
    oahFloatAtom (
      shortName,
      longName,
      description,
      valueSpecification,
      variableName,
      floatVariable);
  assert(o!=0);
  return o;
}

oahFloatAtom::oahFloatAtom (
  string shortName,
  string longName,
  string description,
  string valueSpecification,
  string variableName,
  float& floatVariable)
  : oahValuedAtom (
      shortName,
      longName,
      description,
      valueSpecification,
      variableName),
    fFloatVariable (
      floatVariable)
{}

oahFloatAtom::~oahFloatAtom ()
{}

S_oahValuedAtom oahFloatAtom::handleOptionUnderName (
  string   optionName,
  ostream& os)
{
  // an option value is needed
  return this;
}

void oahFloatAtom::handleValue (
  string   theString,
  ostream& os)
{
  // theString contains the float value

  // check whether it is well-formed
  string regularExpression (
   // no sign, a '-' would be handled as an option name JMI   "([+|-]?)"
    "([[:digit:]]+)"
    "."
    "([[:digit:]]*)");

  regex e (regularExpression);
  smatch sm;

  regex_match (theString, sm, e);

  unsigned smSize = sm.size ();

  if (smSize) {
#ifdef TRACE_OAH
    if (gExecutableOah->fTraceOah) {
      os <<
        "There are " << smSize << " matches" <<
        " for float string '" << theString <<
        "' with regex '" << regularExpression <<
        "'" <<
        endl;

      for (unsigned i = 0; i < smSize; ++i) {
        os <<
          "[" << sm [i] << "] ";
      } // for

      os << endl;
    }
#endif

    // leave the low level details to the STL...
    float floatValue;
    {
      stringstream s;

      s << theString;
      s >> floatValue;
    }

    fFloatVariable = floatValue;
  }

  else {
    stringstream s;

    s <<
      "float value '" << theString <<
      "' for option '" << fetchNames () <<
      "' is ill-formed";

    optionError (s.str ());
    exit (4);
  }
}

string oahFloatAtom::asShortNamedOptionString () const
{
  stringstream s;

  s <<
    "-"  << fShortName << " " << fFloatVariable;

  return s.str ();
}

string oahFloatAtom::asLongNamedOptionString () const
{
  stringstream s;

  s <<
    "-" << fLongName << " " << fFloatVariable;

  return s.str ();
}

void oahFloatAtom::print (ostream& os) const
{
  const int fieldWidth = K_OPTIONS_FIELD_WIDTH;

  os <<
    "FloatAtom:" <<
    endl;

  gIndenter++;

  printValuedAtomEssentials (
    os, fieldWidth);

  os << left <<
    setw (fieldWidth) <<
    "fVariableName" << " : " <<
    fVariableName <<
    endl <<
    setw (fieldWidth) <<
    "fFloatVariable" << " : " <<
    fFloatVariable <<
    endl;

  gIndenter--;
}

void oahFloatAtom::printAtomOptionsValues (
  ostream& os,
  int      valueFieldWidth) const
{
  os << left <<
    setw (valueFieldWidth) <<
    fVariableName <<
    " : " <<
    fFloatVariable <<
    endl;
}

ostream& operator<< (ostream& os, const S_oahFloatAtom& elt)
{
  elt->print (os);
  return os;
}

//______________________________________________________________________________
S_oahStringAtom oahStringAtom::create (
  string  shortName,
  string  longName,
  string  description,
  string  valueSpecification,
  string  variableName,
  string& stringVariable)
{
  oahStringAtom* o = new
    oahStringAtom (
      shortName,
      longName,
      description,
      valueSpecification,
      variableName,
      stringVariable);
  assert(o!=0);
  return o;
}

oahStringAtom::oahStringAtom (
  string  shortName,
  string  longName,
  string  description,
  string  valueSpecification,
  string  variableName,
  string& stringVariable)
  : oahValuedAtom (
      shortName,
      longName,
      description,
      valueSpecification,
      variableName),
    fStringVariable (
      stringVariable)
{}

oahStringAtom::~oahStringAtom ()
{}

S_oahValuedAtom oahStringAtom::handleOptionUnderName (
  string   optionName,
  ostream& os)
{
  // an option value is needed
  return this;
}

void oahStringAtom::handleValue (
  string   theString,
  ostream& os)
{
  fStringVariable = theString;
}

string oahStringAtom::asShortNamedOptionString () const
{
  stringstream s;

  s <<
    "-"  << fShortName << " " << fStringVariable;

  return s.str ();
}

string oahStringAtom::asLongNamedOptionString () const
{
  stringstream s;

  s <<
    "-" << fLongName << " " << fStringVariable;

  return s.str ();
}

void oahStringAtom::print (ostream& os) const
{
  const int fieldWidth = K_OPTIONS_FIELD_WIDTH;

  os <<
    "StringAtom:" <<
    endl;

  gIndenter++;

  printValuedAtomEssentials (
    os, fieldWidth);

  os << left <<
    setw (fieldWidth) <<
    "fVariableName" << " : " <<
    fVariableName <<
    endl <<
    setw (fieldWidth) <<
    "fStringVariable" << " : " <<
    fStringVariable <<
    endl;

  gIndenter--;
}

void oahStringAtom::printAtomOptionsValues (
  ostream& os,
  int      valueFieldWidth) const
{
  os << left <<
    setw (valueFieldWidth) <<
    fVariableName <<
    " : \"" <<
    fStringVariable <<
    "\"" <<
    endl;
}

ostream& operator<< (ostream& os, const S_oahStringAtom& elt)
{
  os <<
    "StringAtom:" <<
    endl;
  elt->print (os);
  return os;
}

//______________________________________________________________________________
S_oahStringWithDefaultValueAtom oahStringWithDefaultValueAtom::create (
  string  shortName,
  string  longName,
  string  description,
  string  valueSpecification,
  string  variableName,
  string& stringVariable,
  string  defaultStringValue)
{
  oahStringWithDefaultValueAtom* o = new
    oahStringWithDefaultValueAtom (
      shortName,
      longName,
      description,
      valueSpecification,
      variableName,
      stringVariable,
      defaultStringValue);
  assert(o!=0);
  return o;
}

oahStringWithDefaultValueAtom::oahStringWithDefaultValueAtom (
  string  shortName,
  string  longName,
  string  description,
  string  valueSpecification,
  string  variableName,
  string& stringVariable,
  string  defaultStringValue)
  : oahStringAtom (
      shortName,
      longName,
      description,
      valueSpecification,
      variableName,
      stringVariable),
    fDefaultStringValue (
      defaultStringValue)
{
  setValueIsOptional ();
}

oahStringWithDefaultValueAtom::~oahStringWithDefaultValueAtom ()
{}

S_oahValuedAtom oahStringWithDefaultValueAtom::handleOptionUnderName (
  string   optionName,
  ostream& os)
{
  // an option value is needed
  return this;
}

void oahStringWithDefaultValueAtom::handleValue (
  string   theString,
  ostream& os)
{
  oahStringAtom::handleValue (theString, os); // JMI ???
}

string oahStringWithDefaultValueAtom::asShortNamedOptionString () const
{
  stringstream s;

  s <<
    "-"  << fShortName << " " << fStringVariable;

  return s.str ();
}

string oahStringWithDefaultValueAtom::asLongNamedOptionString () const
{
  stringstream s;

  s <<
    "-" << fLongName << " " << fStringVariable;

  return s.str ();
}

void oahStringWithDefaultValueAtom::print (ostream& os) const
{
  const int fieldWidth = K_OPTIONS_FIELD_WIDTH;

  os <<
    "StringWithDefaultValueAtom:" <<
    endl;

  gIndenter++;

  printValuedAtomEssentials (
    os, fieldWidth);

  os << left <<
    setw (fieldWidth) <<
    "fVariableName" << " : " <<
    fVariableName <<
    endl <<
    setw (fieldWidth) <<
    "fStringVariable" << " : " <<
    fStringVariable <<
    endl;

  gIndenter--;
}

void oahStringWithDefaultValueAtom::printAtomOptionsValues (
  ostream& os,
  int      valueFieldWidth) const
{
  os << left <<
    setw (valueFieldWidth) <<
    fVariableName <<
    " : \"" <<
    fStringVariable <<
    "\"" <<
    endl;
}

ostream& operator<< (ostream& os, const S_oahStringWithDefaultValueAtom& elt)
{
  os <<
    "StringWithDefaultValueAtom:" <<
    endl;
  elt->print (os);
  return os;
}

//______________________________________________________________________________
S_oahRationalAtom oahRationalAtom::create (
  string    shortName,
  string    longName,
  string    description,
  string    valueSpecification,
  string    variableName,
  rational& rationalVariable)
{
  oahRationalAtom* o = new
    oahRationalAtom (
      shortName,
      longName,
      description,
      valueSpecification,
      variableName,
      rationalVariable);
  assert(o!=0);
  return o;
}

oahRationalAtom::oahRationalAtom (
  string    shortName,
  string    longName,
  string    description,
  string    valueSpecification,
  string    variableName,
  rational& rationalVariable)
  : oahValuedAtom (
      shortName,
      longName,
      description,
      valueSpecification,
      variableName),
    fRationalVariable (
      rationalVariable)
{}

oahRationalAtom::~oahRationalAtom ()
{}

S_oahValuedAtom oahRationalAtom::handleOptionUnderName (
  string   optionName,
  ostream& os)
{
  // an option value is needed
  return this;
}

void oahRationalAtom::handleValue (
  string   theString,
  ostream& os)
{

  // theString contains the fraction:
  // decipher it to extract numerator and denominator values

  string regularExpression (
    "[[:space:]]*([[:digit:]]+)[[:space:]]*"
    "/"
    "[[:space:]]*([[:digit:]]+)[[:space:]]*");

  regex e (regularExpression);
  smatch sm;

  regex_match (theString, sm, e);

  unsigned smSize = sm.size ();

  if (smSize == 3) {
#ifdef TRACE_OAH
    if (gExecutableOah->fTraceOah) {
      os <<
        "There are " << smSize << " matches" <<
        " for rational string '" << theString <<
        "' with regex '" << regularExpression <<
        "'" <<
        endl;

      for (unsigned i = 0; i < smSize; ++i) {
        os <<
          "[" << sm [i] << "] ";
      } // for

      os << endl;
    }
#endif
  }

  else {
    stringstream s;

    s <<
      "rational atom value '" << theString <<
      "' is ill-formed";

    optionError (s.str ());
    exit (4);
  }

  int
    numerator,
    denominator;

  {
    stringstream s;
    s << sm [1];
    s >> numerator;
  }
  {
    stringstream s;
    s << sm [2];
    s >> denominator;
  }

  rational
    rationalValue =
      rational (numerator, denominator);

#ifdef TRACE_OAH
  if (gExecutableOah->fTraceOah) {
    os <<
      "rationalValue = " <<
      rationalValue <<
      endl;
  }
#endif

  fRationalVariable = rationalValue;
}

string oahRationalAtom::asShortNamedOptionString () const
{
  stringstream s;

  s <<
    "-"  << fShortName << " " << fRationalVariable;

  return s.str ();
}

string oahRationalAtom::asLongNamedOptionString () const
{
  stringstream s;

  s <<
    "-" << fLongName << " " << fRationalVariable;

  return s.str ();
}

void oahRationalAtom::print (ostream& os) const
{
  const int fieldWidth = K_OPTIONS_FIELD_WIDTH;

  os <<
    "RationalAtom:" <<
    endl;

  gIndenter++;

  printValuedAtomEssentials (
    os, fieldWidth);

  os << left <<
    setw (fieldWidth) <<
    "fVariableName" << " : " <<
    fVariableName <<
    endl <<
    setw (fieldWidth) <<
    "fRationalVariable" << " : " <<
    fRationalVariable <<
    endl;

  gIndenter--;
}

void oahRationalAtom::printAtomOptionsValues (
  ostream& os,
  int      valueFieldWidth) const
{
  os << left <<
    setw (valueFieldWidth) <<
    fVariableName <<
    " : " <<
    fRationalVariable <<
    endl;
}

ostream& operator<< (ostream& os, const S_oahRationalAtom& elt)
{
  elt->print (os);
  return os;
}

//______________________________________________________________________________
S_oahNaturalNumbersSetAtom oahNaturalNumbersSetAtom::create (
  string    shortName,
  string    longName,
  string    description,
  string    valueSpecification,
  string    variableName,
  set<int>& naturalNumbersSetVariable)
{
  oahNaturalNumbersSetAtom* o = new
    oahNaturalNumbersSetAtom (
      shortName,
      longName,
      description,
      valueSpecification,
      variableName,
      naturalNumbersSetVariable);
  assert(o!=0);
  return o;
}

oahNaturalNumbersSetAtom::oahNaturalNumbersSetAtom (
  string    shortName,
  string    longName,
  string    description,
  string    valueSpecification,
  string    variableName,
  set<int>& naturalNumbersSetVariable)
  : oahValuedAtom (
      shortName,
      longName,
      description,
      valueSpecification,
      variableName),
    fNaturalNumbersSetVariable (
      naturalNumbersSetVariable)
{}

oahNaturalNumbersSetAtom::~oahNaturalNumbersSetAtom ()
{}

S_oahValuedAtom oahNaturalNumbersSetAtom::handleOptionUnderName (
  string   optionName,
  ostream& os)
{
  // an option value is needed
  return this;
}

void oahNaturalNumbersSetAtom::handleValue (
  string   theString,
  ostream& os)
{
  fNaturalNumbersSetVariable =
    decipherNaturalNumbersSetSpecification (
      theString,
      false); // 'true' to debug it
}

string oahNaturalNumbersSetAtom::asShortNamedOptionString () const
{
  stringstream s;

  s <<
    "-"  << fShortName << " " <<
    "[";

  set<int>::const_iterator
    iBegin = fNaturalNumbersSetVariable.begin (),
    iEnd   = fNaturalNumbersSetVariable.end (),
    i      = iBegin;

  for ( ; ; ) {
    s << (*i);
    if (++i == iEnd) break;
    s << " ";
  } // for

  s <<
    "]";

  return s.str ();
}

string oahNaturalNumbersSetAtom::asLongNamedOptionString () const
{
  stringstream s;

  s <<
    "-"  << fLongName << " " <<
    "[";

  set<int>::const_iterator
    iBegin = fNaturalNumbersSetVariable.begin (),
    iEnd   = fNaturalNumbersSetVariable.end (),
    i      = iBegin;

  for ( ; ; ) {
    s << (*i);
    if (++i == iEnd) break;
    s << " ";
  } // for

  s <<
    "]";

  return s.str ();
}

void oahNaturalNumbersSetAtom::print (ostream& os) const
{
  const int fieldWidth = K_OPTIONS_FIELD_WIDTH;

  os <<
    "NaturalNumbersSetAtom:" <<
    endl;

  gIndenter++;

  printValuedAtomEssentials (
    os, fieldWidth);

  os << left <<
    setw (fieldWidth) <<
    "fVariableName" << " : " <<
    fVariableName <<
    setw (fieldWidth) <<
    "fNaturalNumbersSetVariable" << " : " <<
    endl;

  if (! fNaturalNumbersSetVariable.size ()) {
    os <<
      "none";
  }

  else {
    os <<
      "'";

    set<int>::const_iterator
      iBegin = fNaturalNumbersSetVariable.begin (),
      iEnd   = fNaturalNumbersSetVariable.end (),
      i      = iBegin;

    for ( ; ; ) {
      os << (*i);
      if (++i == iEnd) break;
      os << " ";
    } // for

    os <<
      "'";
  }

  os << endl;

  gIndenter--;
}

void oahNaturalNumbersSetAtom::printAtomOptionsValues (
  ostream& os,
  int      valueFieldWidth) const
{
  os << left <<
    setw (valueFieldWidth) <<
    fVariableName <<
    " : ";

  if (! fNaturalNumbersSetVariable.size ()) {
    os <<
      "none";
  }

  else {
    os <<
      "'";

    set<int>::const_iterator
      iBegin = fNaturalNumbersSetVariable.begin (),
      iEnd   = fNaturalNumbersSetVariable.end (),
      i      = iBegin;

    for ( ; ; ) {
      os << (*i);
      if (++i == iEnd) break;
      os << " ";
    } // for

    os <<
      "'";
  }

  os << endl;
}

ostream& operator<< (ostream& os, const S_oahNaturalNumbersSetAtom& elt)
{
  elt->print (os);
  return os;
}

//______________________________________________________________________________
S_oahStringsSetAtom oahStringsSetAtom::create (
  string       shortName,
  string       longName,
  string       description,
  string       valueSpecification,
  string       variableName,
  set<string>& stringsSetVariable)
{
  oahStringsSetAtom* o = new
    oahStringsSetAtom (
      shortName,
      longName,
      description,
      valueSpecification,
      variableName,
      stringsSetVariable);
  assert(o!=0);
  return o;
}

oahStringsSetAtom::oahStringsSetAtom (
  string       shortName,
  string       longName,
  string       description,
  string       valueSpecification,
  string       variableName,
  set<string>& stringsSetVariable)
  : oahValuedAtom (
      shortName,
      longName,
      description,
      valueSpecification,
      variableName),
    fStringsSetVariable (
      stringsSetVariable)
{}

oahStringsSetAtom::~oahStringsSetAtom ()
{}

S_oahValuedAtom oahStringsSetAtom::handleOptionUnderName (
  string   optionName,
  ostream& os)
{
  // an option value is needed
  return this;
}

void oahStringsSetAtom::handleValue (
  string   theString,
  ostream& os)
{
  fStringsSetVariable =
    decipherStringsSetSpecification (
      theString,
      false); // 'true' to debug it
}

string oahStringsSetAtom::asShortNamedOptionString () const
{
  stringstream s;

  s <<
    "-"  << fShortName << " " <<
    "[";

  set<string>::const_iterator
    iBegin = fStringsSetVariable.begin (),
    iEnd   = fStringsSetVariable.end (),
    i      = iBegin;

  for ( ; ; ) {
    s << (*i);
    if (++i == iEnd) break;
    s << " ";
  } // for

  s <<
    "]";

  return s.str ();
}

string oahStringsSetAtom::asLongNamedOptionString () const
{
  stringstream s;

  s <<
    "-"  << fLongName << " " <<
    "[";

  set<string>::const_iterator
    iBegin = fStringsSetVariable.begin (),
    iEnd   = fStringsSetVariable.end (),
    i      = iBegin;

  for ( ; ; ) {
    s << (*i);
    if (++i == iEnd) break;
    s << " ";
  } // for

  s <<
    "]";

  return s.str ();
}

void oahStringsSetAtom::print (ostream& os) const
{
  const int fieldWidth = K_OPTIONS_FIELD_WIDTH;

  os <<
    "StringsSetAtom:" <<
    endl;

  gIndenter++;

  printValuedAtomEssentials (
    os, fieldWidth);

  os << left <<
    setw (fieldWidth) <<
    "fVariableName" << " : " <<
    fVariableName <<
    setw (fieldWidth) <<
    "fStringsSetVariable" << " : " <<
    endl;

  if (! fStringsSetVariable.size ()) {
    os <<
      "none";
  }

  else {
    os <<
      "'";

    set<string>::const_iterator
      iBegin = fStringsSetVariable.begin (),
      iEnd   = fStringsSetVariable.end (),
      i      = iBegin;

    for ( ; ; ) {
      os << (*i);
      if (++i == iEnd) break;
      os << " ";
    } // for

    os <<
      "'";
  }

  os << endl;

  gIndenter--;
}

void oahStringsSetAtom::printAtomOptionsValues (
  ostream& os,
  int      valueFieldWidth) const
{
  os << left <<
    setw (valueFieldWidth) <<
    fVariableName <<
    " : ";

  if (! fStringsSetVariable.size ()) {
    os <<
      "none";
  }

  else {
    os <<
      "'";

    set<string>::const_iterator
      iBegin = fStringsSetVariable.begin (),
      iEnd   = fStringsSetVariable.end (),
      i      = iBegin;

    for ( ; ; ) {
      os << (*i);
      if (++i == iEnd) break;
      os << " ";
    } // for

    os <<
      "'";
  }

  os << endl;
}

ostream& operator<< (ostream& os, const S_oahStringsSetAtom& elt)
{
  elt->print (os);
  return os;
}

//______________________________________________________________________________
S_oahOptionNameHelpAtom oahOptionNameHelpAtom::create (
  string shortName,
  string longName,
  string description,
  string valueSpecification,
  string variableName,
  string& stringVariable,
  string  defaultOptionName)
{
  oahOptionNameHelpAtom* o = new
    oahOptionNameHelpAtom (
      shortName,
      longName,
      description,
      valueSpecification,
      variableName,
      stringVariable,
      defaultOptionName);
  assert(o!=0);
  return o;
}

oahOptionNameHelpAtom::oahOptionNameHelpAtom (
  string shortName,
  string longName,
  string description,
  string valueSpecification,
  string variableName,
  string& stringVariable,
  string  defaultOptionName)
  : oahStringWithDefaultValueAtom (
      shortName,
      longName,
      description,
      valueSpecification,
      variableName,
      stringVariable,
      defaultOptionName)
{}

oahOptionNameHelpAtom::~oahOptionNameHelpAtom ()
{}

S_oahValuedAtom oahOptionNameHelpAtom::handleOptionUnderName (
  string   optionName,
  ostream& os)
{
  // an option value is needed
  return this;
}

void oahOptionNameHelpAtom::handleValue (
  string   theString,
  ostream& os)
{
  // delegate this to the handler
  fHandlerUpLink->
    printOptionSpecificHelp (
      os,
      theString);

  // exit
  exit (23);
}

void oahOptionNameHelpAtom::handleDefaultValue ()
{
  // delegate this to the handler
  fHandlerUpLink->
    printOptionSpecificHelp (
      fHandlerUpLink->getHandlerLogOstream (),
      fDefaultStringValue);
}

string oahOptionNameHelpAtom::asShortNamedOptionString () const
{
  stringstream s;

  s <<
    "-"  << fShortName << " " << fVariableName;

  return s.str ();
}

string oahOptionNameHelpAtom::asLongNamedOptionString () const
{
  stringstream s;

  s <<
    "-" << fLongName << " " << fVariableName;

  return s.str ();
}

void oahOptionNameHelpAtom::print (ostream& os) const
{
  const int fieldWidth = K_OPTIONS_FIELD_WIDTH;

  os <<
    "OptionNameHelpAtom:" <<
    endl;

  gIndenter++;

  printOptionEssentials (
    os, fieldWidth);

  gIndenter--;
}

void oahOptionNameHelpAtom::printAtomOptionsValues (
  ostream& os,
  int      valueFieldWidth) const
{
  // nothing to print here
}

ostream& operator<< (ostream& os, const S_oahOptionNameHelpAtom& elt)
{
  elt->print (os);
  return os;
}

//______________________________________________________________________________
S_oahSubGroup oahSubGroup::create (
  string                  subGroupHeader,
  string                  shortName,
  string                  longName,
  string                  description,
  oahOptionVisibilityKind optionVisibilityKind,
  S_oahGroup              groupUpLink)
{
  oahSubGroup* o = new
    oahSubGroup (
      subGroupHeader,
      shortName,
      longName,
      description,
      optionVisibilityKind,
      groupUpLink);
  assert(o!=0);
  return o;
}

oahSubGroup::oahSubGroup (
  string                  subGroupHeader,
  string                  shortName,
  string                  longName,
  string                  description,
  oahOptionVisibilityKind optionVisibilityKind,
  S_oahGroup              groupUpLink)
  : oahOption (
      shortName,
      longName,
      description,
      optionVisibilityKind)
{
  fGroupUpLink = groupUpLink;

  fSubGroupHeader = subGroupHeader;
}

oahSubGroup::~oahSubGroup ()
{}

void oahSubGroup::underlineSubGroupHeader (ostream& os) const
{
  /* JMI ???
  for (unsigned int i = 0; i < fSubGroupHeader.size (); i++) {
    os << "-";
  } // for
  os << endl;
  */
  os << "--------------------------" << endl;
}

void oahSubGroup::registerSubGroupInHandler (
  S_oahHandler handler)
{
  handler->
    registerOptionInHandler (this);

  fHandlerUpLink = handler;

  for (
    list<S_oahAtom>::const_iterator
      i = fAtomsList.begin ();
    i != fAtomsList.end ();
    i++
  ) {
    // register the options sub group
    (*i)->
      registerAtomInHandler (
        handler);
  } // for
}

void oahSubGroup::appendAtom (
  S_oahAtom oahAtom)
{
  // sanity check
  msrAssert (
    oahAtom != nullptr,
    "oahAtom is null");

  // append atom
  fAtomsList.push_back (
    oahAtom);

  // set atom subgroup upLink
  oahAtom->
    setSubGroupUpLink (this);
}

S_oahOption oahSubGroup::fetchOptionByName (
  string name)
{
  S_oahOption result;

  for (
    list<S_oahAtom>::const_iterator
      i = fAtomsList.begin ();
    i != fAtomsList.end ();
    i++
  ) {
    // search name in the options group
    result =
      (*i)->fetchOptionByName (name);

    if (result != 0) {
      break;
    }
  } // for

  return result;
}

void oahSubGroup::print (ostream& os) const
{
  const int fieldWidth = 27;

  os <<
   "SubGroup:" <<
    endl;

  gIndenter++;

  oahOption::printOptionEssentials (
    os, fieldWidth);

  os << left <<
    setw (fieldWidth) <<
    "fOptionVisibilityKind" << " : " <<
      optionVisibilityKindAsString (
        fOptionVisibilityKind) <<
    endl <<
    endl;

  os <<
    "AtomsList (" <<
    singularOrPlural (
      fAtomsList.size (), "atom",  "atoms") <<
    "):" <<
    endl;

  if (fAtomsList.size ()) {
    os << endl;

    gIndenter++;

    list<S_oahAtom>::const_iterator
      iBegin = fAtomsList.begin (),
      iEnd   = fAtomsList.end (),
      i      = iBegin;
    for ( ; ; ) {
      // print the atom
      os << (*i);
      if (++i == iEnd) break;
      os << endl;
    } // for

    gIndenter--;
  }

  gIndenter--;
}

void oahSubGroup::printSubGroupHeader (ostream& os) const
{
  // print the header and option names
  os <<
    fSubGroupHeader;

  os <<
    " " <<
    fetchNamesBetweenParentheses ();

  switch (fOptionVisibilityKind) {
    case kElementVisibilityAlways:
      os <<
        ":";
      break;

    case kElementVisibilityHiddenByDefault:
      os <<
        " (hidden by default)";
      break;
  } // switch

  os << endl;
}

void oahSubGroup::printSubGroupHeaderWithHeaderWidth (
  ostream& os,
  int      subGroupHeaderWidth) const
{
  // print the header and option names
  os << left <<
    setw (subGroupHeaderWidth) <<
    fSubGroupHeader <<
    " " <<
    fetchNamesBetweenParentheses ();

  switch (fOptionVisibilityKind) {
    case kElementVisibilityAlways:
      os <<
        ":";
      break;

    case kElementVisibilityHiddenByDefault:
      os <<
        " (hidden by default)";
      break;
  } // switch

  os << endl;

}

void oahSubGroup::printHelp (ostream& os)
{
  // print the header and option names
  printSubGroupHeader (os);

  // print the description if any
  if (fDescription.size ()) {
    gIndenter++;
    os <<
      gIndenter.indentMultiLineString (
        fDescription);
    gIndenter--;

    os << endl;
  }

  // print the options atoms if relevant
  switch (fOptionVisibilityKind) {
    case kElementVisibilityAlways:
      if (fAtomsList.size ()) {
        gIndenter++;

        list<S_oahAtom>::const_iterator
          iBegin = fAtomsList.begin (),
          iEnd   = fAtomsList.end (),
          i      = iBegin;
        for ( ; ; ) {
          S_oahAtom oahAtom = (*i);
          bool
            oahAtomIsHidden =
              oahAtom->getIsHidden ();

          // print the atom help unless it is hidden
          if (! oahAtomIsHidden) {
            oahAtom->
              printHelp (os);
          }
          if (++i == iEnd) break;
          if (! oahAtomIsHidden) {
   // JMI         os << endl;
          }
        } // for

        gIndenter--;
      }
      break;

    case kElementVisibilityHiddenByDefault:
      break;
  } // switch

  // register help print action in options groups's options handler upLink
  fGroupUpLink->
    getHandlerUpLink ()->
      setOptionsHandlerFoundAHelpOption ();
}

void oahSubGroup::printHelpWithHeaderWidth (
  ostream& os,
  int      subGroupHeaderWidth)
{
  // print the header and option names
  printSubGroupHeaderWithHeaderWidth (
    os,
    subGroupHeaderWidth);

  // print the description if any
  if (fDescription.size ()) {
    gIndenter++;
    os <<
      gIndenter.indentMultiLineString (
        fDescription);
    gIndenter--;

    os << endl;
  }

  // print the options atoms if relevant
  switch (fOptionVisibilityKind) {
    case kElementVisibilityAlways:
      if (fAtomsList.size ()) {
        gIndenter++;

        list<S_oahAtom>::const_iterator
          iBegin = fAtomsList.begin (),
          iEnd   = fAtomsList.end (),
          i      = iBegin;
        for ( ; ; ) {
          S_oahAtom oahAtom = (*i);
          bool
            oahAtomIsHidden =
              oahAtom->getIsHidden ();

          // print the atom help unless it is hidden
          if (! oahAtomIsHidden) {
            oahAtom->
              printHelp (os);
          }
          if (++i == iEnd) break;
          if (! oahAtomIsHidden) {
   // JMI         os << endl;
          }
        } // for

        gIndenter--;
      }
      break;

    case kElementVisibilityHiddenByDefault:
      break;
  } // switch

  // register help print action in options groups's options handler upLink
  fGroupUpLink->
    getHandlerUpLink ()->
      setOptionsHandlerFoundAHelpOption ();
}

void oahSubGroup::printSubGroupHelp (ostream& os) const
{
  // print the header and option names
  printSubGroupHeader (os);

  // print the description if any
  if (fDescription.size ()) {
    gIndenter++;
    os <<
      gIndenter.indentMultiLineString (
        fDescription) <<
      endl;
    gIndenter--;

    os << endl;
  }

  if (fAtomsList.size ()) {
    gIndenter++;

    list<S_oahAtom>::const_iterator
      iBegin = fAtomsList.begin (),
      iEnd   = fAtomsList.end (),
      i      = iBegin;
    for ( ; ; ) {
      // print the atom help
      (*i)->printHelp (os);
      if (++i == iEnd) break;
  // JMI    os << endl;
    } // for

    gIndenter--;
  }

  os << endl;

  // register help print action in options groups's options handler upLink
  fGroupUpLink->
    getHandlerUpLink ()->
      setOptionsHandlerFoundAHelpOption ();

}

S_oahValuedAtom oahSubGroup::handleOptionUnderName (
  string   optionName,
  ostream& os)
{
  printSubGroupHelp (os);

  // no option value is needed
  return nullptr;
}

void oahSubGroup::printOptionsSummary (
  ostream& os) const
{
  // fetch maximum subgroups help headers size
  int maximumSubGroupsHelpHeadersSize =
    getGroupUpLink ()->
      getHandlerUpLink ()->
        getMaximumSubGroupsHeadersSize ();

  // fetch maximum short name width
  int maximumShortNameWidth =
    getGroupUpLink ()->
      getHandlerUpLink ()->
        getMaximumShortNameWidth ();

  // print the summary
  os << left <<
    setw (maximumSubGroupsHelpHeadersSize) <<
    fSubGroupHeader <<
    " " <<
    fetchNamesInColumnsBetweenParentheses (
      maximumShortNameWidth);

  switch (fOptionVisibilityKind) {
    case kElementVisibilityAlways:
      break;

    case kElementVisibilityHiddenByDefault:
      os <<
        " (hidden by default)";
      break;
  } // switch

  os << endl;

  // print the description if any
  if (fDescription.size ()) {
    gIndenter++;
    os <<
      gIndenter.indentMultiLineString (
        fDescription) <<
      endl;
    gIndenter--;
  }

  // register help print action in options groups's options handler upLink
  fGroupUpLink->
    getHandlerUpLink ()->
      setOptionsHandlerFoundAHelpOption ();
}

void oahSubGroup::printSubGroupSpecificHelpOrOptionsSummary (
  ostream&      os,
  S_oahSubGroup subGroup) const
{
#ifdef TRACE_OAH
  if (gExecutableOah->fTraceOah) {
    os << "oahSubGroup::printSubGroupSpecificHelpOrOptionsSummary" << endl;
  }
#endif

  // print only the summary if this is not the desired subgroup,
  // otherwise print the regular help
  if (subGroup == this) {
    os << endl;
    printSubGroupSpecificHelpOrOptionsSummary (
      os,
      subGroup);
  }
  else {
    printOptionsSummary (os);
  }
 }

void oahSubGroup::printSubGroupAndAtomHelp (
  ostream&  os,
  S_oahAtom targetAtom) const
{
  // print the header
  os <<
    fSubGroupHeader <<
    " " <<
    fetchNamesBetweenParentheses () <<
    ":" <<
    endl <<
    endl;

  // underline the options subgroup header
// JMI  underlineSubGroupHeader (os);

  // print the subgroup atoms
  if (fAtomsList.size ()) {
    gIndenter++;

    list<S_oahAtom>::const_iterator
      iBegin = fAtomsList.begin (),
      iEnd   = fAtomsList.end (),
      i      = iBegin;
    for ( ; ; ) {
      S_oahAtom oahAtom = (*i);

      if (oahAtom == targetAtom) {
        // print the target atom's help
        // target options atom's help
        (*i)->
          printHelp (
            os);
      }
      if (++i == iEnd) break;
      if (oahAtom == targetAtom) {
        os << endl;
      }
    } // for

    gIndenter--;
  }

  // register help print action in options groups's options handler upLink
  fGroupUpLink->
    getHandlerUpLink ()->
      setOptionsHandlerFoundAHelpOption ();
}

void oahSubGroup::printSubGroupOptionsValues (
  ostream& os,
  int      valueFieldWidth) const
{
  // print the header
  os <<
    fSubGroupHeader <<
    " " <<
    fetchNamesBetweenParentheses () <<
    ":" <<
    endl;

  // underline the options subgroup header
// JMI  underlineSubGroupHeader (os);

  // print the subgroup atoms values
  if (fAtomsList.size ()) {
    gIndenter++;

    list<S_oahAtom>::const_iterator
      iBegin = fAtomsList.begin (),
      iEnd   = fAtomsList.end (),
      i      = iBegin;
    for ( ; ; ) {
      // print the atom values
      (*i)->
        printAtomOptionsValues (
          os, valueFieldWidth);
      if (++i == iEnd) break;
  //    os << endl;
    } // for

    gIndenter--;
  }
}

ostream& operator<< (ostream& os, const S_oahSubGroup& elt)
{
  elt->print (os);
  return os;
}

//______________________________________________________________________________
S_oahGroup oahGroup::create (
  string                  header,
  string                  shortName,
  string                  longName,
  string                  description,
  oahOptionVisibilityKind optionVisibilityKind,
  S_oahHandler            groupHandlerUpLink)
{
  oahGroup* o = new
    oahGroup (
      header,
      shortName,
      longName,
      description,
      optionVisibilityKind,
      groupHandlerUpLink);
  assert(o!=0);
  return o;
}

oahGroup::oahGroup (
  string                  header,
  string                  shortName,
  string                  longName,
  string                  description,
  oahOptionVisibilityKind optionVisibilityKind,
  S_oahHandler            groupHandlerUpLink)
  : oahOption (
      shortName,
      longName,
      description,
      optionVisibilityKind)
{
  fHandlerUpLink = groupHandlerUpLink;

  fGroupHeader = header;
}

oahGroup::~oahGroup ()
{}

void oahGroup::underlineGroupHeader (ostream& os) const
{
  /* JMI
  for (unsigned int i = 0; i < fGroupHeader.size (); i++) {
    os << "-";
  } // for
  os << endl;
  */
  os << "--------------------------" << endl;
}

void oahGroup::registerOptionsGroupInHandler (
  S_oahHandler handler)
{
  // sanity check
  msrAssert (
    handler != nullptr,
    "handler is null");

  // set options handler upLink
  fHandlerUpLink = handler;

  // register options group in options handler
  handler->
    registerOptionInHandler (this);

  for (
    list<S_oahSubGroup>::const_iterator
      i = fSubGroupsList.begin ();
    i != fSubGroupsList.end ();
    i++
  ) {
    // register the options sub group
    (*i)->
      registerSubGroupInHandler (
        handler);
  } // for
}

void  oahGroup::appendSubGroup (
  S_oahSubGroup subGroup)
{
  // sanity check
  msrAssert (
    subGroup != nullptr,
    "subGroup is null");

  // append options subgroup
  fSubGroupsList.push_back (
    subGroup);

  // set options subgroup group upLink
  subGroup->
    setGroupUpLink (this);
}

S_oahOption oahGroup::fetchOptionByName (
  string name)
{
  S_oahOption result;

  for (
    list<S_oahSubGroup>::const_iterator
      i = fSubGroupsList.begin ();
    i != fSubGroupsList.end ();
    i++
  ) {
    // search name in the options group
    result =
      (*i)->fetchOptionByName (name);

    if (result != 0) {
      break;
    }
  } // for

  return result;
}

void oahGroup::handleAtomValue (
  ostream&  os,
  S_oahAtom atom,
  string    theString)
{
  os <<
    endl <<
    "---> Options atom '" <<
    atom <<
    "' with value '" <<
    theString <<
    "' is not handled" <<
    endl <<
    endl;
}

void oahGroup::checkOptionsConsistency ()
{}

void oahGroup::print (ostream& os) const
{
  const int fieldWidth = 27;

  os <<
    "Group:" <<
    endl;

  gIndenter++;

  oahOption::printOptionEssentials (
    os, fieldWidth);

  os <<
    "SubgroupsList (" <<
    singularOrPlural (
      fSubGroupsList.size (), "element",  "elements") <<
    "):" <<
    endl;

  if (fSubGroupsList.size ()) {
    os << endl;

    gIndenter++;

    list<S_oahSubGroup>::const_iterator
      iBegin = fSubGroupsList.begin (),
      iEnd   = fSubGroupsList.end (),
      i      = iBegin;
    for ( ; ; ) {
      // print the options subgroup
      os << (*i);
      if (++i == iEnd) break;
      os << endl;
    } // for

    gIndenter--;
  }

  gIndenter--;
}

void oahGroup::printGroupHeader (ostream& os) const
{
  // print the header and option names
  os <<
    fGroupHeader <<
    " " <<
    fetchNamesBetweenParentheses ();

  switch (fOptionVisibilityKind) {
    case kElementVisibilityAlways:
      os <<
        ":";
      break;

    case kElementVisibilityHiddenByDefault:
      os <<
        " (hidden by default)";
      break;
  } // switch

  os << endl;
}

void oahGroup::printHelp (ostream& os)
{
  // print the header and option names
  printGroupHeader (os);

  // print the description if any
  if (fDescription.size ()) {
    gIndenter++;
    os <<
      gIndenter.indentMultiLineString (
        fDescription) <<
      endl;
    gIndenter--;
  }

  // underline the options group header
  underlineGroupHeader (os);

  // print the options subgroups
  if (fSubGroupsList.size ()) {
    gIndenter++;

    list<S_oahSubGroup>::const_iterator
      iBegin = fSubGroupsList.begin (),
      iEnd   = fSubGroupsList.end (),
      i      = iBegin;
    for ( ; ; ) {
      // print the options subgroup help
      (*i)->printHelp (os);
      if (++i == iEnd) break;
  // JMI    os << endl;
    } // for

    gIndenter--;
  }

  // register help print action in options handler upLink
  fHandlerUpLink->setOptionsHandlerFoundAHelpOption ();
}

void oahGroup::printGroupAndSubGroupHelp (
  ostream&      os,
  S_oahSubGroup targetSubGroup) const
{

  // print the header and option names
  os <<
    fGroupHeader <<
    " " <<
    fetchNamesBetweenParentheses () <<
    ":" <<
    endl;

  // print the description if any
  if (fDescription.size ()) {
    gIndenter++;
    os <<
      gIndenter.indentMultiLineString (
        fDescription) <<
      endl;
    gIndenter--;

    os << endl;
  }

  // underline the options group header
  underlineGroupHeader (os);

  // print the target options subgroup
  if (fSubGroupsList.size ()) {
    gIndenter++;

    list<S_oahSubGroup>::const_iterator
      iBegin = fSubGroupsList.begin (),
      iEnd   = fSubGroupsList.end (),
      i      = iBegin;
    for ( ; ; ) {
      S_oahSubGroup
        subGroup = (*i);

      if (subGroup == targetSubGroup) {
        // print the target options subgroup help
        subGroup->
          printSubGroupHelp (
            os);
      }
      if (++i == iEnd) break;
      if (subGroup == targetSubGroup) {
        os << endl;
      }
    } // for

    gIndenter--;
  }
}

void oahGroup::printGroupAndSubGroupAndAtomHelp (
  ostream&      os,
  S_oahSubGroup targetSubGroup,
  S_oahAtom     targetAtom) const
{
  // print the header and option names
  os <<
    fGroupHeader <<
    " " <<
    fetchNamesBetweenParentheses () <<
    ":" <<
    endl;

  // print the description if any
  if (fDescription.size ()) {
    gIndenter++;
    os <<
      gIndenter.indentMultiLineString (
        fDescription) <<
      endl;
    gIndenter--;

    os << endl;
  }

  // underline the options group header
  underlineGroupHeader (os);

  // print the target options subgroup
  if (fSubGroupsList.size ()) {
    gIndenter++;

    list<S_oahSubGroup>::const_iterator
      iBegin = fSubGroupsList.begin (),
      iEnd   = fSubGroupsList.end (),
      i      = iBegin;
    for ( ; ; ) {
      S_oahSubGroup subGroup = (*i);

      if (subGroup == targetSubGroup) {
        // print the target options subgroup's
        // target options targetAtom's help
        subGroup->
          printSubGroupAndAtomHelp (
            os,
            targetAtom);
      }
      if (++i == iEnd) break;
      if (subGroup == targetSubGroup) {
        os << endl;
      }
    } // for

    os << endl;

    gIndenter--;
  }

  // register help print action in options handler upLink
  fHandlerUpLink->setOptionsHandlerFoundAHelpOption ();
}

S_oahValuedAtom oahGroup::handleOptionUnderName (
  string   optionName,
  ostream& os)
{
  printHelp (os);

  // no option value is needed
  return nullptr;
}

void oahGroup::printOptionsSummary (ostream& os) const
{
  // the description is the header of the information
  os <<
    fGroupHeader <<
    " " <<
    fetchNamesBetweenParentheses () <<
    ":" <<
    endl;

  // print the description if any
  if (fDescription.size ()) {
    gIndenter++;
    os <<
      gIndenter.indentMultiLineString (
        fDescription) <<
      endl;
    gIndenter--;
  }

  // underline the options group header
  underlineGroupHeader (os);

  // print the options subgroups
  if (fSubGroupsList.size ()) {
    gIndenter++;

    list<S_oahSubGroup>::const_iterator
      iBegin = fSubGroupsList.begin (),
      iEnd   = fSubGroupsList.end (),
      i      = iBegin;
    for ( ; ; ) {
      // print the options subgroup description
      (*i)->
        printOptionsSummary (os);
      if (++i == iEnd) break;
 //     os << endl;
    } // for

    gIndenter--;
  }

  // register help print action in options handler upLink
  fHandlerUpLink->setOptionsHandlerFoundAHelpOption ();
}

void oahGroup::printGroupAndSubGroupSpecificHelp (
  ostream&      os,
  S_oahSubGroup subGroup) const
{
#ifdef TRACE_OAH
  if (gExecutableOah->fTraceOah) {
    os << "oahGroup::printGroupAndSubGroupSpecificHelp" << endl;
  }
#endif

  // the description is the header of the information
  os <<
    fGroupHeader <<
    " " <<
    fetchNamesBetweenParentheses () <<
    ":" <<
    endl;

  // print the description if any
  if (fDescription.size ()) {
    gIndenter++;
    os <<
      gIndenter.indentMultiLineString (
        fDescription) <<
      endl;
    gIndenter--;
  }

  // underline the options group header
  underlineGroupHeader (os);

  // print the options subgroups
  if (fSubGroupsList.size ()) {
    gIndenter++;

    list<S_oahSubGroup>::const_iterator
      iBegin = fSubGroupsList.begin (),
      iEnd   = fSubGroupsList.end (),
      i      = iBegin;
    for ( ; ; ) {
      S_oahSubGroup subGroup = (*i);

      // print the options subgroup specific subgroup help
      subGroup->
        printSubGroupSpecificHelpOrOptionsSummary (
          os,
          subGroup);
      if (++i == iEnd) break;
      os << endl;
    } // for

    gIndenter--;
  }

  // register help print action in options handler upLink
  fHandlerUpLink->setOptionsHandlerFoundAHelpOption ();
}

void oahGroup::printGroupOptionsValues (
  ostream& os,
  int      valueFieldWidth) const
{
  // print the header
  os <<
    fGroupHeader <<
    " " <<
    fetchNamesBetweenParentheses () <<
    ":" <<
    endl;

  // underline the options group header
  underlineGroupHeader (os);

  // print the options subgroups values
  if (fSubGroupsList.size ()) {
    gIndenter++;

    list<S_oahSubGroup>::const_iterator
      iBegin = fSubGroupsList.begin (),
      iEnd   = fSubGroupsList.end (),
      i      = iBegin;
    for ( ; ; ) {
      // print the options subgroup values
      (*i)->
        printSubGroupOptionsValues (
          os, valueFieldWidth);
      if (++i == iEnd) break;
      os << endl;
    } // for

    gIndenter--;
  }
}

ostream& operator<< (ostream& os, const S_oahGroup& elt)
{
  elt->print (os);
  return os;
}

//______________________________________________________________________________
S_oahPrefix oahPrefix::create (
  string prefixName,
  string prefixErsatz,
  string prefixDescription)
{
  oahPrefix* o = new
    oahPrefix (
      prefixName,
      prefixErsatz,
      prefixDescription);
  assert(o!=0);
  return o;
}

oahPrefix::oahPrefix (
  string prefixName,
  string prefixErsatz,
  string prefixDescription)
{
  fPrefixName        = prefixName;
  fPrefixErsatz      = prefixErsatz;
  fPrefixDescription = prefixDescription;
}

oahPrefix::~oahPrefix ()
{}

/* JMI
S_oahPrefix oahPrefix::fetchOptionByName (
  string name)
{
  S_oahPrefix result;

  if (name == fPrefixName) {
    result = this;
  }

  return result;
}
*/

string oahPrefix::prefixNames () const
{
  stringstream s;

  if (fPrefixName.size ()) {
      s <<
        "-" << fPrefixName;
  }

  return s.str ();
}

string oahPrefix::prefixNamesInColumns (
  int subGroupsShortNameFieldWidth) const
{
  stringstream s;

  if (fPrefixName.size ()) {
      s << left <<
        setw (subGroupsShortNameFieldWidth) <<
        "-" + fPrefixName;
  }

  return s.str ();
}

string oahPrefix::prefixNamesBetweenParentheses () const
{
  stringstream s;

  s <<
    "(" <<
    prefixNames () <<
    ")";

  return s.str ();
}

string oahPrefix::prefixNamesInColumnsBetweenParentheses (
  int subGroupsShortNameFieldWidth) const
{
  stringstream s;

  s <<
    "(" <<
    prefixNamesInColumns (
      subGroupsShortNameFieldWidth) <<
    ")";

  return s.str ();
}

void oahPrefix::printPrefixHeader (ostream& os) const
{
  os <<
    "'" << fPrefixName <<
    "' translates to '" << fPrefixErsatz <<
    "':" <<
    endl;

  if (fPrefixDescription.size ()) {
    // indent a bit more for readability
    gIndenter++;

    os <<
      gIndenter.indentMultiLineString (
        fPrefixDescription) <<
      endl;

    gIndenter--;
  }
}

void oahPrefix::printPrefixEssentials (
  ostream& os,
  int      fieldWidth) const
{
  os << left <<
    setw (fieldWidth) <<
    "prefixName" << " : " <<
    fPrefixName <<
    endl <<
    setw (fieldWidth) <<
    "prefixErsatz" << " : " <<
    fPrefixErsatz <<
    endl <<
    setw (fieldWidth) <<
    "prefixDescription" << " : " <<
    fPrefixDescription <<
    endl;
}

void oahPrefix::print (ostream& os) const
{
  os <<
    "??? oahPrefix ???" <<
    endl;

  printPrefixEssentials (os, 40); // JMI
}

void oahPrefix::printHelp (ostream& os)
{
  os <<
    prefixNames () <<
    endl;

  if (fPrefixErsatz.size ()) {
    // indent a bit more for readability
    gIndenter.increment (K_OPTIONS_ELEMENTS_INDENTER_OFFSET);

    os <<
      gIndenter.indentMultiLineString (
        fPrefixErsatz) <<
      endl;

    gIndenter.decrement (K_OPTIONS_ELEMENTS_INDENTER_OFFSET);
  }

  if (fPrefixDescription.size ()) {
    // indent a bit more for readability
    gIndenter.increment (K_OPTIONS_ELEMENTS_INDENTER_OFFSET);

    os <<
      gIndenter.indentMultiLineString (
        fPrefixDescription) <<
      endl;

    gIndenter.decrement (K_OPTIONS_ELEMENTS_INDENTER_OFFSET);
  }

  // register help print action in options handler upLink
//  fHandlerUpLink->setOptionsHandlerFoundAHelpOption ();
}

ostream& operator<< (ostream& os, const S_oahPrefix& elt)
{
  elt->print (os);
  return os;
}

//______________________________________________________________________________
/* JMI
S_oahHandler oahHandler::create (
  string           handlerHeader,
  string           handlerValuesHeader,
  string           handlerShortName,
  string           handlerLongName,
  string           handlerSummaryShortName,
  string           handlerSummaryLongName,
  string           handlerPreamble,
  string           handlerDescription,
  indentedOstream& handlerLogOstream)
{
  oahHandler* o = new
    oahHandler (
      handlerHeader,
      handlerValuesHeader,
      handlerShortName,
      handlerLongName,
      handlerSummaryShortName,
      handlerSummaryLongName,
      handlerPreamble,
      handlerDescription,
      handlerLogOstream);
  assert(o!=0);
  return o;
}
*/

oahHandler::oahHandler (
  string           handlerHeader,
  string           handlerValuesHeader,
  string           handlerShortName,
  string           handlerLongName,
  string           handlerSummaryShortName,
  string           handlerSummaryLongName,
  string           handlerPreamble,
  string           handlerDescription,
  indentedOstream& handlerLogOstream)
  : oahOption (
      handlerShortName,
      handlerLongName,
      handlerDescription,
      kElementVisibilityAlways),
    fHandlerLogOstream (
      handlerLogOstream)
{
  fHandlerHeader =
    handlerHeader;

  fHandlerValuesHeader =
    handlerValuesHeader;

  fHandlerSummaryShortName =
    handlerSummaryShortName;
  fHandlerSummaryLongName =
    handlerSummaryLongName;

  fHandlerPreamble =
    handlerPreamble;

  fMaximumSubGroupsHeadersSize = 1;

  fMaximumShortNameWidth   = 1;
  fMaximumLongNameWidth    = 1;

  fMaximumVariableNameWidth = 0;

  fHandlerFoundAHelpOption = false;
}

oahHandler::~oahHandler ()
{}

void oahHandler::registerHandlerInItself ()
{
  this->
    registerOptionInHandler (this);

/* JMI ???
  // register the help summary names in handler
  registerOptionNamesInHandler (
    fHandlerSummaryShortName,
    fHandlerSummaryLongName,
    this);
*/

//* JMI
  for (
    list<S_oahGroup>::const_iterator
      i = fHandlerGroupsList.begin ();
    i != fHandlerGroupsList.end ();
    i++
  ) {
    // register the options group
    (*i)->
      registerOptionsGroupInHandler (
        this);
  } // for
 // */
}

S_oahPrefix oahHandler::fetchPrefixFromMap (
  string name) const
{
  S_oahPrefix result;

  // is name known in prefixes map?
  map<string, S_oahPrefix>::const_iterator
    it =
      fHandlerPrefixesMap.find (
        name);

  if (it != fHandlerPrefixesMap.end ()) {
    // yes, name is known in the map
    result = (*it).second;
  }

  return result;
}

S_oahOption oahHandler::fetchOptionFromMap (
  string name) const
{
  S_oahOption result;

  // is name known in options map?
  map<string, S_oahOption>::const_iterator
    it =
      fHandlerOptionsMap.find (
        name);

  if (it != fHandlerOptionsMap.end ()) {
    // yes, name is known in the map
    result = (*it).second;
  }

  return result;
}

string oahHandler::handlerOptionNamesBetweenParentheses () const
{
  stringstream s;

  s <<
    "(" <<
    fetchNames () <<
    ", ";

  if (
    fHandlerSummaryShortName.size ()
        &&
    fHandlerSummaryLongName.size ()
    ) {
      s <<
        "-" << fHandlerSummaryShortName <<
        ", " <<
        "-" << fHandlerSummaryLongName;
  }

  else {
    if (fHandlerSummaryShortName.size ()) {
      s <<
      "-" << fHandlerSummaryShortName;
    }
    if (fHandlerSummaryLongName.size ()) {
      s <<
      "-" << fHandlerSummaryLongName;
    }
  }

  s <<
    ")";

  return s.str ();
}

void oahHandler::registerOptionNamesInHandler (
  string      optionShortName,
  string      optionLongName,
  S_oahOption option)
{
  int
    optionShortNameSize =
      optionShortName.size (),
    optionLongNameSize =
      optionLongName.size ();

  if (optionShortNameSize == 0 && optionLongNameSize == 0) {
    stringstream s;

    s <<
      "option long name and short name are both empty";

    optionError (s.str ());
    exit (33);
  }

  if (optionShortName == optionLongName) {
    stringstream s;

    s <<
      "option long name '" << optionLongName << "'" <<
      " is the same as the short name for the same";

    optionError (s.str ());
    exit (33);
  }

  for (
    map<string, S_oahOption>::iterator i =
      fHandlerOptionsMap.begin ();
    i != fHandlerOptionsMap.end ();
    i++
  ) {

    // is optionLongName already in the options names map?
    if ((*i).first == optionLongName) {
      stringstream s;

      s <<
        "option long name '" << optionLongName << "'" <<
          " for option short name '" << optionShortName << "'" <<
        " is specified more that once";

      optionError (s.str ());
      exit (33);
    }

    // is optionShortName already in the options names map?
    if ((*i).first == optionShortName) {
      if (optionShortName.size ()) {
        stringstream s;

        s <<
          "option short name '" << optionShortName << "'" <<
          " for option long name '" << optionLongName << "'" <<
          " is specified more that once";

        optionError (s.str ());
        exit (33);
      }
    }
  } // for

  // register option's names
  if (optionLongNameSize) {
    fHandlerOptionsMap [optionLongName] =
      option;

    if (optionLongNameSize > fMaximumLongNameWidth) {
      fMaximumLongNameWidth = optionLongNameSize;
    }
  }

  if (optionShortNameSize) {
    fHandlerOptionsMap [optionShortName] =
      option;

    if (optionShortNameSize > fMaximumShortNameWidth) {
      fMaximumShortNameWidth = optionShortNameSize;
    }
  }

  // take option's display variable name length into account
  int
    oahOptionVariableNameLength =
      option->
        fetchVariableNameLength ();

    if (
      oahOptionVariableNameLength
        >
      fMaximumVariableNameWidth
    ) {
      fMaximumVariableNameWidth =
        oahOptionVariableNameLength;
    }
}

void oahHandler::registerOptionInHandler (
  S_oahOption option)
{
  string
    optionShortName =
      option->getShortName (),
    optionLongName =
      option->getLongName ();

  // register the option names in handler
  registerOptionNamesInHandler (
    optionShortName,
    optionLongName,
    option);

  if (
    // subgroup?
    S_oahSubGroup
      subGroup =
        dynamic_cast<oahSubGroup*>(&(*option))
    ) {

    string
      subHeader=
        subGroup-> getSubGroupHeader ();
    int
      subHeaderSize =
        subHeader.size ();

    // account for subGroup's header size
    if (subHeaderSize > fMaximumSubGroupsHeadersSize) {
      fMaximumSubGroupsHeadersSize =
        subHeaderSize;
    }
  }
}

void oahHandler::print (ostream& os) const
{
  const int fieldWidth = 27;

  os <<
    "Handler:" <<
    endl;

  gIndenter++;

  printOptionEssentials (os, fieldWidth);

  os << left <<
    setw (fieldWidth) <<
    "fHandlerSummaryShortName" << " : " <<
    fHandlerSummaryShortName <<
    endl <<
    setw (fieldWidth) <<
    "fHandlerSummaryLongName" << " : " <<
    fHandlerSummaryLongName <<
    endl <<
    setw (fieldWidth) <<
    "fShortName" << " : " <<
    fShortName <<
    endl <<
    setw (fieldWidth) <<
    "fLongName" << " : " <<
    fLongName <<
    endl <<
    setw (fieldWidth) <<
    "oahHandlerFoundAHelpOption" << " : " <<
    fHandlerFoundAHelpOption <<
    endl;

  // print the options prefixes if any
  if (fHandlerPrefixesMap.size ()) {
    printKnownPrefixes ();
  }

  // print the options groups if any
  if (fHandlerGroupsList.size ()) {
    os << endl;

    gIndenter++;

    list<S_oahGroup>::const_iterator
      iBegin = fHandlerGroupsList.begin (),
      iEnd   = fHandlerGroupsList.end (),
      i      = iBegin;
    for ( ; ; ) {
      // print the options group
      os << (*i);
      if (++i == iEnd) break;
      os << endl;
    } // for

    gIndenter--;
  }

  gIndenter--;
}

void oahHandler::printHelp (ostream& os)
{
  // print the options handler preamble
  os <<
    gIndenter.indentMultiLineString (
      fHandlerPreamble);

  os << endl;

  // print the options handler help header and option names
  os <<
    fHandlerHeader <<
    " " <<
    fetchNamesBetweenParentheses () <<
    ":" <<
    endl;

  // print the options handler description
  gIndenter++;
  os <<
    gIndenter.indentMultiLineString (
      fDescription);
  gIndenter--;

  os <<
    endl <<
    endl;

  // print the known options prefixes
  gIndenter++;
  printKnownPrefixes ();
  gIndenter--;

  // print information about options default values
  gIndenter++;
  printOptionsDefaultValuesInformation ();
  gIndenter--;

  os << endl;

  // print the options groups help
  if (fHandlerGroupsList.size ()) {
    gIndenter++;

    list<S_oahGroup>::const_iterator
      iBegin = fHandlerGroupsList.begin (),
      iEnd   = fHandlerGroupsList.end (),
      i      = iBegin;
    for ( ; ; ) {
      S_oahGroup group = (*i);

      // print the options group help
//      group->printHelp (os);

      // print the options subgroups if relevant
      switch (group->getOptionVisibilityKind ()) {
        case kElementVisibilityAlways:
          group->printHelp (os);
          break;

        case kElementVisibilityHiddenByDefault:
          group->printGroupHeader (os);
          group->underlineGroupHeader(os);
          break;
      } // switch

      if (++i == iEnd) break;
      os << endl;
    } // for

    gIndenter--;
  }

  // register help print action in options handler upLink
//  fHandlerUpLink->setOptionsHandlerFoundAHelpOption ();
}

void oahHandler::printOptionsSummary (ostream& os) const
{
  // print the options handler preamble
  os <<
    gIndenter.indentMultiLineString (
      fHandlerPreamble);

  // print the options handler help header and option names
  os <<
    fHandlerHeader <<
    " " <<
    fetchNamesBetweenParentheses () <<
    ":" <<
    endl;

  // print the options handler description
  gIndenter++;
  os <<
    gIndenter.indentMultiLineString (
      fDescription) <<
    endl;
  gIndenter--;

  // print the options groups help summaries
  if (fHandlerGroupsList.size ()) {
    gIndenter++;

    list<S_oahGroup>::const_iterator
      iBegin = fHandlerGroupsList.begin (),
      iEnd   = fHandlerGroupsList.end (),
      i      = iBegin;
    for ( ; ; ) {
      // print the options group summary
      (*i)->printOptionsSummary (os);
      if (++i == iEnd) break;
      os << endl;
    } // for

    gIndenter--;
  }
}

void oahHandler::printHandlerAndGroupAndSubGroupSpecificHelp (
  ostream&      os,
  S_oahSubGroup subGroup) const
{
#ifdef TRACE_OAH
  if (gExecutableOah->fTraceOah) {
    os << "oahHandler::printHandlerAndGroupAndSubGroupSpecificHelp" << endl;
  }
#endif

  // print the options handler help header and option names
  os <<
    fHandlerHeader <<
    " " <<
    fetchNamesBetweenParentheses () <<
    ":" <<
    endl;

  // print the optons group subgroups specific help
  if (fHandlerGroupsList.size ()) {
    gIndenter++;

    list<S_oahGroup>::const_iterator
      iBegin = fHandlerGroupsList.begin (),
      iEnd   = fHandlerGroupsList.end (),
      i      = iBegin;
    for ( ; ; ) {
      S_oahGroup group = (*i);

      // print the options group specific subgroup help
      group->
        printGroupAndSubGroupSpecificHelp (
          os,
          subGroup);
      if (++i == iEnd) break;
      os << endl;
    } // for

    gIndenter--;
  }
}

void oahHandler::printOptionSpecificHelp (
  ostream& os,
  string   name) const
{
#ifdef TRACE_OAH
  if (gExecutableOah->fTraceOah) {
    os << "oahHandler::printOptionSpecificHelp" << endl;
  }
#endif

  // is name known in options map?
  S_oahOption
    option =
      fetchOptionFromMap (name);

  if (! option) {
    // name is is not well handled by this options handler
    stringstream s;

    s <<
      "option name '" << name <<
      "' is unknown, cannot deliver specific help";

    optionError (s.str ());
    exit (33);
  }

  else {
    // name is known, let's handle it
    if (
      // handler?
      S_oahHandler
        handler =
          dynamic_cast<oahHandler*>(&(*option))
    ) {
      // print the option handler help or help summary
      if (
        name == handler->getHandlerSummaryShortName ()
          ||
        name == handler->getHandlerSummaryLongName ()
     ) {
        handler->
          printOptionsSummary (
            fHandlerLogOstream);
      }
      else {
        handler->
          printHelp (
            fHandlerLogOstream);
      }

      fHandlerLogOstream << endl;
    }

    else if (
      // group?
      S_oahGroup
        group =
          dynamic_cast<oahGroup*>(&(*option))
    ) {
      // print the help
      fHandlerLogOstream <<
        endl <<
        "--- Help for option '" <<
        name <<
        "' at help top level ---" <<
        endl <<
        endl;

      group->
        printHelp (
          fHandlerLogOstream);

      fHandlerLogOstream << endl;
    }

    else if (
      // subgroup?
      S_oahSubGroup
        subGroup =
          dynamic_cast<oahSubGroup*>(&(*option))
    ) {
      // get the options group upLink
      S_oahGroup
        group =
          subGroup->
            getGroupUpLink ();

      // print the help
      fHandlerLogOstream <<
        endl <<
        "--- Help for option '" <<
        name <<
        "' in group \"" <<
        group->getGroupHeader () <<
        "\" ---" <<
        endl <<
        endl;

      group->
        printGroupAndSubGroupHelp (
          fHandlerLogOstream,
          subGroup);
    }

    else if (
      // atom?
      S_oahAtom
        atom =
          dynamic_cast<oahAtom*>(&(*option))
    ) {
      // get the subgroup upLink
      S_oahSubGroup
        subGroup =
          atom->
            getSubGroupUpLink ();

      // get the group upLink
      S_oahGroup
        group =
          subGroup->
            getGroupUpLink ();

      // print the help
      fHandlerLogOstream <<
        endl <<
        "--- Help for option '" <<
        name <<
        "' in subgroup \"" <<
        subGroup->
          getSubGroupHeader () <<
        "\"" <<
        " of group \"" <<
        group->
          getGroupHeader () <<
        "\" ---" <<
        endl <<
        endl;

      group->
        printGroupAndSubGroupAndAtomHelp (
          fHandlerLogOstream,
          subGroup,
          atom);
    }

    else {
      stringstream s;

      s <<
        "cannot provide specific help about option name \"" <<
        name <<
        "\"";

      optionError (s.str ());
      exit (33);
    }
  }
}

void oahHandler::printAllOahValues (
  ostream& os) const
{
  // print the options handler values header
  os <<
    fHandlerValuesHeader <<
    " " <<
    fetchNamesBetweenParentheses () <<
    ":" <<
    endl;

  // print the options groups values
  if (fHandlerGroupsList.size ()) {
    os << endl;

    gIndenter++;

    list<S_oahGroup>::const_iterator
      iBegin = fHandlerGroupsList.begin (),
      iEnd   = fHandlerGroupsList.end (),
      i      = iBegin;
    for ( ; ; ) {
      // print the options group values
      (*i)->
        printGroupOptionsValues (
          os, fMaximumVariableNameWidth);
      if (++i == iEnd) break;
      os << endl;
    } // for

    gIndenter--;
  }
}

ostream& operator<< (ostream& os, const S_oahHandler& elt)
{
  elt->print (os);
  return os;
}

void oahHandler::appendPrefixToHandler (
  S_oahPrefix prefix)
{
  // sanity check
  msrAssert (
    prefix != nullptr,
    "prefix is null");

  string prefixName = prefix->getPrefixName ();

  S_oahPrefix result;

  // is prefixName already known in options map?
  map<string, S_oahPrefix>::const_iterator
    it =
      fHandlerPrefixesMap.find (
        prefixName);

  if (it != fHandlerPrefixesMap.end ()) {
    // prefixName is already known in the map
    stringstream s;

    s <<
      "option prefix name '" << prefixName <<
      "' is already known";

    optionError (s.str ());
    exit (7);
  }

  // register prefix in the options prefixes map
  fHandlerPrefixesMap [prefix->getPrefixName ()] =
    prefix;
}

void oahHandler::appendGroupToHandler (
  S_oahGroup oahGroup)
{
  // sanity check
  msrAssert (
    oahGroup != nullptr,
    "oahGroup is null");

  // append the options group
  fHandlerGroupsList.push_back (
    oahGroup);

  // set the upLink
  oahGroup->
    setHandlerUpLink (this);
}

void oahHandler::printKnownPrefixes () const
{
  int oahHandlerPrefixesListSize =
    fHandlerPrefixesMap.size ();

  fHandlerLogOstream <<
    "There are " <<
    oahHandlerPrefixesListSize <<
    " options prefixes:" <<
    endl;

  gIndenter++;

  if (oahHandlerPrefixesListSize) {
    // indent a bit more for readability
    for (
      map<string, S_oahPrefix>::const_iterator i =
        fHandlerPrefixesMap.begin ();
      i != fHandlerPrefixesMap.end ();
      i++
    ) {
      S_oahPrefix
        prefix = (*i).second;

      prefix->
        printPrefixHeader (
          fHandlerLogOstream);
    } // for
  }
  else {
    fHandlerLogOstream <<
      "none" <<
      endl;
  }

  gIndenter--;
}

void oahHandler::printOptionsDefaultValuesInformation () const
{
  fHandlerLogOstream <<
    endl <<
    "Some options needing a value can use a default value:" <<
    endl;

  gIndenter++;

  fHandlerLogOstream  <<
    gIndenter.indentMultiLineString (
R"(this is possible only if such an option is followed by another option
or it is the last one in the command line;
otherwise the following argument, typically a file name,
would be taken as the option value.)") <<
    endl;

  gIndenter--;
}

string oahHandler::commandLineWithShortNamesAsString () const
{
  stringstream s;

  s <<
    fHandlerExecutableName;

  if (fHandlerArgumentsVector.size ()) {
    s << " ";

    vector<string>::const_iterator
      iBegin = fHandlerArgumentsVector.begin (),
      iEnd   = fHandlerArgumentsVector.end (),
      i      = iBegin;
    for ( ; ; ) {
      s << (*i);

      if (++i == iEnd) break;

      s << " ";
    } // for
  }

  if (fHandlerOptionsList.size ()) {
    s << " ";

    list<S_oahOption>::const_iterator
      iBegin = fHandlerOptionsList.begin (),
      iEnd   = fHandlerOptionsList.end (),
      i      = iBegin;
    for ( ; ; ) {
      S_oahOption option = (*i);

      s <<
        option-> asShortNamedOptionString ();

      if (++i == iEnd) break;

      s << " ";
    } // for
  }

  return s.str ();
}

string oahHandler::commandLineWithLongNamesAsString () const
{
  stringstream s;

  s <<
    fHandlerExecutableName;

  if (fHandlerArgumentsVector.size ()) {
    s << " ";

    vector<string>::const_iterator
      iBegin = fHandlerArgumentsVector.begin (),
      iEnd   = fHandlerArgumentsVector.end (),
      i      = iBegin;
    for ( ; ; ) {
      s << (*i);

      if (++i == iEnd) break;

      s << " ";
    } // for
  }

  if (fHandlerOptionsList.size ()) {
    s << " ";

    list<S_oahOption>::const_iterator
      iBegin = fHandlerOptionsList.begin (),
      iEnd   = fHandlerOptionsList.end (),
      i      = iBegin;
    for ( ; ; ) {
      S_oahOption option = (*i);

      s <<
        option-> asLongNamedOptionString ();

      if (++i == iEnd) break;

      s << " ";
    } // for
  }

  return s.str ();
}

void oahHandler::printKnownOptions () const
{
  int handlerOptionsMapSize =
    fHandlerOptionsMap.size ();

  // print the options map
  fHandlerLogOstream <<
    "The " <<
    handlerOptionsMapSize <<
    " known options are:" <<
    endl;

  gIndenter++;

  if (handlerOptionsMapSize) {
    map<string, S_oahOption>::const_iterator
      iBegin = fHandlerOptionsMap.begin (),
      iEnd   = fHandlerOptionsMap.end (),
      i      = iBegin;
    for ( ; ; ) {
      fHandlerLogOstream <<
        (*i).first << "-->" <<
        endl;

      gIndenter++;

      (*i).second->
        printOptionHeader (
          fHandlerLogOstream);

      if (++i == iEnd) break;

      gIndenter--;
    } // for
  }
  else {
    fHandlerLogOstream <<
      "none" <<
      endl;
  }

  gIndenter--;
}

S_oahOption oahHandler::fetchOptionByName (
  string name)
{
  S_oahOption result;

  for (
    list<S_oahGroup>::const_iterator
      i = fHandlerGroupsList.begin ();
    i != fHandlerGroupsList.end ();
    i++
  ) {
    // search name in the options group
    result =
      (*i)->fetchOptionByName (name);

    if (result != 0) {
      break;
    }
  } // for

  return result;
}

void oahHandler::checkMissingPendingValuedAtomValue (
  string context)
{
  if (fPendingValuedAtom) {
    if (fPendingValuedAtom->getValueIsOptional ()) {
      // handle the valued atom using the default value
      fPendingValuedAtom->
        handleDefaultValue ();

      fPendingValuedAtom = nullptr;
    }
    else {
      // an option requiring a value is expecting it,
      // but another option, an argument or the end of the command line
      // has been found instead
      stringstream s;

      s <<
        "option " <<
       fPendingValuedAtom->asString () <<
       " expects a value" <<
       " (" << context << ")";

      optionError (s.str ());
      exit (9);
    }
  }
}

const vector<string> oahHandler::decipherOptionsAndArguments (
  int   argc,
  char* argv[])
{
// JMI  gExecutableOah->fTraceOah = true; // TEMP

  // fetch program name
  fHandlerExecutableName = string (argv [0]);

  // decipher the command options and arguments
  int n = 1;

  while (true) {
    if (argv [n] == 0)
      break;

    string currentOption = string (argv [n]);

#ifdef TRACE_OAH
    if (gExecutableOah->fTraceOah) {
      // print current option
      fHandlerLogOstream <<
        "Command line option " << n <<
        ": " << currentOption << " "<<
        endl;
    }
#endif

    // handle current option
    if (currentOption [0] == '-') {
      // stdin or option?

      if (currentOption.size () == 1) {
        // this is the stdin indicator
#ifdef TRACE_OAH
          if (gExecutableOah->fTraceOah) {
          fHandlerLogOstream <<
            "'" << currentOption <<
              "' is the '-' stdin indicator" <<
            endl;
        }
#endif

        fHandlerArgumentsVector.push_back (currentOption);
      }

      else {
        // this is an option
        string currentOptionName;

        string optionTrailer =
          currentOption.substr (1, string::npos);

        /* JMI
        fHandlerLogOstream <<
          "optionTrailer '" << optionTrailer << "' is preceded by a dash" <<
          endl;
        */

        if (optionTrailer.size ()) {
          if (optionTrailer [0] == '-') {
            // it is a double-dashed option
            currentOptionName =
              optionTrailer.substr (1, string::npos);

#ifdef TRACE_OAH
            if (gExecutableOah->fTraceOah) {
              fHandlerLogOstream <<
                "'" << currentOptionName << "' is a double-dashed option" <<
                endl;
            }
#endif
          }
          else {
            // it is a single-dashed option
            currentOptionName =
              optionTrailer; //.substr (1, string::npos);

#ifdef TRACE_OAH
            if (gExecutableOah->fTraceOah) {
              fHandlerLogOstream <<
                "'" << currentOptionName << "' is a single-dashed option" <<
                endl;
            }
#endif
          }
        }

        else {
#ifdef TRACE_OAH
          if (gExecutableOah->fTraceOah) {
            fHandlerLogOstream <<
              "'-' is the minimal single-dashed option" <<
              endl;
          }
#endif
        }

        // does currentOptionName contain an equal sign?
        size_t equalsSignPosition =
          currentOptionName.find ("=");

        if (equalsSignPosition != string::npos) {
          // yes

#ifdef TRACE_OAH
          if (gExecutableOah->fTraceOah) {
            printKnownPrefixes ();
          }
#endif

          // fetch the prefix name and the string after the equals sign
          string prefixName =
            currentOptionName.substr (0, equalsSignPosition);
          string stringAfterEqualsSign =
            currentOptionName.substr (equalsSignPosition + 1);


#ifdef TRACE_OAH
          if (gExecutableOah->fTraceOah) {
            fHandlerLogOstream <<
              "===> equalsSignPosition = '" << equalsSignPosition <<
              "', " <<
              "===> prefixName = '" << prefixName <<
              "', " <<
              "===> stringAfterEqualsSign = '" << stringAfterEqualsSign <<
              "'" <<
              endl;
          }
#endif

          // split stringAfterEqualsSign into a list of string
          // using the comma as separator
          list<string> chunksList;

          splitStringIntoChunks (
            stringAfterEqualsSign,
            ",",
            chunksList);

          unsigned chunksListSize = chunksList.size ();

#ifdef TRACE_OAH
          if (gExecutableOah->fTraceOah) {
            fHandlerLogOstream <<
              "There are " << chunksListSize << " chunks" <<
              " in '" << stringAfterEqualsSign <<
              "'" <<
              endl;

            gIndenter++;

            list<string>::const_iterator
              iBegin = chunksList.begin (),
              iEnd   = chunksList.end (),
              i      = iBegin;

            for ( ; ; ) {
              fHandlerLogOstream <<
                "[" << (*i) << "]";
              if (++i == iEnd) break;
              fHandlerLogOstream <<
                " ";
            } // for

            fHandlerLogOstream << endl;

            gIndenter--;
          }
#endif

          S_oahPrefix
            prefix =
              fetchPrefixFromMap (prefixName);

          if (prefix) {
            if (chunksListSize) {
              // expand the option names contained in chunksList
              for (
                list<string>::const_iterator i =
                  chunksList.begin ();
                i != chunksList.end ();
                i++
              ) {
                string singleOptionName = (*i);

                // build uncontracted option name
                string
                  uncontractedOptionName =
                    prefix->getPrefixErsatz () + singleOptionName;

#ifdef TRACE_OAH
                if (gExecutableOah->fTraceOah) {
                  fHandlerLogOstream <<
                    "Expanding option '" << singleOptionName <<
                    "' to '" << uncontractedOptionName <<
                    "'" <<
                    endl;
                }
#endif

                // handle the uncontracted option name
                handleOptionName (uncontractedOptionName);
              } // for
            }
          }

          else {
            printKnownPrefixes ();

            stringstream s;

            s <<
              "option prefix '" << prefixName <<
              "' is unknown, see help summary below";

            optionError (s.str ());
          }


        }

        else {
          // no
          // handle the current option name
          handleOptionName (currentOptionName);
        }
      }
    }

    else {
      // currentOption is no oahOption,
      // i.e. it is an atom value or an argument
      handleOptionValueOrArgument (currentOption);
    }

    // next please
    n++;
  } // while

  // is a pending valued atom value missing?
  checkMissingPendingValuedAtomValue (
    "last option in command line");

  unsigned int argumentsVectorSize =
    fHandlerArgumentsVector.size ();

#ifdef TRACE_OAH
  // display arc and argv only now, to wait for the options to have been handled
  if (
    gExecutableOah->fTraceOah
      ||
    gExecutableOah->fShowOptionsAndArguments
  ) {
    fHandlerLogOstream <<
      "argc: " << argc <<
      endl <<
      "argv:" <<
      endl;

    gIndenter++;

    for (int i = 0; i < argc; i++) {
      fHandlerLogOstream <<
        "argv [" << i << "]: " << argv [i] <<
        endl;
    } // for

    gIndenter--;
  }
#endif

#ifdef TRACE_OAH
  if (gExecutableOah->fTraceOahDetails) {
    printKnownPrefixes ();
    printKnownOptions ();
  }
#endif

#ifdef TRACE_OAH
  if (gExecutableOah->fTraceOah) {
    // print the arguments vector
    fHandlerLogOstream <<
      "Arguments vector (" <<
      argumentsVectorSize <<
      " elements):" <<
      endl;

    if (argumentsVectorSize) {
      gIndenter++;
      for (unsigned int i = 0; i < argumentsVectorSize; i++) {
        fHandlerLogOstream <<
          fHandlerArgumentsVector [i] <<
          endl;
      } // for
      gIndenter--;
    }
  }
#endif

  // was this run a 'pure help' one?
#ifdef TRACE_OAH
  if (gExecutableOah->fTraceOah) {
    fHandlerLogOstream <<
      "==> fHandlerFoundAHelpOption: " <<
      booleanAsString (fHandlerFoundAHelpOption) <<
      endl;
  }
#endif

  if (fHandlerFoundAHelpOption) {
    exit (0);
  }

  // exit if there are no arguments
  if (argumentsVectorSize == 0) {
// NO JMI ???    exit (1);
  }

  // check the options and arguments
  checkOptionsAndArguments ();

  // store the command line with options in gExecutableOah
  // for whoever need them
  gExecutableOah->fCommandLineWithShortOptionsNames =
      commandLineWithShortNamesAsString ();
  gExecutableOah->fCommandLineWithLongOptionsNames =
      commandLineWithLongNamesAsString ();

  // return arguments vector for handling by caller
  return fHandlerArgumentsVector;
}

void oahHandler::handleHandlerName (
  S_oahHandler handler,
  string       name)
{
  // print the handler help or help summary
#ifdef TRACE_OAH
  if (gExecutableOah->fTraceOah) {
    fHandlerLogOstream <<
      "==> option '" << name << "' is of type 'oahHandler'" <<
      endl;
  }
#endif

  if (
    name == handler->getHandlerSummaryShortName ()
      ||
    name == handler->getHandlerSummaryLongName ()
  ) {
    handler->
      printOptionsSummary (
        fHandlerLogOstream);
  }
  else {
    handler->
      printHelp (
        fHandlerLogOstream);
  }

  fHandlerLogOstream << endl;
}

void oahHandler::handleGroupName (
  S_oahGroup group,
  string     groupName)
{
#ifdef TRACE_OAH
  if (gExecutableOah->fTraceOah) {
    fHandlerLogOstream <<
      "==> option '" << groupName << "' is of type 'oahGroup'" <<
      endl;
  }
#endif

  // print the help
  fHandlerLogOstream <<
    endl <<
    "--- Help for group \"" <<
    group->
      getGroupHeader () <<
    "\" ---" <<
    endl <<
    endl;

  group->
    printHelp (
      fHandlerLogOstream);
}

void oahHandler::handleSubGroupName (
  S_oahSubGroup subGroup,
  string        subGroupName)
{
#ifdef TRACE_OAH
  if (gExecutableOah->fTraceOah) {
    fHandlerLogOstream <<
      "==> option '" << subGroupName << "' is of type 'subGroup'" <<
      endl;
  }
#endif

  // get the options group upLink
  S_oahGroup
    group =
      subGroup->
        getGroupUpLink ();

  // print the help
  fHandlerLogOstream <<
    endl <<
    "--- Help for subgroup \"" <<
    subGroup->
      getSubGroupHeader () <<
    "\"" <<
    " in group \"" <<
    group->
      getGroupHeader () <<
    "\" ---" <<
    endl <<
    endl;

  group->
    printGroupAndSubGroupHelp (
      fHandlerLogOstream,
      subGroup);
}

void oahHandler::handleAtomName (
  S_oahAtom atom,
  string    atomName)
{
#ifdef TRACE_OAH
  if (gExecutableOah->fTraceOah) {
    fHandlerLogOstream <<
      "==> option '" << atomName << "' is of type 'oahAtom'" <<
      endl;
  }
#endif

  if (
    // atom synonym?
    S_oahAtomSynonym
      atomSynonym =
        dynamic_cast<oahAtomSynonym*>(&(*atom))
  ) {
    // yes, use the original atom instead

    S_oahAtom
      originalOahAtom =
        atomSynonym->getOriginalOahAtom ();

#ifdef TRACE_OAH
  if (gExecutableOah->fTraceOah) {
    fHandlerLogOstream <<
      "==> option '" << atomName << "' is a synonym for '" <<
      originalOahAtom->asString () <<
      "'" <<
      endl;
  }
#endif

    this->handleAtomName (
      originalOahAtom,
      atomSynonym->getShortName ());
  }

  else {
    // is a pending valued atom value missing?
    string context =
      "before option " + atom->asString ();

    checkMissingPendingValuedAtomValue (
      context);

    // delegate the handling to the atom
    fPendingValuedAtom =
      atom->
        handleOptionUnderName (
          atomName,
          fHandlerLogOstream);
  }
}

void oahHandler::handleOptionName (
  string name)
{
#ifdef TRACE_OAH
  if (gExecutableOah->fTraceOah) {
    fHandlerLogOstream <<
      "==> handleOptionName (), name = \"" <<
      name <<
      "\"" <<
      endl;
  }
#endif

  // is name known in options map?
  S_oahOption
    option =
      fetchOptionFromMap (name);

  if (! option) {
    // name is is not well handled by this options handler
 // JMI   printOptionsSummary ();

    stringstream s;

    s <<
      "option name '" << name <<
      "' is unknown";

    optionError (s.str ());
    exit (6);
  }

  else {
    // name is known, let's handle it
#ifdef TRACE_OAH
  if (gExecutableOah->fTraceOah) {
    fHandlerLogOstream <<
      "==> handleOptionName (), name = \"" <<
      name <<
      "\" is described by option:" <<
      endl;
    gIndenter++;
    option->print (fHandlerLogOstream);
    gIndenter--;
  }
#endif

    // remember this option
    fHandlerOptionsList.push_back (option);

    // determine option short and long names to be used,
    // in case one of them (short or long) is empty
    string
      shortName =
        option->getShortName (),
      longName =
        option->getLongName ();

    string
      shortNameToBeUsed = shortName,
      longNameToBeUsed = longName;

    // replace empty option name if any by the other one,
    // since they can't both be empty
    if (! shortNameToBeUsed.size ()) {
      shortNameToBeUsed = longNameToBeUsed;
    }
    if (! longNameToBeUsed.size ()) {
      longNameToBeUsed = shortNameToBeUsed;
    }

    // handle the option
    if (
      // options handler?
      S_oahHandler
        handler =
          dynamic_cast<oahHandler*>(&(*option))
    ) {
        handleHandlerName (
          handler,
          name);
    }

    else if (
      // options group?
      S_oahGroup
        group =
          dynamic_cast<oahGroup*>(&(*option))
    ) {
      handleGroupName (
        group,
        name);
    }

    else if (
      // options subgroup?
      S_oahSubGroup
        subGroup =
          dynamic_cast<oahSubGroup*>(&(*option))
    ) {
      handleSubGroupName (
        subGroup,
        name);
    }

    else if (
      // atom?
      S_oahAtom
        atom =
          dynamic_cast<oahAtom*>(&(*option))
    ) {
      handleAtomName (
        atom,
        name);
    }

    else {
      stringstream s;

      s <<
        "INTERNAL ERROR: option name '" << name <<
        "' cannot be handled";

      optionError (s.str ());
      exit (7);
    }
  }
}

void oahHandler::handleOptionValueOrArgument (
  string theString)
{
#ifdef TRACE_OAH
  if (gExecutableOah->fTraceOah) {
    fHandlerLogOstream <<
      "==> handleOptionValueOrArgument ()" <<
      endl;

    gIndenter++;

    fHandlerLogOstream <<
      "fPendingValuedAtom:" <<
      endl;

    gIndenter++;
    if (fPendingValuedAtom) {
      fHandlerLogOstream <<
        fPendingValuedAtom;
    }
    else {
      fHandlerLogOstream <<
        "null" <<
        endl;
    }
    gIndenter--;

    fHandlerLogOstream <<
      "theString:" <<
      endl;

    gIndenter++;
    fHandlerLogOstream <<
      " \"" <<
      theString <<
      "\"" <<
      endl;
    gIndenter--;

    gIndenter--;
  }
#endif

  // options are handled at once, unless they are valued,
  // in which case the handling of the option and its value
  // are postponed until the latter is available
  if (fPendingValuedAtom) {
    // theString is the value for the pending valued atom
    fPendingValuedAtom->handleValue (
      theString,
      fHandlerLogOstream);

    fPendingValuedAtom = nullptr;
  }

  else {
    // theString is an argument

#ifdef TRACE_OAH
      if (gExecutableOah->fTraceOah) {
        fHandlerLogOstream <<
          "'" << theString << "'" <<
          " is an argument, not an option" <<
          endl;
      }
#endif

    fHandlerArgumentsVector.push_back (theString);
  }
}


}
