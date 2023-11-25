# BrewReality
 A bit of a tech demo made with Libdragon unstable, features 128x128 color textures with trilinear mipmap interpolation, fully fitting TMEM, and a fully dynamic sky.

Graphics in this demo are fully done with the principles of a Reality Engine, meaning the display output you see: 

must not contain noise (antialiasing of polygons with N64's hardware AA, antialiasing of textures using trilinear mipmap interpolation), 

must be pixelless (bilinear filtering on textures and display, high-resolution output), 

and must have no banding artifacts (HighColor framebuffer is dithered and then dedithered into a TrueColor output).

Uses textures from Google Earth.

Uses Quaternion.h A basic quaternion library written in C by Martin Weigel.

Airplane model based on Mig 29 by Rifeor.

https://sketchfab.com/3d-models/mig-29-9-13-2c8a8f454fa04a8b914aa9a227f67bbe

Building with Libdragon unstable may need some code adjustments because of some changes made to the sdk.
