/*
  MusicXML Library
  Copyright (C) Grame 2006-2013

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Grame Research Laboratory, 11, cours de Verdun Gensoul 69002 Lyon - France
  research@grame.fr
*/

#ifndef ___lpsrLayouts___
#define ___lpsrLayouts___

#include "lpsrVarValAssocs.h"

#include "lpsrScheme.h"


namespace MusicXML2
{

//______________________________________________________________________________
class lpsrLayout : public lpsrElement
{
  public:

    // creation from MusicXML
    // ------------------------------------------------------

    static SMARTP<lpsrLayout> create (
      int inputLineNumber);

  protected:

    // constructors/destructor
    // ------------------------------------------------------

    lpsrLayout (
      int inputLineNumber);

    virtual ~lpsrLayout ();

  public:

    // set and get
    // ------------------------------------------------------

    float                 getLayoutGlobalStaffSize () const
                              { return fLayoutGlobalStaffSize; }

    void                  setLayoutGlobalStaffSize (float size)
                              { fLayoutGlobalStaffSize = size; }

  public:

    // services
    // ------------------------------------------------------

/* JMI
    void                  addLilypondVarValAssoc (
                            S_lpsrVarValAssoc assoc)
                              {
                                fLpsrVarValAssocsVector.push_back (assoc);
                              }
*/

    void                  addSchemeVariable (
                            S_lpsrSchemeVariable assoc)
                              {
                                fLpsrSchemeVariablesVector.push_back (assoc);
                              }

  public:

    // visitors
    // ------------------------------------------------------

    virtual void          acceptIn  (basevisitor* v);
    virtual void          acceptOut (basevisitor* v);

    virtual void          browseData (basevisitor* v);

  public:

    // print
    // ------------------------------------------------------

    virtual void          print (ostream& os) const;

  private:

    // fields
    // ------------------------------------------------------

    float                 fLayoutGlobalStaffSize;

/* JMI
    vector<S_lpsrVarValAssoc>
                          fLpsrVarValAssocsVector;
                          */
    vector<S_lpsrSchemeVariable>
                          fLpsrSchemeVariablesVector;
};
typedef SMARTP<lpsrLayout> S_lpsrLayout;
EXP ostream& operator<< (ostream& os, const S_lpsrLayout& elt);


}


#endif
