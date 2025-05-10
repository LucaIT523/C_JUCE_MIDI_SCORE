# Microsoft Developer Studio Project File - Name="gmn2musicxml" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=gmn2musicxml - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "gmn2musicxml.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "gmn2musicxml.mak" CFG="gmn2musicxml - Win32 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "gmn2musicxml - Win32 Release" (basierend auf  "Win32 (x86) Console Application")
!MESSAGE "gmn2musicxml - Win32 Debug" (basierend auf  "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "gmn2musicxml - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GR /GX /O2 /I "..\..\parser-kit" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "gmn2musicxml - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GR /GX /ZI /Od /I "..\..\parser-kit" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "gmn2musicxml - Win32 Release"
# Name "gmn2musicxml - Win32 Debug"
# Begin Group "Quellcodedateien"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\duration.cpp
# End Source File
# Begin Source File

SOURCE=.\gmn2musicxml.cpp
# End Source File
# Begin Source File

SOURCE="..\..\parser-kit\guido.cpp"
# End Source File
# Begin Source File

SOURCE=..\lgchord.cpp
# End Source File
# Begin Source File

SOURCE=..\lgempty.cpp
# End Source File
# Begin Source File

SOURCE=..\lgevent.cpp
# End Source File
# Begin Source File

SOURCE=..\lgfactory.cpp
# End Source File
# Begin Source File

SOURCE=..\lgguido.cpp
# End Source File
# Begin Source File

SOURCE=..\lgnote.cpp
# End Source File
# Begin Source File

SOURCE=..\lgobject.cpp
# End Source File
# Begin Source File

SOURCE=..\lgrest.cpp
# End Source File
# Begin Source File

SOURCE=..\lgsegment.cpp
# End Source File
# Begin Source File

SOURCE=..\lgsequence.cpp
# End Source File
# Begin Source File

SOURCE=..\lgtag.cpp
# End Source File
# Begin Source File

SOURCE=..\lgtagarg.cpp
# End Source File
# Begin Source File

SOURCE=..\lgVisitor.cpp
# End Source File
# Begin Source File

SOURCE=..\lgvoice.cpp
# End Source File
# Begin Source File

SOURCE=.\lgXMLfactory.cpp
# End Source File
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# Begin Source File

SOURCE="..\..\parser-kit\strlist.c"
# End Source File
# End Group
# Begin Group "Header-Dateien"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\duration.h
# End Source File
# Begin Source File

SOURCE=.\gmn2musicxml.h
# End Source File
# Begin Source File

SOURCE="..\..\parser-kit\guido.h"
# End Source File
# Begin Source File

SOURCE=..\lgchord.h
# End Source File
# Begin Source File

SOURCE=..\lgempty.h
# End Source File
# Begin Source File

SOURCE=..\lgevent.h
# End Source File
# Begin Source File

SOURCE=..\lgfactory.h
# End Source File
# Begin Source File

SOURCE=..\lgnote.h
# End Source File
# Begin Source File

SOURCE=..\lgobject.h
# End Source File
# Begin Source File

SOURCE=..\lgrest.h
# End Source File
# Begin Source File

SOURCE=..\lgsegment.h
# End Source File
# Begin Source File

SOURCE=..\lgsequence.h
# End Source File
# Begin Source File

SOURCE=..\lgtag.h
# End Source File
# Begin Source File

SOURCE=..\lgtagarg.h
# End Source File
# Begin Source File

SOURCE=..\lgVisitor.h
# End Source File
# Begin Source File

SOURCE=..\lgvoice.h
# End Source File
# Begin Source File

SOURCE=.\lgxmlfactory.h
# End Source File
# Begin Source File

SOURCE="..\..\parser-kit\naguido.h"
# End Source File
# Begin Source File

SOURCE="..\..\parser-kit\strlist.h"
# End Source File
# End Group
# Begin Group "Ressourcendateien"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Source File

SOURCE=..\test.gmn
# End Source File
# Begin Source File

SOURCE=..\test.gmn.xml
# End Source File
# End Target
# End Project
