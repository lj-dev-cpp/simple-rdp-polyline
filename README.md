// Simple RDP Polyline

This is a small C++ project for simplifying 2D polylines using the
Ramer–Douglas–Peucker algorithm.  
It was written to better understand the algorithm and to test it inside
AutoCAD using ObjectARX, with a demo on noisy room boundary points.

// About the Algorithm

The idea is simple:

1. Start with the first and last point of the polyline.
2. Find the point in between that has the largest distance to this line.
3. If that distance is larger than a tolerance `epsilon`, keep the point
   and recursively process the two sub-segments.
4. If the distance is not larger than `epsilon`, remove all points
   between the two endpoints.

The result is a simplified polyline that keeps the overall shape while
reducing the number of vertices.

// Files

- `RDP_Arx.h` – declarations of the RDP functions (2D points)
- `RDP_Arx.cpp` – implementation and an AutoCAD test command
- `demo.gif` – short clip showing noisy points and the simplified room outline

![demo](demo.gif)
