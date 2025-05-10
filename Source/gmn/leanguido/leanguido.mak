# Microsoft Developer Studio Generated NMAKE File, Based on leanguido.dsp
!IF "$(CFG)" == ""
CFG=leanguido - Win32 Debug
!MESSAGE Keine Konfiguration angegeben. leanguido - Win32 Debug wird als Standard verwendet.
!ENDIF 

!IF "$(CFG)" != "leanguido - Win32 Release" && "$(CFG)" != "leanguido - Win32 Debug"
!MESSAGE UngÅltige Konfiguration "$(CFG)" angegeben.
!MESSAGE Sie kînnen beim AusfÅhren von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "leanguido.mak" CFG="leanguido - Win32 Debug"
!MESSAGE 
!MESSAGE FÅr die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "leanguido - Win32 Release" (basierend auf  "Win32 (x86) Console Application")
!MESSAGE "leanguido - Win32 Debug" (basierend auf  "Win32 (x86) Console Application")
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

!IF  "$(CFG)" == "leanguido - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\leanguido.exe"


CLEAN :
	-@erase "$(INTDIR)\duration.obj"
	-@erase "$(INTDIR)\gmntools.obj"
	-@erase "$(INTDIR)\guido.obj"
	-@erase "$(INTDIR)\lgchord.obj"
	-@erase "$(INTDIR)\lgempty.obj"
	-@erase "$(INTDIR)\lgevent.obj"
	-@erase "$(INTDIR)\lgfactory.obj"
	-@erase "$(INTDIR)\lgguido.obj"
	-@erase "$(INTDIR)\lgmain.obj"
	-@erase "$(INTDIR)\lgnote.obj"
	-@erase "$(INTDIR)\lgobject.obj"
	-@erase "$(INTDIR)\lgrest.obj"
	-@erase "$(INTDIR)\lgsegment.obj"
	-@erase "$(INTDIR)\lgsequence.obj"
	-@erase "$(INTDIR)\lgtag.obj"
	-@erase "$(INTDIR)\lgtagarg.obj"
	-@erase "$(INTDIR)\lgVisitor.obj"
	-@erase "$(INTDIR)\lgvoice.obj"
	-@erase "$(INTDIR)\strlist.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\leanguido.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /ML /W3 /GR /GX /O2 /I "..\parser-kit" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\leanguido.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\leanguido.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\leanguido.pdb" /machine:I386 /out:"$(OUTDIR)\leanguido.exe" 
LINK32_OBJS= \
	"$(INTDIR)\duration.obj" \
	"$(INTDIR)\gmntools.obj" \
	"$(INTDIR)\guido.obj" \
	"$(INTDIR)\lgchord.obj" \
	"$(INTDIR)\lgempty.obj" \
	"$(INTDIR)\lgevent.obj" \
	"$(INTDIR)\lgfactory.obj" \
	"$(INTDIR)\lgguido.obj" \
	"$(INTDIR)\lgmain.obj" \
	"$(INTDIR)\lgnote.obj" \
	"$(INTDIR)\lgobject.obj" \
	"$(INTDIR)\lgrest.obj" \
	"$(INTDIR)\lgsegment.obj" \
	"$(INTDIR)\lgsequence.obj" \
	"$(INTDIR)\lgtag.obj" \
	"$(INTDIR)\lgtagarg.obj" \
	"$(INTDIR)\lgVisitor.obj" \
	"$(INTDIR)\lgvoice.obj" \
	"$(INTDIR)\strlist.obj"

"$(OUTDIR)\leanguido.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "leanguido - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\leanguido.exe" "$(OUTDIR)\leanguido.bsc"


