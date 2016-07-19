# vray_al_surface

A (partial) V-Ray port of the alSurface shader
==============================================

General
-------

This plugin is a partial port of the alSurface shader by Anders Langlands. The original alSurface shader is available on Bitbucket at
https://bitbucket.org/anderslanglands/alshaders/wiki/Home

The main purpose of this port is to allow rendering of skin materials set up for the Arnold renderer.

Installation for 3ds Max
------------------------

Copy the .dlt file for your 3ds Max version to the [maxroot]\plugins\vrayplugins folder.

Usage
-----

The material is called VRayALMtl.

Notes
-----

*) Not all parameters of alSurface are ported;
*) The specular BRDFs are fixed to GGX for both primary and secondary reflections;
*) Sub-surface scattering is fixed to "directional" right now;
*) Refractions and thin translucency are not ported yet.
