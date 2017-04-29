BabyGL
======
Perhaps not boasting the most descriptive of names, **BabyGL** is a learning
experiment that renders geometries illuminated via adjustable point-lights.

![gif]()
![gif]()

Note that although the running demo only features spherical geometry, the other
parametric surfaces within [par_shapes](http://github.prideout.net/shapes) are
also supported. 

Installation
------------

A pre-built binary is available within the build folder.
```
./build/main.exe
```

To build a new binary on a Windows environment, first run the appropriate 64-bit
vcvarsall.bat, then `./build.bat`.

Usage
-----

Standard Controls

| Controls | Description |
| :---: | --- |
| ` | Toggle debug mode |
| Mouse Movement | Move view |
| WASD, Shift, Space | Move position |

Debug Mode

| Controls | Description |
| :---: | --- |
| 1-5 | Select slider |
| Q, E | Shift Slider Value |
| Tab, Shift + Tab | Cycle point-light selection |

Sliders are numbered from top to bottom. 

| Slider # | Affected Point-Light Property | 
| :---: | --- |
| 1 | Ambient Intensity | 
| 2 | Diffuse Intensity | 
| 3 | Specular Intensity |
| 4 | Linear Attenuation | 
| 5 | Quadratic Attenuation |
  
Motivation
----------

This project was created as a way of learning opengl, and graphics programming
in general. The amount of learned material is both varied and vast, and that
list is best discovered via a peek over the project source. For the sake of
brevity, a list of each shader with a notable highlight are included below.

| Shader | Notable Highlight |
| --- | --- |
| GPass | Writing geometry information to texture. |
| LPass | Passing information via uniform buffer object. |
| Bloom | Altering shader via subroutines. |
| GUI | Building geometry from information passed via vertex buffers. |
| Outline | Working with stencil buffers. |
| PerlinTexture | Precomputing a texture via shader for future use. |
