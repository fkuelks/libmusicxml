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
//#include <algorithm>    // for_each

#include "conversions.h"

#include "msr2MxmltreeTranslator.h"

#include "elements.h"
#include "factory.h"
#include "xml.h"
#include "xmlfile.h"

#include "generalOah.h"

#include "setTraceOahIfDesired.h"
#ifdef TRACE_OAH
  #include "traceOah.h"
#endif

#include "mxmlTreeOah.h"
#include "msrOah.h"
#include "lilypondOah.h"

#include "version.h"

#include "mxmlTree.h"


using namespace std;

namespace MusicXML2
{
//______________________________________________________________________________
string msr2MxmltreeTranslator::msrLengthAsTenths (
  msrLength length)
{
  float lengthValue = length.getLengthValue ();

  // convert lengthValue to millimeters
  switch (length.getLengthUnitKind ()) {
    case kInchUnit:
      lengthValue *= 25.4;
      break;
    case kCentimeterUnit:
      lengthValue *= 10;
      break;
    case kMillimeterUnit:
      break;
  } // switch

  // compute the number of tenths
  float tenths = lengthValue / fMillimeters * fTenths;

  // compute result
  stringstream s;

  s << tenths;

  return s.str ();
}

string msr2MxmltreeTranslator::S_msrLengthAsTenths (
  S_msrLength length)
{
  return msrLengthAsTenths (* length);
}

//______________________________________________________________________________
void msr2MxmltreeTranslator::populatePageMarginsElement (
  Sxmlelement       elem,
  S_msrMarginsGroup marginsGroup)
{
  // left margin
  S_msrMargin leftMargin = marginsGroup->getLeftMargin ();

  if (leftMargin) {
    // append a left margin element to the page margins element
    msrLength
      leftMarginLength =
        leftMargin->getMarginLength ();

    elem->push (
      createElement (
        k_left_margin,
        msrLengthAsTenths (leftMarginLength)));
  }

  // right margin
  S_msrMargin rightMargin = marginsGroup->getRightMargin ();

  if (rightMargin) {
    // append a right margin element to the page margins element
    msrLength
      rightMarginLength =
        rightMargin->getMarginLength ();

    elem->push (
      createElement (
        k_right_margin,
        msrLengthAsTenths (rightMarginLength)));
  }

  // top margin
  S_msrMargin topMargin = marginsGroup->getTopMargin ();

  if (topMargin) {
    // append a top margin element to the page margins element
    msrLength
      topMarginLength =
        topMargin->getMarginLength ();

    elem->push (
      createElement (
        k_top_margin,
        msrLengthAsTenths (topMarginLength)));
  }

  // bottom margin
  S_msrMargin bottomMargin = marginsGroup->getBottomMargin ();

  if (bottomMargin) {
    // append a bottom margin element to the page margins element
    msrLength
      bottomMarginLength =
        bottomMargin->getMarginLength ();

    elem->push (
      createElement (
        k_bottom_margin,
        msrLengthAsTenths (bottomMarginLength)));
  }
}

void msr2MxmltreeTranslator::appendPageMarginsElementToScoreDefaultsPageLayout (
  S_msrMarginsGroup marginsGroup)
{
  // create a page margins element
  Sxmlelement
    pageMarginsElement =
      createElement (k_page_margins, "");

  // set its type element
  pageMarginsElement->add (
    createAttribute (
      "type",
      msrMarginTypeKindAsString (
        marginsGroup->getMarginsGroupTypeKind ())));

  // populate it
  populatePageMarginsElement (
    pageMarginsElement,
    marginsGroup);

  // append it to the defaults page layout element
  appendSubElementToScoreDefaultsPageLayout (
    pageMarginsElement);
}

//______________________________________________________________________________
void msr2MxmltreeTranslator::populateSystemMarginsElement (
  Sxmlelement       elem,
  S_msrSystemLayout systemLayout)
{
  // left margin
  S_msrMargin leftMargin = systemLayout->getLeftMargin ();

  if (leftMargin) {
    // append a left margin element to the page margins element
    msrLength
      leftMarginLength =
        leftMargin->getMarginLength ();

    elem->push (
      createElement (
        k_left_margin,
        msrLengthAsTenths (leftMarginLength)));
  }

  // right margin
  S_msrMargin rightMargin = systemLayout->getRightMargin ();

  if (rightMargin) {
    // append a right margin element to the page margins element
    msrLength
      rightMarginLength =
        rightMargin->getMarginLength ();

    elem->push (
      createElement (
        k_right_margin,
        msrLengthAsTenths (rightMarginLength)));
  }
}

void msr2MxmltreeTranslator::appendSystemMarginsElementToScoreDefaultsSystemLayout (
  S_msrSystemLayout systemLayout)
{
  // JMI
}

//______________________________________________________________________________
string msr2MxmltreeTranslator::msrPlacementKindAsMusicXMLString (
  msrPlacementKind placementKind)
{
  string result;

  switch (placementKind) {
    case msrPlacementKind::kPlacementNone:
      result = "";
      break;
    case msrPlacementKind::kPlacementAbove:
      result = "above";
      break;
    case msrPlacementKind::kPlacementBelow:
      result = "below";
      break;
  } // switch

  return result;
}

//______________________________________________________________________________
string msr2MxmltreeTranslator::msrSpannerTypeKindAsMusicXMLString (
  msrSpannerTypeKind spannerTypeKind)
{
  string result;

  switch (spannerTypeKind) {
    case k_NoSpannerType:
      // should not occur
      break;
    case kSpannerTypeStart:
      result = "start";
      break;
    case kSpannerTypeStop:
      result = "stop";
      break;
    case kSpannerTypeContinue:
      result = "continue";
      break;
  } // switch

  return result;
}

//________________________________________________________________________
void msr2MxmltreeTranslator::appendSubElementToScoreWork (
  Sxmlelement elem)
{
  if (! fScoreWorkElement) {
    // create a work element
    fScoreWorkElement = createElement (k_work, "");
  }

  fScoreWorkElement->push (elem);
}

//________________________________________________________________________
void msr2MxmltreeTranslator::appendSubElementToScoreIdentification (
  Sxmlelement elem)
{
  if (! fScoreIdentificationElement) {
    // create an identification element
    fScoreIdentificationElement = createElement (k_identification, "");
  }

  fScoreIdentificationElement->push (elem);
}

void msr2MxmltreeTranslator::appendSubElementToScoreIdentificationEncoding (
  Sxmlelement elem)
{
  if (! fScoreIdentificationEncodingElement) {
    // create an encoding element
    fScoreIdentificationEncodingElement = createElement (k_encoding, "");
  }

  fScoreIdentificationEncodingElement->push (elem);
}

//________________________________________________________________________
void msr2MxmltreeTranslator::appendSubElementToScoreDefaults (
  Sxmlelement elem)
{
  if (! fScoreDefaultsElement) {
    // create a defaults element
    fScoreDefaultsElement = createElement (k_defaults, "");
  }

  fScoreDefaultsElement->push (elem);
}

void msr2MxmltreeTranslator::appendSubElementToScoreDefaultsPageLayout (
  Sxmlelement elem)
{
  if (! fScoreDefaultsPageLayoutElement) {
    // create a page layout element
    fScoreDefaultsPageLayoutElement = createElement (k_page_layout, "");
  }

  fScoreDefaultsPageLayoutElement->push (elem);
}

void msr2MxmltreeTranslator::appendSubElementToScoreDefaultsSystemLayout (
  Sxmlelement elem)
{
  if (! fScoreDefaultsSystemLayoutElement) {
    // create a system layout element
    fScoreDefaultsSystemLayoutElement = createElement (k_system_layout, "");
  }

  fScoreDefaultsSystemLayoutElement->push (elem);
}

//________________________________________________________________________
void msr2MxmltreeTranslator::appendSubElementToMeasureAttributes (
  Sxmlelement elem)
{
  if (! fCurrentMeasureAttributesElement) {
    // create an attributes element
    fCurrentMeasureAttributesElement = createElement (k_attributes, "");

    // append the attributes element to the current measure element
    fCurrentMeasureElement->push (fCurrentMeasureAttributesElement);
  }

  // append elem to the current measure attributes element
  fCurrentMeasureAttributesElement->push (elem);
}

//________________________________________________________________________
void msr2MxmltreeTranslator::appendSubElementToMeasureDirection (
  Sxmlelement      elem,
  msrPlacementKind placementKind)
{
  // create a direction element
  Sxmlelement directionElement = createElement (k_direction, "");

  // set it's "placement" attribute if relevant
  string
    placementString =
      msrPlacementKindAsMusicXMLString (placementKind);

  if (placementString.size ()) {
    directionElement->add (createAttribute ("placement",  placementString));
  }

  // append the direction element to the current measure element
  fCurrentMeasureElement->push (directionElement);

  // create a direction type element
  Sxmlelement directionTypeElement = createElement (k_direction_type, "");

  // append it to the current direction element
  directionElement->push (directionTypeElement);

  // append elem to the direction type element
  directionTypeElement->push (elem);
}

//________________________________________________________________________
void msr2MxmltreeTranslator::appendNoteSubElementToMeasure (
  S_msrNote   note,
  Sxmlelement elem)
{
  int inputLineNumber =
    elem->getInputLineNumber ();

#ifdef TRACE_OAH
  if (gTraceOah->fTraceNotes) {
    fLogOutputStream <<
      "--> appendNoteSubElementToMeasure(), elem = " <<
      ", elem: " << xmlelementAsString (elem) <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  // append the 'before spanner' elements if any
  appendNoteSpannersBeforeNoteElement (note);

  // append elem to the current measure element
  fCurrentMeasureElement->push (elem);

  // append the 'after spanner' elements if any
  appendNoteSpannersAfterNoteElement (note);
}

void msr2MxmltreeTranslator::appendOtherSubElementToMeasure (
  Sxmlelement elem)
{
  int inputLineNumber =
    elem->getInputLineNumber ();

#ifdef TRACE_OAH
  if (gTraceOah->fTraceNotes) {
    fLogOutputStream <<
      "--> appendOtherSubElementToMeasure()" <<
      ", elem: " << xmlelementAsString (elem) <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  // append elem to the current measure element
  fCurrentMeasureElement->push (elem);
}

//________________________________________________________________________
void msr2MxmltreeTranslator::appendSubElementToNoteNotations (
  Sxmlelement      elem,
  msrPlacementKind placementKind)
{
  if (! fCurrentNoteNotationsElement) {
    // create an notations element
    fCurrentNoteNotationsElement = createElement (k_notations, "");

    // append it to fCurrentNoteElement
    fCurrentNoteElement->push (fCurrentNoteNotationsElement);
  }

  // set elem's "placement" attribute if relevant
  string
    placementString =
      msrPlacementKindAsMusicXMLString (placementKind);

  if (placementString.size ()) {
    elem->add (createAttribute ("placement", placementString));
  }

  // append elem to the note notations element
  fCurrentNoteNotationsElement->push (elem);
}

void msr2MxmltreeTranslator::appendSubElementToNoteNotationsOrnaments (
  Sxmlelement      elem,
  msrPlacementKind placementKind)
{
  if (! fCurrentNoteNotationsOrnamentsElement) {
    // create an notations element
    fCurrentNoteNotationsOrnamentsElement = createElement (k_ornaments, "");

    // append it to fCurrentNoteNotationsElement
    appendSubElementToNoteNotations (
      fCurrentNoteNotationsOrnamentsElement,
      kPlacementNone); // no placement for '<ornaments />'
  }

  // set elem's "placement" attribute if relevant
  string
    placementString =
      msrPlacementKindAsMusicXMLString (placementKind);

  if (placementString.size ()) {
    elem->add (createAttribute ("placement", placementString));
  }

  // append elem to the note notations ornaments element
  fCurrentNoteNotationsOrnamentsElement->push (elem);
}

//________________________________________________________________________
void msr2MxmltreeTranslator::appendSubElementToNoteNotationsArticulations (
  Sxmlelement      elem,
  msrPlacementKind placementKind)
{
  if (! fCurrentNoteNotationsElement) {
    // create the note articulations element
    fCurrentNoteNotationsArticulationsElement = createElement (k_articulations, "");

    // append it to fCurrentNoteNotationsElement
    appendSubElementToNoteNotations (
      fCurrentNoteNotationsArticulationsElement,
      placementKind);
  }

  // set elem's "placement" attribute if relevant
  string
    placementString =
      msrPlacementKindAsMusicXMLString (placementKind);

  if (placementString.size ()) {
    elem->add (createAttribute ("placement", placementString));
  }

  // append elem to the note notations articulations element
  fCurrentNoteNotationsArticulationsElement->push (elem);
}

//________________________________________________________________________
void msr2MxmltreeTranslator::appendSubElementToNoteNotationsTechnicals (
  Sxmlelement      elem,
  msrPlacementKind placementKind)
{
  if (! fCurrentNoteNotationsTechnicalsElement) {
    // create an notations element
    fCurrentNoteNotationsTechnicalsElement = createElement (k_technical, "");

    // append it to fCurrentNoteNotationsElement
    appendSubElementToNoteNotations (
      fCurrentNoteNotationsTechnicalsElement,
      placementKind);
  }

  // set elem's "placement" attribute if relevant
  string
    placementString =
      msrPlacementKindAsMusicXMLString (placementKind);

  if (placementString.size ()) {
    elem->add (createAttribute ("placement", placementString));
  }

  // append elem to the note notations technicals element
  fCurrentNoteNotationsTechnicalsElement->push (elem);
}

//________________________________________________________________________
msr2MxmltreeTranslator::msr2MxmltreeTranslator (
  indentedOstream& ios,
  S_msrScore       mScore)
    : fLogOutputStream (ios)
{
  // the MSR score we're visiting
  fVisitedMsrScore = mScore;

  // create the current score part-wise element
  fScorePartWiseElement = createScorePartWiseElement ();

  // print layouts
  fOnGoingPrintElement = false;

  // notes
  fCurrentNoteElementAwaitsGraceNotes = false;
  fPendingNoteAwaitingGraceNotes = nullptr;
  fPendingNoteElement = nullptr;

/*
  // double tremolos
  fOnGoingDoubleTremolo = false;

  // stanzas
  fOnGoingStanza = false;

  // syllables
  fOnGoingSyllableExtend = false;
  */
};

msr2MxmltreeTranslator::~msr2MxmltreeTranslator ()
{}

//________________________________________________________________________
void msr2MxmltreeTranslator::buildMxmltreeFromMsrScore ()
{
  if (fVisitedMsrScore) {
    // create a msrScore browser
    msrBrowser<msrScore> browser (this);

    // browse the visited score with the browser
    browser.browse (*fVisitedMsrScore);
  }
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrScore& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();

#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrScore" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  // create the initial creation comment
  stringstream s;
  s <<
    " ===== " <<
    "Created by " <<
    gOahOah->fHandlerExecutableName <<
    " " <<
    currentVersionNumber () <<
    " on " <<
    gGeneralOah->fTranslationDateFull <<
    " from " <<
    gOahOah->fInputSourceName <<
    " ===== ";

  // append the initial creation comment to the score part wise element
  fScorePartWiseElement->push (createElement (kComment, s.str ()));

  // create a software element
  Sxmlelement
    softwareElement =
      createElement (
        k_software,
        gOahOah->fHandlerExecutableName
          + " "
          + currentVersionNumber () +
          ", https://github.com/dfober/libmusicxml/tree/lilypond");

  // append it to the identification encoding
  appendSubElementToScoreIdentificationEncoding (softwareElement);

  // create an encoding date element
  Sxmlelement
    encodingDateElement =
      createElement (
        k_encoding_date,
        gGeneralOah->fTranslationDateYYYYMMDD);

  // append it to the identification encoding
  appendSubElementToScoreIdentificationEncoding (encodingDateElement);

  // create the part list element
  fScorePartListElement = createElement (k_part_list, "");
}

void msr2MxmltreeTranslator::visitEnd (S_msrScore& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();

#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrScore" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  // append the score work element if any to the score part wise element
  if (fScoreWorkElement) {
    fScorePartWiseElement->push (fScoreWorkElement);
  }

  // append the score movement number element if any to the score part wise element
  if (fScoreMovementNumberElement) {
    fScorePartWiseElement->push (fScoreMovementNumberElement);
  }

  // append the score movement title element if any to the score part wise element
  if (fScoreMovementTitleElement) {
    fScorePartWiseElement->push (fScoreMovementTitleElement);
  }

  // append the score identification element if any to the score part wise element
  if (fScoreIdentificationElement) {
    fScorePartWiseElement->push (fScoreIdentificationElement);
  }

  if (fScoreDefaultsScalingElement) {
    // append the scaling element to the score defaults elements
    appendSubElementToScoreDefaults (fScoreDefaultsScalingElement);
  }

  if (fScoreDefaultsPageLayoutElement) {
    // append the page layout element to the score defaults elements
    appendSubElementToScoreDefaults (fScoreDefaultsPageLayoutElement);
  }

  if (fScoreDefaultsSystemLayoutElement) {
    // append the system layout element to the score defaults elements
    appendSubElementToScoreDefaults (fScoreDefaultsSystemLayoutElement);
  }

  // append the score defaults element if any to the score part wise element
  if (fScoreDefaultsElement) {
    fScorePartWiseElement->push (fScoreDefaultsElement);
  }

  // append the score credits element if any to the score part wise element
  for (
    list<Sxmlelement>::const_iterator i = fPendingScoreCreditElementsList.begin ();
    i!=fPendingScoreCreditElementsList.end ();
    i++
  ) {
    Sxmlelement creditElement = (*i);

    fScorePartWiseElement->push (creditElement);
  } // for

  // append the part list element to the score part wise element
  fScorePartWiseElement->push (fScorePartListElement);

  // append the pending parts elements to the score part wise element
  for (
    list<Sxmlelement>::const_iterator i = fPendingPartElementsList.begin ();
    i!=fPendingPartElementsList.end ();
    i++
  ) {
    Sxmlelement partElement = (*i);

    // create a part comment
    stringstream s;
    s <<
      " ============================ " <<
      "PART" <<
      " \"" << partElement->getAttributeValue ("id") << "\"" <<
      ", line " << inputLineNumber <<
      " ============================= ";
    Sxmlelement partComment = createElement (kComment, s.str ());

    // append it to the score part wise element
    fScorePartWiseElement->push (partComment);

    // append the part element to the score part wise element
    fScorePartWiseElement->push (partElement);
  } // for
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrIdentification& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrIdentification" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  // work number
  S_msrVarValAssoc
    workNumber = elt->getWorkNumber ();

  if (workNumber) {
    string
      variableValue =
        workNumber->getVariableValue ();

    // create the work number element
    Sxmlelement
      workNumberElement =
        createElement (
          k_work_number,
          variableValue);

    // append it to the score part wise element
    appendSubElementToScoreWork (workNumberElement);
  }

  // work title
  S_msrVarValAssoc
    workTitle = elt->getWorkTitle ();

  if (workTitle) {
    string
      variableValue =
        workTitle->getVariableValue ();

    // create the work title element
    Sxmlelement
      workTitleElement =
        createElement (
          k_work_title,
          variableValue);

    // append it to the score work element
    appendSubElementToScoreWork (workTitleElement);
  }

  // opus
  S_msrVarValAssoc
    opus = elt->getOpus ();

  if (opus) {
    string
      variableValue =
        opus->getVariableValue ();

    // create the opus element
    Sxmlelement
      opusElement =
        createElement (
          k_opus,
          variableValue);

    // append it to the score part wise element
    appendSubElementToScoreWork (opusElement);
  }

  // movement number
  S_msrVarValAssoc
    movementNumber = elt->getMovementNumber ();

  if (movementNumber) {
    string
      variableValue =
        movementNumber->getVariableValue ();

    // create the movement number element
    Sxmlelement
      movementNumberElement =
        createElement (
          k_movement_number,
          variableValue);

    // append it to the score part wise element
    fScorePartWiseElement->push (movementNumberElement);
  }

  // movement title
  S_msrVarValAssoc
    movementTitle = elt->getMovementTitle ();

  if (movementTitle) {
    string
      variableValue =
        movementTitle->getVariableValue ();

    // create the movement title element
    Sxmlelement
      movementTitleElement =
        createElement (
          k_movement_title,
          variableValue);

    // append it to the score part wise element
    fScorePartWiseElement->push (movementTitleElement);
  }

  // miscellaneous field
  S_msrVarValAssoc
    miscellaneousField = elt->getMiscellaneousField ();

  if (miscellaneousField) {
    string
      variableValue =
        miscellaneousField->getVariableValue ();

    // create the miscellaneous field element
    Sxmlelement
      miscellaneousFieldElement =
        createElement (
          k_miscellaneous_field,
          variableValue);

    // set its name attribute
    miscellaneousFieldElement->add (
      createAttribute ("name", "description")); // ??? JMI sometines "comment"

    // create a miscellaneous element
    fIdentificationMiscellaneousElement = createElement (k_miscellaneous, "");

    // append the miscellaneous field element to it
    fIdentificationMiscellaneousElement->push (miscellaneousFieldElement);
  }

  // score instrument
  S_msrVarValAssoc
    scoreInstrument = elt->getScoreInstrument ();

  if (scoreInstrument) {
    string
      variableValue =
        scoreInstrument->getVariableValue ();

    // create the score instrument element
    Sxmlelement
      scoreInstrumentElement =
        createElement (
          k_score_instrument,
          variableValue);

    // append it to the score part wise element
    fScorePartWiseElement->push (scoreInstrumentElement);
  }

/*
<!--
	Identification contains basic metadata about the score.
	It includes the information in MuseData headers that
	may apply at a score-wide, movement-wide, or part-wide
	level. The creator, rights, source, and relation elements
	are based on Dublin Core.
-->
<!ELEMENT identification (creator*, rights*, encoding?,
	source?, relation*, miscellaneous?)>

<!--
	The creator element is borrowed from Dublin Core. It is
	used for the creators of the score. The type attribute is
	used to distinguish different creative contributions. Thus,
	there can be multiple creators within an identification.
	Standard type values are composer, lyricist, and arranger.
	Other type values may be used for different types of
	creative roles. The type attribute should usually be used
	even if there is just a single creator element. The MusicXML
	format does not use the creator / contributor distinction
	from Dublin Core.
-->
<!ELEMENT creator (#PCDATA)>
<!ATTLIST creator
    type CDATA #IMPLIED
>

<!--
	Rights is borrowed from Dublin Core. It contains
	copyright and other intellectual property notices.
	Words, music, and derivatives can have different types,
	so multiple rights tags with different type attributes
	are supported. Standard type values are music, words,
	and arrangement, but other types may be used. The
	type attribute is only needed when there are multiple
	rights elements.
-->
<!ELEMENT rights (#PCDATA)>
<!ATTLIST rights
    type CDATA #IMPLIED
>



<!--
	Encoding contains information about who did the digital
	encoding, when, with what software, and in what aspects.
	Standard type values for the encoder element are music,
	words, and arrangement, but other types may be used. The
	type attribute is only needed when there are multiple
	encoder elements.

	The supports element indicates if the encoding supports
	a particular MusicXML element. This is recommended for
	elements like beam, stem, and accidental, where the
	absence of an element is ambiguous if you do not know
	if the encoding supports that element. For Version 2.0,
	the supports element is expanded to allow programs to
	indicate support for particular attributes or particular
	values. This lets applications communicate, for example,
	that all system and/or page breaks are contained in the
	MusicXML file.
-->
<!ELEMENT encoding ((encoding-date | encoder | software |
	encoding-description | supports)*)>
<!ELEMENT encoding-date %yyyy-mm-dd;>
<!ELEMENT encoder (#PCDATA)>
<!ATTLIST encoder
    type CDATA #IMPLIED
>
<!ELEMENT software (#PCDATA)>
<!ELEMENT encoding-description (#PCDATA)>
<!ELEMENT supports EMPTY>
<!ATTLIST supports
    type %yes-no; #REQUIRED
    element CDATA #REQUIRED
    attribute CDATA #IMPLIED
    value CDATA #IMPLIED
>

<!--
	The source for the music that is encoded. This is similar
	to the Dublin Core source element.
-->
<!ELEMENT source (#PCDATA)>

<!--
	A related resource for the music that is encoded. This is
	similar to the Dublin Core relation element. Standard type
	values are music, words, and arrangement, but other
	types may be used.
-->
<!ELEMENT relation (#PCDATA)>
<!ATTLIST relation
    type CDATA #IMPLIED
>

<!--
	If a program has other metadata not yet supported in the
	MusicXML format, it can go in the miscellaneous area.
-->
<!ELEMENT miscellaneous (miscellaneous-field*)>
<!ELEMENT miscellaneous-field (#PCDATA)>
<!ATTLIST miscellaneous-field
    name CDATA #REQUIRED
>
*/

  // composers
  S_msrVarValsListAssoc
    composers = elt->getComposers ();

  if (composers) {
    const list<string>&
      variableValuesList =
        composers->getVariableValuesList ();

    list<string>::const_iterator i;

    for (i=variableValuesList.begin (); i!=variableValuesList.end (); i++) {
      string variableValue = (*i);

      // append a creator element
      Sxmlelement creatorElement = createElement (k_creator, variableValue);

      // set its type attribute
      creatorElement->add (createAttribute ("type", "composer"));

      // append it to the composers elements list
      fComposersElementsList.push_back (creatorElement);
    } // for
  }

  // arrangers
  S_msrVarValsListAssoc
    arrangers = elt->getArrangers ();

  if (arrangers) {
    const list<string>&
      variableValuesList =
        arrangers->getVariableValuesList ();

    list<string>::const_iterator i;

    for (i=variableValuesList.begin (); i!=variableValuesList.end (); i++) {
      string variableValue = (*i);

      // append a creator element
      Sxmlelement creatorElement = createElement (k_creator, variableValue);

      // set its type attribute
      creatorElement->add (createAttribute ("type", "arranger"));

      // append it to the composers elements list
      fComposersElementsList.push_back (creatorElement);
    } // for
  }

  // lyricists
  S_msrVarValsListAssoc
    lyricists = elt->getLyricists ();

  if (lyricists) {
    const list<string>&
      variableValuesList =
        lyricists->getVariableValuesList ();

    list<string>::const_iterator i;

    for (i=variableValuesList.begin (); i!=variableValuesList.end (); i++) {
      string variableValue = (*i);

      // append a creator element
      Sxmlelement creatorElement = createElement (k_creator, variableValue);

      // set its type attribute
      creatorElement->add (createAttribute ("type", "lyricist"));

      // append it to the composers elements list
      fComposersElementsList.push_back (creatorElement);
    } // for
  }

  // poets
  S_msrVarValsListAssoc
    poets = elt->getPoets ();

  if (poets) {
    const list<string>&
      variableValuesList =
        poets->getVariableValuesList ();

    list<string>::const_iterator i;

    for (i=variableValuesList.begin (); i!=variableValuesList.end (); i++) {
      string variableValue = (*i);

      // append a creator element
      Sxmlelement creatorElement = createElement (k_creator, variableValue);

      // set its type attribute
      creatorElement->add (createAttribute ("type", "poet"));

      // append it to the composers elements list
      fComposersElementsList.push_back (creatorElement);
    } // for
  }

  // translators
  S_msrVarValsListAssoc
    translators = elt->getTranslators ();

  if (translators) {
    const list<string>&
      variableValuesList =
        translators->getVariableValuesList ();

    list<string>::const_iterator i;

    for (i=variableValuesList.begin (); i!=variableValuesList.end (); i++) {
      string variableValue = (*i);

      // append a creator element
      Sxmlelement creatorElement = createElement (k_creator, variableValue);

      // set its type attribute
      creatorElement->add (createAttribute ("type", "translator"));

      // append it to the composers elements list
      fComposersElementsList.push_back (creatorElement);
    } // for
  }

  // artists
  S_msrVarValsListAssoc
    artists = elt->getArtists ();

  if (artists) {
    const list<string>&
      variableValuesList =
        artists->getVariableValuesList ();

    list<string>::const_iterator i;

    for (i=variableValuesList.begin (); i!=variableValuesList.end (); i++) {
      string variableValue = (*i);

      // append a creator element
      Sxmlelement creatorElement = createElement (k_creator, variableValue);

      // set its type attribute
      creatorElement->add (createAttribute ("type", "artist"));

      // append it to the composers elements list
      fComposersElementsList.push_back (creatorElement);
    } // for
  }

  // softwares
  S_msrVarValsListAssoc
    softwares = elt->getSoftwares ();

  if (softwares) {
    const list<string>&
      variableValuesList =
        softwares->getVariableValuesList ();

    list<string>::const_iterator i;

    for (i=variableValuesList.begin (); i!=variableValuesList.end (); i++) {
      string variableValue = (*i);

      // append a software element to the softwares elements list
      fSoftwaresElementsList.push_back (
        createElement (
        k_software,
        variableValue));
    } // for
  }

  // rights
  S_msrVarValsListAssoc
    rights = elt->getRights ();

  if (rights) {
    const list<string>&
      variableValuesList =
        rights->getVariableValuesList ();

    list<string>::const_iterator i;

    for (i=variableValuesList.begin (); i!=variableValuesList.end (); i++) {
      string variableValue = (*i);

      // append a rights element to the rights elements list
      fRightsElementsList.push_back (
        createElement (
        k_rights,
        variableValue));
    } // for
  }
}

void msr2MxmltreeTranslator::visitEnd (S_msrIdentification& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrIdentification" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  if (fRightsElementsList.size ()) {
    // append the rights elements to the score identification element
    list<Sxmlelement>::const_iterator i;

    for (i=fRightsElementsList.begin (); i!=fRightsElementsList.end (); i++) {
      Sxmlelement rightsElement = (*i);

      appendSubElementToScoreIdentification (rightsElement);
    } // for
  }

  if (fScoreIdentificationEncodingElement) {
    if (fSoftwaresElementsList.size ()) {
      // append the software elements to the score identification  encoding element
      for (
        list<Sxmlelement>::const_iterator i = fSoftwaresElementsList.begin ();
        i!=fSoftwaresElementsList.end ();
        i++
      ) {
        Sxmlelement softwareElement = (*i);

        appendSubElementToScoreIdentificationEncoding (softwareElement);
      } // for
    }

    // append the score identification element to the score identification
    appendSubElementToScoreIdentification (fScoreIdentificationEncodingElement);
  }

  if (fIdentificationMiscellaneousElement) {
    // append the miscellaneous element to the score identification element
    appendSubElementToScoreIdentification (fIdentificationMiscellaneousElement);
  }
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrScaling& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrScaling" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  gIndenter++;

  // get the values
  fMillimeters = elt->getMillimeters ();
  fTenths      = elt->getTenths ();

  if (fMillimeters > 0) {
    // create a scaling element
    fScoreDefaultsScalingElement = createElement (k_scaling, "");

    // append a millimeters sub-element to it
    {
      stringstream s;

      s << fMillimeters;

      fScoreDefaultsScalingElement->push (
        createElement (
          k_millimeters,
          s.str ()));
    }

    // append a tenths sub-element to it
    {
      stringstream s;

      s << fTenths;

      fScoreDefaultsScalingElement->push (
        createElement (
          k_tenths,
          s.str ()));
    }
  }
}

void msr2MxmltreeTranslator::visitEnd (S_msrScaling& elt)
{
  gIndenter--;

#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrScaling" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrPageLayout& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrPageLayout" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  // page size
  S_msrLength pageHeight = elt->getPageHeight ();

  if (pageHeight) {
    // append a page height element to the defaults page layout element
    appendSubElementToScoreDefaultsPageLayout (
      createElement (
        k_page_height,
        S_msrLengthAsTenths (pageHeight)));
  }

  S_msrLength pageWidth = elt->getPageWidth ();

  if (pageWidth) {
    // append a page width element to the defaults page layout element
    appendSubElementToScoreDefaultsPageLayout (
      createElement (
        k_page_width,
        S_msrLengthAsTenths (pageWidth)));
  }

  // margins
  S_msrMarginsGroup oddMarginsGroup = elt->getOddMarginsGroup ();

  if (oddMarginsGroup) {
    // append an odd page margins element to the defaults page layout element
    appendPageMarginsElementToScoreDefaultsPageLayout (
      oddMarginsGroup);
  }

  S_msrMarginsGroup evenMarginsGroup = elt->getEvenMarginsGroup ();

  if (evenMarginsGroup) {
    // append an even page margins element to the defaults page layout element
    appendPageMarginsElementToScoreDefaultsPageLayout (
      evenMarginsGroup);
  }

  S_msrMarginsGroup bothMarginsGroup = elt->getBothMarginsGroup ();

  if (bothMarginsGroup) {
    // append a both page margins element to the defaults page layout element
    appendPageMarginsElementToScoreDefaultsPageLayout (
      bothMarginsGroup);
  }
}

void msr2MxmltreeTranslator::visitEnd (S_msrPageLayout& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrPageLayout" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrSystemLayout& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrSystemLayout" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  // margins
//  appendSystemMarginsElementToScoreDefaultsSystemLayout (elt); JMI
  // create a system margins element
  Sxmlelement
    systemMarginsElement =
      createElement (k_system_margins, "");

  // populate it
  populateSystemMarginsElement (
    systemMarginsElement,
    elt);

  if (fOnGoingPrintElement) {
    // append it to the current print element
    fCurrentPrintElement->push (systemMarginsElement);
  }
  else {
    // append it to the defaults system layout element
    appendSubElementToScoreDefaultsSystemLayout (
      systemMarginsElement);
  }

  // distances
  S_msrLength systemDistance = elt->getSystemDistance ();

  if (systemDistance) {
    // create a system distance element
    Sxmlelement
      systemDistanceElement =
        createElement (
          k_system_distance,
          S_msrLengthAsTenths (systemDistance));

    if (fOnGoingPrintElement) {
      // append it to the current print element
      fCurrentPrintElement->push (systemDistanceElement);
    }
    else {
      // append it to the score defaults system layout element
      appendSubElementToScoreDefaultsSystemLayout (
        systemDistanceElement);
    }
  }

  S_msrLength topSystemDistance = elt->getTopSystemDistance ();

  if (topSystemDistance) {
    // create a top system distance element
    Sxmlelement
      topSystemDistanceElement =
        createElement (
          k_top_system_distance,
          S_msrLengthAsTenths (topSystemDistance));

    if (fOnGoingPrintElement) {
      // append it to the current print element
      fCurrentPrintElement->push (topSystemDistanceElement);
    }
    else {
      // append it to the score defaults system layout element
      appendSubElementToScoreDefaultsSystemLayout (
        topSystemDistanceElement);
    }
  }
}

void msr2MxmltreeTranslator::visitEnd (S_msrSystemLayout& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrSystemLayout" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrCredit& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrCredit" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  // create a credit element
  fCurrentScoreCreditElement = createElement (k_credit, "");

  // set its page attribute
  fCurrentScoreCreditElement->add (
    createIntegerAttribute ("page", elt->getCreditPageNumber ()));

  // append the credit element to the credit elements pending list
  fPendingScoreCreditElementsList.push_back (fCurrentScoreCreditElement);
}

void msr2MxmltreeTranslator::visitEnd (S_msrCredit& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrCredit" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  // forget about the current credit element
  fCurrentScoreCreditElement = nullptr;
}

void msr2MxmltreeTranslator::visitStart (S_msrCreditWords& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrCreditWords" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  // create a credit words element
  Sxmlelement
    creditWordsElement =
      createElement (
        k_credit_words,
        elt->getCreditWordsContents ());

  // set its font family attribute
  string
    creditWordsFontFamilyString =
      elt->getCreditWordsFontFamily ();

  if (creditWordsFontFamilyString.size ()) {
    creditWordsElement->add (
      createAttribute (
        "font-family",
        creditWordsFontFamilyString));
  }

  // set its font size attribute
  float
    creditWordsFontSize = elt->getCreditWordsFontSize ();
  stringstream s;
  s << setprecision (2) << creditWordsFontSize;
  creditWordsElement->add (createAttribute ("font-size", s.str ()));

  // set its font weight attribute
  string fontWeightString;

  switch (elt->getCreditWordsFontWeightKind ()) {
    case kFontWeightNone:
      break;
    case kFontWeightNormal:
      fontWeightString = "normal";
      break;
    case kFontWeightBold:
      fontWeightString = "bold";
      break;
    } // switch

  if (fontWeightString.size ()) {
    creditWordsElement->add (createAttribute ("font-weight", fontWeightString));
  }

  // set its font style attribute
  string fontStyleString;

  switch (elt->getCreditWordsFontStyleKind ()) {
    case kFontStyleNone:
      break;
    case kFontStyleNormal:
      fontStyleString = "normal";
      break;
    case KFontStyleItalic:
      fontStyleString = "italic";
      break;
    } // switch

  if (fontStyleString.size ()) {
    creditWordsElement->add (createAttribute ("font-style", fontStyleString));
  }

  // set its justify attribute
  string justifyString;

  switch (elt->getCreditWordsJustifyKind ()) {
    case kJustifyNone:
      break;
    case kJustifyLeft:
      justifyString = "left";
      break;
    case kJustifyCenter:
      justifyString = "center";
      break;
    case kJustifyRight:
      justifyString = "right";
      break;
    } // switch

  if (justifyString.size ()) {
    creditWordsElement->add (createAttribute ("justify", justifyString));
  }

  // set its halign attribute
  string horizontalAlignmentString;

  switch (elt->getCreditWordsHorizontalAlignmentKind ()) {
    case kHorizontalAlignmentNone:
      break;
    case kHorizontalAlignmentLeft:
      horizontalAlignmentString = "left";
      break;
    case kHorizontalAlignmentCenter:
      horizontalAlignmentString = "center";
      break;
    case kHorizontalAlignmentRight:
      horizontalAlignmentString = "right";
      break;
    } // switch

  if (horizontalAlignmentString.size ()) {
    creditWordsElement->add (createAttribute ("halign", horizontalAlignmentString));
  }

  // set its valign attribute
  string verticalAlignmentString;

  switch (elt->getCreditWordsVerticalAlignmentKind ()) {
    case kVerticalAlignmentNone:
      break;
    case kVerticalAlignmentTop:
      verticalAlignmentString = "top";
      break;
    case kVerticalAlignmentMiddle:
      verticalAlignmentString = "middle";
      break;
    case kVerticalAlignmentBottom:
      verticalAlignmentString = "bottom";
      break;
    } // switch

  if (verticalAlignmentString.size ()) {
    creditWordsElement->add (createAttribute ("valign", verticalAlignmentString));
  }

  // set its xml:lang attribute
  creditWordsElement->add (
    createAttribute (
      "xml:lang",
      msrXMLLangKindAsString (elt->getCreditWordsXMLLang ())));

  // append it to the current credit element
  fCurrentScoreCreditElement->push (creditWordsElement);
}

void msr2MxmltreeTranslator::visitEnd (S_msrCreditWords& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrCreditWords" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrPartGroup& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();

#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrPartGroup " <<
      elt->getPartGroupCombinedName () <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  switch (elt->getPartGroupImplicitKind ()) {
    case msrPartGroup::kPartGroupImplicitYes:
      // this is an MSR concept, ignore this part group
      break;

    case msrPartGroup::kPartGroupImplicitNo:
      {
        // create a start comment
        stringstream s;
        s <<
          " ========== " <<
          elt->getPartGroupCombinedName () <<
          " START" <<
            ", line " << inputLineNumber <<
          " ========== ";
        Sxmlelement comment = createElement (kComment, s.str ());

        // append it to the current part list element
        fScorePartListElement->push (comment);

        // create a part group element
        Sxmlelement scorePartGroupElement = createElement (k_part_group, "");

        // set it's "number" attribute
        scorePartGroupElement->add (
          createIntegerAttribute (
            "number", elt->getPartGroupNumber ()));

        // set it's "type" attribute
        scorePartGroupElement->add (createAttribute ("type", "start"));

        // create a group symbol element to the part group element if relevant
        string groupSymbolString;

        switch (elt->getPartGroupSymbolKind ()) {
          case msrPartGroup::kPartGroupSymbolNone:
            break;
          case msrPartGroup::kPartGroupSymbolBrace:
            groupSymbolString = "brace";
            break;
          case msrPartGroup::kPartGroupSymbolBracket:
            groupSymbolString = "bracket";
            break;
          case msrPartGroup::kPartGroupSymbolLine:
            groupSymbolString = "line";
            break;
          case msrPartGroup::kPartGroupSymbolSquare:
            groupSymbolString = "square";
            break;
        } // switch

        if (groupSymbolString.size ()) {
          Sxmlelement groupSymbolElement =
            createElement (
              k_group_symbol,
              groupSymbolString);

          // set its default X attribute if relevant
          int
            partGroupSymbolDefaultX  =
              elt->getPartGroupSymbolDefaultX ();

          if (partGroupSymbolDefaultX != INT_MIN) { // JMI superfluous???
            groupSymbolElement->add (
              createIntegerAttribute (
                "default-x",
                partGroupSymbolDefaultX));
          }

          // append it to the part group element
          scorePartGroupElement->push (groupSymbolElement);
        }

        // append a group name element to the part group element
        string
          groupName = elt->getPartGroupName ();

        if (groupName.size ()) {
          scorePartGroupElement->push (
            createElement (
              k_group_name,
              groupName));
        }

        // append a group barline element to the part group element
        string groupBarlineString;

        switch (elt->getPartGroupBarlineKind ()) {
          case msrPartGroup::kPartGroupBarlineYes:
            groupBarlineString = "yes";
            break;
          case msrPartGroup::kPartGroupBarlineNo:
            groupBarlineString = "no";
            break;
        } // switch

        scorePartGroupElement->push (
          createElement (
            k_group_barline,
            groupBarlineString));

        // append the part group element to the part list element
        fScorePartListElement->push (scorePartGroupElement);

        // push the part group element onto the stack
        fPartGroupElementsStack.push (scorePartGroupElement);
      }
      break;
  } // switch
}

void msr2MxmltreeTranslator::visitEnd (S_msrPartGroup& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();

#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrPartGroup " <<
      elt->getPartGroupCombinedName () <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  switch (elt->getPartGroupImplicitKind ()) {
    case msrPartGroup::kPartGroupImplicitYes:
      // this is an MSR concept, ignore this part group
      break;

    case msrPartGroup::kPartGroupImplicitNo:
      {
        // create an end comment
        stringstream s;
        s <<
          " ========== " <<
          elt->getPartGroupCombinedName () <<
          " END" <<
            ", line " << inputLineNumber <<
          " ========== ";
        Sxmlelement comment = createElement (kComment, s.str ());

        // append it to the current part list element
        fScorePartListElement->push (comment);

        // fetch the top-most part group element on the stack
        Sxmlelement
          partGroupElementsStackTop =
            fPartGroupElementsStack.top ();

/* JMI ???
        // sanity check
        if (elt != partGroupElementsStackTop) {
          s <<
            "elt " <<
            xmlelementAsString (elt) <<
            " and partGroupElementsStackTop " <<
            xmlelementAsString (partGroupElementsStackTop) <<
            " are different" <<
            ", line " << inputLineNumber;

          msrInternalError (
            gOahOah->fInputSourceName,
            inputLineNumber,
            __FILE__, __LINE__,
            s.str ());
        }
*/
        // create a part group element
        Sxmlelement scorePartGroupElement = createElement (k_part_group, "");

        // set it's "number" attribute
        scorePartGroupElement->add (
          createIntegerAttribute (
            "number", elt->getPartGroupNumber ()));

        // set it's "type" attribute
        scorePartGroupElement->add (createAttribute ("type", "stop"));

        // append the part group element to the part list element
        fScorePartListElement->push (scorePartGroupElement);

        // pop the part group element from the stack
        fPartGroupElementsStack.pop ();
      }
      break;
  } // switch
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrPart& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();

  string
    partID =
      elt->getPartID (),
    partName =
      elt->getPartName (),
    partAbbreviation =
      elt->getPartAbbreviation (),
    partCombinedName =
      elt->getPartCombinedName ();

#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrPart " <<
      partCombinedName <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

#ifdef TRACE_OAH
  if (gTraceOah->fTraceParts) {
    fLogOutputStream <<
      endl <<
      "<!--=== part \"" << partCombinedName << "\"" <<
      ", line " << inputLineNumber << " ===-->" <<
      endl;
  }
#endif

  gIndenter++;

  // create a score part element
  Sxmlelement scorePartElement = createElement (k_score_part, "");
  // set it's "id" attribute
  scorePartElement->add (createAttribute ("id", partID));

  // append it to the part list element
  fScorePartListElement->push (scorePartElement);

  // append a part name element to the score part element
  scorePartElement->push (
    createElement (
      k_part_name,
      partName));

  if (partAbbreviation.size ()) {
    scorePartElement->push (
      createElement (
        k_part_abbreviation,
        partAbbreviation));
  }

  // create a part element
  fCurrentPartElement = createElement (k_part, "");
  // set its "id" attribute
	fCurrentPartElement->add (createAttribute ("id", partID));

  // append it to the pending part elements list
  fPendingPartElementsList.push_back (fCurrentPartElement);

  // get the part shortest note's duration
  fPartShortestNoteDuration =
    elt->getPartShortestNoteDuration ();
    /*
  fPartShortestNoteTupletFactor =
    elt->getPartShortestNoteTupletFactor ();

  // compute the divisions per quarter note
  if (fPartShortestNoteDuration > rational (1, 4)) {
    // the shortest duration should be a quarter note at most
    fPartShortestNoteDuration = rational (1, 4);
  }
*/

/*
  rational
    partShortestNoteTupletFactorAsRational =
      fPartShortestNoteTupletFactor.asRational ();
*/

  rational
    divisionsPerQuarterNoteAsRational =
      rational (1, 4)
       /
      fPartShortestNoteDuration;
  divisionsPerQuarterNoteAsRational.rationalise ();

    /* JMI
  rational
    rationalDivisionsPerQuarterNote =
      divisionsPerQuarterNoteAsRational
        *
      partShortestNoteTupletFactorAsRational;
      divisionsPerQuarterNoteAsRational.getNumerator ()
        *
      divisionsPerQuarterNoteAsRational.getDenominator ();
  rationalDivisionsPerQuarterNote.rationalise ();
    */

  // compute divisions per quarter note and multiplying factor
  fDivisionsPerQuarterNote =
    divisionsPerQuarterNoteAsRational.getNumerator ();
  fDivisionsMultiplyingFactor =
    divisionsPerQuarterNoteAsRational.getDenominator ();

  if (fDivisionsMultiplyingFactor != 1) {
    stringstream s;

    s <<
      "divisionsPerQuarterNoteAsRational '" << divisionsPerQuarterNoteAsRational <<
      "' is no integer number" <<
      ", line " << inputLineNumber;

// JMI    msrInternalError (
    msrInternalWarning (
      gOahOah->fInputSourceName,
      inputLineNumber,
//      __FILE__, __LINE__,
      s.str ());
  }

#ifdef TRACE_OAH
  if (gTraceOah->fTraceNotes) {
    fLogOutputStream <<
      "-->  partShortestNoteDuration: " <<
      fPartShortestNoteDuration <<
      endl <<
      "-->  divisionsPerQuarterNoteAsRational: " <<
      divisionsPerQuarterNoteAsRational <<
      endl <<
      /* JMI
      "-->  partShortestNoteTupletFactor: " <<
      fPartShortestNoteTupletFactor.asString () <<
      endl <<
      "-->  partShortestNoteTupletFactorAsRational: " <<
      partShortestNoteTupletFactorAsRational <<
      endl <<
      */
      "-->  divisionsPerQuarterNoteAsRational: " <<
      divisionsPerQuarterNoteAsRational <<
      endl <<
      "-->  fDivisionsPerQuarterNote: " <<
      fDivisionsPerQuarterNote <<
      endl <<
      "-->  fDivisionsMultiplyingFactor: " <<
      fDivisionsMultiplyingFactor <<
      endl;
  }
#endif

  // a divisions element has to be appended for this part
  fPartDivisionsElementHasToBeAppended = true;
}

void msr2MxmltreeTranslator::visitEnd (S_msrPart& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();

  gIndenter--;

#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrPart " <<
      elt->getPartCombinedName () <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  // forget about the current part element
  fCurrentPartElement = nullptr;
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrSegment& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber () ;

#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrSegment '" <<
      elt->getSegmentAbsoluteNumber () << "'" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  // create a start comment
  stringstream s;
  s <<
    " ==================== " <<
    "Segment " <<
    elt->getSegmentAbsoluteNumber () <<
    " START" <<
      ", line " << inputLineNumber <<
    " ==================== ";
  Sxmlelement comment = createElement (kComment, s.str ());

  // append it to the current part element
  fCurrentPartElement->push (comment);
}

void msr2MxmltreeTranslator::visitEnd (S_msrSegment& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();

#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrSegment '" <<
      elt->getSegmentAbsoluteNumber () << "'" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  // create an end comment
  stringstream s;
  s <<
    " ==================== " <<
    "Segment " <<
    elt->getSegmentAbsoluteNumber () <<
    " END" <<
      ", line " << inputLineNumber <<
    " ==================== ";
  Sxmlelement comment = createElement (kComment, s.str ());

  // append it to the current part element
  fCurrentPartElement->push (comment);
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrMeasure& elt)
{
  int
    inputLineNumber =
      elt->getInputLineNumber ();

  string
    measureNumber =
      elt->getMeasureElementMeasureNumber ();

  int
    measurePuristNumber =
      elt->getMeasurePuristNumber ();

#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrMeasure '" <<
      measureNumber <<
      "', measurePuristNumber = '" <<
      measurePuristNumber <<
      "', line " << inputLineNumber <<
      endl;
  }
#endif

#ifdef TRACE_OAH
  if (gTraceOah->fTraceMeasures) {
    fLogOutputStream <<
      endl <<
      "<!--=== measure '" << measureNumber <<
//      "', voice \"" <<
//      fCurrentVoiceClone->getVoiceName () <<
      "\"" <<
      ", line " << inputLineNumber << " ===-->" <<
      endl;
  }
#endif

  // create a measure comment
  stringstream s;
  s <<
    " ===== " <<
    "MEASURE " <<
    "ordinal number: " << elt->getMeasureOrdinalNumberInVoice () <<
    ", line " << inputLineNumber <<
    " ===== ";
  Sxmlelement comment = createElement (kComment, s.str ());

  // append it to the current part element
  fCurrentPartElement->push (comment);

  // create a measure element
  fCurrentMeasureElement = createElement (k_measure, "");
  // set its "number" attribute
	fCurrentMeasureElement->add (createAttribute ("number", measureNumber));
  // append it to the current part element
  fCurrentPartElement->push (fCurrentMeasureElement);

  // is there a divisions element to be appended?
  if (fPartDivisionsElementHasToBeAppended) {
    // append a divisions element to the attributes element
    appendSubElementToMeasureAttributes (
      createIntegerElement (
        k_divisions,
        fDivisionsPerQuarterNote));

    fPartDivisionsElementHasToBeAppended = false;
  }
}

void msr2MxmltreeTranslator::visitEnd (S_msrMeasure& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();

  string
    measureNumber =
      elt->getMeasureElementMeasureNumber ();

  string
    nextMeasureNumber =
      elt->getNextMeasureNumber ();

  int
    measurePuristNumber =
      elt->getMeasurePuristNumber ();

#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrMeasure '" <<
      measureNumber <<
      "', nextMeasureNumber = '" <<
      nextMeasureNumber <<
      "', measurePuristNumber = '" <<
      measurePuristNumber <<
      "', line " << inputLineNumber <<
      endl;
  }
#endif

