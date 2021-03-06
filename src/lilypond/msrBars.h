/*
  MusicXML Library
  Copyright (C) Grame 2006-2013

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Grame Research Laboratory, 11, cours de Verdun Gensoul 69002 Lyon - France
  research@grame.fr
*/

#ifndef ___msrBars___
#define ___msrBars___

#include <list>

#include "msrMeasureElements.h"

#include "msrBasicTypes.h"


namespace MusicXML2
{

//______________________________________________________________________________
class msrBarCheck : public msrMeasureElement
{
  public:

    // creation from MusicXML
    // ------------------------------------------------------

    static SMARTP<msrBarCheck> create (
      int inputLineNumber);

    static SMARTP<msrBarCheck> createWithNextBarPuristNumber (
      int    inputLineNumber,
      string nextBarOriginalNumber,
      int    nextBarPuristNumber);

  protected:

    // constructors/destructor
    // ------------------------------------------------------

    msrBarCheck (
      int inputLineNumber);

    msrBarCheck (
      int    inputLineNumber,
      string nextBarOriginalNumber,
      int    nextBarPuristNumber);

    virtual ~msrBarCheck ();

  public:

    // set and get
    // ------------------------------------------------------

    string                getNextBarOriginalNumber () const
                              { return fNextBarOriginalNumber; }

    void                  setNextBarPuristNumber (int puristNumber);

    int                   getNextBarPuristNumber () const
                              { return fNextBarPuristNumber; }

    // services
    // ------------------------------------------------------

  public:

    // visitors
    // ------------------------------------------------------

    virtual void          acceptIn  (basevisitor* v);
    virtual void          acceptOut (basevisitor* v);

    virtual void          browseData (basevisitor* v);

  public:

    // print
    // ------------------------------------------------------

    string                asString () const;

    virtual void          print (ostream& os) const;

  private:

    // fields
    // ------------------------------------------------------

    string                fNextBarOriginalNumber;
    int                   fNextBarPuristNumber;
};
typedef SMARTP<msrBarCheck> S_msrBarCheck;
EXP ostream& operator<< (ostream& os, const S_msrBarCheck& elt);

//______________________________________________________________________________
class msrBarNumberCheck : public msrMeasureElement
{
  public:

    // creation from MusicXML
    // ------------------------------------------------------

    static SMARTP<msrBarNumberCheck> create (
      int    inputLineNumber,
      string nextBarOriginalNumber,
      int    nextBarPuristNumber);

  protected:

    // constructors/destructor
    // ------------------------------------------------------

    msrBarNumberCheck (
      int    inputLineNumber,
      string nextBarOriginalNumber,
      int    nextBarPuristNumber);

    virtual ~msrBarNumberCheck ();

  public:

    // set and get
    // ------------------------------------------------------

    string                getNextBarOriginalNumber () const
                              { return fNextBarOriginalNumber; }

    int                   getNextBarPuristNumber () const
                              { return fNextBarPuristNumber; }

    // services
    // ------------------------------------------------------

    string                asString () const;

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

    string                fNextBarOriginalNumber;
    int                   fNextBarPuristNumber;
};
typedef SMARTP<msrBarNumberCheck> S_msrBarNumberCheck;
EXP ostream& operator<< (ostream& os, const S_msrBarNumberCheck& elt);

//______________________________________________________________________________
class msrBarline : public msrMeasureElement
{
  public:

    // data types
    // ------------------------------------------------------

    // location
    enum msrBarlineLocationKind {
      kBarlineLocationNone,

      kBarlineLocationLeft,
      kBarlineLocationMiddle,
      kBarlineLocationRight }; // by default

    static string barlineLocationKindAsString (
      msrBarlineLocationKind barlineLocationKind);

    // style
    enum msrBarlineStyleKind {
      kBarlineStyleNone,

      kBarlineStyleRegular,  // by default

      kBarlineStyleDotted, kBarlineStyleDashed, kBarlineStyleHeavy,
      kBarlineStyleLightLight, kBarlineStyleLightHeavy,
      kBarlineStyleHeavyLight, kBarlineStyleHeavyHeavy,
      kBarlineStyleTick, kBarlineStyleShort };

    static string barlineStyleKindAsString (
      msrBarlineStyleKind barlineStyleKind);

    // repeat direction
    enum msrBarlineRepeatDirectionKind {
      kBarlineRepeatDirectionNone,
      kBarlineRepeatDirectionForward, kBarlineRepeatDirectionBackward };

    static string barlineRepeatDirectionKindAsString (
      msrBarlineRepeatDirectionKind barlineRepeatDirectionKind);

    // ending type
    enum msrBarlineEndingTypeKind {
      kBarlineEndingNone,

      kBarlineEndingTypeStart,
      kBarlineEndingTypeStop,
      kBarlineEndingTypeDiscontinue };

    static string barlineEndingTypeKindAsString (
      msrBarlineEndingTypeKind barlineEndingTypeKind);

    // category
    enum msrBarlineCategoryKind {
      k_NoBarlineCategory,

      kBarlineCategoryStandalone,

      kBarlineCategoryRepeatStart, kBarlineCategoryRepeatEnd,

      kBarlineCategoryHookedEndingStart, kBarlineCategoryHookedEndingEnd,
      kBarlineCategoryHooklessEndingStart, kBarlineCategoryHooklessEndingEnd };

