# Microsoft Developer Studio Project File - Name="powtty" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=powtty - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "powtty.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "powtty.mak" CFG="powtty - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "powtty - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "powtty - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "powtty - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "powtty - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "powtty - Win32 Release"
# Name "powtty - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\BE_NOSSH.C
# End Source File
# Begin Source File

SOURCE=.\LDISC.C
# End Source File
# Begin Source File

SOURCE=.\MISC.C
# End Source File
# Begin Source File

SOURCE=.\powbeam.c
# End Source File
# Begin Source File

SOURCE=.\powcmd.c
# End Source File
# Begin Source File

SOURCE=.\powcmd2.c
# End Source File
# Begin Source File

SOURCE=.\powedit.c
# End Source File
# Begin Source File

SOURCE=.\poweval.c
# End Source File
# Begin Source File

SOURCE=.\powhook.c
# End Source File
# Begin Source File

SOURCE=.\powlist.c
# End Source File
# Begin Source File

SOURCE=.\powlog.c
# End Source File
# Begin Source File

SOURCE=.\powmain.c
# End Source File
# Begin Source File

SOURCE=.\powmap.c
# End Source File
# Begin Source File

SOURCE=.\powptr.c
# End Source File
# Begin Source File

SOURCE=.\powtty.c
# End Source File
# Begin Source File

SOURCE=.\powutils.c
# End Source File
# Begin Source File

SOURCE=.\RAW.C
# End Source File
# Begin Source File

SOURCE=.\regex.c
# End Source File
# Begin Source File

SOURCE=.\SETTINGS.C
# End Source File
# Begin Source File

SOURCE=.\SIZETIP.C
# End Source File
# Begin Source File

SOURCE=.\TELNET.C
# End Source File
# Begin Source File

SOURCE=.\TERMINAL.C
# End Source File
# Begin Source File

SOURCE=.\TREE234.C
# End Source File
# Begin Source File

SOURCE=.\VERSION.C
# End Source File
# Begin Source File

SOURCE=.\WIN_RES.RC
# End Source File
# Begin Source File

SOURCE=.\WINCTRLS.C
# End Source File
# Begin Source File

SOURCE=.\WINDLG.C
# End Source File
# Begin Source File

SOURCE=.\WINDOW.C
# End Source File
# Begin Source File

SOURCE=.\WINNET.C
# End Source File
# Begin Source File

SOURCE=.\WINSTORE.C
# End Source File
# Begin Source File

SOURCE=.\XLAT.C
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\NETWORK.H
# End Source File
# Begin Source File

SOURCE=.\powbeam.h
# End Source File
# Begin Source File

SOURCE=.\powcmd.h
# End Source File
# Begin Source File

SOURCE=.\powcmd2.h
# End Source File
# Begin Source File

SOURCE=.\powdefines.h
# End Source File
# Begin Source File

SOURCE=.\powedit.h
# End Source File
# Begin Source File

SOURCE=.\poweval.h
# End Source File
# Begin Source File

SOURCE=.\powhook.h
# End Source File
# Begin Source File

SOURCE=.\powlist.h
# End Source File
# Begin Source File

SOURCE=.\powlog.h
# End Source File
# Begin Source File

SOURCE=.\powmain.h
# End Source File
# Begin Source File

SOURCE=.\powmap.h
# End Source File
# Begin Source File

SOURCE=.\powptr.h
# End Source File
# Begin Source File

SOURCE=.\powtty.h
# End Source File
# Begin Source File

SOURCE=.\powutils.h
# End Source File
# Begin Source File

SOURCE=.\PUTTY.H
# End Source File
# Begin Source File

SOURCE=.\PUTTYMEM.H
# End Source File
# Begin Source File

SOURCE=.\regex.h
# End Source File
# Begin Source File

SOURCE=.\RESOURCE.H
# End Source File
# Begin Source File

SOURCE=.\STORAGE.H
# End Source File
# Begin Source File

SOURCE=.\TREE234.H
# End Source File
# Begin Source File

SOURCE=.\WIN_RES.H
# End Source File
# Begin Source File

SOURCE=.\WINSTUFF.H
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\Ico00001.ico
# End Source File
# Begin Source File

SOURCE=.\Ico00002.ico
# End Source File
# Begin Source File

SOURCE=.\Icon1.ico
# End Source File
# Begin Source File

SOURCE=.\Icon2.ico
# End Source File
# Begin Source File

SOURCE=.\powtty2.ico
# End Source File
# Begin Source File

SOURCE=.\PUTTY.ICO
# End Source File
# End Group
# End Target
# End Project