  // forget about the current measure element
  fCurrentMeasureElement = nullptr;

  // forget about the current part measure attributes element
  fCurrentMeasureAttributesElement = nullptr;
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrPrintLayout& elt)
{
  int
    inputLineNumber =
      elt->getInputLineNumber ();

#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrPrintLayout '" <<
      "', line " << inputLineNumber <<
      endl;
  }
#endif

#ifdef TRACE_OAH
  if (gTraceOah->fTracePrintLayouts) {
    fLogOutputStream <<
      endl <<
//      "', voice \"" <<
//      fCurrentVoiceClone->getVoiceName () <<
      "\"" <<
      ", line " << inputLineNumber << " ===-->" <<
      endl;
  }
#endif

  // create a print comment
  stringstream s;
  s <<
    " ===== " <<
    "PrintLayout " <<
    ", line " << inputLineNumber <<
    " ===== ";
  Sxmlelement comment = createElement (kComment, s.str ());

  // append it to the current measure element
  fCurrentMeasureElement->push (comment);

  // create a print element
  fCurrentPrintElement = createElement (k_print, "");

// JMI  // set its "number" attribute
//	fCurrentPrintElement->add (createAttribute ("number", measureNumber));

  // append it to the current measure element
  fCurrentMeasureElement->push (fCurrentPrintElement);

  fOnGoingPrintElement = true;
}

