/*
  This file is to be included only by msrMutualDependencies.h,
  to satisfy declarations mutual dependencies.
*/

//______________________________________________________________________________
class msrSyllable : public msrMeasureElement
{
  public:

    // data types
    // ------------------------------------------------------

    // we want to end the line in the LilyPond code at a break
    enum msrSyllableKind {
      kSyllableNone,
      kSyllableSingle,
      kSyllableBegin, kSyllableMiddle, kSyllableEnd,

      kSyllableOnRestNote,
      kSyllableSkipRestNote,
      kSyllableSkipNonRestNote,

      kSyllableMeasureEnd,
      kSyllableLineBreak, kSyllablePageBreak};

    static string syllableKindAsString (
      msrSyllableKind syllableKind);

    enum msrSyllableExtendKind {
      kSyllableExtendNone,
      kSyllableExtendSingle,
      kSyllableExtendStart, kSyllableExtendContinue, kSyllableExtendStop };

    static string syllableExtendKindAsString (
      msrSyllableExtendKind syllableExtendKind);


    // creation from MusicXML
    // ------------------------------------------------------

    static SMARTP<msrSyllable> create (
      int                   inputLineNumber,
      msrSyllableKind       syllableKind,
      msrSyllableExtendKind syllableExtendKind,
      rational              syllableWholeNotes,
      msrTupletFactor       syllableTupletFactor,
      S_msrStanza           syllableStanzaUpLink);

    static SMARTP<msrSyllable> createWithNextMeasurePuristNumber (
      int                   inputLineNumber,
      msrSyllableKind       syllableKind,
      msrSyllableExtendKind syllableExtendKind,
      rational              syllableWholeNotes,
      msrTupletFactor       syllableTupletFactor,
      S_msrStanza           syllableStanzaUpLink,
      int                   syllableNextMeasurePuristNumber);

    SMARTP<msrSyllable> createSyllableNewbornClone (
      S_msrPart containingPart); // JMI

    SMARTP<msrSyllable> createSyllableDeepCopy (
      S_msrPart containingPart);

  protected:

    // constructors/destructor
    // ------------------------------------------------------

    msrSyllable (
      int                   inputLineNumber,
      msrSyllableKind       syllableKind,
      msrSyllableExtendKind syllableExtendKind,
      rational              syllableWholeNotes,
      msrTupletFactor       syllableTupletFactor,
      S_msrStanza           syllableStanzaUpLink);

    msrSyllable (
      int                   inputLineNumber,
      msrSyllableKind       syllableKind,
      msrSyllableExtendKind syllableExtendKind,
      rational              syllableWholeNotes,
      msrTupletFactor       syllableTupletFactor,
      S_msrStanza           syllableStanzaUpLink,
      int                   syllableNextMeasurePuristNumber);

    virtual ~msrSyllable ();

  public:

    // set and get
    // ------------------------------------------------------

    // upLinks
    S_msrNote             getSyllableNoteUpLink () const
                              { return fSyllableNoteUpLink; }

    S_msrStanza           getSyllableStanzaUpLink () const
                              { return fSyllableStanzaUpLink; }

    // syllable kind and contents
    msrSyllableKind       getSyllableKind () const
                              { return fSyllableKind; }

    const list<string>&   getSyllableTextsList ()
                              { return fSyllableTextsList; }

    msrSyllableExtendKind getSyllableExtendKind () const
                              { return fSyllableExtendKind; }

    // syllable whole notes
    rational              getSyllableWholeNotes () const
                              { return fSyllableWholeNotes; }

    // syllable tuplet factor
    msrTupletFactor       getSyllableTupletFactor () const
                              { return fSyllableTupletFactor; }

    // syllable next measure purist number
    void                  setSyllableNextMeasurePuristNumber (
                            int puristMeasureNumber)
                              {
                                fSyllableNextMeasurePuristNumber =
                                  puristMeasureNumber;
                              }

    int                   getSyllableNextMeasurePuristNumber () const
                              { return fSyllableNextMeasurePuristNumber; }

    // services
    // ------------------------------------------------------

    void                  appendSyllableToNoteAndSetItsNoteUpLink (
                            S_msrNote note);

    void                  appendLyricTextToSyllable (string text);

  public:

    // visitors
    // ------------------------------------------------------

    virtual void          acceptIn  (basevisitor* v);
    virtual void          acceptOut (basevisitor* v);

    virtual void          browseData (basevisitor* v);

  public:

    // print
    // ------------------------------------------------------

    string                syllableNoteUpLinkAsString () const;

    string                syllableWholeNotesAsMsrString () const;

    string                syllableKindAsString () const;

    string                syllableExtendKindAsString () const;

    string                syllableTextsListAsString () const;

    static void           writeTextsList (
                            const list<string>& textsList,
                            ostream&            os);

    string                asString () const;

    virtual void          print (ostream& os) const;

  private:

    // fields
    // ------------------------------------------------------

    // upLinks
    S_msrNote             fSyllableNoteUpLink;
    S_msrStanza           fSyllableStanzaUpLink;

    // syllable kind and contents
    msrSyllableKind       fSyllableKind;
    list<string>          fSyllableTextsList;
    msrSyllableExtendKind fSyllableExtendKind;

    // syllable whole notes
    rational              fSyllableWholeNotes;

    // syllable tuplet factor
    msrTupletFactor       fSyllableTupletFactor;

    // syllable complementary measure number
    // for kSyllableMeasureEnd, kSyllableLineBreak and kSyllablePageBreak
    int                   fSyllableNextMeasurePuristNumber;
};
typedef SMARTP<msrSyllable> S_msrSyllable;
EXP ostream& operator<< (ostream& os, const S_msrSyllable& elt);

