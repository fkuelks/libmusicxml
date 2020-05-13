\version "2.19.83"

% Generated by xml2ly 0.93 from "partgroups/NestedPartGroups.xml"
% on Tuesday 2020-03-24 @ 18:30:04 CET

% The translation command line was:
%   xml2ly '-xi' '-aofn' '-dmsr' '-dpg' '--dlpsr' '-abn' '-lpcd' '-rbracks' '-mssp' '1cm' '-loop' 'partgroups/NestedPartGroups.xml'
% or, with long option names:
%  xml2ly partgroups/NestedPartGroups.xml -xml2ly-infos -auto-output-file-name -display-msr -display-partgroups -display-lpsr -all-bar-numbers -lilypond-compile-date -repeat-brackets -markup-system-spacing.padding '1 cm' -loop-to-musicxml
% or, with short option names:
%   xml2ly partgroups/NestedPartGroups.xml -xi -aofn -dmsr -dpg -dlpsr -abn -lpcd -rbracks -mssp '1 cm' -loop

% Comment or adapt next line as needed (default is 20)
#(set-global-staff-size 11.416)


% Scheme function(s): "date & time"
% A set of functions to obtain a source file's modification time.

#(define comml           (object->string (command-line)))
#(define loc             (+ (string-rindex comml #\space ) 2))
#(define commllen        (- (string-length comml) 2))
#(define filen           (substring comml loc commllen))
#(define siz             (object->string (stat:size (stat filen))))
#(define ver             (object->string (lilypond-version)))
#(define dat             (strftime "%d/%m/%Y" (localtime (current-time))))
#(define tim             (strftime "%H:%M:%S" (localtime (current-time))))
#(define modTime         (stat:mtime (stat filen)))
#(define modTimeAsString (strftime "%d/%m/%Y - %H:%M:%S" (localtime modTime)))


\header {
  title              = "Nested part groups"
  subtitle           = ""
  miscellaneousField = ""
}

\paper {
  paper-height                   = 297\mm
  paper-width                    = 210\mm
  
  %left-margin                    = 0.0\mm
  %right-margin                   = 0.0\mm
  %top-margin                     = 0.0\mm
  %bottom-margin                  = 0.0\mm
  
  %horizontal-shift               = 0.0\mm
  %indent                         = 0.0\mm
  %short-indent                   = 0.0\mm
  
  markup-system-spacing.padding  = 1\cm
  %between-system-space           = 0.0\mm
  %page-top-space                 = 0.0\mm
  
  %page-count                     = -1
  %system-count                   = -1
  
  oddHeaderMarkup                = \markup {
    \fill-line {
      \on-the-fly \not-first-page {
        \fromproperty #'page:page-number-string
        " "
        \fromproperty #'header:title
        " "
        \fromproperty #'header:subtitle
      }
    }
  }

  evenHeaderMarkup               = \markup {
    \fill-line {
      \on-the-fly \not-first-page {
        \fromproperty #'page:page-number-string
        " "
        \fromproperty #'header:title
        " "
        \fromproperty #'header:subtitle
      }
    }
  }

  oddFooterMarkup                = \markup {
    \tiny
    \column {
      \fill-line {
        #(string-append
"Music generated from MusicXML by xml2ly 0.93 and engraved by LilyPond " (lilypond-version))
      }
      \fill-line { \italic { \modTimeAsString }}
    }
  }

  evenFooterMarkup               = ""
}

\layout {
  \context {
    \Score
    autoBeaming = ##f % to display tuplets brackets
  }
  \context {
    \Voice
  }
  \context {
    \Score
    % defaultBarType = #"!"
    startRepeatType = #"[|:"
    endRepeatType = #":|]"
    doubleRepeatType = #":|][|:"
  }
}

% Pick your choice from the next two lines as needed
myBreak = { \break }
%myBreak = { }

% Pick your choice from the next two lines as needed
myPageBreak = { \pageBreak }
%myPageBreak = { }

Part_POne_Staff_One_Voice_One = \absolute {
  \language "nederlands"
  \set Score.barNumberVisibility = #all-bar-numbers-visible
  \override Score.BarNumber.break-visibility = ##(#f #t #t)
  
  \key c \major
  \time 4/4
  
  \clef "treble"
  b'1 | % 2
  \barNumberCheck #2
  a'1 | % 3
  \barNumberCheck #3
  R%{1%}1
  \bar "|."
  \bar ".|:" | % 1
  \barNumberCheck #4
}

Part_PTwo_Staff_One_Voice_One = \absolute {
  \language "nederlands"
  \set Score.barNumberVisibility = #all-bar-numbers-visible
  \override Score.BarNumber.break-visibility = ##(#f #t #t)
  
  \key c \major
  \time 4/4
  
  \clef "treble"
  a'1 | % 2
  \barNumberCheck #2
  a'1 | % 3
  \barNumberCheck #3
  R%{1%}1
  \bar "|."
  \bar ".|:" | % 1
  \barNumberCheck #4
}

