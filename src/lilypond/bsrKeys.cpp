/*
  MusicXML Library
  Copyright (C) Grame 2006-2013

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Grame Research Laboratory, 11, cours de Verdun Gensoul 69002 Lyon - France
  research@grame.fr
*/

#ifdef VC6
# pragma warning (disable : 4786)
#endif

#include <iostream>
#include <sstream>

#include "bsrKeys.h"

#include "setTraceOptionsIfDesired.h"
#ifdef TRACE_OPTIONS
  #include "traceOptions.h"
#endif

#include "bsrOptions.h"

#include "messagesHandling.h"


using namespace std;

namespace MusicXML2
{

//______________________________________________________________________________
S_bsrKey bsrKey::create (
  int        inputLineNumber,
  bsrKeyKind keyKind,
  int        numberOfAlterations)
{
  bsrKey* o =
    new bsrKey (
      inputLineNumber, keyKind, numberOfAlterations);
  assert (o!=0);

  return o;
}

bsrKey::bsrKey (
  int        inputLineNumber,
  bsrKeyKind keyKind,
  int        numberOfAlterations)
    : bsrElement (inputLineNumber)
{
  fKeyKind             = keyKind;
  fNumberOfAlterations = numberOfAlterations;
}

bsrKey::~bsrKey ()
{}

void bsrKey::acceptIn (basevisitor* v)
{
  if (gBsrOptions->fTraceBsrVisitors) {
    gLogIOstream <<
      "% ==> bsrKey::acceptIn ()" <<
      endl;
  }
      
  if (visitor<S_bsrKey>*
    p =
      dynamic_cast<visitor<S_bsrKey>*> (v)) {
        S_bsrKey elem = this;
        
        if (gBsrOptions->fTraceBsrVisitors) {
          gLogIOstream <<
            "% ==> Launching bsrKey::visitStart ()" <<
            endl;
        }
        p->visitStart (elem);
  }
}

void bsrKey::acceptOut (basevisitor* v)
{
  if (gBsrOptions->fTraceBsrVisitors) {
    gLogIOstream <<
      "% ==> bsrKey::acceptOut ()" <<
      endl;
  }

  if (visitor<S_bsrKey>*
    p =
      dynamic_cast<visitor<S_bsrKey>*> (v)) {
        S_bsrKey elem = this;
      
        if (gBsrOptions->fTraceBsrVisitors) {
          gLogIOstream <<
            "% ==> Launching bsrKey::visitEnd ()" <<
            endl;
        }
        p->visitEnd (elem);
  }
}

void bsrKey::browseData (basevisitor* v)
{}

string bsrKey::keyKindAsString (
  bsrKeyKind keyKind)
{
  string result;
 
  switch (keyKind) {
    case bsrKey::kKeyKindFlats:
      result = "keyKindFlats";
      break;
    case bsrKey::kKeyKindSharps:
      result = "keyKindSharps";
      break;
  } // switch

  return result;
}

string bsrKey::asString () const
{
  stringstream s;

  s <<
    "Key" <<
    ", " << keyKindAsString (fKeyKind) <<
    ", numberOfAlterations : " << fNumberOfAlterations <<
    ", line " << fInputLineNumber;

  return s.str ();
}

void bsrKey::print (ostream& os)
{
  os <<
    asString () <<
    endl;
}

ostream& operator<< (ostream& os, const S_bsrKey& elt)
{
  elt->print (os);
  return os;
}


}