void msr2MxmltreeTranslator::visitEnd (S_msrPrintLayout& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();

#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrPrintLayout '" <<
      "', line " << inputLineNumber <<
      endl;
  }
#endif

  // forget about the current print layout element
  fCurrentPrintElement = nullptr;

  fOnGoingPrintElement = false;
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrClef& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrClef" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  switch (elt->getClefKind ()) {
    case k_NoClef:
      {
        Sxmlelement clefElement = createElement (k_clef, "");

        clefElement->push (
          createElement (
            k_sign,
            "none"));
      }
      break;
    case kTrebleClef:
      {
        Sxmlelement clefElement = createElement (k_clef, "");

        clefElement->push (
          createElement (
            k_sign,
            "G"));
        clefElement->push (
          createIntegerElement (
            k_line,
            2));

        appendSubElementToMeasureAttributes (clefElement);
      }
      break;
    case kSopranoClef:
      {
        Sxmlelement clefElement = createElement (k_clef, "");

        clefElement->push (
          createElement (
            k_sign,
            "G"));
        clefElement->push (
          createIntegerElement (
            k_line,
            2));

        appendSubElementToMeasureAttributes (clefElement);
      }
      break;
    case kMezzoSopranoClef:
      {
        Sxmlelement clefElement = createElement (k_clef, "");

        clefElement->push (
          createElement (
            k_sign,
            "G"));
        clefElement->push (
          createIntegerElement (
            k_line,
            2));

        appendSubElementToMeasureAttributes (clefElement);
      }
      break;
    case kAltoClef:
      {
        Sxmlelement clefElement = createElement (k_clef, "");

        clefElement->push (
          createElement (
            k_sign,
            "G"));
        clefElement->push (
          createIntegerElement (
            k_line,
            2));

        appendSubElementToMeasureAttributes (clefElement);
      }
      break;
    case kTenorClef:
      {
        Sxmlelement clefElement = createElement (k_clef, "");

        clefElement->push (
          createElement (
            k_sign,
            "G"));
        clefElement->push (
          createIntegerElement (
            k_line,
            2));

        appendSubElementToMeasureAttributes (clefElement);
      }
      break;
    case kBaritoneClef:
      {
        Sxmlelement clefElement = createElement (k_clef, "");

        clefElement->push (
          createElement (
            k_sign,
            "G"));
        clefElement->push (
          createIntegerElement (
            k_line,
            2));

        appendSubElementToMeasureAttributes (clefElement);
      }
      break;
    case kBassClef:
      {
        Sxmlelement clefElement = createElement (k_clef, "");

        clefElement->push (
          createElement (
            k_sign,
            "F"));
        clefElement->push (
          createIntegerElement (
            k_line,
            4));

        appendSubElementToMeasureAttributes (clefElement);
      }
      break;
    case kTrebleLine1Clef:
      {
        Sxmlelement clefElement = createElement (k_clef, "");

        clefElement->push (
          createElement (
            k_sign,
            "G"));
        clefElement->push (
          createIntegerElement (
            k_line,
            1));

        appendSubElementToMeasureAttributes (clefElement);
      }
      break;
    case kTrebleMinus15Clef:
      {
        Sxmlelement clefElement = createElement (k_clef, "");

        clefElement->push (
          createElement (
            k_sign,
            "G"));
        clefElement->push (
          createIntegerElement (
            k_line,
            2));
        clefElement->push (
          createIntegerElement (
            k_clef_octave_change,
            -2));

        appendSubElementToMeasureAttributes (clefElement);
      }
      break;
    case kTrebleMinus8Clef:
      {
        Sxmlelement clefElement = createElement (k_clef, "");

        clefElement->push (
          createElement (
            k_sign,
            "G"));
        clefElement->push (
          createIntegerElement (
            k_line,
            2));
        clefElement->push (
          createIntegerElement (
            k_clef_octave_change,
            -1));

        appendSubElementToMeasureAttributes (clefElement);
      }
      break;
    case kTreblePlus8Clef:
      {
        Sxmlelement clefElement = createElement (k_clef, "");

        clefElement->push (
          createElement (
            k_sign,
            "G"));
        clefElement->push (
          createIntegerElement (
            k_line,
            2));
        clefElement->push (
          createIntegerElement (
            k_clef_octave_change,
            1));

        appendSubElementToMeasureAttributes (clefElement);
      }
      break;
    case kTreblePlus15Clef:
      {
        Sxmlelement clefElement = createElement (k_clef, "");

        clefElement->push (
          createElement (
            k_sign,
            "G"));
        clefElement->push (
          createIntegerElement (
            k_line,
            2));
        clefElement->push (
          createIntegerElement (
            k_clef_octave_change,
            2));

        appendSubElementToMeasureAttributes (clefElement);
      }
      break;
    case kBassMinus15Clef:
      {
        Sxmlelement clefElement = createElement (k_clef, "");

        clefElement->push (
          createElement (
            k_sign,
            "F"));
        clefElement->push (
          createIntegerElement (
            k_line,
            4));
        clefElement->push (
          createIntegerElement (
            k_clef_octave_change,
            -2));

        appendSubElementToMeasureAttributes (clefElement);
      }
      break;
    case kBassMinus8Clef:
      {
        Sxmlelement clefElement = createElement (k_clef, "");

        clefElement->push (
          createElement (
            k_sign,
            "F"));
        clefElement->push (
          createIntegerElement (
            k_line,
            4));
        clefElement->push (
          createIntegerElement (
            k_clef_octave_change,
            -1));

        appendSubElementToMeasureAttributes (clefElement);
      }
      break;
    case kBassPlus8Clef:
      {
        Sxmlelement clefElement = createElement (k_clef, "");

        clefElement->push (
          createElement (
            k_sign,
            "F"));
        clefElement->push (
          createIntegerElement (
            k_line,
            4));
        clefElement->push (
          createIntegerElement (
            k_clef_octave_change,
            1));

        appendSubElementToMeasureAttributes (clefElement);
      }
      break;
    case kBassPlus15Clef:
      {
        Sxmlelement clefElement = createElement (k_clef, "");

        clefElement->push (
          createElement (
            k_sign,
            "F"));
        clefElement->push (
          createIntegerElement (
            k_line,
            4));
        clefElement->push (
          createIntegerElement (
            k_clef_octave_change,
            2));

        appendSubElementToMeasureAttributes (clefElement);
      }
      break;
    case kVarbaritoneClef:
      {
        Sxmlelement clefElement = createElement (k_clef, "");

        clefElement->push (
          createElement (
            k_sign,
            "F"));
        clefElement->push (
          createIntegerElement (
            k_line,
            3));

        appendSubElementToMeasureAttributes (clefElement);
      }
      break;

    case kTablature4Clef:
      {
        Sxmlelement clefElement = createElement (k_clef, "");

        clefElement->push (
          createElement (
            k_sign,
            "tab"));
        clefElement->push (
          createIntegerElement (
            k_line,
            4));

        appendSubElementToMeasureAttributes (clefElement);
      }
    case kTablature5Clef:
      {
        Sxmlelement clefElement = createElement (k_clef, "");

        clefElement->push (
          createElement (
            k_sign,
            "tab"));
        clefElement->push (
          createIntegerElement (
            k_line,
            5));

        appendSubElementToMeasureAttributes (clefElement);
      }
      break;
    case kTablature6Clef:
      {
        Sxmlelement clefElement = createElement (k_clef, "");

        clefElement->push (
          createElement (
            k_sign,
            "tab"));
        clefElement->push (
          createIntegerElement (
            k_line,
            6));

        appendSubElementToMeasureAttributes (clefElement);
      }
    case kTablature7Clef:
      {
        Sxmlelement clefElement = createElement (k_clef, "");

        clefElement->push (
          createElement (
            k_sign,
            "tab"));
        clefElement->push (
          createIntegerElement (
            k_line,
            7));

        appendSubElementToMeasureAttributes (clefElement);
      }
      break;

    case kPercussionClef:
      {
        Sxmlelement clefElement = createElement (k_clef, "");

        clefElement->push (
          createElement (
            k_sign,
            "percussion"));

        appendSubElementToMeasureAttributes (clefElement);
      }
      break;

    case kJianpuClef:
      {
        Sxmlelement clefElement = createElement (k_clef, "");

        clefElement->push (
          createElement (
            k_sign,
            "jianpu"));

        appendSubElementToMeasureAttributes (clefElement);
      }
      break;
  } // switch
}

void msr2MxmltreeTranslator::visitEnd (S_msrClef& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrClef" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrKey& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();

#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrKey" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  switch (elt->getKeyKind ()) {
    case msrKey::kTraditionalKind:
      {
        // create the key element
        Sxmlelement keyElement = createElement (k_key, "");

        // compute the number of fifths
        const int K_NO_FIFTHS_NUMBER = -99;
        int       fifthsNumber = K_NO_FIFTHS_NUMBER;

        msrQuarterTonesPitchKind
          keyTonicQuarterTonesPitchKind =
            elt->getKeyTonicQuarterTonesPitchKind ();

        switch (keyTonicQuarterTonesPitchKind) {
          case k_NoQuarterTonesPitch_QTP:
          case k_Rest_QTP:
            // should not occur
            break;

          case kC_Natural_QTP:
            fifthsNumber = 0;
            break;
          case kG_Natural_QTP:
            fifthsNumber = 1;
            break;
          case kD_Natural_QTP:
            fifthsNumber = 2;
            break;
          case kA_Natural_QTP:
            fifthsNumber = 3;
            break;
          case kE_Natural_QTP:
            fifthsNumber = 4;
            break;
          case kB_Natural_QTP:
            fifthsNumber = 5;
            break;
          case kF_Sharp_QTP:
            fifthsNumber = 6;
            break;
          case kC_Sharp_QTP:
            fifthsNumber = 7;
            break;

          case kG_Sharp_QTP: // JMI
            fifthsNumber = 8;
            break;
          case kD_Sharp_QTP:
            fifthsNumber = 9;
            break;
          case kA_Sharp_QTP:
            fifthsNumber = 10;
            break;
          case kE_Sharp_QTP:
            fifthsNumber = 11;
            break;

          case kF_Natural_QTP:
            fifthsNumber = -1;
            break;
          case kB_Flat_QTP:
            fifthsNumber = -2;
            break;
          case kE_Flat_QTP:
            fifthsNumber = -3;
            break;
          case kA_Flat_QTP:
            fifthsNumber = -4;
            break;
          case kD_Flat_QTP:
            fifthsNumber = -5;
            break;
          case kG_Flat_QTP:
            fifthsNumber = -6;
            break;
          case kC_Flat_QTP:
            fifthsNumber = -7;
            break;

          case kF_Flat_QTP: // JMI
            fifthsNumber = -8;
            break;
          case kB_DoubleFlat_QTP:
            fifthsNumber = -9;
            break;
          case kE_DoubleFlat_QTP:
            fifthsNumber = -10;
            break;
          case kA_DoubleFlat_QTP:
            fifthsNumber = -11;
            break;

          default:
            // should not occur
            break;
        } // switch

        if (fifthsNumber != K_NO_FIFTHS_NUMBER) {
          // populate the key element
          keyElement->push (
            createIntegerElement (
              k_fifths,
              fifthsNumber));

          keyElement->push (
            createElement (
              k_mode,
              msrKey::keyModeKindAsString (elt->getKeyModeKind ())));

          // append it to the attributes element
          appendSubElementToMeasureAttributes (keyElement);
        }

        else {
          stringstream s;

          s <<
            "key fifthsNumber not specified for key '" <<
            elt->asShortString ();

          msrInternalError (
            gOahOah->fInputSourceName,
            inputLineNumber,
            __FILE__, __LINE__,
            s.str ());
        }
      }
      break;

    case msrKey::kHumdrumScotKind:
      {
        // JMI
      }
      break;
  } // switch
}

void msr2MxmltreeTranslator::visitEnd (S_msrKey& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrKey" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrTime& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrTime" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

/*
  if       (timeSymbol == "common") {
    fCurrentTimeSymbolKind = msrTime::kTimeSymbolCommon;
  }
  else  if (timeSymbol == "cut") {
    fCurrentTimeSymbolKind = msrTime::kTimeSymbolCut;
  }
  else  if (timeSymbol == "note") {
    fCurrentTimeSymbolKind = msrTime::kTimeSymbolNote;
  }
  else  if (timeSymbol == "dotted-note") {
    fCurrentTimeSymbolKind = msrTime::kTimeSymbolDottedNote;
  }
  else  if (timeSymbol == "single-number") {
    fCurrentTimeSymbolKind = msrTime::kTimeSymbolSingleNumber;
  }


  // handle the time
  if (timeSymbolKind == msrTime::kTimeSymbolSenzaMisura) {
    // senza misura time

    fVoiceIsCurrentlySenzaMisura = true;
  }

  else {
    // con misura time

    int timesItemsNumber =
      timeItemsVector.size ();

    if (timesItemsNumber) {
      // should there be a single number?
      switch (timeSymbolKind) {
        case msrTime::kTimeSymbolCommon:
          break;
        case msrTime::kTimeSymbolCut:
          break;
        case msrTime::kTimeSymbolNote:
          break;
        case msrTime::kTimeSymbolDottedNote:
          break;
        case msrTime::kTimeSymbolSingleNumber:
          fLilypondCodeOstream <<
            "\\once\\override Staff.TimeSignature.style = #'single-digit" <<
            endl;
          break;
        case msrTime::kTimeSymbolSenzaMisura:
          break;
        case msrTime::kTimeSymbolNone:
          break;
      } // switch

      if (! elt->getTimeIsCompound ()) {
        // simple time
        // \time "3/4" for 3/4
        // or senza misura

        S_msrTimeItem
          timeItem =
            timeItemsVector [0]; // the only element;

        // fetch the time item beat numbers vector
        const vector<int>&
          beatsNumbersVector =
            timeItem->
              getTimeBeatsNumbersVector ();

        // should the time be numeric?
        if (
          timeSymbolKind == msrTime::kTimeSymbolNone
            ||
          gLilypondOah->fNumericalTime) {
          fLilypondCodeOstream <<
            "\\numericTimeSignature ";
        }

        fLilypondCodeOstream <<
          "\\time " <<
          beatsNumbersVector [0] << // the only element
          "/" <<
          timeItem->getTimeBeatValue () <<
          endl;
      }

      else {
        // compound time
        // \compoundMeter #'(3 2 8) for 3+2/8
        // \compoundMeter #'((3 8) (2 8) (3 4)) for 3/8+2/8+3/4
        // \compoundMeter #'((3 2 8) (3 4)) for 3+2/8+3/4

        fLilypondCodeOstream <<
          "\\compoundMeter #`(";

        // handle all the time items in the vector
        for (int i = 0; i < timesItemsNumber; i++) {
          S_msrTimeItem
            timeItem =
              timeItemsVector [i];

          // fetch the time item beat numbers vector
          const vector<int>&
            beatsNumbersVector =
              timeItem->
                getTimeBeatsNumbersVector ();

          int beatsNumbersNumber =
            beatsNumbersVector.size ();

          // first generate the opening parenthesis
          fLilypondCodeOstream <<
            "(";

          // then generate all beats numbers in the vector
          for (int j = 0; j < beatsNumbersNumber; j++) {
            fLilypondCodeOstream <<
              beatsNumbersVector [j] <<
              ' ';
          } // for

          // then generate the beat type
          fLilypondCodeOstream <<
            timeItem->getTimeBeatValue ();

          // and finally generate the closing parenthesis
          fLilypondCodeOstream <<
            ")";

          if (i != timesItemsNumber - 1) {
            fLilypondCodeOstream <<
              ' ';
          }
        } // for

      fLilypondCodeOstream <<
        ")" <<
        endl;
      }
    }

    else {
      // there are no time items
      if (timeSymbolKind != msrTime::kTimeSymbolSenzaMisura) {
        msrInternalError (
          gOahOah->fInputSourceName,
          elt->getInputLineNumber (),
          __FILE__, __LINE__,
          "time items vector is empty");
      }
    }
  }
*/

  // create a time element
  Sxmlelement timeElement = createElement (k_time, "");

  switch (elt->getTimeSymbolKind ()) {
    case msrTime::kTimeSymbolCommon:
      {
        timeElement->add (createAttribute ("symbol", "common"));

        timeElement->push (
          createIntegerElement (
            k_beats,
            4));
        timeElement->push (
          createIntegerElement (
            k_beat_type,
            4));
      }
      break;

    case msrTime::kTimeSymbolCut:
       {
        timeElement->add (createAttribute ("symbol", "cut"));

        timeElement->push (
          createIntegerElement (
            k_beats,
            2));
        timeElement->push (
          createIntegerElement (
            k_beat_type,
            2));
      }
     break;

    case msrTime::kTimeSymbolNote:
      break;

    case msrTime::kTimeSymbolDottedNote:
      break;

    case msrTime::kTimeSymbolSingleNumber:
      break;

    case msrTime::kTimeSymbolSenzaMisura:
      break;

    case msrTime::kTimeSymbolNone:
      {
        const vector<S_msrTimeItem>&
          timeItemsVector =
            elt->getTimeItemsVector ();

        if (! elt->getTimeIsCompound ()) {
          // simple time
          // \time "3/4" for 3/4
          // or senza misura

          S_msrTimeItem
            timeItem =
              timeItemsVector [0]; // the only element;

          // fetch the time item beat numbers vector
          const vector<int>&
            beatsNumbersVector =
              timeItem->
                getTimeBeatsNumbersVector ();

          timeElement->push (
            createIntegerElement (
              k_beats,
              beatsNumbersVector [0])); // the only element
          timeElement->push (
            createIntegerElement (
              k_beat_type,
              timeItem->getTimeBeatValue ()));
        }

        else {
          // compound time
          // \compoundMeter #'(3 2 8) for 3+2/8
          // \compoundMeter #'((3 8) (2 8) (3 4)) for 3/8+2/8+3/4
          // \compoundMeter #'((3 2 8) (3 4)) for 3+2/8+3/4

        }
      }
      break;
  } // switch

  appendSubElementToMeasureAttributes (timeElement);
}