Part_PThree_Staff_One_Voice_One = \absolute {
  \language "nederlands"
  \set Score.barNumberVisibility = #all-bar-numbers-visible
  \override Score.BarNumber.break-visibility = ##(#f #t #t)
  
  \key c \major
  \time 4/4
  
  \clef "treble"
  a'1 | % 2
  \barNumberCheck #2
  a'1 | % 3
  \barNumberCheck #3
  R%{1%}1
  \bar "|."
  \bar ".|:" | % 1
  \barNumberCheck #4
}

Part_PFour_Staff_One_Voice_One = \absolute {
  \language "nederlands"
  \set Score.barNumberVisibility = #all-bar-numbers-visible
  \override Score.BarNumber.break-visibility = ##(#f #t #t)
  
  \key c \major
  \time 4/4
  
  \clef "treble"
  f'1 | % 2
  \barNumberCheck #2
  b'1 | % 3
  \barNumberCheck #3
  R%{1%}1
  \bar "|."
  \bar ".|:" | % 1
  \barNumberCheck #4
}

Part_PFive_Staff_One_Voice_One = \absolute {
  \language "nederlands"
  \set Score.barNumberVisibility = #all-bar-numbers-visible
  \override Score.BarNumber.break-visibility = ##(#f #t #t)
  
  \key c \major
  \time 4/4
  
  \clef "treble"
  d''1 | % 2
  \barNumberCheck #2
  f'1 | % 3
  \barNumberCheck #3
  R%{1%}1
  \bar "|."
  \bar ".|:" | % 4
  \barNumberCheck #4
}

\book {
  \header {
  }
  
  \score {
    <<
      \new StaffGroup
      \with {
      }
       <<
      
        \new Staff = "Part_POne_Staff_One"
        \with {
        }
        <<
          \context Voice = "Part_POne_Staff_One_Voice_One" <<
            \Part_POne_Staff_One_Voice_One
          >>
        >>
        \new StaffGroup
        \with {
          systemStartDelimiter = #'SystemStartBar
        }
         <<
        
          \new Staff = "Part_PTwo_Staff_One"
          \with {
          }
          <<
            \context Voice = "Part_PTwo_Staff_One_Voice_One" <<
              \Part_PTwo_Staff_One_Voice_One
            >>
          >>
          
          \new StaffGroup <<
          
            \new Staff = "Part_PThree_Staff_One"
            \with {
            }
            <<
              \context Voice = "Part_PThree_Staff_One_Voice_One" <<
                \Part_PThree_Staff_One_Voice_One
              >>
            >>
            
            \new Staff = "Part_PFour_Staff_One"
            \with {
            }
            <<
              \context Voice = "Part_PFour_Staff_One_Voice_One" <<
                \Part_PFour_Staff_One_Voice_One
              >>
            >>
            
          >>
          
        >>
        
        \new Staff = "Part_PFive_Staff_One"
        \with {
        }
        <<
          \context Voice = "Part_PFive_Staff_One_Voice_One" <<
            \Part_PFive_Staff_One_Voice_One
          >>
        >>
        
      >>
      
      
    >>
    
    \layout {
      \context {
        \Score
        autoBeaming = ##f % to display tuplets brackets
      }
      \context {
        \Voice
      }
      \context {
        \Score
        % defaultBarType = #"!"
        startRepeatType = #"[|:"
        endRepeatType = #":|]"
        doubleRepeatType = #":|][|:"
      }
    }
    
    \midi {
      \tempo 4 = 90
    }
  }
  
  \paper {
    paper-height                   = 297\mm
    paper-width                    = 210\mm
    
    %left-margin                    = 0.0\mm
    %right-margin                   = 0.0\mm
    %top-margin                     = 0.0\mm
    %bottom-margin                  = 0.0\mm
    
    %horizontal-shift               = 0.0\mm
    %indent                         = 0.0\mm
    %short-indent                   = 0.0\mm
    
    markup-system-spacing.padding  = 1\cm
    %between-system-space           = 0.0\mm
    %page-top-space                 = 0.0\mm
    
    %page-count                     = -1
    %system-count                   = -1
    
    oddHeaderMarkup                = \markup {
    \fill-line {
      \on-the-fly \not-first-page {
        \fromproperty #'page:page-number-string
        " "
        \fromproperty #'header:title
        " "
        \fromproperty #'header:subtitle
      }
    }
  }

    evenHeaderMarkup               = \markup {
    \fill-line {
      \on-the-fly \not-first-page {
        \fromproperty #'page:page-number-string
        " "
        \fromproperty #'header:title
        " "
        \fromproperty #'header:subtitle
      }
    }
  }

    oddFooterMarkup                = \markup {
    \tiny
    \column {
      \fill-line {
        #(string-append
"Music generated from MusicXML by xml2ly 0.93 and engraved by LilyPond " (lilypond-version))
      }
      \fill-line { \italic { \modTimeAsString }}
    }
  }

    evenFooterMarkup               = ""
  }
  
}
