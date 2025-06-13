# Octree

This project implements and compares three different octree data structure implementations:
1. Classic Octree - Traditional implementation using fixed-size arrays
2. Hashmap-based Octree - Using hash maps for dynamic child node storage
3. Morton Key-based Octree - Using Morton keys for spatial indexing

## Building

```bash
mkdir build
cd build
cmake ..
make
```

## Usage

```bash
./octree <tree_type> <distribution_type> <num_points>
```

### Parameters

- `tree_type`: The octree implementation to use
  - `classic` - Classic octree implementation
  - `hashmap` - Hashmap-based octree implementation
  - `morton` - Morton key-based octree implementation

- `distribution_type`: The pattern of points to generate
  - `random` - Random points in 3D space
  - `grid` - Points in a regular 3D grid
  - `spiral` - Points in a 3D spiral pattern

- `num_points`: Number of points to generate and insert

### Examples

```bash
# Create a classic octree with 1000 random points
./octree classic random 1000

# Create a hashmap-based octree with 512 points in a grid
./octree hashmap grid 512

# Create a Morton key-based octree with 2000 points in a spiral
./octree morton spiral 2000
```
### Output
When running, the octree is saved as a vtk grid, which can be opened with paraview for visualization
