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

#include "bsrMutualDependencies.h"

#include "bsrOah.h"
#include "brailleOah.h"


using namespace std;

namespace MusicXML2
{

//_______________________________________________________________________________
/*
S_bsrPageElement bsrPageElement::create (
  int inputLineNumber)
{
  bsrPageElement* o =
    new bsrPageElement (
      inputLineNumber);
  assert(o!=0);
  return o;
}
*/

bsrPageElement::bsrPageElement (
  int inputLineNumber)
    : bsrElement (inputLineNumber)
{
  fSpacesBefore = 0;
//  fSpacesAfter  = 0;
}

bsrPageElement::~bsrPageElement ()
{}

/*
void bsrPageElement::setBsrLineUpLink (
S_bsrLine bsrLineUpLink)
{
  fBsrLineUpLink = bsrLineUpLink;
}

S_bsrLine bsrPageElement::getBsrLineUpLink () const
{
  return fBsrLineUpLink;
}
*/

void bsrPageElement::acceptIn (basevisitor* v)
{
#ifdef TRACE_OAH
  if (gBsrOah->fTraceBsrVisitors) {
    gLogOstream <<
      "% ==> bsrPageElement::acceptIn ()" <<
      endl;
  }
#endif

  if (visitor<S_bsrPageElement>*
    p =
      dynamic_cast<visitor<S_bsrPageElement>*> (v)) {
        S_bsrPageElement elem = this;

#ifdef TRACE_OAH
        if (gBsrOah->fTraceBsrVisitors) {
          gLogOstream <<
            "% ==> Launching bsrPageElement::visitStart ()" <<
            endl;
        }
#endif
        p->visitStart (elem);
  }
}

void bsrPageElement::acceptOut (basevisitor* v)
{
#ifdef TRACE_OAH
  if (gBsrOah->fTraceBsrVisitors) {
    gLogOstream <<
      "% ==> bsrPageElement::acceptOut ()" <<
      endl;
  }
#endif

  if (visitor<S_bsrPageElement>*
    p =
      dynamic_cast<visitor<S_bsrPageElement>*> (v)) {
        S_bsrPageElement elem = this;

#ifdef TRACE_OAH
        if (gBsrOah->fTraceBsrVisitors) {
          gLogOstream <<
            "% ==> Launching bsrPageElement::visitEnd ()" <<
            endl;
        }
#endif
        p->visitEnd (elem);
  }
}

void bsrPageElement::browseData (basevisitor* v)
{}


string bsrPageElement::asString () const
{
  // this is overriden all in actual elements
  return "??? bsrPageElement::asString () ???";
}

string bsrPageElement::asShortString () const
{
  // this can be overriden in actual elements
  return asString ();
}

void bsrPageElement::print (ostream& os) const
{
  os << asString () << endl;
}

ostream& operator<< (ostream& os, const S_bsrPageElement& elt)
{
  elt->print (os);
  return os;
}


}
