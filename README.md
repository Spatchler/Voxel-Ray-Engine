# Voxel Ray Engine

![](.github/Sponza4096.png)
![](.github/Sponza4096View2.png)
![](.github/Sponza4096Lion.png)
Crytek sponza voxelized with a generated palette at 4096 resolution

A Sparse Voxel DAG ray caster for [VMesh-CLI](https://github.com/Spatchler/VMesh-CLI) vm8 files written in C++ with OpenGL.

### Dependencies:

- glfw
- glm
- stb/stb_image.h
- [VMesh](https://github.com/Spatchler/VMesh)
- [glad](https://glad.dav1d.de/#language=c&specification=gl&api=gl%3D4.6&api=gles1%3Dnone&api=gles2%3Dnone&api=glsc2%3Dnone&profile=core&loader=on) (OpenGL core v4.6)

Generate the glad files from the link and download and extract `glad.zip` in `dependencies/`

Install dependencies with your system package manager e.g. `# pacman -S glm glfw stb` and they should work straight away since the premake config includes from `/usr/include` and shared objects are linked by the os

Clone VMesh and copy everything from 'include' to 'dependencies/include/VMesh'

Please create an issue if you have trouble with anything.

### Build:

`premake5 gmake && make`

![](.github/debugRendering.png)

### Some more screenshots:

Crytek sponza voxelized using the duel palette from lospec at 512 resolution
![](.github/SponzaDuel.png)
![](.github/SponzaDuel2.png)

Crytek sponza voxelized using the journey palette from lospec at 512 resolution
![](.github/SponzaJourney.png)
