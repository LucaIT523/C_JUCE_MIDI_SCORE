# Microsoft Developer Studio Generated NMAKE File, Based on musicxml2gmn.dsp
!IF "$(CFG)" == ""
CFG=musicxml2gmn - Win32 Debug
!MESSAGE Keine Konfiguration angegeben. musicxml2gmn - Win32 Debug wird als Standard verwendet.
!ENDIF 

!IF "$(CFG)" != "musicxml2gmn - Win32 Release" && "$(CFG)" != "musicxml2gmn - Win32 Debug"
!MESSAGE UngÅltige Konfiguration "$(CFG)" angegeben.
!MESSAGE Sie kînnen beim AusfÅhren von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "musicxml2gmn.mak" CFG="musicxml2gmn - Win32 Debug"
!MESSAGE 
!MESSAGE FÅr die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "musicxml2gmn - Win32 Release" (basierend auf  "Win32 (x86) Console Application")
!MESSAGE "musicxml2gmn - Win32 Debug" (basierend auf  "Win32 (x86) Console Application")
!MESSAGE 
!ERROR Eine ungÅltige Konfiguration wurde angegeben.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "musicxml2gmn - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\musicxml2gmn.exe"


CLEAN :
	-@erase "$(INTDIR)\duration.obj"
	-@erase "$(INTDIR)\guido.obj"
	-@erase "$(INTDIR)\lgchord.obj"
	-@erase "$(INTDIR)\lgempty.obj"
	-@erase "$(INTDIR)\lgevent.obj"
	-@erase "$(INTDIR)\lgfactory.obj"
	-@erase "$(INTDIR)\lgguido.obj"
	-@erase "$(INTDIR)\lgnote.obj"
	-@erase "$(INTDIR)\lgobject.obj"
	-@erase "$(INTDIR)\lgrest.obj"
	-@erase "$(INTDIR)\lgsegment.obj"
	-@erase "$(INTDIR)\lgsequence.obj"
	-@erase "$(INTDIR)\lgtag.obj"
	-@erase "$(INTDIR)\lgtagarg.obj"
	-@erase "$(INTDIR)\lgVisitor.obj"
	-@erase "$(INTDIR)\lgvoice.obj"
	-@erase "$(INTDIR)\musicxml2gmn.obj"
	-@erase "$(INTDIR)\strlist.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\musicxml2gmn.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /ML /W3 /GR /GX /O2 /I "..\..\parser-kit" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\musicxml2gmn.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\musicxml2gmn.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\musicxml2gmn.pdb" /machine:I386 /out:"$(OUTDIR)\musicxml2gmn.exe" 
LINK32_OBJS= \
	"$(INTDIR)\duration.obj" \
	"$(INTDIR)\guido.obj" \
	"$(INTDIR)\lgchord.obj" \
	"$(INTDIR)\lgempty.obj" \
	"$(INTDIR)\lgevent.obj" \
	"$(INTDIR)\lgfactory.obj" \
	"$(INTDIR)\lgguido.obj" \
	"$(INTDIR)\lgnote.obj" \
	"$(INTDIR)\lgobject.obj" \
	"$(INTDIR)\lgrest.obj" \
	"$(INTDIR)\lgsegment.obj" \
	"$(INTDIR)\lgsequence.obj" \
	"$(INTDIR)\lgtag.obj" \
	"$(INTDIR)\lgtagarg.obj" \
	"$(INTDIR)\lgVisitor.obj" \
	"$(INTDIR)\lgvoice.obj" \
	"$(INTDIR)\musicxml2gmn.obj" \
	"$(INTDIR)\strlist.obj" \
	"..\..\guidoxml\xml2guido\EXPAT_WIN32BIN_1_95_1\LIB\DYNAMISCH\xmlparse.lib"

"$(OUTDIR)\musicxml2gmn.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "musicxml2gmn - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\musicxml2gmn.exe" "$(OUTDIR)\musicxml2gmn.bsc"


