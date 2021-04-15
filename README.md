#List of Features
Ray triangle and other primatives intersection
Phong interpolation
Whitted illumination model
Antialiasing
Texture map support
Cube map support
Multithreading of individual rays
Spatial partition of objects

Gui and scenes were provided.

Antialiasing is done with shooting x^2 rays for each pixel by subdividing each pixel into subpixels and doing a average.
 (does not use original ray cast from before AA)

Refraction ignores backface reflections. 
Refraction also ignores attenuation within object. (line 144 of RayTracer.cpp)
This was done due to the fact that the images created with attenuation (multiply by m.kt())
was much dimmer than the reference solution. However, I do believe attenuating the light
as it bounces within the object should be done.

Cubemap was implement via heuristics. Once I figured out the face of the cubemap to use (via largest element in ray direction),
The ray direction was scaled then transformed into UV coords for use.

Texture mapping utilizes the standard bilinear interpolation. This is a naive implementation in which 
only considers the pixels 1 over and 1 below the original pixel. There was some testing to try 
looking at other pixels heuristically, but they did not give any satisfying results.

The acceleration data structure implemented is the BVH. In every recursive step,
the current geometries are separated into two groups based on their Bounding Box's centroid's position.
Although this is a very naive method of separating the two groups, it is still satifactory in perfomance.

Multithreading via pthreads was implemented for faster computation. 
Some additional optimizations that can be done is within the shadow handling code (Should not be calling interesect 
multiple times when ray is traveling in a line) but time did not permit.