CLEAN :
	-@erase "$(INTDIR)\duration.obj"
	-@erase "$(INTDIR)\duration.sbr"
	-@erase "$(INTDIR)\gmntools.obj"
	-@erase "$(INTDIR)\gmntools.sbr"
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
	-@erase "$(INTDIR)\lgmain.obj"
	-@erase "$(INTDIR)\lgmain.sbr"
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
	-@erase "$(INTDIR)\strlist.obj"
	-@erase "$(INTDIR)\strlist.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\leanguido.bsc"
	-@erase "$(OUTDIR)\leanguido.exe"
	-@erase "$(OUTDIR)\leanguido.ilk"
	-@erase "$(OUTDIR)\leanguido.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MLd /W3 /Gm /GR /GX /ZI /Od /I "..\parser-kit" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\leanguido.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\duration.sbr" \
	"$(INTDIR)\gmntools.sbr" \
	"$(INTDIR)\guido.sbr" \
	"$(INTDIR)\lgchord.sbr" \
	"$(INTDIR)\lgempty.sbr" \
	"$(INTDIR)\lgevent.sbr" \
	"$(INTDIR)\lgfactory.sbr" \
	"$(INTDIR)\lgguido.sbr" \
	"$(INTDIR)\lgmain.sbr" \
	"$(INTDIR)\lgnote.sbr" \
	"$(INTDIR)\lgobject.sbr" \
	"$(INTDIR)\lgrest.sbr" \
	"$(INTDIR)\lgsegment.sbr" \
	"$(INTDIR)\lgsequence.sbr" \
	"$(INTDIR)\lgtag.sbr" \
	"$(INTDIR)\lgtagarg.sbr" \
	"$(INTDIR)\lgVisitor.sbr" \
	"$(INTDIR)\lgvoice.sbr" \
	"$(INTDIR)\strlist.sbr"

"$(OUTDIR)\leanguido.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\leanguido.pdb" /debug /machine:I386 /out:"$(OUTDIR)\leanguido.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\duration.obj" \
	"$(INTDIR)\gmntools.obj" \
	"$(INTDIR)\guido.obj" \
	"$(INTDIR)\lgchord.obj" \
	"$(INTDIR)\lgempty.obj" \
	"$(INTDIR)\lgevent.obj" \
	"$(INTDIR)\lgfactory.obj" \
	"$(INTDIR)\lgguido.obj" \
	"$(INTDIR)\lgmain.obj" \
	"$(INTDIR)\lgnote.obj" \
	"$(INTDIR)\lgobject.obj" \
	"$(INTDIR)\lgrest.obj" \
	"$(INTDIR)\lgsegment.obj" \
	"$(INTDIR)\lgsequence.obj" \
	"$(INTDIR)\lgtag.obj" \
	"$(INTDIR)\lgtagarg.obj" \
	"$(INTDIR)\lgVisitor.obj" \
	"$(INTDIR)\lgvoice.obj" \
	"$(INTDIR)\strlist.obj"

"$(OUTDIR)\leanguido.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("leanguido.dep")
!INCLUDE "leanguido.dep"
!ELSE 
!MESSAGE Warning: cannot find "leanguido.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "leanguido - Win32 Release" || "$(CFG)" == "leanguido - Win32 Debug"
SOURCE=.\duration.cpp

!IF  "$(CFG)" == "leanguido - Win32 Release"

CPP_SWITCHES=/nologo /ML /W3 /GR /GX /O2 /I "..\parser-kit" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\leanguido.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\duration.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "leanguido - Win32 Debug"

CPP_SWITCHES=/nologo /MLd /W3 /Gm /GR /GX /ZI /Od /I "..\parser-kit" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\duration.obj"	"$(INTDIR)\duration.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE="..\parser-kit\gmntools.c"

!IF  "$(CFG)" == "leanguido - Win32 Release"


"$(INTDIR)\gmntools.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "leanguido - Win32 Debug"


"$(INTDIR)\gmntools.obj"	"$(INTDIR)\gmntools.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE="..\parser-kit\guido.cpp"

!IF  "$(CFG)" == "leanguido - Win32 Release"


"$(INTDIR)\guido.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "leanguido - Win32 Debug"


"$(INTDIR)\guido.obj"	"$(INTDIR)\guido.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\lgchord.cpp

!IF  "$(CFG)" == "leanguido - Win32 Release"


"$(INTDIR)\lgchord.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "leanguido - Win32 Debug"