CLEAN :
	-@erase "$(INTDIR)\duration.obj"
	-@erase "$(INTDIR)\duration.sbr"
	-@erase "$(INTDIR)\guido.obj"
	-@erase "$(INTDIR)\guido.sbr"
	-@erase "$(INTDIR)\lgchord.obj"
	-@erase "$(INTDIR)\lgchord.sbr"
	-@erase "$(INTDIR)\lgempty.obj"
	-@erase "$(INTDIR)\lgempty.sbr"
	-@erase "$(INTDIR)\lgevent.obj"
	-@erase "$(INTDIR)\lgevent.sbr"
	-@erase "$(INTDIR)\lgfactory.obj"
	-@erase "$(INTDIR)\lgfactory.sbr"
	-@erase "$(INTDIR)\lgguido.obj"
	-@erase "$(INTDIR)\lgguido.sbr"
	-@erase "$(INTDIR)\lgnote.obj"
	-@erase "$(INTDIR)\lgnote.sbr"
	-@erase "$(INTDIR)\lgobject.obj"
	-@erase "$(INTDIR)\lgobject.sbr"
	-@erase "$(INTDIR)\lgrest.obj"
	-@erase "$(INTDIR)\lgrest.sbr"
	-@erase "$(INTDIR)\lgsegment.obj"
	-@erase "$(INTDIR)\lgsegment.sbr"
	-@erase "$(INTDIR)\lgsequence.obj"
	-@erase "$(INTDIR)\lgsequence.sbr"
	-@erase "$(INTDIR)\lgtag.obj"
	-@erase "$(INTDIR)\lgtag.sbr"
	-@erase "$(INTDIR)\lgtagarg.obj"
	-@erase "$(INTDIR)\lgtagarg.sbr"
	-@erase "$(INTDIR)\lgVisitor.obj"
	-@erase "$(INTDIR)\lgVisitor.sbr"
	-@erase "$(INTDIR)\lgvoice.obj"
	-@erase "$(INTDIR)\lgvoice.sbr"
	-@erase "$(INTDIR)\musicxml2gmn.obj"
	-@erase "$(INTDIR)\musicxml2gmn.sbr"
	-@erase "$(INTDIR)\strlist.obj"
	-@erase "$(INTDIR)\strlist.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\musicxml2gmn.bsc"
	-@erase "$(OUTDIR)\musicxml2gmn.exe"
	-@erase "$(OUTDIR)\musicxml2gmn.ilk"
	-@erase "$(OUTDIR)\musicxml2gmn.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MLd /W3 /Gm /GR /GX /ZI /Od /I "..\..\parser-kit" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\musicxml2gmn.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\musicxml2gmn.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\duration.sbr" \
	"$(INTDIR)\guido.sbr" \
	"$(INTDIR)\lgchord.sbr" \
	"$(INTDIR)\lgempty.sbr" \
	"$(INTDIR)\lgevent.sbr" \
	"$(INTDIR)\lgfactory.sbr" \
	"$(INTDIR)\lgguido.sbr" \
	"$(INTDIR)\lgnote.sbr" \
	"$(INTDIR)\lgobject.sbr" \
	"$(INTDIR)\lgrest.sbr" \
	"$(INTDIR)\lgsegment.sbr" \
	"$(INTDIR)\lgsequence.sbr" \
	"$(INTDIR)\lgtag.sbr" \
	"$(INTDIR)\lgtagarg.sbr" \
	"$(INTDIR)\lgVisitor.sbr" \
	"$(INTDIR)\lgvoice.sbr" \
	"$(INTDIR)\musicxml2gmn.sbr" \
	"$(INTDIR)\strlist.sbr"

"$(OUTDIR)\musicxml2gmn.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\musicxml2gmn.pdb" /debug /machine:I386 /out:"$(OUTDIR)\musicxml2gmn.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\duration.obj" \
	"$(INTDIR)\guido.obj" \
	"$(INTDIR)\lgchord.obj" \
	"$(INTDIR)\lgempty.obj" \
	"$(INTDIR)\lgevent.obj" \
	"$(INTDIR)\lgfactory.obj" \
	"$(INTDIR)\lgguido.obj" \
	"$(INTDIR)\lgnote.obj" \
	"$(INTDIR)\lgobject.obj" \
	"$(INTDIR)\lgrest.obj" \
	"$(INTDIR)\lgsegment.obj" \
	"$(INTDIR)\lgsequence.obj" \
	"$(INTDIR)\lgtag.obj" \
	"$(INTDIR)\lgtagarg.obj" \
	"$(INTDIR)\lgVisitor.obj" \
	"$(INTDIR)\lgvoice.obj" \
	"$(INTDIR)\musicxml2gmn.obj" \
	"$(INTDIR)\strlist.obj" \
	"..\..\guidoxml\xml2guido\EXPAT_WIN32BIN_1_95_1\LIB\DYNAMISCH\xmlparse.lib"

"$(OUTDIR)\musicxml2gmn.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("musicxml2gmn.dep")
!INCLUDE "musicxml2gmn.dep"
!ELSE 
!MESSAGE Warning: cannot find "musicxml2gmn.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "musicxml2gmn - Win32 Release" || "$(CFG)" == "musicxml2gmn - Win32 Debug"
SOURCE=..\duration.cpp

!IF  "$(CFG)" == "musicxml2gmn - Win32 Release"


"$(INTDIR)\duration.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "musicxml2gmn - Win32 Debug"


"$(INTDIR)\duration.obj"	"$(INTDIR)\duration.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE="..\..\parser-kit\guido.cpp"

!IF  "$(CFG)" == "musicxml2gmn - Win32 Release"