void msr2MxmltreeTranslator::visitEnd (S_msrTime& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrTime" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrTempo& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();

#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrTempo" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

/*
  const list<S_msrWords>&
    tempoWordsList =
      elt->getTempoWordsList ();

 int tempoWordsListSize = tempoWordsList.size ();
*/

  msrDottedDuration tempoBeatUnit  = elt->getTempoBeatUnit ();
  string            tempoPerMinute = elt->getTempoPerMinute ();

  msrTempo::msrTempoParenthesizedKind
    tempoParenthesizedKind =
      elt->getTempoParenthesizedKind ();

  msrPlacementKind
    tempoPlacementKind =
      elt->getTempoPlacementKind ();

  switch (elt->getTempoKind ()) {
    case msrTempo::k_NoTempoKind:
      break;

    case msrTempo::kTempoBeatUnitsWordsOnly:
      {
    /*
        fLilypondCodeOstream <<
          "\\tempo ";

        if (tempoWordsListSize) {
          list<S_msrWords>::const_iterator
            iBegin = tempoWordsList.begin (),
            iEnd   = tempoWordsList.end (),
            i      = iBegin;

          for ( ; ; ) {
            S_msrWords words = (*i);

            fLilypondCodeOstream <<
              "\"" << words->getWordsContents () << "\"";

            if (++i == iEnd) break;

            fLilypondCodeOstream <<
              ' ';
          } // for

          fLilypondCodeOstream <<
            endl;
        }
        */
        }
      break;

    case msrTempo::kTempoBeatUnitsPerMinute:
      switch (tempoParenthesizedKind) {
        case msrTempo::kTempoParenthesizedYes:
          {
          // create the metronome element
          Sxmlelement metronomeElement = createElement (k_metronome, "");

          // set its "parentheses" attribute
          metronomeElement->add (createAttribute ("parentheses", "yes"));

          // append the beat unit element to the metronome elements
          msrDurationKind
            durationKind =
              tempoBeatUnit.getDurationKind ();
              /* JMI
          int
            dotsNumber =
              tempoBeatUnit.getDotsNumber ();
              */

          metronomeElement-> push (
            createElement (
              k_beat_unit,
              msrDurationKindAsMusicXMLType (durationKind)));

          // append the per minute element to the metronome elements
          metronomeElement-> push (createElement (k_per_minute, tempoPerMinute));

          // append the dynamics element to the current measure element
          appendSubElementToMeasureDirection (
            metronomeElement,
            tempoPlacementKind);

  /*
          if (tempoWordsListSize) {
            list<S_msrWords>::const_iterator
              iBegin = tempoWordsList.begin (),
              iEnd   = tempoWordsList.end (),
              i      = iBegin;

            for ( ; ; ) {
              S_msrWords words = (*i);

              fLilypondCodeOstream <<
                "\"" << words->getWordsContents () << "\"";

              if (++i == iEnd) break;

              fLilypondCodeOstream <<
                ' ';
            } // for
          }
          */
        }
        break;

      case msrTempo::kTempoParenthesizedNo:
        {
          // create the metronome element
          Sxmlelement metronomeElement = createElement (k_metronome, "");

          // append the beat unit element to the metronome elements
          msrDurationKind
            durationKind =
              tempoBeatUnit.getDurationKind ();
              /*
          int
            dotsNumber =
              tempoBeatUnit.getDotsNumber ();
              */

          metronomeElement-> push (
            createElement (
              k_beat_unit,
              msrDurationKindAsMusicXMLType (durationKind)));

          // append the per minute element to the metronome elements
          metronomeElement-> push (createElement (k_per_minute, tempoPerMinute));

          // append the metronome element to the current measure element
          appendSubElementToMeasureDirection (
            metronomeElement,
            tempoPlacementKind);

  /*
          if (tempoWordsListSize) {
            list<S_msrWords>::const_iterator
              iBegin = tempoWordsList.begin (),
              iEnd   = tempoWordsList.end (),
              i      = iBegin;

            for ( ; ; ) {
              S_msrWords words = (*i);

              fLilypondCodeOstream <<
                "\"" << words->getWordsContents () << "\"";

              if (++i == iEnd) break;

              fLilypondCodeOstream <<
                ' ';
            } // for
          }
          */
        }
          break;
        } // switch
      break;

    case msrTempo::kTempoBeatUnitsEquivalence:
      {
    /*
      fLilypondCodeOstream <<
        "\\tempo ";

      if (tempoWordsListSize) {
        list<S_msrWords>::const_iterator
          iBegin = tempoWordsList.begin (),
          iEnd   = tempoWordsList.end (),
          i      = iBegin;

        for ( ; ; ) {
          S_msrWords words = (*i);

          fLilypondCodeOstream <<
            "\"" << words->getWordsContents () << "\"";

          if (++i == iEnd) break;

          fLilypondCodeOstream <<
            ' ';
        } // for
      }

      fLilypondCodeOstream <<
        ' ' <<
        "\\markup {" <<
        endl;

      gIndenter++;

      fLilypondCodeOstream <<
        "\\concat {" <<
        endl;

      gIndenter++;

      switch (tempoParenthesizedKind) {
        case msrTempo::kTempoParenthesizedYes:
          fLilypondCodeOstream <<
            "(" <<
            endl;
          break;
        case msrTempo::kTempoParenthesizedNo:
          break;
      } // switch

      gIndenter++;

      if (gLpsrOah->versionNumberGreaterThanOrEqualTo ("2.20")) {
        fLilypondCodeOstream <<
          " \\smaller \\general-align #Y #DOWN \\note {";
      }
      else {
        fLilypondCodeOstream <<
          " \\smaller \\general-align #Y #DOWN \\note #\"";
      }

      fLilypondCodeOstream <<
        dottedDurationAsLilypondStringWithoutBackSlash (
          inputLineNumber,
          tempoBeatUnit);

      if (gLpsrOah->versionNumberGreaterThanOrEqualTo ("2.20")) {
        fLilypondCodeOstream <<
          "} #UP";
      }
      else {
        fLilypondCodeOstream <<
          "\" #UP";
      }

      fLilypondCodeOstream <<
        endl <<
        "\" = \"" <<
        endl;

      fLilypondCodeOstream <<
        "(";
      if (gLpsrOah->versionNumberGreaterThanOrEqualTo ("2.20")) {
        fLilypondCodeOstream <<
          " \\smaller \\general-align #Y #DOWN \\note {";
      }
      else {
        fLilypondCodeOstream <<
          " \\smaller \\general-align #Y #DOWN \\note #\"";
      }

      fLilypondCodeOstream <<
        dottedDurationAsLilypondStringWithoutBackSlash (
          inputLineNumber,
          elt->getTempoEquivalentBeatUnit ());

      if (gLpsrOah->versionNumberGreaterThanOrEqualTo ("2.20")) {
        fLilypondCodeOstream <<
          "} #UP";
      }
      else {
        fLilypondCodeOstream <<
          "\" #UP";
      }

      fLilypondCodeOstream << endl;

      gIndenter--;

      switch (tempoParenthesizedKind) {
        case msrTempo::kTempoParenthesizedYes:
          fLilypondCodeOstream <<
            ")" <<
            endl;
          break;
        case msrTempo::kTempoParenthesizedNo:
          break;
      } // switch

      gIndenter--;

      fLilypondCodeOstream <<
        "}" <<
        endl;

      gIndenter--;

      fLilypondCodeOstream <<
        "}" <<
        endl;
        */
        }
      break;

    case msrTempo::kTempoNotesRelationShip:
      {
    /*
      fLilypondCodeOstream <<
        "\\tempoRelationship #\"";

      if (tempoWordsListSize) {
        list<S_msrWords>::const_iterator
          iBegin = tempoWordsList.begin (),
          iEnd   = tempoWordsList.end (),
          i      = iBegin;

        for ( ; ; ) {
          S_msrWords words = (*i);

          fLilypondCodeOstream <<
     // JMI       "\"" <<
            words->getWordsContents (); // JMI <<
      // JMI      "\"";

          if (++i == iEnd) break;

          fLilypondCodeOstream <<
            ' ';
        } // for
      }

      fLilypondCodeOstream <<
        "\"";

      switch (tempoParenthesizedKind) {
        case msrTempo::kTempoParenthesizedYes:
          fLilypondCodeOstream <<
            " ##t";
          break;
        case msrTempo::kTempoParenthesizedNo:
          fLilypondCodeOstream <<
            " ##f";
          break;
      } // switch

      fLilypondCodeOstream << endl;
      */
      }
      break;
  } // switch
}

void msr2MxmltreeTranslator::visitEnd (S_msrTempo& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrTempo" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrChord& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();

#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrChord" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  // create a chord start comment
  stringstream s;
  s <<
    " ===== " <<
    "Chord start " <<
    ", chordSoundingWholeNotes: " <<
    elt->getChordSoundingWholeNotes () <<
    ", " <<
    elt->getChordNotesVector ().size () <<
    " elements" <<
    ", line " << inputLineNumber <<
    " ===== ";
  fPendingChordStartComment = createElement (kComment, s.str ());

  // append it to the current measure element
  fCurrentMeasureElement->push (fPendingChordStartComment);
}

void msr2MxmltreeTranslator::visitEnd (S_msrChord& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();

#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrChord" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  // create a chord end comment
  stringstream s;
  s <<
    " ===== " <<
    "Chord end " <<
    ", chordSoundingWholeNotes: " <<
    elt->getChordSoundingWholeNotes () <<
    ", " <<
    elt->getChordNotesVector ().size () <<
    " elements" <<
    ", line " << inputLineNumber <<
    " ===== ";
  Sxmlelement comment = createElement (kComment, s.str ());

  // append it to the current measure element
  fCurrentMeasureElement->push (comment);

  // forget about the pending chord start comment
  fPendingChordStartComment = nullptr;
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrTuplet& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();

#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrTuplet" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  // create a tuplet start comment
  stringstream s;
  s <<
    " ===== " <<
    "Tuplet start " <<
    ", tupletFactor: " <<
    elt->getTupletFactor ().asRational () <<
    ", " <<
    elt->getTupletElementsList ().size () << " elements" <<
    ", line " << inputLineNumber <<
    " ===== ";
  Sxmlelement comment = createElement (kComment, s.str ());

  // append it to the current measure element
  fCurrentMeasureElement->push (comment);
}

void msr2MxmltreeTranslator::visitEnd (S_msrTuplet& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();

#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrTuplet" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  // create a tuplet end comment
  stringstream s;
  s <<
    " ===== " <<
    "Tuplet end " <<
    ", tupletFactor: " <<
    elt->getTupletFactor ().asRational () <<
    ", tupletElementsList: " <<
    elt->getTupletElementsList ().size () << " elements" <<
    ", line " << inputLineNumber <<
    " ===== ";
  Sxmlelement comment = createElement (kComment, s.str ());

  // append it to the current measure element
  fCurrentMeasureElement->push (comment);
}

//________________________________________________________________________
void msr2MxmltreeTranslator:: appendNoteWedges (S_msrNote note)
{
#ifdef TRACE_OAH
  if (gTraceOah->fTraceNotes) {
    fLogOutputStream <<
      "--> appendNoteWedges, note = " <<
      note->asShortString () <<
      endl;
  }
#endif

  const list<S_msrWedge>&
    noteWedges =
      note->getNoteWedges () ;

  if (noteWedges.size ()) {
    list<S_msrWedge>::const_iterator i;

    for (i=noteWedges.begin (); i!=noteWedges.end (); i++) {
      S_msrWedge wedge = (*i);

      msrWedge::msrWedgeKind wedgeKind = wedge->getWedgeKind ();

      string typeString;

      switch (wedgeKind) {
        case msrWedge::kWedgeKindNone:
          // should not occur
          break;
        case msrWedge::kWedgeCrescendo:
          typeString = "crescendo";
          break;
        case msrWedge::kWedgeDecrescendo:
          typeString = "diminuendo";
          break;
        case msrWedge::kWedgeStop:
          typeString = "stop";
          break;
      } // switch

      // create the wedge element
      Sxmlelement wedgeElement = createElement (k_wedge, "");

      // set its "type" attribute
      wedgeElement->add (createAttribute ("type", typeString));

      // append the wedge element to the current measure element
      appendSubElementToMeasureDirection (
        wedgeElement,
        wedge->getWedgePlacementKind ());
    } // for
  }
}

//________________________________________________________________________
void msr2MxmltreeTranslator:: appendNoteDynamics (S_msrNote note)
{
#ifdef TRACE_OAH
  if (gTraceOah->fTraceNotes) {
    fLogOutputStream <<
      "--> appendNoteDynamics, note = " <<
      note->asShortString () <<
      endl;
  }
#endif

  const list<S_msrDynamics>&
    noteDynamics =
      note->getNoteDynamics () ;

  if (noteDynamics.size ()) {
    list<S_msrDynamics>::const_iterator i;

    for (i=noteDynamics.begin (); i!=noteDynamics.end (); i++) {
      S_msrDynamics dynamics = (*i);

      // create the dynamics element
      Sxmlelement dynamicsElement = createElement (k_dynamics, "");

      // create the dynamics specific sub-element
      int subElementID = -1;

      switch (dynamics->getDynamicsKind ()) {
        case msrDynamics::kF:
          subElementID = k_f;
          break;
        case msrDynamics::kFF:
          subElementID = k_ff;
          break;
        case msrDynamics::kFFF:
          subElementID = k_fff;
          break;
        case msrDynamics::kFFFF:
          subElementID = k_ffff;
          break;
        case msrDynamics::kFFFFF:
          subElementID = k_fffff;
          break;
        case msrDynamics::kFFFFFF:
          subElementID = k_ffffff;
          break;

        case msrDynamics::kP:
          subElementID = k_p;
          break;
        case msrDynamics::kPP:
          subElementID = k_pp;
          break;
        case msrDynamics::kPPP:
          subElementID = k_ppp;
          break;
        case msrDynamics::kPPPP:
          subElementID = k_pppp;
          break;
        case msrDynamics::kPPPPP:
          subElementID = k_ppppp;
          break;
        case msrDynamics::kPPPPPP:
          subElementID = k_pppppp;
          break;

        case msrDynamics::kMF:
          subElementID = k_mf;
          break;
        case msrDynamics::kMP:
          subElementID = k_mp;
          break;
        case msrDynamics::kFP:
          subElementID = k_fp;
          break;
        case msrDynamics::kFZ:
          subElementID = k_fz;
          break;
        case msrDynamics::kRF:
          subElementID = k_rf;
          break;
        case msrDynamics::kSF:
          subElementID = k_sf;
          break;

        case msrDynamics::kRFZ:
          subElementID = k_rfz;
          break;
        case msrDynamics::kSFZ:
          subElementID = k_sfz;
          break;
        case msrDynamics::kSFP:
          subElementID = k_sfp;
          break;
        case msrDynamics::kSFPP:
          subElementID = k_sfpp;
          break;
        case msrDynamics::kSFFZ:
          subElementID = k_sffz;
          break;
        case msrDynamics::k_NoDynamics:
          ; // should not occur
          break;
      } // switch

      Sxmlelement dynamicsSpecificSubElement = createElement (subElementID, "");

      // append the dynamics specific sub-element to the dynamics elements
      dynamicsElement-> push (dynamicsSpecificSubElement);

      // append the dynamics element to the current measure element
      appendSubElementToMeasureDirection (
        dynamicsElement,
        dynamics->getDynamicsPlacementKind ());
    } // for
  }
}

//________________________________________________________________________
void msr2MxmltreeTranslator:: appendSubElementsToNoteDirections (S_msrNote note)
{
#ifdef TRACE_OAH
  if (gTraceOah->fTraceNotes) {
    fLogOutputStream <<
      "--> appendSubElementsToNoteDirections, note = " <<
      note->asShortString () <<
      endl;
  }
#endif

/*
<!ELEMENT direction-type (rehearsal+ | segno+ | coda+ |
	(words | symbol)+ | wedge | dynamics+ | dashes |
	bracket | pedal | metronome | octave-shift | harp-pedals |
	damp | damp-all | eyeglasses | string-mute |
	scordatura | image | principal-voice | percussion+ |
	accordion-registration | staff-divide | other-direction)>
<!ATTLIST direction-type
    %optional-unique-id;
>
*/

  // append the wedges elements if any
  appendNoteWedges (note);

  // append the dynamics elements if any
  appendNoteDynamics (note);
}

//________________________________________________________________________
void msr2MxmltreeTranslator:: appendNoteOrnaments (S_msrNote note)
{
#ifdef TRACE_OAH
  if (gTraceOah->fTraceNotes) {
    fLogOutputStream <<
      "--> appendNoteOrnaments, note = " <<
      note->asShortString () <<
      endl;
  }
#endif

/* JMI
<!ELEMENT ornaments
	(((trill-mark
	| turn | delayed-turn | inverted-turn |
	   delayed-inverted-turn | vertical-turn |
	   inverted-vertical-turn | shake | wavy-line |
	   mordent | inverted-mordent | schleifer | tremolo |
	   haydn | other-ornament), accidental-mark*)*)>
<!ATTLIST ornaments
    %optional-unique-id;
>
<!ELEMENT trill-mark EMPTY>
<!ATTLIST trill-mark
    %print-style;
    %placement;
    %trill-sound;
>
	  */

  // append the ornament elements if any
  const list<S_msrOrnament>&
    noteOrnaments =
      note->getNoteOrnaments () ;

  if (noteOrnaments.size ()) {
    list<S_msrOrnament>::const_iterator i;

    for (i=noteOrnaments.begin (); i!=noteOrnaments.end (); i++) {
      S_msrOrnament ornament = (*i);

      msrOrnament::msrOrnamentKind
        ornamentKind =
          ornament->getOrnamentKind ();

      int ornamentType = kComment; // JMI

      switch (ornamentKind) {
        case msrOrnament::kOrnamentTrill:
          ornamentType = k_trill_mark;
          break;
        case msrOrnament::kOrnamentTurn:
          ornamentType = k_turn;
          break;
        case msrOrnament::kOrnamentInvertedTurn:
          ornamentType = k_inverted_turn;
          break;
        case msrOrnament::kOrnamentDelayedTurn:
          ornamentType = k_delayed_turn;
          break;
        case msrOrnament::kOrnamentDelayedInvertedTurn:
          ornamentType = k_delayed_inverted_turn;
          break;
        case msrOrnament::kOrnamentVerticalTurn:
          ornamentType = k_vertical_turn;
          break;
        case msrOrnament::kOrnamentMordent:
          ornamentType = k_mordent;
          break;
        case msrOrnament::kOrnamentInvertedMordent:
          ornamentType = k_inverted_mordent;
          break;
        case msrOrnament::kOrnamentSchleifer:
          ornamentType = k_schleifer;
          break;
        case msrOrnament::kOrnamentShake:
          ornamentType = k_shake;
          break;
        case msrOrnament::kOrnamentAccidentalMark:
          ornamentType = k_accidental_mark;
          break;
      } // switch

      // append the note ornament element to the current note element
      switch (ornamentKind) {
        case msrOrnament::kOrnamentAccidentalMark:
          {
            Sxmlelement ornamentElement =
              createElement (
                ornamentType,
                accidentalKindAsMusicXMLString (
                  ornament->getOrnamentAccidentalMark ()));

            appendSubElementToNoteNotations (
              ornamentElement,
              ornament->getOrnamentPlacementKind ());
          }
          break;

        default:
          {
            Sxmlelement ornamentElement =
              createElement (
                ornamentType,
                "");

            appendSubElementToNoteNotationsOrnaments (
              ornamentElement,
              ornament->getOrnamentPlacementKind ());
          }
      } // switch
    } // for
  }
}

//________________________________________________________________________
void msr2MxmltreeTranslator:: appendNoteTechnicals (S_msrNote note)
{
#ifdef TRACE_OAH
  if (gTraceOah->fTraceNotes) {
    fLogOutputStream <<
      "--> appendNoteTechnicals, note = " <<
      note->asShortString () <<
      endl;
  }
#endif

/* JMI
<!ELEMENT technical
	((up-bow | down-bow | harmonic | open-string |
	  thumb-position | fingering | pluck | double-tongue |
	  triple-tongue | stopped | snap-pizzicato | fret |
	  string | hammer-on | pull-off | bend | tap | heel |
	  toe | fingernails | hole | arrow | handbell |
	  brass-bend | flip | smear | open | half-muted |
	  harmon-mute | golpe | other-technical)*)>
<!ATTLIST technical
    %optional-unique-id;

	  */

  // append the technical elements if any
  const list<S_msrTechnical>&
    noteTechnicals =
      note->getNoteTechnicals () ;

  if (noteTechnicals.size ()) {
    list<S_msrTechnical>::const_iterator i;

    for (i=noteTechnicals.begin (); i!=noteTechnicals.end (); i++) {
      S_msrTechnical
        technical = (*i);

      msrTechnical::msrTechnicalKind
        technicalKind =
          technical->getTechnicalKind ();

      int technicalType = kComment; // JMI

      switch (technicalKind) {
        case msrTechnical::kArrow:
          technicalType = k_arrow;
          break;
        case msrTechnical::kDoubleTongue:
          technicalType = k_double_tongue;
          break;
        case msrTechnical::kDownBow:
          technicalType = k_down_bow;
          break;
        case msrTechnical::kFingernails:
          technicalType = k_fingernails;
          break;
        case msrTechnical::kHarmonic:
          technicalType = k_harmonic;
          break;
        case msrTechnical::kHeel:
          technicalType = k_heel;
          break;
        case msrTechnical::kHole:
          technicalType = k_hole;
          break;
        case msrTechnical::kOpenString:
          technicalType = k_open_string;
          break;
        case msrTechnical::kSnapPizzicato:
          technicalType = k_snap_pizzicato;
          break;
        case msrTechnical::kStopped:
          technicalType = k_stopped;
          break;
        case msrTechnical::kTap:
          technicalType = k_tap;
          break;
        case msrTechnical::kThumbPosition:
          technicalType = k_thumb_position;
          break;
        case msrTechnical::kToe:
          technicalType = k_toe;
          break;
        case msrTechnical::kTripleTongue:
          technicalType = k_triple_tongue;
          break;
        case msrTechnical::kUpBow:
          technicalType = k_up_bow;
          break;
      } // switch

      // create the technical element
      Sxmlelement technicalElement = createElement (technicalType, "");

      // append the note technicals element to the current note element
      appendSubElementToNoteNotationsTechnicals (
        technicalElement,
        technical->getTechnicalPlacementKind ());
    } // for
  }
}

//________________________________________________________________________
void msr2MxmltreeTranslator:: appendNoteTechnicalWithIntegers (
  S_msrNote note)
{
#ifdef TRACE_OAH
  if (gTraceOah->fTraceNotes) {
    fLogOutputStream <<
      "--> appendNoteTechnicalWithIntegers, note = " <<
      note->asShortString () <<
      endl;
  }
#endif

/* JMI
<!ELEMENT technical
	((up-bow | down-bow | harmonic | open-string |
	  thumb-position | fingering | pluck | double-tongue |
	  triple-tongue | stopped | snap-pizzicato | fret |
	  string | hammer-on | pull-off | bend | tap | heel |
	  toe | fingernails | hole | arrow | handbell |
	  brass-bend | flip | smear | open | half-muted |
	  harmon-mute | golpe | other-technical)*)>
<!ATTLIST technical
    %optional-unique-id;

	  */

  // append the technicalWithInteger elements if any
  const list<S_msrTechnicalWithInteger>&
    noteTechnicalWithIntegers =
      note->getNoteTechnicalWithIntegers () ;

  if (noteTechnicalWithIntegers.size ()) {
    list<S_msrTechnicalWithInteger>::const_iterator i;

    for (i=noteTechnicalWithIntegers.begin (); i!=noteTechnicalWithIntegers.end (); i++) {
      S_msrTechnicalWithInteger
        technicalWithInteger = (*i);

      msrTechnicalWithInteger::msrTechnicalWithIntegerKind
        technicalWithIntegerKind =
          technicalWithInteger->getTechnicalWithIntegerKind ();

      int technicalWithIntegerType = kComment; // JMI

      switch (technicalWithIntegerKind) {
        case msrTechnicalWithInteger::kFingering:
          technicalWithIntegerType = k_fingering;
          break;
        case msrTechnicalWithInteger::kFret:
          technicalWithIntegerType = k_fret;
          break;
        case msrTechnicalWithInteger::kString:
          technicalWithIntegerType = k_string;
          break;
      } // switch

      // create the technicalWithInteger element
      int technicalWithIntegerValue =
        technicalWithInteger->getTechnicalWithIntegerValue ();

      Sxmlelement technicalWithIntegerElement =
        technicalWithIntegerValue > 0
          ? createIntegerElement (
              technicalWithIntegerType,
              technicalWithIntegerValue)
          : createElement (
              technicalWithIntegerType,
              "");

      // append the note technicalWithIntegers element to the current note element
      appendSubElementToNoteNotationsTechnicals (
        technicalWithIntegerElement,
        technicalWithInteger->getTechnicalWithIntegerPlacementKind ());
    } // for
  }
}

//________________________________________________________________________
void msr2MxmltreeTranslator:: appendNoteTechnicalWithFloats (
  S_msrNote note)
{
#ifdef TRACE_OAH
  if (gTraceOah->fTraceNotes) {
    fLogOutputStream <<
      "--> appendNoteTechnicalWithFloats, note = " <<
      note->asShortString () <<
      endl;
  }
#endif

/* JMI
<!ELEMENT technical
	((up-bow | down-bow | harmonic | open-string |
	  thumb-position | fingering | pluck | double-tongue |
	  triple-tongue | stopped | snap-pizzicato | fret |
	  string | hammer-on | pull-off | bend | tap | heel |
	  toe | fingernails | hole | arrow | handbell |
	  brass-bend | flip | smear | open | half-muted |
	  harmon-mute | golpe | other-technical)*)>
<!ATTLIST technical
    %optional-unique-id;

	  */

  // append the technicalWithFloat elements if any
  const list<S_msrTechnicalWithFloat>&
    noteTechnicalWithFloats =
      note->getNoteTechnicalWithFloats () ;

  if (noteTechnicalWithFloats.size ()) {
    list<S_msrTechnicalWithFloat>::const_iterator i;

    for (i=noteTechnicalWithFloats.begin (); i!=noteTechnicalWithFloats.end (); i++) {
      S_msrTechnicalWithFloat
        technicalWithFloat = (*i);

      msrTechnicalWithFloat::msrTechnicalWithFloatKind
        technicalWithFloatKind =
          technicalWithFloat->getTechnicalWithFloatKind ();

      int technicalWithFloatType = kComment; // JMI

      switch (technicalWithFloatKind) {
        case msrTechnicalWithFloat::kBend:
          technicalWithFloatType = k_bend;
          break;
      } // switch

      // create the technicalWithFloat element
      stringstream s;

      s << technicalWithFloat->getTechnicalWithFloatValue ();

      Sxmlelement technicalWithFloatElement =
        createElement (
          technicalWithFloatType,
          s.str ());

      // append the note technicalWithFloats element to the current note element
      appendSubElementToNoteNotationsTechnicals (
        technicalWithFloatElement,
        technicalWithFloat->getTechnicalWithFloatPlacementKind ());
    } // for
  }
}

