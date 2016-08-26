# Microsoft Developer Studio Project File - Name="vray_mtlalsurface" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=vray_mtlalsurface - Win32 Max Release Official
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "vray_mtlalsurface.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "vray_mtlalsurface.mak" CFG="vray_mtlalsurface - Win32 Max Release Official"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "vray_mtlalsurface - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "vray_mtlalsurface - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=xicl6.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "vray_mtlalsurface - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "vray_mtlalsurface___Win32_Release"
# PROP BASE Intermediate_Dir "vray_mtlalsurface___Win32_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\build\release"
# PROP Intermediate_Dir ".\build\release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /MD /W3 /GX /O2 /I "m:\vraysdk\include" /I "..\..\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "vray_mtlalsurface_EXPORTS" /YX /FD /Qprec_div /c
# ADD CPP /nologo /G6 /MD /W3 /GX /Zi /O2 /I "m:\vraysdk\include" /I "..\..\..\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "vray_mtlalsurface_EXPORTS" /YX /FD /Qprec_div /Qvc7 /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /out:".\bin\vray_MtlAlSurface.dll" /debug
# SUBTRACT BASE LINK32
# ADD LINK32 comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib vray.lib vutils_s.lib plugman_s.lib /dll /pdb:".\pdb\vray_MtlAlSurface.pdb" /debug /machine:I386 /out:".\bin\vray_MtlAlSurface.dll" /libpath:"m:\vraysdk\lib\x86" /libpath:"m:\vraysdk\lib\x86\vc6"
# SUBTRACT LINK32 /nologo /pdb:none

!ELSEIF  "$(CFG)" == "vray_mtlalsurface - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "vray_mtlalsurface___Win32_Max6_Debug"
# PROP BASE Intermediate_Dir "vray_mtlalsurface___Win32_Max6_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\build\debug"
# PROP Intermediate_Dir ".\build\debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /MD /W3 /GX /O2 /I "m:\vraysdk\include" /I "..\..\..\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "vray_mtlalsurface_EXPORTS" /YX /FD /Qprec_div /c
# ADD CPP /nologo /G6 /MD /W3 /GX /O2 /I "m:\vraysdk\include" /I "..\..\..\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "vray_mtlalsurface_EXPORTS" /YX /FD /Qprec_div /Qvc7 /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /out:".\bin\vray_MtlAlSurface.dll" /libpath:"m:\vraysdk\lib\x86" /libpath:"m:\vraysdk\lib\x86\vc6" /debug
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib vray.lib vutils_s.lib plugman_s.lib /nologo /dll /machine:I386 /out:".\bin\vray_MtlAlSurface.dll" /libpath:"m:\vraysdk\lib\x86" /libpath:"m:\vraysdk\lib\x86\vc6" /debug
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "vray_mtlalsurface - Win32 Release"
# Name "vray_mtlalsurface - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\common\albrdf.cpp
# End Source File
# Begin Source File

SOURCE=..\common\beckmann.cpp
# End Source File
# Begin Source File

SOURCE=..\common\sss.cpp

# End Source File
# Begin Source File

SOURCE=.\vray_mtlalsurface.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\common\albrdf.h
# End Source File
# Begin Source File

SOURCE=..\common\beckmann.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\common\sss.h
# End Source File
# End Group
# Begin Group "ReadMe"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Skeleton.txt
# End Source File
# End Group
# End Target
# End Project
