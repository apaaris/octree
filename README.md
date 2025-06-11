# A simple Octree implemenentation 

A C++ implementation of a spatial octree data structure with VTK export capabilities for 3D visualization.

## Overview

This project implements a 3D octree (8-tree) spatial data structure that efficiently organizes points in 3D space by recursively subdividing space into octants. The octree automatically subdivides when nodes contain too many points, creating a hierarchical spatial index.

## Project Structure

```
octree/
├── src/
│   ├── main.cpp      # Main application with test data
│   └── octree.h      # Octree class implementation
├── out/              # Output directory for generated files
│   └── octree.vtk    # Generated VTK file (after running)
└── README.md         # This file
```


### Compilation
```bash
# Navigate to project directory
cd src/octree

# Compile the project
g++ -o octree main.cpp

# Run the program
./octree
```

### Output
The program will:
1. Create an octree with bounds from (-10,-10,-10) to (10,10,10)
2. Insert 15 test points
3. Display the octree structure in the console
4. Export the octree to `out/octree.vtk` for visualization

## Octree Algorithm

### Insertion Process
1. Check if point is within node bounds
2. If node is a leaf and under capacity → add point
3. If node exceeds capacity → subdivide into 8 octants
4. If node has children → route point to appropriate octant
5. Create child nodes on-demand as needed

### Octant Indexing
The octree uses a binary indexing system where each octant is identified by a 3-bit number:
- Bit 0: X-axis (0=left, 1=right)  
- Bit 1: Y-axis (0=bottom, 1=top)
- Bit 2: Z-axis (0=back, 1=front)

```
Octant Layout:
    4 ---- 5
   /|     /|
  7 ---- 6 |
  | 0 ---|-1
  |/     |/
  3 ---- 2
```

## VTK Visualization

### Generated Data
The VTK file contains:
- **Points**: Original data points as vertices
- **Boxes**: Octree node boundaries as hexahedrons
- **Levels**: Color-coded octree depth levels

## Configuration

### Adjusting Subdivision Threshold
In `src/octree.h`, line 32:
```cpp
if (points.size() > 1) subdivide();  // Change threshold here
```

### Modifying Test Data
In `src/main.cpp`, update the `testPoints` vector:
```cpp
std::vector<Point> testPoints = {
    {1, 1, 1}, {2, 2, 2}, // Add your points here
    // ...
};
```

### Changing Octree Bounds
In `src/main.cpp`, modify the octree constructor:
```cpp
OctreeNode octree({-10, -10, -10}, {10, 10, 10}); // min, max bounds
```

## API Reference

### Core Methods
- `insert(const Point& p)` - Insert a point into the octree
- `contains(const Point& p)` - Check if point is within bounds
- `exportToVTK(const string& filename)` - Export to VTK format
- `print(int depth)` - Print octree structure to console

### Data Structures
```cpp
struct Point { 
    float x, y, z; 
};

class OctreeNode {
    Point min, max;                    // Bounding box
    OctreeNode* children[8];           // 8 octant children
    std::vector<Point> points;         // Points in this node
};
```
