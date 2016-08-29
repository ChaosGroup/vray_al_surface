# Compilation instructions

## Windows

*) Open a Visual Studio command-line prompt

*) Use the 'cd' command to navigate to the folder where the build_windows.bat
file is located

*) Set the environment variable VRAY_SDK to point to the folder of the
V-Ray SDK, f.e. C:\Program Files\Chaos Group\V-Ray\Maya 2016 for x64

*) Set the environment variable VC_LIBS to the version of Visual Studio
for which you have the V-Ray SDK, f.e. vc11 for Maya 2016

*) Run the build_windows.bat command

*) The resulting .dll with the BRDF plugin is in the "bin" sub-folder of the
current folder

## Linux

*) Open a Terminal command-line prompt

*) Use the 'cd' command to navigate to the folder the build_linux.sh file
is located

*) Set the environment variable VRAY_SDK to point to the folder of the
V-Ray SDK, f.e. /usr/ChaosGroup/V-Ray/Maya2016-x64/

*) Run the build_linux.sh command

*) The resulting .so file with the BRDF plugin is in the "bin" sub-folder of
the current folder
