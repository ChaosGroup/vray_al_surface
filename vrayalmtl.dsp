# Microsoft Developer Studio Project File - Name="vrayalmtl" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=vrayalmtl - Win32 Max Release Official
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "vrayalmtl.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "vrayalmtl.mak" CFG="vrayalmtl - Win32 Max Release Official"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "vrayalmtl - Win32 Max Release Official" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "vrayalmtl - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=xicl6.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "vrayalmtl - Win32 Max Release Official"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "vrayalmtl___Win32_Official_Max_6"
# PROP BASE Intermediate_Dir "vrayalmtl___Win32_Official_Max_6"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "t:\build\vrayalmtl\max60\x86\official"
# PROP Intermediate_Dir "t:\build\vrayalmtl\max60\x86\official"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /MD /W3 /GX /O2 /I "k:\3dsmax\maxsdk60\include" /I "..\..\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "vrayalmtl_EXPORTS" /YX /FD /Qprec_div /c
# ADD CPP /nologo /G6 /MD /W3 /GX /Zi /O2 /I "k:\3dsmax\maxsdk60\include" /I "..\..\..\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "vrayalmtl_EXPORTS" /YX /FD /Qprec_div /Qvc7 /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 paramblk2.lib core.lib geom.lib maxutil.lib bmm.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x08B70000" /dll /machine:I386 /out:"w:\program files\max70\plugins\vrayplugins\vrayalmtl60.dlt" /libpath:"k:\3dsmax\maxsdk60\lib" /libpath:"k:\msvs2000\lib" /release
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 paramblk2.lib core.lib geom.lib maxutil.lib bmm.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib vray60.lib vutils_s.lib plugman_s.lib vrender60.lib /base:"0x08B70000" /dll /pdb:"o:\vray\1.0\currentbuild\3dsmax\x86\adv\vrayalmtl60.pdb" /debug /machine:I386 /def:".\plugin.def" /out:"o:\vray\1.0\currentbuild\3dsmax\x86\adv\vrayalmtl60.dlt" /libpath:"k:\3dsmax\maxsdk60\Lib\x86" /libpath:"..\..\..\lib\x86" /libpath:"..\..\..\lib\x86\vc6" /release
# SUBTRACT LINK32 /nologo /pdb:none

!ELSEIF  "$(CFG)" == "vrayalmtl - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "vrayalmtl___Win32_Max6_Release"
# PROP BASE Intermediate_Dir "vrayalmtl___Win32_Max6_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "t:\build\vrayalmtl\debug\x86"
# PROP Intermediate_Dir "t:\build\vrayalmtl\debug\x86"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /MD /W3 /GX /O2 /I "k:\3dsmax\maxsdk60\include" /I "..\..\..\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "vrayalmtl_EXPORTS" /YX /FD /Qprec_div /c
# ADD CPP /nologo /G6 /MD /W3 /GX /O2 /I "k:\3dsmax\maxsdk60\include" /I "..\..\..\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "vrayalmtl_EXPORTS" /YX /FD /Qprec_div /Qvc7 /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 paramblk2.lib core.lib geom.lib maxutil.lib bmm.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x08B70000" /dll /machine:I386 /out:"o:\vray\1.0\currentbuild\x86\adv\vrayalmtl60.dlt" /libpath:"k:\3dsmax\maxsdk60\Lib" /libpath:"..\..\..\lib\x86" /libpath:"..\..\..\lib\x86\vc6" /libpath:"k:\msvs2000\lib" /release
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 paramblk2.lib core.lib geom.lib maxutil.lib bmm.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib vray60.lib vutils_s.lib plugman_s.lib /nologo /base:"0x08B70000" /dll /machine:I386 /out:"w:\program files\max80\plugins\vrayplugins\vrayalmtl60.dlt" /libpath:"k:\3dsmax\maxsdk60\Lib" /libpath:"..\..\..\lib\x86" /libpath:"..\..\..\lib\x86\vc6" /libpath:"k:\msvs2000\lib" /release
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "vrayalmtl - Win32 Max Release Official"
# Name "vrayalmtl - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\blinnbrdf.cpp
# End Source File
# Begin Source File

SOURCE=.\mtlshade.cpp
# End Source File
# Begin Source File

SOURCE=.\plugin.cpp
# End Source File
# Begin Source File

SOURCE=.\plugin.def

!IF  "$(CFG)" == "vrayalmtl - Win32 Max Release Official"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "vrayalmtl - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\vrayalmtl.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\blinnbrdf.h
# End Source File
# Begin Source File

SOURCE=.\vrayalmtl.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\common\powershader.bmp
# End Source File
# Begin Source File

SOURCE=.\vrayalmtl.rc
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