"$(INTDIR)\lgchord.obj"	"$(INTDIR)\lgchord.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\lgempty.cpp

!IF  "$(CFG)" == "leanguido - Win32 Release"


"$(INTDIR)\lgempty.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "leanguido - Win32 Debug"


"$(INTDIR)\lgempty.obj"	"$(INTDIR)\lgempty.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\lgevent.cpp

!IF  "$(CFG)" == "leanguido - Win32 Release"


"$(INTDIR)\lgevent.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "leanguido - Win32 Debug"


"$(INTDIR)\lgevent.obj"	"$(INTDIR)\lgevent.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\lgfactory.cpp

!IF  "$(CFG)" == "leanguido - Win32 Release"


"$(INTDIR)\lgfactory.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "leanguido - Win32 Debug"


"$(INTDIR)\lgfactory.obj"	"$(INTDIR)\lgfactory.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\lgguido.cpp

!IF  "$(CFG)" == "leanguido - Win32 Release"


"$(INTDIR)\lgguido.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "leanguido - Win32 Debug"


"$(INTDIR)\lgguido.obj"	"$(INTDIR)\lgguido.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\lgmain.cpp

!IF  "$(CFG)" == "leanguido - Win32 Release"


"$(INTDIR)\lgmain.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "leanguido - Win32 Debug"


"$(INTDIR)\lgmain.obj"	"$(INTDIR)\lgmain.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\lgnote.cpp

!IF  "$(CFG)" == "leanguido - Win32 Release"


"$(INTDIR)\lgnote.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "leanguido - Win32 Debug"


"$(INTDIR)\lgnote.obj"	"$(INTDIR)\lgnote.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\lgobject.cpp

!IF  "$(CFG)" == "leanguido - Win32 Release"


"$(INTDIR)\lgobject.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "leanguido - Win32 Debug"


"$(INTDIR)\lgobject.obj"	"$(INTDIR)\lgobject.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\lgrest.cpp

!IF  "$(CFG)" == "leanguido - Win32 Release"


"$(INTDIR)\lgrest.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "leanguido - Win32 Debug"


"$(INTDIR)\lgrest.obj"	"$(INTDIR)\lgrest.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\lgsegment.cpp

!IF  "$(CFG)" == "leanguido - Win32 Release"


"$(INTDIR)\lgsegment.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "leanguido - Win32 Debug"


"$(INTDIR)\lgsegment.obj"	"$(INTDIR)\lgsegment.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\lgsequence.cpp

!IF  "$(CFG)" == "leanguido - Win32 Release"


"$(INTDIR)\lgsequence.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "leanguido - Win32 Debug"


"$(INTDIR)\lgsequence.obj"	"$(INTDIR)\lgsequence.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\lgtag.cpp

!IF  "$(CFG)" == "leanguido - Win32 Release"


"$(INTDIR)\lgtag.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "leanguido - Win32 Debug"


"$(INTDIR)\lgtag.obj"	"$(INTDIR)\lgtag.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\lgtagarg.cpp

!IF  "$(CFG)" == "leanguido - Win32 Release"


"$(INTDIR)\lgtagarg.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "leanguido - Win32 Debug"


"$(INTDIR)\lgtagarg.obj"	"$(INTDIR)\lgtagarg.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\lgVisitor.cpp

!IF  "$(CFG)" == "leanguido - Win32 Release"


"$(INTDIR)\lgVisitor.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "leanguido - Win32 Debug"


"$(INTDIR)\lgVisitor.obj"	"$(INTDIR)\lgVisitor.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\lgvoice.cpp

!IF  "$(CFG)" == "leanguido - Win32 Release"


"$(INTDIR)\lgvoice.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "leanguido - Win32 Debug"


"$(INTDIR)\lgvoice.obj"	"$(INTDIR)\lgvoice.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE="..\parser-kit\strlist.c"

!IF  "$(CFG)" == "leanguido - Win32 Release"


"$(INTDIR)\strlist.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "leanguido - Win32 Debug"


"$(INTDIR)\strlist.obj"	"$(INTDIR)\strlist.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 


!ENDIF 