    static string barlineCategoryKindAsString (
      msrBarlineCategoryKind barlineCategoryKind);

    // segno
    enum msrBarlineHasSegnoKind {
      kBarlineHasSegnoYes, kBarlineHasSegnoNo};

    static string barlineHasSegnoKindAsString (
      msrBarlineHasSegnoKind barlineHasSegnoKind);

    // coda
    enum msrBarlineHasCodaKind {
      kBarlineHasCodaYes, kBarlineHasCodaNo};

    static string barlineHasCodaKindAsString (
      msrBarlineHasCodaKind barlineHasCodaKind);

    // repeat winged
    enum msrBarlineRepeatWingedKind {
      kBarlineRepeatWingedNone,

      kBarlineRepeatWingedStraight, kBarlineRepeatWingedCurved,
      kBarlineRepeatWingedDoubleStraight, kBarlineRepeatWingedDoubleCurved };

    static string barlineRepeatWingedKindAsString (
      msrBarlineRepeatWingedKind barlineRepeatWingedKind);

    // creation from MusicXML
    // ------------------------------------------------------

    static SMARTP<msrBarline> create (
      int                           inputLineNumber,
      msrBarlineLocationKind        barlineLocationKind,
      msrBarlineStyleKind           barlineStyleKind,
      msrBarlineRepeatDirectionKind barlineRepeatDirectionKind,
      msrBarlineEndingTypeKind      barlineEndingTypeKind,
      string                        endingNumber,
      int                           barlineTimes,
      msrBarlineCategoryKind        barlineCategoryKind,
      msrBarlineHasSegnoKind        barlineHasSegnoKind,
      msrBarlineHasCodaKind         barlineHasCodaKind,
      msrBarlineRepeatWingedKind    barlineRepeatWingedKind);

  protected:

    // constructors/destructor
    // ------------------------------------------------------

    msrBarline (
      int                           inputLineNumber,
      msrBarlineLocationKind        barlineLocationKind,
      msrBarlineStyleKind           barlineStyleKind,
      msrBarlineRepeatDirectionKind barlineRepeatDirectionKind,
      msrBarlineEndingTypeKind      barlineEndingTypeKind,
      string                        endingNumber,
      int                           barlineTimes,
      msrBarlineCategoryKind        barlineCategoryKind,
      msrBarlineHasSegnoKind        barlineHasSegnoKind,
      msrBarlineHasCodaKind         barlineHasCodaKind,
      msrBarlineRepeatWingedKind    barlineRepeatWingedKind);

    virtual ~msrBarline ();

  public:

    // set and get
    // ------------------------------------------------------

    msrBarlineLocationKind
                          getLocation () const // JMI
                              { return fLocationKind; }

    msrBarlineStyleKind   getBarlineStyleKind () const
                              { return fStyleKind; }

    msrBarlineRepeatDirectionKind
                          getRepeatDirection () const
                              { return fRepeatDirectionKind; }

    msrBarlineRepeatWingedKind
                          getRepeatWinged () const
                              { return fRepeatWingedKind; }

    msrBarlineEndingTypeKind
                          getEndingType () const
                              { return fEndingTypeKind; }

    string                getEndingNumber () const
                              { return fEndingNumber; }

    msrBarlineCategoryKind
                          getBarlineCategory () const
                              { return fBarlineCategoryKind; }

    void                  setBarlineCategory (
                            msrBarlineCategoryKind barlineCategoryKind);

    int                   getBarlineTimes () const
                              { return fBarlineTimes; }

    msrBarlineHasSegnoKind
                          getBarlineHasSegnoKind () const
                              { return fBarlineHasSegnoKind; }
    msrBarlineHasCodaKind getBarlineHasCodaKind () const
                              { return fBarlineHasCodaKind; }

  private:

    // private services
    // ------------------------------------------------------

    const list<int>&      getEndingNumbersList () const
                              { return fEndingNumbersList; }

  public:

    // visitors
    // ------------------------------------------------------

    virtual void          acceptIn  (basevisitor* v);
    virtual void          acceptOut (basevisitor* v);

    virtual void          browseData (basevisitor* v);

  public:

    // print
    // ------------------------------------------------------

    string                endingNumbersListAsString () const;

    string                asString () const;
    string                asShortString () const;

    virtual void          print (ostream& os) const;

  private:

    // fields
    // ------------------------------------------------------

    msrBarlineLocationKind
                          fLocationKind;

    msrBarlineStyleKind   fStyleKind;

    msrBarlineRepeatDirectionKind
                          fRepeatDirectionKind;

    msrBarlineRepeatWingedKind
                          fRepeatWingedKind;



    msrBarlineEndingTypeKind
                          fEndingTypeKind;
    string                fEndingNumber; // may be "1, 2"

    int                   fBarlineTimes;

    msrBarlineCategoryKind
                          fBarlineCategoryKind;

    msrBarlineHasSegnoKind
                          fBarlineHasSegnoKind;
    msrBarlineHasCodaKind fBarlineHasCodaKind;

    // the numbers extracted from fEndingNumber
    list<int>             fEndingNumbersList;
};
typedef SMARTP<msrBarline> S_msrBarline;
EXP ostream& operator<< (ostream& os, const S_msrBarline& elt);


} // namespace MusicXML2


#endif