//________________________________________________________________________
void msr2MxmltreeTranslator:: appendNoteTechnicalWithStrings (
  S_msrNote note)
{
#ifdef TRACE_OAH
  if (gTraceOah->fTraceNotes) {
    fLogOutputStream <<
      "--> appendNoteTechnicalWithStrings, note = " <<
      note->asShortString () <<
      endl;
  }
#endif

/* JMI
<!ELEMENT technical
	((up-bow | down-bow | harmonic | open-string |
	  thumb-position | fingering | pluck | double-tongue |
	  triple-tongue | stopped | snap-pizzicato | fret |
	  string | hammer-on | pull-off | bend | tap | heel |
	  toe | fingernails | hole | arrow | handbell |
	  brass-bend | flip | smear | open | half-muted |
	  harmon-mute | golpe | other-technical)*)>
<!ATTLIST technical
    %optional-unique-id;

	  */

  // append the technicalWithString elements if any
  const list<S_msrTechnicalWithString>&
    noteTechnicalWithStrings =
      note->getNoteTechnicalWithStrings () ;

  if (noteTechnicalWithStrings.size ()) {
    list<S_msrTechnicalWithString>::const_iterator i;

    for (i=noteTechnicalWithStrings.begin (); i!=noteTechnicalWithStrings.end (); i++) {
      S_msrTechnicalWithString
        technicalWithString = (*i);

      msrTechnicalWithString::msrTechnicalWithStringKind
        technicalWithStringKind =
          technicalWithString->getTechnicalWithStringKind ();

      int technicalWithStringType = kComment; // JMI

      switch (technicalWithStringKind) {
        case msrTechnicalWithString::kHammerOn:
          technicalWithStringType = k_hammer_on;
          break;
        case msrTechnicalWithString::kHandbell:
          technicalWithStringType = k_handbell;
          break;
        case msrTechnicalWithString::kOtherTechnical:
          technicalWithStringType = k_other_technical;
          break;
        case msrTechnicalWithString::kPluck:
          technicalWithStringType = k_pluck;
          break;
        case msrTechnicalWithString::kPullOff:
          technicalWithStringType = k_pull_off;
          break;
      } // switch

      // create the technicalWithString element
      Sxmlelement technicalWithStringElement =
        createElement (
          technicalWithStringType,
          technicalWithString->getTechnicalWithStringValue ());

      // append the note technicalWithStrings element to the current note element
      appendSubElementToNoteNotationsTechnicals (
        technicalWithStringElement,
        technicalWithString->getTechnicalWithStringPlacementKind ());
    } // for
  }
}

//________________________________________________________________________
void msr2MxmltreeTranslator:: appendNoteArticulations (S_msrNote note)
{
#ifdef TRACE_OAH
  if (gTraceOah->fTraceNotes) {
    fLogOutputStream <<
      "--> appendNoteArticulations, note = " <<
      note->asShortString () <<
      endl;
  }
#endif

  // append the articulation elements if any
  const list<S_msrArticulation>&
    noteArticulations =
      note->getNoteArticulations () ;

  if (noteArticulations.size ()) {
    list<S_msrArticulation>::const_iterator i;

    for (i=noteArticulations.begin (); i!=noteArticulations.end (); i++) {
      S_msrArticulation articulation = (*i);

      msrArticulation::msrArticulationKind
        articulationKind =
          articulation->getArticulationKind ();

      int articulationType = kComment; // JMI

/* JMI
	((accent | strong-accent | staccato | tenuto |
	  detached-legato | staccatissimo | spiccato |
	  scoop | plop | doit | falloff | breath-mark |
	  caesura | stress | unstress | soft-accent |
	  other-articulation)*)>
	  */

      switch (articulationKind) {
        case msrArticulation::k_NoArticulation:
          // JMI ???
          break;

        case msrArticulation::kAccent:
          articulationType = k_accent;
          break;
        case msrArticulation::kBreathMark:
          articulationType = k_breath_mark;
          break;
        case msrArticulation::kCaesura:
          articulationType = k_caesura;
          break;
        case msrArticulation::kSpiccato:
          articulationType = k_spiccato;
          break;
        case msrArticulation::kStaccato:
          articulationType = k_staccato;
          break;
        case msrArticulation::kStaccatissimo:
          articulationType = k_staccatissimo;
          break;
        case msrArticulation::kStress:
          articulationType = k_stress;
          break;
        case msrArticulation::kUnstress:
          articulationType = k_unstress;
          break;
        case msrArticulation::kDetachedLegato:
          articulationType = k_detached_legato;
          break;
        case msrArticulation::kStrongAccent:
          articulationType = k_strong_accent;
          break;
        case msrArticulation::kTenuto:
          articulationType = k_tenuto;
          break;
        case msrArticulation::kFermata:
          articulationType = k_fermata;
          break;
        case msrArticulation::kArpeggiato:
          articulationType = k_arpeggiate;
          break;
        case msrArticulation::kNonArpeggiato:
          articulationType = k_non_arpeggiate;
          break;
        case msrArticulation::kDoit:
          articulationType = k_doit;
          break;
        case msrArticulation::kFalloff:
          articulationType = k_falloff;
          break;
        case msrArticulation::kPlop:
          articulationType = k_plop;
          break;
        case msrArticulation::kScoop:
          articulationType = k_scoop;
          break;
      } // switch

      // create the articulation element
      Sxmlelement articulationElement = createElement (articulationType, "");

      // append it to the current note notations articulations element
      switch (articulationKind) {
        case msrArticulation::k_NoArticulation:
          // JMI ???
          break;

        case msrArticulation::kAccent:
        case msrArticulation::kBreathMark:
        case msrArticulation::kCaesura:
        case msrArticulation::kSpiccato:
        case msrArticulation::kStaccato:
        case msrArticulation::kStaccatissimo:
        case msrArticulation::kStress:
        case msrArticulation::kUnstress:
        case msrArticulation::kDetachedLegato:
        case msrArticulation::kStrongAccent:
        case msrArticulation::kTenuto:
        case msrArticulation::kDoit:
        case msrArticulation::kFalloff:
        case msrArticulation::kPlop:
        case msrArticulation::kScoop:

        case msrArticulation::kFermata:
          appendSubElementToNoteNotations (
            articulationElement,
            articulation->getArticulationPlacementKind ());
          break;

        case msrArticulation::kArpeggiato:
        case msrArticulation::kNonArpeggiato:
          appendSubElementToNoteNotationsArticulations (
            articulationElement,
            articulation->getArticulationPlacementKind ());
          break;
      } // switch

        /*
        */
    } // for
  }
}

//________________________________________________________________________
void msr2MxmltreeTranslator:: appendNoteTieIfAny (
  S_msrNote note)
{
#ifdef TRACE_OAH
  if (gTraceOah->fTraceNotes) {
    fLogOutputStream <<
      "--> appendNoteTechnicalWithStrings, note = " <<
      note->asShortString () <<
      endl;
  }
#endif

  // append the tie element if any
  S_msrTie noteTie = note->getNoteTie ();

  if (noteTie) {
    // create the tied element
    Sxmlelement tiedElement = createElement (k_tied, "");

    // set its type if any
    string typeString;

    switch (noteTie->getTieKind ()) {
      case msrTie::kTieNone:
        break;
      case msrTie::kTieStart:
        typeString = "start";
        break;
      case msrTie::kTieContinue:
        typeString = "continue";
        break;
      case msrTie::kTieStop:
        typeString = "stop";
        break;
    } // switch

    if (typeString.size ()) {
      tiedElement->add (
        createAttribute ("type", typeString));
    }

    // append it to the current note notations element
    appendSubElementToNoteNotations (
      tiedElement,
      kPlacementNone); // no placement for '<tied />'
  }
}

//________________________________________________________________________
void msr2MxmltreeTranslator:: appendNoteSlursIfAny (
  S_msrNote note)
{
#ifdef TRACE_OAH
  if (gTraceOah->fTraceNotes) {
    fLogOutputStream <<
      "--> appendNoteSlursIfAny, note = " <<
      note->asShortString () <<
      endl;
  }
#endif

  // append the slur elements if any
  const list<S_msrSlur>&
    noteSlurs =
      note->getNoteSlurs () ;

  if (noteSlurs.size ()) {
    list<S_msrSlur>::const_iterator i;

    for (i=noteSlurs.begin (); i!=noteSlurs.end (); i++) {
      S_msrSlur
        slur = (*i);

      msrSlur::msrSlurTypeKind
        slurTypeKind =
          slur->getSlurTypeKind ();

      // create the slur element
      Sxmlelement slurElement = createElement (k_slur, "");

      // create the slur number attribute
      int slurNumber = slur->getSlurNumber ();

      if (slurNumber > 0) {
        slurElement->add (createIntegerAttribute ("number", slurNumber));
      }

      // create the slur type attribute
      string slurTypeString;

      switch (slurTypeKind) {
        case msrSlur::k_NoSlur:
          break;
        case msrSlur::kRegularSlurStart:
          slurTypeString = "start";
          break;
        case msrSlur::kPhrasingSlurStart:
          slurTypeString = "start";
          break;
        case msrSlur::kSlurContinue:
          slurTypeString = "slurContinue";
          break;
        case msrSlur::kRegularSlurStop:
          slurTypeString = "stop";
          break;
        case msrSlur::kPhrasingSlurStop:
          slurTypeString = "stop";
          break;
      } // switch

      if (slurTypeString.size ()) {
        slurElement->add (createAttribute ("type", slurTypeString));
      }

      // append the slur element to the current note note notations element
      appendSubElementToNoteNotations (
        slurElement,
        kPlacementNone); // no placement for '<slur />'
    } // for
  }
}

//________________________________________________________________________
void msr2MxmltreeTranslator:: appendNoteTupletIfRelevant (
  S_msrNote note)
{
#ifdef TRACE_OAH
  if (gTraceOah->fTraceNotes) {
    fLogOutputStream <<
      "--> appendNoteSlursIfAny, note = " <<
      note->asShortString () <<
      endl;
  }
#endif

  switch (note->getNoteKind ()) {
    case msrNote::k_NoNoteKind:
    case msrNote::kRestNote:
    case msrNote::kSkipNote:
    case msrNote::kUnpitchedNote:
    case msrNote::kRegularNote:
    case msrNote::kChordMemberNote:
      break;

    case msrNote::kTupletMemberNote:
      {
        // get the note's tuplet uplink
        S_msrTuplet
          noteTupletUpLink =
            note->getNoteTupletUpLink ();

        // get the note's position in tuplet
        int notePositionInTuplet =
          note->getPositionInTuplet ();

        // compute the type string if relevant
        string typeString;

        if (notePositionInTuplet == 1) {
          typeString = "start";
        }
        else if (
          notePositionInTuplet
            ==
          noteTupletUpLink->getTupletElementsList ().size ()
        ) {
          typeString = "stop";
        }

        if (typeString.size ()) {
          // create a tuplet element
          Sxmlelement tupletElement = createElement (k_tuplet, "");

          // set its number attribute
          tupletElement->add (
            createIntegerAttribute (
              "number",
              noteTupletUpLink->getTupletNumber ()));

          // set its type attribute
          tupletElement->add (
            createAttribute (
              "type",
              typeString));

          // append it to the current note notations elements
          appendSubElementToNoteNotations (
            tupletElement,
            kPlacementNone);
        }
      }
      break;

    case msrNote::kDoubleTremoloMemberNote:
    case msrNote::kGraceNote:
      break;

    case msrNote::kGraceChordMemberNote:
      break;

    case msrNote::kGraceTupletMemberNote:
      break;

    case msrNote::kTupletMemberUnpitchedNote:
      break;
  } // switch
}

//________________________________________________________________________
void msr2MxmltreeTranslator:: appendNoteSpannersBeforeNoteElement (
  S_msrNote note)
{
#ifdef TRACE_OAH
  if (gTraceOah->fTraceNotes) {
    fLogOutputStream <<
      "--> appendNoteSpannersBeforeNoteElement, note = " <<
      note->asShortString () <<
      endl;
  }
#endif

  // append the spanner elements if any
  const list<S_msrSpanner>&
    noteSpanners =
      note->getNoteSpanners () ;

  if (noteSpanners.size ()) {
    list<S_msrSpanner>::const_iterator i;

    for (i=noteSpanners.begin (); i!=noteSpanners.end (); i++) {
      S_msrSpanner spanner = (*i);

      msrSpannerTypeKind
        spannerTypeKind =
          spanner->getSpannerTypeKind ();

      // should we handle this spanner at all?
      bool doHandleSpanner = true;

      switch (spannerTypeKind) {
        case kSpannerTypeStart:
          break;
        case kSpannerTypeStop:
          doHandleSpanner = false;;
          break;
        case kSpannerTypeContinue:
          break;
        case k_NoSpannerType:
          // should not occur
          break;
      } // switch

      if (doHandleSpanner) {
        msrSpanner::msrSpannerKind
          spannerKind =
            spanner->getSpannerKind ();

        int spannerType = kComment; // JMI

        Sxmlelement containingElement;

        switch (spannerKind) {
          case msrSpanner::kSpannerDashes:
            // dashes go into the measure direction element
            spannerType = k_dashes;
            break;
          case msrSpanner::kSpannerWavyLine:
            // wavy lines go into the note notations ornaments
            spannerType = k_wavy_line;
            break;
        } // switch

        // create the spanner element
        Sxmlelement spannerElement = createElement (spannerType, "");

        // set spannerElement's "number" attribute if relevant
        int
          spannerNumber =
            spanner->getSpannerNumber ();

        if (spannerNumber > 0) {
          spannerElement->add (createIntegerAttribute ("number", spannerNumber));
        }

        // set spannerElement's "type" attribute if relevant
        string
          typeString =
            msrSpannerTypeKindAsMusicXMLString (spannerTypeKind);

        if (typeString.size ()) {
          spannerElement->add (createAttribute ("type", typeString));
        }

        switch (spannerKind) {
          case msrSpanner::kSpannerDashes:
            // dashes go into the measure direction element
            appendSubElementToMeasureDirection (
              spannerElement,
              spanner->getSpannerPlacementKind ());
            break;
          case msrSpanner::kSpannerWavyLine:
            // wavy lines go into the note notations ornaments
            appendSubElementToNoteNotationsOrnaments (
              spannerElement,
              spanner->getSpannerPlacementKind ());
              break;
        } // switch
      }
    } // for
  }
}

//________________________________________________________________________
void msr2MxmltreeTranslator:: appendNoteSpannersAfterNoteElement (
  S_msrNote note)
{
#ifdef TRACE_OAH
  if (gTraceOah->fTraceNotes) {
    fLogOutputStream <<
      "--> appendNoteSpannersAfterNoteElement, note = " <<
      note->asShortString () <<
      endl;
  }
#endif

  // append the spanner elements if any
  const list<S_msrSpanner>&
    noteSpanners =
      note->getNoteSpanners () ;

  if (noteSpanners.size ()) {
    list<S_msrSpanner>::const_iterator i;

    for (i=noteSpanners.begin (); i!=noteSpanners.end (); i++) {
      S_msrSpanner spanner = (*i);

      msrSpannerTypeKind
        spannerTypeKind =
          spanner->getSpannerTypeKind ();

      // should we handle this spanner at all?
      bool doHandleSpanner = true;

      switch (spannerTypeKind) {
        case kSpannerTypeStart:
          doHandleSpanner = false;;
          break;
        case kSpannerTypeStop:
          break;
        case kSpannerTypeContinue:
          break;
        case k_NoSpannerType:
          // should not occur
          break;
      } // switch

      if (doHandleSpanner) {
        msrSpanner::msrSpannerKind
          spannerKind =
            spanner->getSpannerKind ();

        int spannerType = kComment; // JMI

        Sxmlelement containingElement;

        switch (spannerKind) {
          case msrSpanner::kSpannerDashes:
            // dashes go into the measure direction element
            spannerType = k_dashes;
            break;
          case msrSpanner::kSpannerWavyLine:
            // wavy lines go into the note notations ornaments
            spannerType = k_wavy_line;
            break;
        } // switch

        // create the spanner element
        Sxmlelement spannerElement = createElement (spannerType, "");

        // set spannerElement's "number" attribute if relevant
        int
          spannerNumber =
            spanner->getSpannerNumber ();

        if (spannerNumber > 1) {
          spannerElement->add (createIntegerAttribute ("number", spannerNumber));
        }

        // set spannerElement's "type" attribute if relevant
        string
          typeString =
            msrSpannerTypeKindAsMusicXMLString (spannerTypeKind);

        if (typeString.size ()) {
          spannerElement->add (createAttribute ("type", typeString));
        }

        switch (spannerKind) {
          case msrSpanner::kSpannerDashes:
            // dashes go into the measure direction element
            appendSubElementToMeasureDirection (
              spannerElement,
              spanner->getSpannerPlacementKind ());
            break;
          case msrSpanner::kSpannerWavyLine:
            // wavy lines go into the note notations ornaments
            appendSubElementToNoteNotationsOrnaments (
              spannerElement,
              spanner->getSpannerPlacementKind ());
              break;
        } // switch
      }
    } // for
  }
}

//________________________________________________________________________
void msr2MxmltreeTranslator:: appendStemToNote (S_msrNote note)
{
#ifdef TRACE_OAH
  if (gTraceOah->fTraceNotes) {
    fLogOutputStream <<
      "--> appendStemToNote, note = " <<
      note->asShortString () <<
      endl;
  }
#endif

  S_msrStem stem = note->getNoteStem ();

  if (stem) {
    msrStem::msrStemKind
      stemKind = stem->getStemKind ();

    string stemString;

    switch (stemKind) {
      case msrStem::kStemNone:
        stemString = "none";
        break;
      case msrStem::kStemUp:
        stemString = "up";
        break;
      case msrStem::kStemDown:
        stemString = "down";
        break;
      case msrStem::kStemDouble:
        stemString = "double";
        break;
    } // switch

    // append a slur element to the current note element
    fCurrentNoteElement->push (
      createElement (
        k_stem,
        stemString));
  }
}

//________________________________________________________________________
void msr2MxmltreeTranslator::appendBeamsToNote (S_msrNote note)
{
#ifdef TRACE_OAH
  if (gTraceOah->fTraceNotes) {
    fLogOutputStream <<
      "--> appendBeamsToNote, note = " <<
      note->asShortString () <<
      endl;
  }
#endif

  // append the beam elements if any
  const list<S_msrBeam>&
    noteBeams =
      note->getNoteBeams () ;

  if (noteBeams.size ()) {
    list<S_msrBeam>::const_iterator i;

    for (i=noteBeams.begin (); i!=noteBeams.end (); i++) {
      S_msrBeam beam = (*i);

      msrBeam::msrBeamKind
        beamKind =
          beam->getBeamKind ();

      // create a beam element
      string beamString;

      switch (beamKind) {
        case msrBeam::k_NoBeam:
          break;
        case msrBeam::kBeginBeam:
          beamString = "begin";
          break;
        case msrBeam::kContinueBeam:
          beamString = "continue";
          break;
        case msrBeam::kEndBeam:
          beamString = "end";
          break;
        case msrBeam::kForwardHookBeam:
          beamString = "forward hook";
          break;
        case msrBeam::kBackwardHookBeam:
          beamString = "backward hook";
          break;
      } // switch

      Sxmlelement beamElement = createElement (k_beam, beamString);

      // set its number attribute
      beamElement->add (createIntegerAttribute ("number", beam->getBeamNumber ()));

      // append the beam element to the current note element
      fCurrentNoteElement->push (beamElement);
    } // for
  }
}

//________________________________________________________________________
void msr2MxmltreeTranslator:: appendNoteNotationsToNote (S_msrNote note)
{
#ifdef TRACE_OAH
  if (gTraceOah->fTraceNotes) {
    fLogOutputStream <<
      "--> appendNoteNotationsToNote, note = " <<
      note->asShortString () <<
      endl;
  }
#endif

/*
<!ELEMENT notations
	(%editorial;,
	 (tied | slur | tuplet | glissando | slide |
	  ornaments | technical | articulations | dynamics |
	  fermata | arpeggiate | non-arpeggiate |
	  accidental-mark | other-notation)*)>
<!ATTLIST notations
    %print-object;
    %optional-unique-id;
>
*/

  // append the ornaments elements if any
  appendNoteOrnaments (note);

  // append the articulation elements if any
  appendNoteArticulations (note);

  // append the technical elements if any
  appendNoteTechnicals (note);
  appendNoteTechnicalWithIntegers (note);
  appendNoteTechnicalWithFloats (note);
  appendNoteTechnicalWithStrings (note);

  // append the tie element if any
  appendNoteTieIfAny (note);

  // append the slur elements if any
  appendNoteSlursIfAny (note);

  // append the tuplet elements if relevant
  appendNoteTupletIfRelevant (note);
}

//________________________________________________________________________
void msr2MxmltreeTranslator:: appendNoteLyricsToNote (S_msrNote note)
{
#ifdef TRACE_OAH
  if (gTraceOah->fTraceNotes) {
    fLogOutputStream <<
      "--> appendNoteLyricsToNote, note = " <<
      note->asShortString () <<
      endl;
  }
#endif

/*
<!ELEMENT lyric
	((((syllabic?, text),
	   (elision?, syllabic?, text)*, extend?) |
	   extend | laughing | humming),
	  end-line?, end-paragraph?, %editorial;)>
<!ATTLIST lyric
    number NMTOKEN #IMPLIED
    name CDATA #IMPLIED
    %justify;
    %position;
    %placement;
    %color;
    %print-object;
>
*/

  // append the lyric elements if any
  const list<S_msrSyllable>&
    noteSyllables =
      note->getNoteSyllables () ;

  if (noteSyllables.size ()) {
    list<S_msrSyllable>::const_iterator i;

    for (i=noteSyllables.begin (); i!=noteSyllables.end (); i++) {
      S_msrSyllable syllable = (*i);

      // create and append a lyric element if relevant
      bool   doCreateALyricElement = false;
      string syllabicString;

      switch ( syllable->getSyllableKind ()) {
        case msrSyllable::kSyllableNone:
          // should not occur
          break;

        case msrSyllable::kSyllableSingle:
          doCreateALyricElement = true;
          syllabicString = "single";
          break;

        case msrSyllable::kSyllableBegin:
          doCreateALyricElement = true;
          syllabicString = "begin";
          break;
        case msrSyllable::kSyllableMiddle:
          doCreateALyricElement = true;
          syllabicString = "middle";
          break;
        case msrSyllable::kSyllableEnd:
          doCreateALyricElement = true;
          syllabicString = "end";
          break;

        case msrSyllable::kSyllableOnRestNote:
          break;

        case msrSyllable::kSyllableSkipRestNote:
          break;
        case msrSyllable::kSyllableSkipNonRestNote:
          break;

        case msrSyllable::kSyllableMeasureEnd:
          break;

        case msrSyllable::kSyllableLineBreak:
          break;
        case msrSyllable::kSyllablePageBreak:
          break;
      } // switch

      if (doCreateALyricElement) {
        // create the lyric element
        Sxmlelement lyricElement = createElement (k_lyric, "");

        // set its number attribute
        lyricElement->add (
          createAttribute (
            "number",
            syllable->getSyllableStanzaNumber ()));

        // append a syllabic element to the lyric element if relevant
        if (syllabicString.size ()) {
          lyricElement->push (
            createElement (k_syllabic, syllabicString));
        }

        // append a text elements to the lyric element if relevant
        const list<string>&
          syllableTextsList =
            syllable->getSyllableTextsList ();

        for (
          list<string>::const_iterator i = syllableTextsList.begin ();
          i!=syllableTextsList.end ();
          i++
        ) {
          string text = (*i);

          lyricElement->push (
            createElement (k_text, text));
        } // for

        // append the extend element to the lyric element if relevant
        string extendTypeString;
        bool   doCreateAnExtendElement = true;

        switch (syllable->getSyllableExtendKind ()) {
          case msrSyllable::kSyllableExtendNone:
          doCreateAnExtendElement = false;
            break;
          case msrSyllable::kSyllableExtendEmpty:
            break;
          case msrSyllable::kSyllableExtendSingle:
            extendTypeString = "single";
            break;
          case msrSyllable::kSyllableExtendStart:
            extendTypeString = "start";
            break;
          case msrSyllable::kSyllableExtendContinue:
            extendTypeString = "continue";
            break;
          case msrSyllable::kSyllableExtendStop:
            extendTypeString = "stop";
            break;
        } // switch

        if (doCreateAnExtendElement) {
          // create an extend element
          Sxmlelement extendElement = createElement (k_extend, "");

          if (extendTypeString.size ()) {
            // set its type attribute
            extendElement->add (createAttribute ("type", extendTypeString));
          }

          // append the extend element to the lyric element
          lyricElement->push (extendElement);
        }

        // append a syllabic element to the lyric element if relevant

        // append the lyric element to the current note element
        fCurrentNoteElement->push (lyricElement);
      }
    } // for
  }
}

