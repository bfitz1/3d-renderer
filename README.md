# 3D Renderer
Backup of my work for Pikuma's "3D Graphics Programming From Scratch" course.
I'm enjoying this course a lot though, so there's a good chance I'll use it to
springboard into other areas.

## Additional Topics/Challenges
I want to maintain the pace that I have, but there are definitely topics worth
revisiting.

### SDL (in general)
It's not the focus of the course clearly, but there's value to be had in
understanding how to use it well.

### Perspective Math
Probably the trickiest part given that it's the most likely to be inconsistent
across engines and platforms since everyone creates their own conventions! Fun!

### OBJ Importer
Mainly, making the one I have more robust. The most pressing issue is that mine
expects face vertices to be listed as "f d/d/d d/d/d d/d/d", but not all files
do that.

### Line Drawing Algorithms
DDA was quick and dirty, but Breseham appears to be the standard that other
algorithms are based off of.
- [x] DDA
- [ ] Breseham

Also, maybe try tackling anti-aliasing?
- [ ] Wu
- [ ] Gupta-Sproull

### Shading Algorithms
IIRC any smooth-shading algorithm is going to require the introduction of
vertex normals, which should be provided in a mesh's OBJ file.
- [x] Flat
- [ ] Gouraud
- [ ] Phong

