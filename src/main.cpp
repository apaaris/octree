#include<iostream>
#include<vector>
#include<fstream>
#include<iomanip>
#include "octree.h"



int main() {
    // Create an octree with bounds from (-10,-10,-10) to (10,10,10)
    OctreeNode octree({-10, -10, -10}, {10, 10, 10});

    // Insert some test points
    std::vector<Point> testPoints = {
        {1, 1, 1}, {2, 2, 2}, {3, 3, 3}, {4, 4, 4}, {5, 5, 5},
        {-1, -1, -1}, {-2, -2, -2}, {-3, -3, -3}, {-4, -4, -4}, {-5, -5, -5},
        {1, -1, 1}, {-1, 1, -1}, {1, 1, -1}, {6, 6, 6}, {7, 7, 7}
    };

    std::cout << "Inserting points into octree..." << std::endl;
    for (const auto& point : testPoints) {
        octree.insert(point);
        std::cout << "Inserted point: (" << point.x << ", " << point.y << ", " << point.z << ")" << std::endl;
    }

    std::cout << "\nOctree structure:" << std::endl;
    octree.print();

    // Export to VTK for ParaView visualization
    std::cout << "\nExporting octree to VTK format..." << std::endl;
    octree.exportToVTK("../out/octree.vtk");

    return 0;
}

