# vray_al_surface

A (partial) V-Ray port of the alSurface shader

General
-------

This plugin is a partial port of the alSurface shader by Anders Langlands. The original alSurface shader is available on Bitbucket at
https://bitbucket.org/anderslanglands/alshaders/wiki/Home

The main purpose of this port is to allow rendering with V-Ray for 3ds Max of skin materials set up for the Arnold renderer.

Installation for 3ds Max
------------------------

Copy the .dlt file for your 3ds Max version to the [maxroot]\plugins\vrayplugins folder.

Usage
-----

The material is called VRayALMtl.

Notes
-----

- Not all parameters of alSurface are ported;
- For the moment, only "diffusion" and "directional" sub-surface scattering modes are supported;
- Refractions and thin translucency are not ported yet.
