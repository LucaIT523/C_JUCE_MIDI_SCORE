# Microsoft Developer Studio Generated NMAKE File, Based on gmn2musicxml.dsp
!IF "$(CFG)" == ""
CFG=gmn2musicxml - Win32 Debug
!MESSAGE Keine Konfiguration angegeben. gmn2musicxml - Win32 Debug wird als Standard verwendet.
!ENDIF 

!IF "$(CFG)" != "gmn2musicxml - Win32 Release" && "$(CFG)" != "gmn2musicxml - Win32 Debug"
!MESSAGE UngÅltige Konfiguration "$(CFG)" angegeben.
!MESSAGE Sie kînnen beim AusfÅhren von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "gmn2musicxml.mak" CFG="gmn2musicxml - Win32 Debug"
!MESSAGE 
!MESSAGE FÅr die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "gmn2musicxml - Win32 Release" (basierend auf  "Win32 (x86) Console Application")
!MESSAGE "gmn2musicxml - Win32 Debug" (basierend auf  "Win32 (x86) Console Application")
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

!IF  "$(CFG)" == "gmn2musicxml - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\gmn2musicxml.exe"


CLEAN :
	-@erase "$(INTDIR)\duration.obj"
	-@erase "$(INTDIR)\gmn2musicxml.obj"
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
	-@erase "$(INTDIR)\lgXMLfactory.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\strlist.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\gmn2musicxml.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /ML /W3 /GR /GX /O2 /I "..\..\parser-kit" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\gmn2musicxml.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\gmn2musicxml.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\gmn2musicxml.pdb" /machine:I386 /out:"$(OUTDIR)\gmn2musicxml.exe" 
LINK32_OBJS= \
	"$(INTDIR)\duration.obj" \
	"$(INTDIR)\gmn2musicxml.obj" \
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
	"$(INTDIR)\lgXMLfactory.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\strlist.obj"

"$(OUTDIR)\gmn2musicxml.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "gmn2musicxml - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\gmn2musicxml.exe"


CLEAN :
	-@erase "$(INTDIR)\duration.obj"
	-@erase "$(INTDIR)\gmn2musicxml.obj"
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
	-@erase "$(INTDIR)\lgXMLfactory.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\strlist.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\gmn2musicxml.exe"
	-@erase "$(OUTDIR)\gmn2musicxml.ilk"
	-@erase "$(OUTDIR)\gmn2musicxml.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MLd /W3 /Gm /GR /GX /ZI /Od /I "..\..\parser-kit" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\gmn2musicxml.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\gmn2musicxml.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\gmn2musicxml.pdb" /debug /machine:I386 /out:"$(OUTDIR)\gmn2musicxml.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\duration.obj" \
	"$(INTDIR)\gmn2musicxml.obj" \
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
	"$(INTDIR)\lgXMLfactory.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\strlist.obj"

"$(OUTDIR)\gmn2musicxml.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("gmn2musicxml.dep")
!INCLUDE "gmn2musicxml.dep"
!ELSE 
!MESSAGE Warning: cannot find "gmn2musicxml.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "gmn2musicxml - Win32 Release" || "$(CFG)" == "gmn2musicxml - Win32 Debug"
SOURCE=..\duration.cpp

"$(INTDIR)\duration.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\gmn2musicxml.cpp

"$(INTDIR)\gmn2musicxml.obj" : $(SOURCE) "$(INTDIR)"


SOURCE="..\..\parser-kit\guido.cpp"

"$(INTDIR)\guido.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\lgchord.cpp

"$(INTDIR)\lgchord.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\lgempty.cpp

"$(INTDIR)\lgempty.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\lgevent.cpp

"$(INTDIR)\lgevent.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\lgfactory.cpp

"$(INTDIR)\lgfactory.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\lgguido.cpp

"$(INTDIR)\lgguido.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\lgnote.cpp

"$(INTDIR)\lgnote.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\lgobject.cpp

"$(INTDIR)\lgobject.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\lgrest.cpp

"$(INTDIR)\lgrest.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\lgsegment.cpp

"$(INTDIR)\lgsegment.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\lgsequence.cpp

"$(INTDIR)\lgsequence.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\lgtag.cpp

"$(INTDIR)\lgtag.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\lgtagarg.cpp

"$(INTDIR)\lgtagarg.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\lgVisitor.cpp

"$(INTDIR)\lgVisitor.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\lgvoice.cpp

"$(INTDIR)\lgvoice.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\lgXMLfactory.cpp

"$(INTDIR)\lgXMLfactory.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\main.cpp

"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)"


SOURCE="..\..\parser-kit\strlist.c"

"$(INTDIR)\strlist.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

