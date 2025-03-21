# 3D Renderer
Backup of my work for Pikuma's "3D Graphics Programming From Scratch" course.
I'm enjoying this course a lot though, so there's a good chance I'll use it to
springboard into other areas.

## Additional Topics/Challenges
I want to maintain the pace that I have, but there are definitely topics worth
revisiting as well as relevant new areas to tackle.

### SDL (in general)
It's not the focus of the course clearly, but there's value to be had in
understanding how to use it well.

### Debug Mode
It would be nice to present on-screen diagnostic information while the program
is running, instead of resorting to printf in the terminal.

### Hot Loading
Being able to save and resume state would be a time saver I think.

### Performance Best Practices
I need to research what this entails, but some of the choices made were definitely
made out of convenience or pedagogical efficiency.

### Perspective Transform Math
Probably the trickiest part given that it's the most likely to be inconsistent
across engines and platforms since everyone creates their own conventions! Fun!

### OBJ Importer
Mainly, making the one I have more robust. This part of the code can be very
segfault happy.

- [x] Handle additional face element variations
- [ ] Remove unneeded baked-in assumptions (like a model necessarily having vertex texture information)

### Line Drawing Algorithms
DDA was quick and dirty, but Breseham appears to be the standard that other
algorithms are based off of.
- [x] DDA
- [ ] Breseham

Also, maybe try tackling anti-aliasing?
- [ ] Wu
- [ ] Gupta-Sproull

Actually, both of those may require me to look into subpixel rendering.
- [ ] Subpixel Rendering

### Shading Algorithms
IIRC any smooth-shading algorithm is going to require the introduction of
vertex normals, which should be provided in a mesh's OBJ file.
- [x] Flat
- [ ] Gouraud
- [ ] Phong

