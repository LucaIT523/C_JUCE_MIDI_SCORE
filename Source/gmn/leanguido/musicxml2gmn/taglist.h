#ifndef TAGLIST_H
#define TAGLIST_H

//
// The xml->guido tag definitions.
//
// The following order is required:
//
// xml-tag:behavior:guido-tag:param1:param2:....:paramX
//
// note: - between every tag in order must be a ':'
//       - at the end of the list there must be a "#"
//       - behaviour is 'b', 'u' or 'a'. 'b' means, that this is a bracketed tag
//         in GUIDO, 'u' is an unbracketed tag and 'a' is either a bracketed or
//         an unbracketed tag, e.g. fermata —”≥§º«∫≈£¨Õ£¡Ùº«∫≈must be set with 'a', because it
//         could be bracketed (if notes or tags are following) or not.
//       - param1 to paramX define the filtered parameters. Here you must place the
//         XML-Definition of those parameters, which you want to be written directly into
//         the GMN-File, all other parameters will be written as following: paramX = value of paramX
//       - if you have changed this file be sure to do first a 'make clean'
//         on unix systems and 'Rebuild All' on Visual C++.
//

//
// 
//
static const char* const tags[] = { 
//dynamic markings
"intens:u:intens:s:r:i",
"i:u:i:s:r:i",
"cresc:b:cresc:s:r",
"crescBegin:u:crescBegin",
"crescEnd:u:crescEnd:s:r",
"dim:b:dim:s:r",
"dimBegin:u:dimBegin",
"dimEnd:u:dimEnd:s:r",

//tempo indications
"tempo:u:tempo:s:s2",
"accel:b:accel:s:s2",
"accelBegin:u:accelBegin",
"accelEnd:u:accelEnd:s:s2",
"rit:b:rit:s:s2",
"ritBegin:u:ritBegin",
"ritEnd:u:ritEnd:s:s2",

//instruments
"instr:u:instr:s:s2",

//ties
"tie:b:tie",
"tieBegin:u:tieBegin",
"tieEnd:u:tieEnd",

//slurs
"slur:b:slur:dy:dx1:dy1:dx2:dy2:dx3:dy3",
"sl:b:sl:dy:dx1:dy1:dx2:dy2:dx3:dy3",
"slurBegin:u:slurBegin:dy:dx1:dy1:dx2:dy2:dx3:dy3",
"slurEnd:u:slurEnd",

//accents and expressive Markings
"stacc:b:stacc",
"accent:b:accent",
"ten:b:ten",
"marcato:b:marcato",

//trills ans ornaments
"trill:b:trill:i",
"mord:b:mord:i",
"turn:b:turn:i",

//tremolo
"trem:b:trem:i",

//fermata
"fermata:a:fermata",

//grace notes and cue notes
"grace:b:grace:i",
"cue:b:cue:s",

//repititiond
"repeatBegin:u:repeatBegin:i",
"repeatEnd:a:repeatEnd:i",

//clefs
"clef:u:clef:s",

//meter - time signatures
"meter:u:meter:s",

//key - key signatures
"key:u:key:s:i",

//octavia traspositions
"oct:a:oct:i",

//staves
"staff:u:staff:i:str",

//beams
"beamsAuto:u:beamsAuto",
"beamsOff:u:beamsOff",
"beam:b:beam:dy:dy1:dy2",
"bm:b:bm:dy:dy1:dy2",

//stems
"stemsAuto:u:stemsAuto",
"stemsUp:u:stemsUp",
"stemsDown:u:stemsDown",

//barlines and tactus strokes
"bar:u:bar:i",
"doubleBar:u:doubleBar:i",
"tactus:u:tactus",

//text
"text:u:text:s:dy",
"t:u:t:s:dy",

//title and composer
"title:u:title:s:",
"composer:u:composer:s",

// markers and named sections
"mark:u:mark:s:",
"label:a:label:s",

//end
"#"
};

#endif