"$(INTDIR)\guido.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "musicxml2gmn - Win32 Debug"


"$(INTDIR)\guido.obj"	"$(INTDIR)\guido.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\lgchord.cpp

!IF  "$(CFG)" == "musicxml2gmn - Win32 Release"


"$(INTDIR)\lgchord.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "musicxml2gmn - Win32 Debug"


"$(INTDIR)\lgchord.obj"	"$(INTDIR)\lgchord.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\lgempty.cpp

!IF  "$(CFG)" == "musicxml2gmn - Win32 Release"


"$(INTDIR)\lgempty.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "musicxml2gmn - Win32 Debug"


"$(INTDIR)\lgempty.obj"	"$(INTDIR)\lgempty.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\lgevent.cpp

!IF  "$(CFG)" == "musicxml2gmn - Win32 Release"


"$(INTDIR)\lgevent.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "musicxml2gmn - Win32 Debug"


"$(INTDIR)\lgevent.obj"	"$(INTDIR)\lgevent.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\lgfactory.cpp

!IF  "$(CFG)" == "musicxml2gmn - Win32 Release"


"$(INTDIR)\lgfactory.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "musicxml2gmn - Win32 Debug"


"$(INTDIR)\lgfactory.obj"	"$(INTDIR)\lgfactory.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\lgguido.cpp

!IF  "$(CFG)" == "musicxml2gmn - Win32 Release"


"$(INTDIR)\lgguido.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "musicxml2gmn - Win32 Debug"


"$(INTDIR)\lgguido.obj"	"$(INTDIR)\lgguido.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\lgnote.cpp

!IF  "$(CFG)" == "musicxml2gmn - Win32 Release"


"$(INTDIR)\lgnote.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "musicxml2gmn - Win32 Debug"


"$(INTDIR)\lgnote.obj"	"$(INTDIR)\lgnote.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\lgobject.cpp

!IF  "$(CFG)" == "musicxml2gmn - Win32 Release"


"$(INTDIR)\lgobject.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "musicxml2gmn - Win32 Debug"


"$(INTDIR)\lgobject.obj"	"$(INTDIR)\lgobject.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\lgrest.cpp

!IF  "$(CFG)" == "musicxml2gmn - Win32 Release"


"$(INTDIR)\lgrest.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "musicxml2gmn - Win32 Debug"


"$(INTDIR)\lgrest.obj"	"$(INTDIR)\lgrest.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\lgsegment.cpp

!IF  "$(CFG)" == "musicxml2gmn - Win32 Release"


"$(INTDIR)\lgsegment.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "musicxml2gmn - Win32 Debug"


"$(INTDIR)\lgsegment.obj"	"$(INTDIR)\lgsegment.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\lgsequence.cpp

!IF  "$(CFG)" == "musicxml2gmn - Win32 Release"


"$(INTDIR)\lgsequence.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "musicxml2gmn - Win32 Debug"


"$(INTDIR)\lgsequence.obj"	"$(INTDIR)\lgsequence.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\lgtag.cpp

!IF  "$(CFG)" == "musicxml2gmn - Win32 Release"


"$(INTDIR)\lgtag.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "musicxml2gmn - Win32 Debug"


"$(INTDIR)\lgtag.obj"	"$(INTDIR)\lgtag.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\lgtagarg.cpp

!IF  "$(CFG)" == "musicxml2gmn - Win32 Release"


"$(INTDIR)\lgtagarg.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "musicxml2gmn - Win32 Debug"


"$(INTDIR)\lgtagarg.obj"	"$(INTDIR)\lgtagarg.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\lgVisitor.cpp

!IF  "$(CFG)" == "musicxml2gmn - Win32 Release"


"$(INTDIR)\lgVisitor.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "musicxml2gmn - Win32 Debug"


"$(INTDIR)\lgVisitor.obj"	"$(INTDIR)\lgVisitor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\lgvoice.cpp

!IF  "$(CFG)" == "musicxml2gmn - Win32 Release"


"$(INTDIR)\lgvoice.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "musicxml2gmn - Win32 Debug"


"$(INTDIR)\lgvoice.obj"	"$(INTDIR)\lgvoice.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\musicxml2gmn.cpp

!IF  "$(CFG)" == "musicxml2gmn - Win32 Release"


"$(INTDIR)\musicxml2gmn.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "musicxml2gmn - Win32 Debug"


"$(INTDIR)\musicxml2gmn.obj"	"$(INTDIR)\musicxml2gmn.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE="..\..\parser-kit\strlist.c"

!IF  "$(CFG)" == "musicxml2gmn - Win32 Release"


"$(INTDIR)\strlist.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "musicxml2gmn - Win32 Debug"


"$(INTDIR)\strlist.obj"	"$(INTDIR)\strlist.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 


!ENDIF 

