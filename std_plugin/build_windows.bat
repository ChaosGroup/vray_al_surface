@echo off

if not defined VRAY_SDK (
	echo VRAY_SDK environment variable not defined.
	goto done
)

if not defined VC_LIBS (
	echo VC_LIBS environment variable not defined. It must match the lib folder of the V-Ray SDK i.e. vc101, vc11 etc
	goto done
)

set VRAY_LIB_PATH=%VRAY_SDK%\lib\x64\%VC_LIBS%
if not exist "%VRAY_LIB_PATH%" (
	echo Folder "%VRAY_LIB_PATH%" does not exist; perhaps VC_LIBS variable has wrong value
	goto done
)

rem Compiler options: full optimization, exception handling, dynamic multithreaded runtime
set CPP_OPTIONS=/Ox /EHa /MD

rem Libraries to link against
set LIBS=vutils_s.lib plugman_s.lib vray.lib kernel32.lib user32.lib advapi32.lib

rem Visual Studio compiler version
set VC_LIBS=vc11

set OUTFILE=.\bin\vray_BRDFAlSurface.dll

cl vray_brdfalsurface.cpp ..\common\albrdf.cpp ..\common\beckmann.cpp ..\common\sss.cpp /I "%VRAY_SDK%\include" /I "%VRAY_SDK%\samples\vray_plugins\brdfs\vray_brdfbump" %CPP_OPTIONS% /link %LIBS% /libpath:"%VRAY_LIB_PATH%" /dll /out:"%OUTFILE%"

if %ERRORLEVEL% == 0 (
	echo Result is in %OUTFILE%
)

:done
echo Done.
