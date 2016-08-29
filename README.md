# vray_al_surface

A (partial) V-Ray port of the alSurface shader

## General

This plugin is a partial port of the alSurface shader by Anders Langlands. The original alSurface shader is available on Bitbucket at
https://bitbucket.org/anderslanglands/alshaders/wiki/Home

The main purpose of this port is to allow rendering with V-Ray for 3ds Max of skin materials set up for the Arnold renderer.

## 3ds Max version

### Installation for 3ds Max

Copy the .dlt file for your 3ds Max version to the [maxroot]\plugins\vrayplugins folder.

### Usage in 3ds Max

The material is called VRayALMtl.

## Maya version

### Installation for Maya

#### Windows

*) Copy the file vray_BRDFAlSurface.dll to the folder with the rest of the V-Ray plugins (f.e. C:\program files\autodesk\maya2016\vray\vrayplugins)

OR

*) Add the path to the vray_BRDFAlSurface.dll to the environment variables VRAY_FOR_MAYA2016_PLUGINS_x64 and VRAY_PLUGINS_x64

#### Linux

*) Copy the file libvray_BRDFAlSurface.so to the folder with the rest of the V-Ray plugins (f.e. /usr/autodesk/maya2016/vray/vrayplugins)

OR

*) Add the path to the libvray_BRDFAlSurface.so file to the environment variables VRAY_FOR_MAYA2016_PLUGINS_x64 and VRAY_PLUGINS_x64

### Usage in Maya

*) You can create the new material from Create > V-Ray > Create from V-Ray plugin > BRDF material


## Notes

- Not all parameters of alSurface are ported;
- For the moment, only "diffusion" and "directional" sub-surface scattering modes are supported;
- Refractions and thin translucency are not ported yet.