//________________________________________________________________________
void msr2MxmltreeTranslator::appendBasicSubElementsToNote (
  S_msrNote note)
{
  int inputLineNumber =
    note->getInputLineNumber ();

#ifdef TRACE_OAH
  if (gTraceOah->fTraceNotes) {
    fLogOutputStream <<
      "--> appendBasicSubElementsToNote(), note = " <<
      note->asShortString () <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  // grab the note's informations
  msrNote::msrNoteKind
    noteKind =
      note->getNoteKind ();

  msrQuarterTonesPitchKind
    noteQuarterTonesPitchKind = note->getNoteQuarterTonesPitchKind ();

  msrDiatonicPitchKind noteDiatonicPitchKind;
  msrAlterationKind    noteAlterationKind;

  fetchDiatonicPitchKindAndAlterationKindFromQuarterTonesPitchKind (
    inputLineNumber,
    noteQuarterTonesPitchKind,
    noteDiatonicPitchKind,
    noteAlterationKind);

  int
    noteOctave     = note->getNoteOctave ();

  float
    noteMusicXMLAlter =
      msrMusicXMLAlterFromAlterationKind (
        noteAlterationKind);

#ifdef TRACE_OAH
  if (gTraceOah->fTraceNotes) {
    fLogOutputStream <<
      "-->  noteKind: " << msrNote::noteKindAsString (noteKind) <<
      "-->  noteOctave: " << noteOctave <<
      "-->  noteDiatonicPitchKind: " <<
      msrDiatonicPitchKindAsString (noteDiatonicPitchKind) <<
      endl;
  }
#endif

  // append the chord sub element if relevant
  switch (noteKind) {
    case msrNote::kChordMemberNote:
      if (! note->getNoteIsAChordsFirstMemberNote ()) {
        fCurrentNoteElement->push (createElement (k_chord, ""));
      }
      break;
    default:
      ;
  } // switch

  // append the grace sub element if relevant
  switch (noteKind) {
    case msrNote::kGraceNote:
      fCurrentNoteElement->push (createElement (k_grace, ""));
      break;
    default:
      ;
  } // switch

  // append the step and pitch or rest sub elements
  switch (noteKind) {
    case msrNote::k_NoNoteKind:
      break;

    case msrNote::kRestNote:
      fCurrentNoteElement->push (createElement (k_rest, ""));
      break;

    case msrNote::kSkipNote:
      break;

    case msrNote::kUnpitchedNote:
      break;

    case msrNote::kRegularNote:
    case msrNote::kChordMemberNote:
    case msrNote::kTupletMemberNote:
      {
        if (note->getNoteIsARest ()) {
          fCurrentNoteElement->push (createElement (k_rest, ""));
        }

        else {
          // create the pitch element
          Sxmlelement pitchElement = createElement (k_pitch, "");

          // append the step element
          pitchElement->push (
            createElement (
              k_step,
              msrDiatonicPitchKindAsString (noteDiatonicPitchKind)));

          if (noteMusicXMLAlter != 0.0) {
            // append the alter element
            stringstream s;
            s << setprecision (2) << noteMusicXMLAlter;
            pitchElement->push (
              createElement (
                k_alter,
                s.str ()));
          }

          // append the octave element
          pitchElement->push (
            createIntegerElement (
              k_octave,
              noteOctave));

          fCurrentNoteElement->push (pitchElement);
        }
      }
      break;

    case msrNote::kDoubleTremoloMemberNote:
      break;

    case msrNote::kGraceNote:
      {
        // create the pitch element
        Sxmlelement pitchElement = createElement (k_pitch, "");

        // append the step element
        pitchElement->push (
          createElement (
            k_step,
            msrDiatonicPitchKindAsString (noteDiatonicPitchKind)));

        if (noteMusicXMLAlter != 0.0) {
          // append the alter element
          stringstream s;
          s << setprecision (2) << noteMusicXMLAlter;
          pitchElement->push (
            createElement (
              k_alter,
              s.str ()));
        }

        // append the octave element
        pitchElement->push (
          createIntegerElement (
            k_octave,
            noteOctave));

        // append the pitch element to the current note
        fCurrentNoteElement->push (pitchElement);
      }
      break;

    case msrNote::kGraceChordMemberNote:
      break;

    case msrNote::kGraceTupletMemberNote:
      break;

    case msrNote::kTupletMemberUnpitchedNote:
      break;
  } // switch
}

//________________________________________________________________________
void msr2MxmltreeTranslator::appendDurationSubElementToNoteIfRelevant (
  S_msrNote note)
{
  int inputLineNumber =
    note->getInputLineNumber ();

#ifdef TRACE_OAH
  if (gTraceOah->fTraceNotes) {
    fLogOutputStream <<
      "--> appendDurationSubElementToNoteIfRelevant(), note = " <<
      note->asShortString () <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  // grab the note's informations
  msrNote::msrNoteKind
    noteKind =
      note->getNoteKind ();

  rational
    noteSoundingWholeNotes =
      note->getMeasureElementSoundingWholeNotes (),
    noteDisplayWholeNotes =
      note->getNoteDisplayWholeNotes ();

#ifdef TRACE_OAH
  if (gTraceOah->fTraceNotes) {
    fLogOutputStream <<
      "-->  noteKind: " << msrNote::noteKindAsString (noteKind) <<
      "-->  noteSoundingWholeNotes: " << noteSoundingWholeNotes <<
      "-->  noteDisplayWholeNotes: " << noteDisplayWholeNotes <<
//      "-->  noteTupletFactor: " << note->getNoteTupletFactor ().asRational () <<
      "-->  fDivisionsPerQuarterNote: " << fDivisionsPerQuarterNote <<
      "-->  fDivisionsMultiplyingFactor: " << fDivisionsMultiplyingFactor <<
      "-->  line " << inputLineNumber <<
      endl;
  }
#endif

  rational
    soundingDurationAsRational =
      noteSoundingWholeNotes
        /
      fPartShortestNoteDuration
        *
      fDivisionsMultiplyingFactor;
  soundingDurationAsRational.rationalise ();

  bool doAppendDurationSubElement = false;

  switch (noteKind) {
    case msrNote::k_NoNoteKind:
      break;

    case msrNote::kSkipNote:
      break;

    case msrNote::kUnpitchedNote:
      break;

    case msrNote::kRestNote:
    case msrNote::kRegularNote:
    case msrNote::kChordMemberNote:
      doAppendDurationSubElement = true;
      break;

    case msrNote::kTupletMemberNote:
      doAppendDurationSubElement = true;
      break;


    case msrNote::kDoubleTremoloMemberNote:
      break;

    case msrNote::kGraceNote:
      break;

    case msrNote::kGraceChordMemberNote:
      break;

    case msrNote::kGraceTupletMemberNote:
      break;

    case msrNote::kTupletMemberUnpitchedNote:
      break;
  } // switch

  if (doAppendDurationSubElement) {
    soundingDurationAsRational.rationalise ();

#ifdef TRACE_OAH
    if (gTraceOah->fTraceNotes) {
      fLogOutputStream <<
        "--> soundingDurationAsRational: " <<
        soundingDurationAsRational <<
        "--> line " << inputLineNumber <<
        endl;
    }
#endif

    if (soundingDurationAsRational.getDenominator () != 1) {
      stringstream s;

      s <<
        "soundingDurationAsRational '" << soundingDurationAsRational <<
        "' is no integer number" <<
        ", line " << inputLineNumber;

      msrInternalError (
        gOahOah->fInputSourceName,
        note->getInputLineNumber (),
        __FILE__, __LINE__,
        s.str ());
    }

    fCurrentNoteElement->push (
      createIntegerElement (
        k_duration,
        soundingDurationAsRational.getNumerator ()));

#ifdef TRACE_OAH
    if (gTraceOah->fTraceNotes) {
      fLogOutputStream <<
        endl <<
        "--> appendDurationSubElementToNoteIfRelevant(): " <<
        note <<
        endl;
    }
#endif
  }
}

//________________________________________________________________________
void msr2MxmltreeTranslator::appendVoiceSubElementToNoteIfRelevant (
  S_msrNote note)
{
  int inputLineNumber =
    note->getInputLineNumber ();

#ifdef TRACE_OAH
  if (gTraceOah->fTraceNotes) {
    fLogOutputStream <<
      "--> appendVoiceSubElementToNoteIfRelevant(), note = " <<
      note->asShortString () <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  msrNote::msrNoteKind
    noteKind =
      note->getNoteKind ();

  // compute the note voice
  S_msrVoice noteVoice;

  switch (noteKind) {
    case msrNote::k_NoNoteKind:
      break;

    case msrNote::kRestNote:
      noteVoice =
        note->
          getNoteMeasureUpLink ()->
            getMeasureSegmentUpLink ()->
              getSegmentVoiceUpLink ();
      break;

    case msrNote::kSkipNote:
      break;

    case msrNote::kUnpitchedNote:
      break;

    case msrNote::kRegularNote:
    case msrNote::kChordMemberNote:
      noteVoice =
        note->
          getNoteMeasureUpLink ()->
            getMeasureSegmentUpLink ()->
              getSegmentVoiceUpLink ();
      break;

    case msrNote::kTupletMemberNote:
      noteVoice =
        note->
          getNoteTupletUpLink ()->
            getTupletMeasureUpLink ()->
              getMeasureSegmentUpLink ()->
                getSegmentVoiceUpLink ();
      break;

    case msrNote::kDoubleTremoloMemberNote:
      break;

    case msrNote::kGraceNote:
      noteVoice =
        note->
          getNoteGraceNotesGroupUpLink ()->
            getGraceNotesGroupVoiceUpLink ();
            /* JMI
            getGraceNotesGroupNoteUpLink ()->
            getNoteMeasureUpLink ()->
              getMeasureSegmentUpLink ()->
                getSegmentVoiceUpLink ();
                */
      break;

    case msrNote::kGraceChordMemberNote:
      break;

    case msrNote::kGraceTupletMemberNote:
      break;

    case msrNote::kTupletMemberUnpitchedNote:
      break;
  } // switch

  // sanity check
  msrAssert (
    noteVoice != nullptr,
    "noteVoice is null");

#ifdef TRACE_OAH
  if (gTraceOah->fTraceNotes) {
    fLogOutputStream <<
      endl <<
      "--> noteVoice: " <<
      noteVoice <<
      endl;
  }
#endif

  // append the voice attribute if relevant
  int
    voiceNumber =
      noteVoice->
        getVoiceNumber ();

  if (voiceNumber != 1) { // options ? JMI
    fCurrentNoteElement->push (
      createIntegerElement (
        k_voice,
        voiceNumber));
  }
}

void msr2MxmltreeTranslator::appendTimeModificationSubElementToNoteIfRelevant (
  S_msrNote note)
{
  int inputLineNumber =
    note->getInputLineNumber ();

#ifdef TRACE_OAH
  if (gTraceOah->fTraceNotes) {
    fLogOutputStream <<
      "--> appendTimeModificationSubElementToNoteIfRelevant(), note = " <<
      note->asShortString () <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  msrNote::msrNoteKind
    noteKind =
      note->getNoteKind ();

  // append the time modification if relevant
  switch (noteKind) {
    case msrNote::k_NoNoteKind:
      break;

    case msrNote::kRestNote:
      break;

    case msrNote::kSkipNote:
      break;

    case msrNote::kUnpitchedNote:
      break;

    case msrNote::kRegularNote:
      break;

    case msrNote::kDoubleTremoloMemberNote:
      break;

    case msrNote::kGraceNote:
      break;

    case msrNote::kGraceChordMemberNote:
      break;

    case msrNote::kChordMemberNote:
      break;

    case msrNote::kTupletMemberNote:
      {
        Sxmlelement
          timeModificationElement = createElement (k_time_modification, "");

        timeModificationElement->push (
          createIntegerElement (
            k_actual_notes,
            note->getNoteTupletFactor ().getTupletActualNotes ()));
        timeModificationElement->push (
          createIntegerElement (
            k_normal_notes,
            note->getNoteTupletFactor ().getTupletNormalNotes ()));

        fCurrentNoteElement->push (timeModificationElement);
      }
      break;

    case msrNote::kGraceTupletMemberNote:
      break;

    case msrNote::kTupletMemberUnpitchedNote:
      break;
  } // switch
}

//________________________________________________________________________
void msr2MxmltreeTranslator::appendNoteSubElementToMesureIfRelevant (
  S_msrNote note)
{
#ifdef TRACE_OAH
  if (gTraceOah->fTraceNotes) {
    fLogOutputStream <<
      "--> appendNoteSubElementToMesureIfRelevant, note = " <<
      note->asShortString () <<
      endl;
  }
#endif

  int inputLineNumber =
    note->getInputLineNumber ();

  switch (note->getNoteKind ()) {
    case msrNote::k_NoNoteKind:
      break;
    case msrNote::kRestNote:
      break;
    case msrNote::kSkipNote:
      break;
    case msrNote::kUnpitchedNote:
      break;
    case msrNote::kRegularNote:
      break;

    case msrNote::kChordMemberNote:
      if (note->getNoteIsAChordsFirstMemberNote ()) {
        if (false && fPendingChordStartComment) { // JMI
          // append the pending chord start comment to the current part element
          fCurrentPartElement->push (fPendingChordStartComment);
        }
      }
      break;

    case msrNote::kTupletMemberNote:
      break;

    case msrNote::kDoubleTremoloMemberNote:
    case msrNote::kGraceNote:
      break;

    case msrNote::kGraceChordMemberNote:
      break;

    case msrNote::kGraceTupletMemberNote:
      break;

    case msrNote::kTupletMemberUnpitchedNote:
      break;
  } // switch

  // create a note element
  fCurrentNoteElement = createElement (k_note, "");

  // append the note basic sub-elements
  appendBasicSubElementsToNote (note);

  // append the duration sub-element if relevant
  appendDurationSubElementToNoteIfRelevant (note);

  // append the voice sub-element if relevant
  appendVoiceSubElementToNoteIfRelevant (note);

  // append the type sub-element if relevant
  bool doGenerateTypeSubElement = true;

  switch (note->getNoteKind ()) {
    case msrNote::k_NoNoteKind:
      break;

    case msrNote::kRestNote:
      doGenerateTypeSubElement = false;
      break;

    case msrNote::kSkipNote:
      break;

    case msrNote::kUnpitchedNote:
      break;

    case msrNote::kRegularNote:
      break;

    case msrNote::kChordMemberNote:
    /* JMI
      if (note->getNoteIsARest ()) {
        doGenerateTypeSubElement = false;
      }
      */
      break;

    case msrNote::kTupletMemberNote:
      break;

    case msrNote::kDoubleTremoloMemberNote:
      break;

    case msrNote::kGraceNote:
      break;

    case msrNote::kGraceChordMemberNote:
      break;

    case msrNote::kGraceTupletMemberNote:
      break;

    case msrNote::kTupletMemberUnpitchedNote:
      break;
  } // switch

  if (doGenerateTypeSubElement) {
    msrDurationKind
      noteGraphicDurationKind =
        note->getNoteGraphicDurationKind ();

    fCurrentNoteElement->push (
      createElement (
        k_type,
        msrDurationKindAsMusicXMLType (noteGraphicDurationKind)));
  }

  // append the time-modification sub-element if relevant
  appendTimeModificationSubElementToNoteIfRelevant (note);

  // append the dots sub-element if relevant
  int
    noteDotsNumber =
      note->getNoteDotsNumber ();

#ifdef TRACE_OAH
  if (gTraceOah->fTraceNotes) {
    fLogOutputStream <<
      "-->  noteDotsNumber: " << noteDotsNumber <<
      "--> line " << inputLineNumber <<
      endl;
  }
#endif

  for (int i = 0; i < noteDotsNumber; i++) {
    fCurrentNoteElement->push (
      createElement (
        k_dot, ""));
  } // for

  // append the accidental if any
  msrAccidentalKind
    accidentalKind =
      note->getNoteAccidentalKind ();

  string
    accidentalString =
      accidentalKindAsMusicXMLString (
        accidentalKind);

  if (accidentalString.size ()) {
    fCurrentNoteElement->push (
      createElement (
        k_accidental,
        accidentalString));
  }

  // append the stem if any
  appendStemToNote (note);

  // append the beams if any
  appendBeamsToNote (note);

  // append the articulations if any
  appendNoteNotationsToNote (note);

  // append the lyrics if any
  appendNoteLyricsToNote (note);

  // append the note element to the current measure element right now,
  // unless it contains a grace notes group
  S_msrGraceNotesGroup
    noteGraceNotesGroupBefore =
      note->getNoteGraceNotesGroupBefore (),
    noteGraceNotesGroupAfter =
      note->getNoteGraceNotesGroupAfter ();

  if (! (noteGraceNotesGroupBefore || noteGraceNotesGroupAfter)) {
    appendNoteSubElementToMeasure (
      note,
      fCurrentNoteElement);
  }
  else {
    fCurrentNoteElementAwaitsGraceNotes = true;
    fPendingNoteAwaitingGraceNotes = note;
    fPendingNoteElement = fCurrentNoteElement;
  }
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrGraceNotesGroup& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber () ;

#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrGraceNotesGroup" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  // create a start comment
  stringstream s;
  s <<
    " ==================== " <<
    "Grace notes group " <<
    inputLineNumber <<
    " START" <<
      ", line " << inputLineNumber <<
    " ==================== ";
  Sxmlelement comment = createElement (kComment, s.str ());

  // append it to the current measure element
  fCurrentMeasureElement->push (comment);
}

void msr2MxmltreeTranslator::visitEnd (S_msrGraceNotesGroup& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber () ;

#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrGraceNotesGroup" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  // create an end comment
  stringstream s;
  s <<
    " ==================== " <<
    "Grace notes group " <<
    inputLineNumber <<
    " END" <<
      ", line " << inputLineNumber <<
    " ==================== ";
  Sxmlelement comment = createElement (kComment, s.str ());

  // append it to the current measure element
  fCurrentMeasureElement->push (comment);

  // append the note element to the current measure element only now,
  // if it contains a grace notes group
  /*
  S_msrGraceNotesGroup
    noteGraceNotesGroupBefore =
      note->getNoteGraceNotesGroupBefore (),
    noteGraceNotesGroupAfter =
      note->getNoteGraceNotesGroupAfter ();
*/
  if (fCurrentNoteElementAwaitsGraceNotes) {
    appendNoteSubElementToMeasure (
      fPendingNoteAwaitingGraceNotes,
      fPendingNoteElement);

    // forget about these after the pending grace notes
    fCurrentNoteElementAwaitsGraceNotes = false;
    fPendingNoteAwaitingGraceNotes = nullptr;
    fPendingNoteElement = nullptr;
  }
}

/*
<!ELEMENT note
	(((grace, ((%full-note;, (tie, tie?)?) | (cue, %full-note;))) |
	  (cue, %full-note;, duration) |
	  (%full-note;, duration, (tie, tie?)?)),
	 instrument?, %editorial-voice;, type?, dot*,
	 accidental?, time-modification?, stem?, notehead?,
	 notehead-text?, staff?, beam*, notations*, lyric*, play?)>
*/

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrNote& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();

#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrNote '" <<
      elt->asString () <<
      "'" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif
  // append the note directions to the note element
  appendSubElementsToNoteDirections (elt);

  // append the note element to the measure element if relevant
  appendNoteSubElementToMesureIfRelevant (elt);
}

void msr2MxmltreeTranslator::visitEnd (S_msrNote& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();

#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrNote " <<
      elt->asString () <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  // forget about the note element
  fCurrentNoteElement = nullptr;

  // forget about the note notations element
  fCurrentNoteNotationsElement              = nullptr;
  fCurrentNoteNotationsOrnamentsElement     = nullptr;
  fCurrentNoteNotationsArticulationsElement = nullptr;
  fCurrentNoteNotationsTechnicalsElement    = nullptr;
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrBarline& elt)
{
#ifdef TRACE_OAH
  int inputLineNumber =
    elt->getInputLineNumber ();
#endif

#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrBarline" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  string barLStyleString;

  switch (elt->getBarlineStyleKind ()) {
    case msrBarline::kBarlineStyleNone:
      break;
    case msrBarline::kBarlineStyleRegular:
      barLStyleString = "regular";
      break;
    case msrBarline::kBarlineStyleDotted:
      barLStyleString = "dotted";
      break;
    case msrBarline::kBarlineStyleDashed:
      barLStyleString = "dashed";
      break;
    case msrBarline::kBarlineStyleHeavy:
      barLStyleString = "heavy";
      break;
    case msrBarline::kBarlineStyleLightLight:
      barLStyleString = "light-light";
      break;
    case msrBarline::kBarlineStyleLightHeavy:
      barLStyleString = "light-heavy";
      break;
    case msrBarline::kBarlineStyleHeavyLight:
      barLStyleString = "heavy-light";
      break;
    case msrBarline::kBarlineStyleHeavyHeavy:
      barLStyleString = "heavy-heavy";
      break;
    case msrBarline::kBarlineStyleTick:
      barLStyleString = "tick";
      break;
    case msrBarline::kBarlineStyleShort:
      barLStyleString = "short";
      break;
  } // switch

  if (barLStyleString.size ()) {
    // create the bar style element
    Sxmlelement barStyleElement = createElement (k_bar_style, barLStyleString);

    // create the barline element
    Sxmlelement barlineElement = createElement (k_barline, "");

    // set its location attribute if any
    msrBarline::msrBarlineLocationKind
      barlineLocationKind =
        elt->getLocationKind ();

    string barLineLocationString;

    switch (barlineLocationKind) {
      case msrBarline::kBarlineLocationNone:
        break;
      case msrBarline::kBarlineLocationLeft:
        barLineLocationString = "left";
        break;
      case msrBarline::kBarlineLocationMiddle:
        barLineLocationString = "middle";
        break;
      case msrBarline::kBarlineLocationRight:
        barLineLocationString = "right";
        break;
    } // switch

    if (barLineLocationString.size ()) {
      barlineElement->add (createAttribute ("location", barLineLocationString));
    }

    // append the barline element to the barline element
    barlineElement->push (barStyleElement);

    // append the barline element to the measure element
    appendOtherSubElementToMeasure (barlineElement);
  }
}

void msr2MxmltreeTranslator::visitEnd (S_msrBarline& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrBarline" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}


/*
//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrStaffLinesNumber& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrStaffLinesNumber" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  // create a staff lines number clone
  fCurrentStaffLinesNumberClone =
    elt->
      createStaffLinesNumberNewbornClone ();
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrStaffTuning& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrStaffTuning" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  // create a staff tuning clone
  fCurrentStaffTuningClone =
    elt->
      createStaffTuningNewbornClone ();
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrStaffDetails& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrStaffDetails" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  fCurrentStaffTuningClone = nullptr;
}

void msr2MxmltreeTranslator::visitEnd (S_msrStaffDetails& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrStaffDetails" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  // append the staff details to the current voice clone
  fCurrentVoiceClone->
    appendStaffDetailsToVoice (
      elt);
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrStaff& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrStaff \"" <<
      elt->getStaffName () << "\"" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  gIndenter++;

  switch (elt->getStaffKind ()) {
    case msrStaff::kStaffRegular:
    case msrStaff::kStaffTablature:
    case msrStaff::kStaffDrum:
    case msrStaff::kStaffRythmic:
      {
        // create a staff clone
        fCurrentStaffClone =
          elt->createStaffNewbornClone (
            fCurrentPartClone);

        // add it to the part clone
        fCurrentPartClone->
          addStaffToPartCloneByItsNumber (
            fCurrentStaffClone);

        // create a staff block
        fCurrentStaffBlock =
          lpsrStaffBlock::create (
            fCurrentStaffClone);

        string
          partName =
            fCurrentPartClone->getPartName (),
          partAbbreviation =
            fCurrentPartClone->getPartAbbreviation ();

        string staffBlockInstrumentName;
        string staffBlockShortInstrumentName;

        // don't set instrument name nor short instrument name // JMI
        // if the staff belongs to a piano part where they're already set
        if (! partName.size ()) {
          staffBlockInstrumentName = partName;
        }
        if (! partAbbreviation.size ()) {
          staffBlockShortInstrumentName = partAbbreviation;
        }

        if (staffBlockInstrumentName.size ()) {
          fCurrentStaffBlock->
            setStaffBlockInstrumentName (staffBlockInstrumentName);
        }

        if (staffBlockShortInstrumentName.size ()) {
          fCurrentStaffBlock->
            setStaffBlockShortInstrumentName (staffBlockShortInstrumentName);
        }

        // append the staff block to the current part block
        fCurrentPartBlock->
          appendStaffBlockToPartBlock (
            fCurrentStaffBlock);

        fOnGoingStaff = true;
      }
      break;

    case msrStaff::kStaffHarmony:
      {
        // create a staff clone
        fCurrentStaffClone =
          elt->createStaffNewbornClone (
            fCurrentPartClone);

        // add it to the part clone
        fCurrentPartClone->
          addStaffToPartCloneByItsNumber (
            fCurrentStaffClone);

        fOnGoingStaff = true;
      }
      break;

    case msrStaff::kStaffFiguredBass:
      {
        // create a staff clone
        fCurrentStaffClone =
          elt->createStaffNewbornClone (
            fCurrentPartClone);

        // add it to the part clone
        fCurrentPartClone->
          addStaffToPartCloneByItsNumber (
            fCurrentStaffClone);

        // register it as the part figured bass staff
        fCurrentPartClone->
          setPartFiguredBassStaff (fCurrentStaffClone);

        fOnGoingStaff = true;
      }
      break;
  } // switch
}

void msr2MxmltreeTranslator::visitEnd (S_msrStaff& elt)
{
  gIndenter--;

#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting S_msrStaff \"" <<
      elt->getStaffName () << "\"" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  switch (elt->getStaffKind ()) {
    case msrStaff::kStaffRegular:
    case msrStaff::kStaffDrum:
    case msrStaff::kStaffRythmic:
      {
        fOnGoingStaff = false;
      }
      break;

    case msrStaff::kStaffTablature:
      // JMI
      break;

    case msrStaff::kStaffHarmony:
      // JMI
      break;

    case msrStaff::kStaffFiguredBass:
      // JMI
      break;
  } // switch
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrVoice& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();

#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrVoice \"" <<
      elt->asString () << "\"" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

}

void msr2MxmltreeTranslator::visitEnd (S_msrVoice& elt)
{
  gIndenter--;

#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrVoice \"" <<
      elt->getVoiceName () << "\"" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  switch (elt->getVoiceKind ()) {
    case msrVoice::kVoiceRegular:
      // JMI
      break;

    case msrVoice::kVoiceHarmony:
      fOnGoingHarmonyVoice = false;
      break;

    case msrVoice::kVoiceFiguredBass:
      fOnGoingFiguredBassVoice = false;
      break;
  } // switch
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrVoiceStaffChange& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrVoiceStaffChange '" <<
      elt->asString () << "'" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  // create a voice staff change clone
  S_msrVoiceStaffChange
    voiceStaffChangeClone =
      elt->
        createStaffChangeNewbornClone ();

  // append it to the current voice clone
  fCurrentVoiceClone->
    appendVoiceStaffChangeToVoice (
      voiceStaffChangeClone);
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrHarmony& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrHarmony '" <<
      elt->asString () <<
      ", fOnGoingNonGraceNote: " << booleanAsString (fOnGoingNonGraceNote) <<
      ", fOnGoingHarmonyVoice: " << booleanAsString (fOnGoingHarmonyVoice) <<
      ", fOnGoingHarmony: " << booleanAsString (fOnGoingHarmony) <<
      "', line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  // create a harmony new born clone
  fCurrentHarmonyClone =
    elt->
      createHarmonyNewbornClone (
        fCurrentVoiceClone);

  if (fOnGoingNonGraceNote) {
    // register the harmony in the current non-grace note clone
    fCurrentNonGraceNoteClone->
      appendHarmonyToNoteHarmoniesList (
        fCurrentHarmonyClone);

    // don't append the harmony to the part harmony,
    // this has been done in pass2b // JMI ???
  }

  else if (fOnGoingHarmonyVoice) {
  / * JMI
    // get the harmony whole notes offset
    rational
      harmonyWholeNotesOffset =
        elt->getHarmonyWholeNotesOffset ();

    // is harmonyWholeNotesOffset not equal to 0?
    if (harmonyWholeNotesOffset.getNumerator () != 0) {
      // create skip with duration harmonyWholeNotesOffset
      S_msrNote
        skip =
          msrNote::createSkipNote (
            elt->                getInputLineNumber (),
            "666", // JMI elt->                getHarmonyMeasureNumber (),
            elt->                getHarmonyDisplayWholeNotes (), // would be 0/1 otherwise JMI
            elt->                getHarmonyDisplayWholeNotes (),
            0, // JMI elt->                getHarmonyDotsNumber (),
            fCurrentVoiceClone-> getRegularVoiceStaffSequentialNumber (), // JMI
            fCurrentVoiceClone-> getVoiceNumber ());

      // append it to the current voice clone
      // to 'push' the harmony aside
      fCurrentVoiceClone->
        appendNoteToVoice (skip);
    }
* /

    // append the harmony to the current voice clone
    fCurrentVoiceClone->
      appendHarmonyToVoiceClone (
        fCurrentHarmonyClone);
  }

  else {
    stringstream s;

    s <<
      "harmony '" << elt->asShortString () <<
      "' is out of context, cannot be handled";

    msrInternalError (
      gOahOah->fInputSourceName,
      elt->getInputLineNumber (),
      __FILE__, __LINE__,
      s.str ());
  }

  fOnGoingHarmony = true;
}

void msr2MxmltreeTranslator::visitStart (S_msrHarmonyDegree& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_msrHarmonyDegree '" <<
      elt->asString () <<
      ", fOnGoingNonGraceNote: " << booleanAsString (fOnGoingNonGraceNote) <<
      ", fOnGoingHarmonyVoice: " << booleanAsString (fOnGoingHarmonyVoice) <<
      ", fOnGoingHarmony: " << booleanAsString (fOnGoingHarmony) <<
      "', line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  // append the harmony degree to the current harmony clone
  fCurrentHarmonyClone->
    appendHarmonyDegreeToHarmony (
      elt);
}

void msr2MxmltreeTranslator::visitEnd (S_msrHarmony& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrHarmony '" <<
      elt->asString () <<
      "'" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  fCurrentHarmonyClone = nullptr;
  fOnGoingHarmony = false;
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrFrame& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrFrame '" <<
      elt->asString () <<
      "'" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  if (fOnGoingHarmony) {
    // register the frame in the current non-grace note clone
    fCurrentHarmonyClone->
      setHarmonyFrame (elt);
  }

  else {
    stringstream s;

    s <<
      "frame '" << elt->asShortString () <<
      "' is out of context, cannot be appendd";

    msrInternalError (
      gOahOah->fInputSourceName,
      elt->getInputLineNumber (),
      __FILE__, __LINE__,
      s.str ());
  }
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrFiguredBass& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrFiguredBass '" <<
      elt->asString () <<
      "'" <<
      ", fOnGoingFiguredBassVoice = " << booleanAsString (fOnGoingFiguredBassVoice) <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  // create a figured bass new born clone
  fCurrentFiguredBassClone =
    elt->
      createFiguredBassNewbornClone (
        fCurrentVoiceClone);

  if (fOnGoingNonGraceNote) {
    // append the figured bass to the current non-grace note clone
    fCurrentNonGraceNoteClone->
      appendFiguredBassToNoteFiguredBassesList (fCurrentFiguredBassClone);

    // don't append the figured bass to the part figured bass,  JMI ???
    // this will be done below
  }

  else if (fOnGoingFiguredBassVoice) { // JMI
    / *
    // register the figured bass in the part clone figured bass
    fCurrentPartClone->
      appendFiguredBassToPartClone (
        fCurrentVoiceClone,
        fCurrentFiguredBassClone);
        * /
    // append the figured bass to the current voice clone
    fCurrentVoiceClone->
      appendFiguredBassToVoiceClone (
        fCurrentFiguredBassClone);
  }

  else {
    stringstream s;

    s <<
      "figured bass '" << elt->asShortString () <<
      "' is out of context, cannot be append";

    msrInternalError (
      gOahOah->fInputSourceName,
      elt->getInputLineNumber (),
      __FILE__, __LINE__,
      s.str ());
  }
}

void msr2MxmltreeTranslator::visitStart (S_msrFigure& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrFigure '" <<
      elt->asString () <<
      "'" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  // append the figure to the current figured bass
  fCurrentFiguredBassClone->
    appendFigureToFiguredBass (
      elt);
}

void msr2MxmltreeTranslator::visitEnd (S_msrFiguredBass& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrFiguredBass '" <<
      elt->asString () <<
      "'" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  fCurrentFiguredBassClone = nullptr;
}
*/

/*
//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrStanza& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrStanza \"" <<
      elt->getStanzaName () <<
      "\"" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  gIndenter++;

//  if (elt->getStanzaTextPresent ()) { // JMI
    fCurrentStanzaClone =
      elt->createStanzaNewbornClone (
        fCurrentVoiceClone);

    // append the stanza clone to the LPSR score elements list
    fScorePartWiseElement->
      appendStanzaToScoreElementsList (
        fCurrentStanzaClone);

    // append a use of the stanza to the current staff block
    fCurrentStaffBlock ->
      appendLyricsUseToStaffBlock (
        fCurrentStanzaClone);
//  }
//  else
  //  fCurrentStanzaClone = 0; // JMI

  fOnGoingStanza = true;
}

void msr2MxmltreeTranslator::visitEnd (S_msrStanza& elt)
{
  gIndenter--;

#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrStanza \"" <<
      elt->getStanzaName () <<
      "\"" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  // forget about this stanza
  fCurrentStanzaClone = nullptr;

  fOnGoingStanza = false;
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrSyllable& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();

#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrSyllable" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  // create the syllable clone
  fCurrentSyllableClone =
    elt->createSyllableNewbornClone (
      fCurrentPartClone);

  // add it to the current stanza clone or current note clone
  if (fOnGoingStanza) { // fCurrentStanzaClone JM
    // visiting a syllable as a stanza member
    fCurrentStanzaClone->
      appendSyllableToStanza (
        fCurrentSyllableClone);
  }

  else if (fOnGoingNonGraceNote) { // JMI
    // visiting a syllable as attached to the current non-grace note
    fCurrentSyllableClone->
      appendSyllableToNoteAndSetItsNoteUpLink (
        fCurrentNonGraceNoteClone);

    if (gLpsrOah->fAddWordsFromTheLyrics) {
      // get the syllable texts list
      const list<string>&
        syllableTextsList =
          elt->getSyllableTextsList ();

      if (syllableTextsList.size ()) {
        // build a single words value from the texts list
        // JMI create an msrWords instance for each???
        string wordsValue =
          elt->syllableTextsListAsString();

        // create the words
#ifdef TRACE_OAH
        if (gTraceOah->fTraceLyrics) {
          fLogOutputStream <<
            "Changing lyrics '" <<
            wordsValue <<
            "' into words for note '" <<
            fCurrentNonGraceNoteClone->asShortString () <<
            "'" <<
      // JMI      fCurrentSyllableClone->asString () <<
            endl;
        }
#endif

        S_msrWords
          words =
            msrWords::create (
              inputLineNumber,
              kPlacementNone,                // default value
              wordsValue,
              kJustifyNone,                  // default value
              kHorizontalAlignmentNone,      // default value
              kVerticalAlignmentNone,        // default value
              kFontStyleNone,                // default value
              msrFontSize::create (
                msrFontSize::kFontSizeNone), // default value
              kFontWeightNone,               // default value
              kXMLLangIt,                    // default value
              elt->getSyllableNoteUpLink ()->getNoteStaffNumber ());

        // append it to the current non-grace note
#ifdef TRACE_OAH
        if (gTraceOah->fTraceWords) {
          fLogOutputStream <<
            "Appending words '" <<
            words->asShortString () <<
            "' to note '" <<
            fCurrentNonGraceNoteClone->asShortString () <<
            "'" <<
            endl;
        }
#endif
        fCurrentNonGraceNoteClone->
          appendWordsToNote (
            words);
      }
    }
  }
  else {
    stringstream s;

    s <<
      "syllable '" << elt->asShortString () <<
      "' is out of context, cannot be append";

    msrInternalError (
      gOahOah->fInputSourceName,
      elt->getInputLineNumber (),
      __FILE__, __LINE__,
      s.str ());
  }

  // a syllable ends the sysllable extend range if any
  if (fOnGoingSyllableExtend) {
    / * JMI ???
    // create melisma end command
    S_lpsrMelismaCommand
      melismaCommand =
        lpsrMelismaCommand::create (
          inputLineNumber,
          lpsrMelismaCommand::kMelismaEnd);

    // append it to current voice clone
    fCurrentVoiceClone->
      appendOtherElementToVoice (melismaCommand);
* /

    fOnGoingSyllableExtend = false;
  }
}

void msr2MxmltreeTranslator::visitEnd (S_msrSyllable& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrSyllable" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrTranspose& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrTranspose" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  // append transpose to voice clone
  fCurrentVoiceClone->
    appendTransposeToVoice (elt);
}

void msr2MxmltreeTranslator::visitEnd (S_msrTranspose& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrTranspose" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrPartNameDisplay& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrPartNameDisplay" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  // append part name display to voice clone
  fCurrentVoiceClone->
    appendPartNameDisplayToVoice (elt);
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrPartAbbreviationDisplay& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrPartAbbreviationDisplay" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  // append part abbreviation display to voice clone
  fCurrentVoiceClone->
    appendPartAbbreviationDisplayToVoice (elt);
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrRehearsal& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrRehearsal" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  fCurrentVoiceClone->
    appendRehearsalToVoice (elt);
}

void msr2MxmltreeTranslator::visitEnd (S_msrRehearsal& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrRehearsal" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrFermata& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrFermata" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  // a fermata is an articulation

  if (fOnGoingNonGraceNote) {
    fCurrentNonGraceNoteClone->
      appendArticulationToNote (elt);
  }
  else {
    stringstream s;

    s <<
      "fermata '" << elt->asShortString () <<
      "' is out of context, cannot be append";

    msrInternalError (
      gOahOah->fInputSourceName,
      elt->getInputLineNumber (),
      __FILE__, __LINE__,
      s.str ());
  }
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrArpeggiato& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrArpeggiato" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  // an arpeggiato is an articulation

  if (fOnGoingNonGraceNote) {
    fCurrentNonGraceNoteClone->
      appendArticulationToNote (elt); // addArpeggiatoToNote ??? JMI
  }
  else {
    stringstream s;

    s <<
      "arpeggiato '" << elt->asShortString () <<
      "' is out of context, cannot be append";

    msrInternalError (
      gOahOah->fInputSourceName,
      elt->getInputLineNumber (),
      __FILE__, __LINE__,
      s.str ());
  }
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrNonArpeggiato& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrNonArpeggiato" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  // an nonArpeggiato is an articulation

  if (fOnGoingNonGraceNote) {
    fCurrentNonGraceNoteClone->
      appendArticulationToNote (elt); // addArpeggiatoToNote ??? JMI
  }
  else if (fOnGoingChord) {
    fCurrentChordClone->
      appendArticulationToChord (elt);
  }
  else {
    stringstream s;

    s <<
      "nonArpeggiato '" << elt->asShortString () <<
      "' is out of context, cannot be append";

    msrInternalError (
      gOahOah->fInputSourceName,
      elt->getInputLineNumber (),
      __FILE__, __LINE__,
      s.str ());
  }
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrTechnical& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrTechnical" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  if (fOnGoingNonGraceNote) {
    fCurrentNonGraceNoteClone->
      appendTechnicalToNote (elt);
  }
  else if (fOnGoingChord) {
    fCurrentChordClone->
      appendTechnicalToChord (elt);
  }
  else {
    stringstream s;

    s <<
      "technical '" << elt->asShortString () <<
      "' is out of context, cannot be append";

    msrInternalError (
      gOahOah->fInputSourceName,
      elt->getInputLineNumber (),
      __FILE__, __LINE__,
      s.str ());
  }

  // doest the score need the 'tongue' function?
  switch (elt->getTechnicalKind ()) {
    case msrTechnical::kArrow:
      break;
    case msrTechnical::kDoubleTongue:
      fScorePartWiseElement->
        // this score needs the 'tongue' Scheme function
        setTongueSchemeFunctionIsNeeded ();
      break;
    case msrTechnical::kDownBow:
      break;
    case msrTechnical::kFingernails:
      break;
    case msrTechnical::kHarmonic:
      break;
    case msrTechnical::kHeel:
      break;
    case msrTechnical::kHole:
      break;
    case msrTechnical::kOpenString:
      break;
    case msrTechnical::kSnapPizzicato:
      break;
    case msrTechnical::kStopped:
      break;
    case msrTechnical::kTap:
      break;
    case msrTechnical::kThumbPosition:
      break;
    case msrTechnical::kToe:
      break;
    case msrTechnical::kTripleTongue:
      fScorePartWiseElement->
        // this score needs the 'tongue' Scheme function
        setTongueSchemeFunctionIsNeeded ();
      break;
    case msrTechnical::kUpBow:
      break;
  } // switch
}

void msr2MxmltreeTranslator::visitEnd (S_msrTechnical& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrTechnical" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrTechnicalWithInteger& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrTechnicalWithInteger" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  if (fOnGoingNonGraceNote) {
    fCurrentNonGraceNoteClone->
      appendTechnicalWithIntegerToNote (elt);
  }
  else if (fOnGoingChord) {
    fCurrentChordClone->
      appendTechnicalWithIntegerToChord (elt);
  }
  else {
    stringstream s;

    s <<
      "technicalWithInteger '" << elt->asShortString () <<
      "' is out of context, cannot be append";

    msrInternalError (
      gOahOah->fInputSourceName,
      elt->getInputLineNumber (),
      __FILE__, __LINE__,
      s.str ());
  }
}

void msr2MxmltreeTranslator::visitEnd (S_msrTechnicalWithInteger& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrTechnicalWithInteger" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrTechnicalWithFloat& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrTechnicalWithFloat" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  if (fOnGoingNonGraceNote) {
    fCurrentNonGraceNoteClone->
      appendTechnicalWithFloatToNote (elt);
  }
  else if (fOnGoingChord) {
    fCurrentChordClone->
      appendTechnicalWithFloatToChord (elt);
  }
  else {
    stringstream s;

    s <<
      "technicalWithFloat '" << elt->asShortString () <<
      "' is out of context, cannot be append";

    msrInternalError (
      gOahOah->fInputSourceName,
      elt->getInputLineNumber (),
      __FILE__, __LINE__,
      s.str ());
  }
}

void msr2MxmltreeTranslator::visitEnd (S_msrTechnicalWithFloat& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrTechnicalWithFloat" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrTechnicalWithString& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrTechnicalWithString" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  if (fOnGoingNonGraceNote) {
    fCurrentNonGraceNoteClone->
      appendTechnicalWithStringToNote (elt);
  }
  else if (fOnGoingChord) {
    fCurrentChordClone->
      appendTechnicalWithStringToChord (elt);
  }
  else {
    stringstream s;

    s <<
      "technicalWithString '" << elt->asShortString () <<
      "' is out of context, cannot be append";

    msrInternalError (
      gOahOah->fInputSourceName,
      elt->getInputLineNumber (),
      __FILE__, __LINE__,
      s.str ());
  }

  switch (elt->getTechnicalWithStringKind ()) {
    case msrTechnicalWithString::kHammerOn:
    case msrTechnicalWithString::kPullOff:
      // this score needs the 'after' Scheme function
      fScorePartWiseElement->
        setAfterSchemeFunctionIsNeeded ();
      break;
    default:
      ;
  } // switch
}

void msr2MxmltreeTranslator::visitEnd (S_msrTechnicalWithString& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrTechnicalWithString" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrOrnament& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrOrnament" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  if (fOnGoingNonGraceNote) {
    fCurrentNonGraceNoteClone->
      appendOrnamentToNote (elt);
  }
  else if (fOnGoingChord) {
    fCurrentChordClone->
      appendOrnamentToChord (elt);
  }
  else {
    stringstream s;

    s <<
      "ornament '" << elt->asShortString () <<
      "' is out of context, cannot be append";

    msrInternalError (
      gOahOah->fInputSourceName,
      elt->getInputLineNumber (),
      __FILE__, __LINE__,
      s.str ());
  }
}

void msr2MxmltreeTranslator::visitEnd (S_msrOrnament& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrOrnament" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrGlissando& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrGlissando" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  if (fOnGoingNonGraceNote) {
    fCurrentNonGraceNoteClone->
      appendGlissandoToNote (elt);
  }
  else if (fOnGoingChord) {
    fCurrentChordClone->
      appendGlissandoToChord (elt);
  }
  else {
    stringstream s;

    s <<
      "glissando '" << elt->asShortString () <<
      "' is out of context, cannot be append";

    msrInternalError (
      gOahOah->fInputSourceName,
      elt->getInputLineNumber (),
      __FILE__, __LINE__,
      s.str ());
  }

  if (elt->getGlissandoTextValue ().size ()) {
    fScorePartWiseElement->
      // this score needs the 'glissandoWithText' Scheme functions
      addGlissandoWithTextSchemeFunctionsToScore ();
  }
}

void msr2MxmltreeTranslator::visitEnd (S_msrGlissando& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrGlissando" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrSlide& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrSlide" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  if (fOnGoingNonGraceNote) {
    fCurrentNonGraceNoteClone->
      appendSlideToNote (elt);
  }
  else if (fOnGoingChord) {
    fCurrentChordClone->
      appendSlideToChord (elt);
  }
  else {
    stringstream s;

    s <<
      "slide '" << elt->asShortString () <<
      "' is out of context, cannot be append";

    msrInternalError (
      gOahOah->fInputSourceName,
      elt->getInputLineNumber (),
      __FILE__, __LINE__,
      s.str ());
  }
}

void msr2MxmltreeTranslator::visitEnd (S_msrSlide& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrSlide" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrSingleTremolo& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrSingleTremolo" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  if (fOnGoingNonGraceNote) {
    fCurrentNonGraceNoteClone->
      setNoteSingleTremolo (elt);
  }
  else if (fOnGoingChord) {
    fCurrentChordClone->
      setChordSingleTremolo (elt);
  }
  else {
    stringstream s;

    s <<
      "singleTremolo '" << elt->asShortString () <<
      "' is out of context, cannot be append";

    msrInternalError (
      gOahOah->fInputSourceName,
      elt->getInputLineNumber (),
      __FILE__, __LINE__,
      s.str ());
  }
}

void msr2MxmltreeTranslator::visitEnd (S_msrSingleTremolo& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrSingleTremolo" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrDoubleTremolo& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrDoubleTremolo" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  // create a double tremolo clone from the two elements
  fCurrentDoubleTremoloClone = elt; // JMI FIX THAT
/ * JMI
    elt->createDoubleTremoloNewbornClone (
      elt->getDoubleTremoloFirstElement ()->
        createNewBornClone (),
      elt->getDoubleTremoloSecondElement ()
        createNewBornClone ());
        * /

  fOnGoingDoubleTremolo = true;
}

void msr2MxmltreeTranslator::visitEnd (S_msrDoubleTremolo& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrSingleTremolo" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  // append the current double tremolo clone to the current voice clone
  fCurrentVoiceClone->
    appendDoubleTremoloToVoice (
      fCurrentDoubleTremoloClone);

  // forget about it
  fCurrentDoubleTremoloClone = nullptr;

  fOnGoingDoubleTremolo = false;
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrDynamics& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrDynamics" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  if (fOnGoingNonGraceNote) {
    fCurrentNonGraceNoteClone->
      appendDynamicsToNote (elt);

    // is this a non LilyPond native dynamics?
    bool knownToLilyPondNatively = true;

    switch (elt->getDynamicsKind ()) {
      case msrDynamics::kFFFFF:
      case msrDynamics::kFFFFFF:
      case msrDynamics::kPPPPP:
      case msrDynamics::kPPPPPP:
      case msrDynamics::kRF:
      case msrDynamics::kSFPP:
      case msrDynamics::kSFFZ:
      case msrDynamics::k_NoDynamics:
        knownToLilyPondNatively = false;

      default:
        ;
    } // switch

    if (! knownToLilyPondNatively) {
      // this score needs the 'dynamics' Scheme function
      fScorePartWiseElement->
        setDynamicsSchemeFunctionIsNeeded ();
    }
  }

  else if (fOnGoingChord) {
    fCurrentChordClone->
      appendDynamicsToChord (elt);
  }

  else {
    stringstream s;

    s <<
      "dynamics '" << elt->asShortString () <<
      "' is out of context, cannot be append";

    msrInternalError (
      gOahOah->fInputSourceName,
      elt->getInputLineNumber (),
      __FILE__, __LINE__,
      s.str ());
  }
}

void msr2MxmltreeTranslator::visitEnd (S_msrDynamics& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrDynamics" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrOtherDynamics& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrOtherDynamics" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  if (fOnGoingNonGraceNote) {
    fCurrentNonGraceNoteClone->
      appendOtherDynamicsToNote (elt);
  }
  else if (fOnGoingChord) {
    fCurrentChordClone->
      appendOtherDynamicsToChord (elt);
  }
  else {
    stringstream s;

    s <<
      "otherDynamics '" << elt->asShortString () <<
      "' is out of context, cannot be append";

    msrInternalError (
      gOahOah->fInputSourceName,
      elt->getInputLineNumber (),
      __FILE__, __LINE__,
      s.str ());
  }

  fScorePartWiseElement->
    // this score needs the 'otherDynamic' Scheme function
    setOtherDynamicSchemeFunctionIsNeeded ();
}

void msr2MxmltreeTranslator::visitEnd (S_msrOtherDynamics& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrOtherDynamics" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrWords& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();

#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrWords" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  if (fOnGoingNonGraceNote || fOnGoingChord) {
    bool wordsHasBeenappendd = false;

    if (gLpsrOah->fConvertWordsToTempo) {
      // create a tempo containing elt
      S_msrTempo
        tempo =
          msrTempo::create (
            inputLineNumber,
            elt);

#ifdef TRACE_OAH
      if (gTraceOah->fTraceWords) {
        fLogOutputStream <<
          "Converting words '" <<
          elt->asShortString () <<
          "' to tempo '" <<
          tempo->asShortString () <<
          "'" <<
          endl;
      }
#endif

      // append the tempo to the current voice clone
      fCurrentVoiceClone->
        appendTempoToVoice (tempo);

      wordsHasBeenappendd = true;
    }

    else if (gLpsrOah->fConvertWordsToRehearsalMarks) {
      // create a rehearsal mark containing elt's words contents
      S_msrRehearsal
        rehearsal =
          msrRehearsal::create (
            inputLineNumber,
            msrRehearsal::kNone,
            elt->getWordsContents (),
            elt->getWordsPlacementKind ()); // above ??? JMI

#ifdef TRACE_OAH
      if (gTraceOah->fTraceWords) {
        fLogOutputStream <<
          "Converting words '" <<
          elt->asShortString () <<
          "' to rehearsal mark '" <<
          rehearsal->asShortString () <<
          "'" <<
          endl;
      }
#endif

      // append the rehearsal to the current voice clone
      fCurrentVoiceClone->
        appendRehearsalToVoice (rehearsal);

      wordsHasBeenappendd = true;
    }

    else {
    / * JMI
      string wordsContents = elt->getWordsContents ();

      // is this words contents in the string to dal segno kind map?
      map<string, msrDalSegno::msrDalSegnoKind>::iterator
        it =
          gLpsrOah->fConvertWordsToDalSegno.find (wordsContents);

      if (it != gLpsrOah->fConvertWordsToDalSegno.end ()) {
        // yes
        msrDalSegno::msrDalSegnoKind
          dalSegnoKind =
            (*it).second;

        // create a dal segno element containing elt's words contents
        S_msrDalSegno
          dalSegno =
            msrDalSegno::create (
              inputLineNumber,
              dalSegnoKind,
              wordsContents,
              elt->getWordsStaffNumber ());

#ifdef TRACE_OAH
        if (gTraceOah->fTraceWords) {
          fLogOutputStream <<
            "Converting words '" <<
            elt->asShortString () <<
            "' to dal segno element '" <<
            dalSegno->asShortString () <<
            "'" <<
            endl;
        }
#endif

        if (fOnGoingNonGraceNote) {
          fCurrentNonGraceNoteClone->
            appendDalSegnoToNote (dalSegno);
        }
        else if (fOnGoingChord) {
          fCurrentChordClone->
            appendDalSegnoToChord (dalSegno);
        }

      wordsHasBeenappendd = true;
      }
      * /
    }

    if (! wordsHasBeenappendd) {
      if (fOnGoingNonGraceNote) {
        fCurrentNonGraceNoteClone->
          appendWordsToNote (elt);
      }
      else if (fOnGoingChord) {
        fCurrentChordClone->
          appendWordsToChord (elt);
      }
    }
  }

  else {
    stringstream s;

    s <<
      "words '" << elt->asShortString () <<
      "' is out of context, cannot be handled";

    msrInternalError (
      gOahOah->fInputSourceName,
      elt->getInputLineNumber (),
      __FILE__, __LINE__,
      s.str ());
  }
}

void msr2MxmltreeTranslator::visitEnd (S_msrWords& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrWords" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrSlur& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrSlur" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  / *
    Only the  first note of the chord should get the slur notation.
    Some applications print out the slur for all notes,
    i.e. a stop and a start in sequence:
    these should be ignored
  * /

  if (fOnGoingNonGraceNote) {
    // don't add slurs to chord member notes except the first one
    switch (fCurrentNonGraceNoteClone->getNoteKind ()) {
      case msrNote::kChordMemberNote:
        if (fCurrentNonGraceNoteClone->getNoteIsAChordsFirstMemberNote ()) {
          fCurrentNonGraceNoteClone->
            appendSlurToNote (elt);
        }
        break;

      default:
        fCurrentNonGraceNoteClone->
          appendSlurToNote (elt);
    } // switch
  }

  else if (fOnGoingChord) {
    fCurrentChordClone->
      appendSlurToChord (elt);
  }

  else {
    displayCurrentOnGoingValues ();

    stringstream s;

    s <<
      "slur '" << elt->asShortString () <<
      "' is out of context, cannot be handled";

    msrInternalError (
      gOahOah->fInputSourceName,
      elt->getInputLineNumber (),
      __FILE__, __LINE__,
      s.str ());
  }
}

void msr2MxmltreeTranslator::visitEnd (S_msrSlur& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrSlur" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrLigature& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrLigature" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  if (fOnGoingNonGraceNote) {
    fCurrentNonGraceNoteClone->
      appendLigatureToNote (elt);
  }
  else if (fOnGoingChord) {
    fCurrentChordClone->
      appendLigatureToChord (elt);
  }
  else {
    stringstream s;

    s <<
      "ligature '" << elt->asShortString () <<
      "' is out of context, cannot be handled";

    msrInternalError (
      gOahOah->fInputSourceName,
      elt->getInputLineNumber (),
      __FILE__, __LINE__,
      s.str ());
  }
}

void msr2MxmltreeTranslator::visitEnd (S_msrLigature& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrLigature" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrSlash& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrSlash" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  if (fOnGoingNonGraceNote) {
    fCurrentNonGraceNoteClone->
      appendSlashToNote (elt);
  }
  else if (fOnGoingChord) {
    fCurrentChordClone->
      appendSlashToChord (elt);
  }
  else {
    stringstream s;

    s <<
      "slash '" << elt->asShortString () <<
      "' is out of context, cannot be handled";

    msrInternalError (
      gOahOah->fInputSourceName,
      elt->getInputLineNumber (),
      __FILE__, __LINE__,
      s.str ());
  }
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrWedge& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrWedge" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  if (fOnGoingNonGraceNote) {
    fCurrentNonGraceNoteClone->
      appendWedgeToNote (elt);
  }
  else if (fOnGoingChord) {
    fCurrentChordClone->
      appendWedgeToChord (elt);
  }
  else {
    stringstream s;

    s <<
      "wedge '" << elt->asShortString () <<
      "' is out of context, cannot be handled";

    msrInternalError (
      gOahOah->fInputSourceName,
      elt->getInputLineNumber (),
      __FILE__, __LINE__,
      s.str ());
  }
}

void msr2MxmltreeTranslator::visitEnd (S_msrWedge& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrWedge" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

*/

/*
//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrOctaveShift& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrOctaveShift" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  if (fOnGoingNonGraceNote) {
    fCurrentNonGraceNoteClone->
      setNoteOctaveShift (elt);
  }
  else if (fOnGoingChord) {
    fCurrentChordClone->
      setChordOctaveShift (elt);
  }
  else {
    stringstream s;

    s <<
      "octaveShift '" << elt->asShortString () <<
      "' is out of context, cannot be handled";

    msrInternalError (
      gOahOah->fInputSourceName,
      elt->getInputLineNumber (),
      __FILE__, __LINE__,
      s.str ());
  }
}

void msr2MxmltreeTranslator::visitEnd (S_msrOctaveShift& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrOctaveShift" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrAccordionRegistration& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrAccordionRegistration" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  // append the accordion registration to the voice clone
  fCurrentVoiceClone->
    appendAccordionRegistrationToVoice (elt);

  // the generated code needs modules scm and accreg
  fScorePartWiseElement->
    setScmAndAccregSchemeModulesAreNeeded ();
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrHarpPedalsTuning& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrHarpPedalsTuning" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  // append the harp pedals tuning to the voice clone
  fCurrentVoiceClone->
    appendHarpPedalsTuningToVoice (elt);
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrStem& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrStem" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  if (fOnGoingNonGraceNote) {
    fCurrentNonGraceNoteClone->
      setNoteStem (elt);
  }
  else if (fOnGoingChord) {
    fCurrentChordClone->
      appendStemToChord (elt);
  }
  else {
    stringstream s;

    s <<
      "stem '" << elt->asShortString () <<
      "' is out of context, cannot be handled";

    msrInternalError (
      gOahOah->fInputSourceName,
      elt->getInputLineNumber (),
      __FILE__, __LINE__,
      s.str ());
  }
}

void msr2MxmltreeTranslator::visitEnd (S_msrStem& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrStem" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrBeam& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrBeam" <<
      ", line " << elt->getInputLineNumber () <<
// JMI      ", fOnGoingNonGraceNote = " << booleanAsString (fOnGoingNonGraceNote) <<
// JMI      ", fOnGoingChord = " << booleanAsString (fOnGoingChord) <<
      endl;
  }
#endif

  // a beam may be present at the same time
  // in a note or grace note and the chord the latter belongs to

  if (fOnGoingGraceNotesGroup) {
    fCurrentGraceNoteClone->
      appendBeamToNote (elt);
  }
  else if (fOnGoingNonGraceNote) {
    fCurrentNonGraceNoteClone->
      appendBeamToNote (elt);
  }

  if (fOnGoingChord) { // else ??? JMI
    fCurrentChordClone->
      appendBeamToChord (elt);
  }
}

void msr2MxmltreeTranslator::visitEnd (S_msrBeam& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrBeam" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrTie& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrTie" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  if (fOnGoingNonGraceNote) {
    fCurrentNonGraceNoteClone->
      setNoteTie (elt);
  }
  else if (fOnGoingChord) {
    fCurrentChordClone->
      appendTieToChord (elt);
  }
  else {
    stringstream s;

    s <<
      "tie '" << elt->asShortString () <<
      "' is out of context, cannot be handled";

    msrInternalError (
      gOahOah->fInputSourceName,
      elt->getInputLineNumber (),
      __FILE__, __LINE__,
      s.str ());
  }
}

void msr2MxmltreeTranslator::visitEnd (S_msrTie& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrTie" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrSegno& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrSegno" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  if (fOnGoingChord || fOnGoingNonGraceNote) {
    if (fOnGoingChord) {
      fCurrentChordClone->
        appendSegnoToChord (elt);
    }
    if (fOnGoingNonGraceNote) {
      fCurrentNonGraceNoteClone->
        appendSegnoToNote (elt);
    }
  }
  else {
    stringstream s;

    s <<
      "segno '" << elt->asShortString () <<
      "' is out of context, cannot be handled";

    msrInternalError (
      gOahOah->fInputSourceName,
      elt->getInputLineNumber (),
      __FILE__, __LINE__,
      s.str ());
  }
}

void msr2MxmltreeTranslator::visitStart (S_msrDalSegno& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();

#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrDalSegno" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  if (fOnGoingChord || fOnGoingNonGraceNote) {
    if (fOnGoingChord) {
      fCurrentChordClone->
        appendDalSegnoToChord (elt);
    }
    if (fOnGoingNonGraceNote) {
      fCurrentNonGraceNoteClone->
        appendDalSegnoToNote (elt);
    }
  }
  else {
    stringstream s;

    s <<
      "dal segno '" << elt->asShortString () <<
      "' is out of context, cannot be handled";

    msrInternalError (
      gOahOah->fInputSourceName,
      inputLineNumber,
      __FILE__, __LINE__,
      s.str ());
  }
}

void msr2MxmltreeTranslator::visitStart (S_msrCoda& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrCoda" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  if (fOnGoingChord || fOnGoingNonGraceNote) {
    if (fOnGoingChord) {
      fCurrentChordClone->
        appendCodaToChord (elt);
    }
    if (fOnGoingNonGraceNote) {
      fCurrentNonGraceNoteClone->
        appendCodaToNote (elt);
    }
  }
  else {
    stringstream s;

    s <<
      "coda '" << elt->asShortString () <<
      "' is out of context, cannot be handled";

    msrInternalError (
      gOahOah->fInputSourceName,
      elt->getInputLineNumber (),
      __FILE__, __LINE__,
      s.str ());
  }
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrEyeGlasses& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting eyeGlasses" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  fCurrentNonGraceNoteClone->
    appendEyeGlassesToNote (elt);
}

void msr2MxmltreeTranslator::visitStart (S_msrScordatura& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting scordatura" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  fCurrentNonGraceNoteClone->
    appendScordaturaToNote (elt);
}

void msr2MxmltreeTranslator::visitStart (S_msrPedal& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting pedal" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  fCurrentNonGraceNoteClone->
    appendPedalToNote (elt);
}

void msr2MxmltreeTranslator::visitStart (S_msrDamp& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting damp" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  fCurrentNonGraceNoteClone->
    appendDampToNote (elt);

  fScorePartWiseElement->
    // this score needs the 'custom short barline' Scheme function
    setDampMarkupIsNeeded ();
}

void msr2MxmltreeTranslator::visitStart (S_msrDampAll& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting dampAll" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  fCurrentNonGraceNoteClone->
    appendDampAllToNote (elt);

  fScorePartWiseElement->
    // this score needs the 'custom short barline' Scheme function
    setDampAllMarkupIsNeeded ();
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrBarCheck& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrBarCheck" <<
      ", nextBarNumber: " <<
      elt->getNextBarPuristNumber () <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  fCurrentVoiceClone->
    appendBarCheckToVoice (elt);
}

void msr2MxmltreeTranslator::visitEnd (S_msrBarCheck& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrBarCheck" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrBarNumberCheck& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrBarNumberCheck" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  fCurrentVoiceClone->
    appendBarNumberCheckToVoice (elt);
}

void msr2MxmltreeTranslator::visitEnd (S_msrBarNumberCheck& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrBarNumberCheck" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrLineBreak& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrLineBreak" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  fCurrentVoiceClone->
    appendLineBreakToVoice (elt);
}

void msr2MxmltreeTranslator::visitEnd (S_msrLineBreak& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrLineBreak" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrPageBreak& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrPageBreak" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  fCurrentVoiceClone->
    appendPageBreakToVoice (elt);
}

void msr2MxmltreeTranslator::visitEnd (S_msrPageBreak& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrPageBreak" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrRepeat& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();

#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrRepeat" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

#ifdef TRACE_OAH
  if (gTraceOah->fTraceRepeats) {
    fLogOutputStream <<
      "Handling repeat start in voice clone \"" <<
      fCurrentVoiceClone->getVoiceName () <<
      "\"" <<
      endl;
  }
#endif

  fCurrentVoiceClone->
    handleRepeatStartInVoiceClone (
      inputLineNumber,
      elt);
}

void msr2MxmltreeTranslator::visitEnd (S_msrRepeat& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();

#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrRepeat" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

#ifdef TRACE_OAH
  if (gTraceOah->fTraceRepeats) {
    fLogOutputStream <<
      "Handling repeat end in voice clone \"" <<
      fCurrentVoiceClone->getVoiceName () <<
//      "\" in part \"" <<
//      fCurrentPartClone->getPartCombinedName () <<
      "\"" <<
      endl;
  }
#endif

  fCurrentVoiceClone->
    handleRepeatEndInVoiceClone (
      inputLineNumber);
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrRepeatCommonPart& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();

#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrRepeatCommonPart" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  fCurrentVoiceClone->
    handleRepeatCommonPartStartInVoiceClone (
      inputLineNumber);
}

void msr2MxmltreeTranslator::visitEnd (S_msrRepeatCommonPart& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();

#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrRepeatCommonPart" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  fCurrentVoiceClone->
    handleRepeatCommonPartEndInVoiceClone (
      inputLineNumber);
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrRepeatEnding& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();

#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrRepeatEnding" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  // handle the repeat ending start in the voice clone
#ifdef TRACE_OAH
  if (gTraceOah->fTraceRepeats) {
    fLogOutputStream <<
      "Handling a repeat ending start in voice clone \"" <<
      fCurrentVoiceClone->getVoiceName () <<
      "\"" <<
      endl;
  }
#endif

  fCurrentVoiceClone->
    handleRepeatEndingStartInVoiceClone (
      inputLineNumber,
      elt->getRepeatEndingKind (),
      elt->getRepeatEndingNumber ());
}

void msr2MxmltreeTranslator::visitEnd (S_msrRepeatEnding& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();

#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrRepeatEnding" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  // handle the repeat ending end in the voice clone
#ifdef TRACE_OAH
  if (gTraceOah->fTraceRepeats) {
    fLogOutputStream <<
      "Handling a repeat ending end in voice clone \"" <<
      fCurrentVoiceClone->getVoiceName () <<
      "\"" <<
      endl;
  }
#endif

  fCurrentVoiceClone->
    handleRepeatEndingEndInVoiceClone (
      inputLineNumber,
      elt->getRepeatEndingNumber (),
      elt->getRepeatEndingKind ());
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrRestMeasures& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();

#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrRestMeasures" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  gIndenter++;

#ifdef TRACE_OAH
  if (gTraceOah->fTraceRestMeasures) {
    fLogOutputStream <<
      "Handling multiple rest start in voice clone \"" <<
      fCurrentVoiceClone->getVoiceName () <<
      "\"" <<
      endl;
  }
#endif

  fCurrentVoiceClone->
    handleRestMeasuresStartInVoiceClone (
      inputLineNumber,
      elt);
}

void msr2MxmltreeTranslator::visitEnd (S_msrRestMeasures& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();

#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrRestMeasures" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  gIndenter--;

#ifdef TRACE_OAH
  if (gTraceOah->fTraceRestMeasures) {
    fLogOutputStream <<
      "Handling multiple rest start in voice clone \"" <<
      fCurrentVoiceClone->getVoiceName () <<
      "\"" <<
      endl;
  }
#endif

  fCurrentVoiceClone->
    handleRestMeasuresEndInVoiceClone (
      inputLineNumber);
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrRestMeasuresContents& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();

#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrRestMeasuresContents" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  gIndenter++;

#ifdef TRACE_OAH
  if (gTraceOah->fTraceRestMeasures) {
    fCurrentVoiceClone->
      displayVoice (
        inputLineNumber,
        "Upon visitStart (S_msrRestMeasuresContents&)");
  }
#endif

  fCurrentVoiceClone->
    handleRestMeasuresContentsStartInVoiceClone (
      inputLineNumber);
}

void msr2MxmltreeTranslator::visitEnd (S_msrRestMeasuresContents& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();

#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrRestMeasuresContents" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  gIndenter--;

#ifdef TRACE_OAH
  if (gTraceOah->fTraceRestMeasures) {
    fCurrentVoiceClone->
      displayVoice (
        inputLineNumber,
        "Upon visitEnd (S_msrRestMeasuresContents&) 1");
  }
#endif

  fCurrentVoiceClone->
    handleRestMeasuresContentsEndInVoiceClone (
      inputLineNumber);
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrMeasuresRepeat& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();

#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrMeasuresRepeat" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  gIndenter++;

#ifdef TRACE_OAH
  if (gTraceOah->fTraceMeasuresRepeats) {
    fLogOutputStream <<
      "Handling measures repeat start in voice clone \"" <<
      fCurrentVoiceClone->getVoiceName () <<
      "\"" <<
      endl;
  }
#endif

  fCurrentVoiceClone->
    handleMeasuresRepeatStartInVoiceClone (
      inputLineNumber,
      elt);
}

void msr2MxmltreeTranslator::visitEnd (S_msrMeasuresRepeat& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();

#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrMeasuresRepeat" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  gIndenter--;

/ * JMI
  // set last segment as the measures repeat pattern segment
#ifdef TRACE_OAH
  if (gTraceOah->fTraceMeasuresRepeats) {
    fLogOutputStream <<
      "Setting current last segment as measures repeat pattern segment in voice \"" <<
      fCurrentVoiceClone->getVoiceName () <<
      "\"" <<
      endl;
  }
#endif
* /

#ifdef TRACE_OAH
  if (gTraceOah->fTraceMeasuresRepeats) {
    fLogOutputStream <<
      "Handling measures repeat end in voice clone \"" <<
      fCurrentVoiceClone->getVoiceName () <<
      "\"" <<
      endl;
  }
#endif

  fCurrentVoiceClone->
    handleMeasuresRepeatEndInVoiceClone (
      inputLineNumber);
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrMeasuresRepeatPattern& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();

#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrMeasuresRepeatPattern" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  gIndenter++;

#ifdef TRACE_OAH
  if (gTraceOah->fTraceMeasuresRepeats) {
    fCurrentVoiceClone->
      displayVoice (
        inputLineNumber,
        "Upon visitStart (S_msrMeasuresRepeatPattern&)");
  }
#endif

  fCurrentVoiceClone->
    handleMeasuresRepeatPatternStartInVoiceClone (
      inputLineNumber);
}

void msr2MxmltreeTranslator::visitEnd (S_msrMeasuresRepeatPattern& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();

#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrMeasuresRepeatPattern" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  gIndenter--;

#ifdef TRACE_OAH
  if (gTraceOah->fTraceMeasuresRepeats) {
    fCurrentVoiceClone->
      displayVoice (
        inputLineNumber,
        "Upon visitEnd (S_msrMeasuresRepeatPattern&) 1");
  }
#endif

  fCurrentVoiceClone->
    handleMeasuresRepeatPatternEndInVoiceClone (
      inputLineNumber);
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrMeasuresRepeatReplicas& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();

#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrMeasuresRepeatReplicas" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  gIndenter++;

#ifdef TRACE_OAH
  if (gTraceOah->fTraceMeasuresRepeats) {
    fCurrentVoiceClone->
      displayVoice (
        inputLineNumber,
        "Upon visitStart (S_msrMeasuresRepeatReplicas&)");
  }
#endif

  fCurrentVoiceClone->
    handleMeasuresRepeatReplicasStartInVoiceClone (
      inputLineNumber);
}

void msr2MxmltreeTranslator::visitEnd (S_msrMeasuresRepeatReplicas& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();

#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting S_msrMeasuresRepeatReplicas" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  gIndenter--;


#ifdef TRACE_OAH
  if (gTraceOah->fTraceMeasuresRepeats) {
    fCurrentVoiceClone->
      displayVoice (
        inputLineNumber,
        "Upon visitEnd (S_msrMeasuresRepeatReplicas&) 1");
  }
#endif

  fCurrentVoiceClone->
    handleMeasuresRepeatReplicasEndInVoiceClone (
      inputLineNumber);
}

//________________________________________________________________________
void msr2MxmltreeTranslator::visitStart (S_msrMidiTempo& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrMidiTempo" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

void msr2MxmltreeTranslator::visitEnd (S_msrMidiTempo& elt)
{
#ifdef TRACE_OAH
  if (gMsrOah->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrMidiTempo" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}
*/

} // namespace


/*
//________________________________________________________________________
void msr2MxmltreeTranslator::displayCurrentOnGoingValues ()
{
  fLogOutputStream <<
    "Ongoing value:" <<
    endl;

  gIndenter++;

  const int fieldWidth = 25;

  fLogOutputStream <<
    setw (fieldWidth) <<
    "fOnGoingStaff" << ": " << booleanAsString (fOnGoingStaff) <<
    endl<<
    setw (fieldWidth) <<
    "fOnGoingHarmonyVoice" << ": " << booleanAsString (fOnGoingHarmonyVoice) <<
    endl<<
    setw (fieldWidth) <<
    "fOnGoingHarmony" << ": " << booleanAsString (fOnGoingHarmony) <<
    endl<<
    setw (fieldWidth) <<
    "fOnGoingFiguredBassVoice" << ": " << booleanAsString (fOnGoingFiguredBassVoice) <<
    endl<<
    setw (fieldWidth) <<
    "fOnGoingNonGraceNote" << ": " << booleanAsString (fOnGoingNonGraceNote) <<
    endl<<
    setw (fieldWidth) <<
    "fOnGoingDoubleTremolo" << ": " << booleanAsString (fOnGoingDoubleTremolo) <<
    endl<<
    setw (fieldWidth) <<
    "fOnGoingGraceNotesGroup" << ": " << booleanAsString (fOnGoingGraceNotesGroup) <<
    endl<<
    setw (fieldWidth) <<
    "fOnGoingChord" << ": " << booleanAsString (fOnGoingChord) <<
    endl<<
    setw (fieldWidth) <<
    "fOnGoingStanza" << ": " << booleanAsString (fOnGoingStanza) <<
    endl<<
    setw (fieldWidth) <<
    "fOnGoingSyllableExtend" << ": " << booleanAsString (fOnGoingSyllableExtend) <<
    endl;

  gIndenter--;
}
*/

/* JMI
//________________________________________________________________________
// a comparison class to sort elements
class musicXMLOrder
{
	public:

    // constructors/destructor
    // ------------------------------------------------------

    musicXMLOrder (
      map<int,int>& order, Sxmlelement container);

		virtual	~musicXMLOrder ();

	public:

    // services
    // ------------------------------------------------------

		bool	                operator()	(Sxmlelement a, Sxmlelement b);

  private:

    // fields
    // ------------------------------------------------------

	  map<int,int>&	        fOrder;
	  Sxmlelement		        fContainer;
};

musicXMLOrder::musicXMLOrder (
  map<int,int>& order, Sxmlelement container)
  : fOrder (order)
{
  fContainer = container;
}

musicXMLOrder::~musicXMLOrder ()
{}

bool musicXMLOrder::operator() (Sxmlelement a, Sxmlelement b)
{
	int aIndex = fOrder [a->getType ()];
	int bIndex = fOrder [b->getType ()];

	if (aIndex == 0) return false; // wrong a element: reject to end of list
	if (bIndex == 0) return true;	 // wrong b element: reject to end of list

	return aIndex < bIndex;
}
*/