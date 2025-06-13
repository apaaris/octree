#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <cmath>

// Include shared Point definition
#include "point.h"

// Include all three octree implementations
#include "octree_classic.h"
#include "octree_hashmap.h" 
#include "octree_morton.h"

// Function to generate random points within bounds
std::vector<Point> generateRandomPoints(int numPoints, const Point& min, const Point& max) {
    std::vector<Point> points;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> distX(min.x, max.x);
    std::uniform_real_distribution<float> distY(min.y, max.y);
    std::uniform_real_distribution<float> distZ(min.z, max.z);

    for (int i = 0; i < numPoints; ++i) {
        points.push_back({distX(gen), distY(gen), distZ(gen)});
    }
    return points;
}

// Function to generate points in a grid pattern
std::vector<Point> generateGridPoints(int pointsPerSide, const Point& min, const Point& max) {
    std::vector<Point> points;
    float stepX = (max.x - min.x) / (pointsPerSide - 1);
    float stepY = (max.y - min.y) / (pointsPerSide - 1);
    float stepZ = (max.z - min.z) / (pointsPerSide - 1);

    for (int x = 0; x < pointsPerSide; ++x) {
        for (int y = 0; y < pointsPerSide; ++y) {
            for (int z = 0; z < pointsPerSide; ++z) {
                points.push_back({
                    min.x + x * stepX,
                    min.y + y * stepY,
                    min.z + z * stepZ
                });
            }
        }
    }
    return points;
}

// Function to generate points in a spiral pattern
std::vector<Point> generateSpiralPoints(int numPoints, const Point& min, const Point& max) {
    std::vector<Point> points;
    float centerX = (min.x + max.x) / 2;
    float centerY = (min.y + max.y) / 2;
    float centerZ = (min.z + max.z) / 2;
    float radius = std::min({max.x - min.x, max.y - min.y, max.z - min.z}) / 2;

    for (int i = 0; i < numPoints; ++i) {
        float t = i * 0.1f;
        float r = radius * (1.0f - float(i) / numPoints);
        points.push_back({
            centerX + r * cos(t),
            centerY + r * sin(t),
            centerZ + t * 0.1f
        });
    }
    return points;
}

void printUsage() {
    std::cout << "Usage: ./octree <tree_type> <distribution_type> <num_points>" << std::endl;
    std::cout << "Tree types:" << std::endl;
    std::cout << "  classic - Classic octree implementation" << std::endl;
    std::cout << "  hashmap - Hashmap-based octree implementation" << std::endl;
    std::cout << "  morton  - Morton code-based octree implementation" << std::endl;
    std::cout << "Distribution types:" << std::endl;
    std::cout << "  random - Random points in 3D space" << std::endl;
    std::cout << "  grid   - Points in a regular 3D grid" << std::endl;
    std::cout << "  spiral - Points in a 3D spiral pattern" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printUsage();
        return 1;
    }

    std::string treeType = argv[1];
    std::string distributionType = argv[2];
    int numPoints = std::stoi(argv[3]);

    // Define the bounding box for the octree
    Point min = {-10.0f, -10.0f, -10.0f};
    Point max = {10.0f, 10.0f, 10.0f};

    // Generate points based on the selected distribution
    std::vector<Point> points;
    if (distributionType == "random") {
        points = generateRandomPoints(numPoints, min, max);
    } else if (distributionType == "grid") {
        int pointsPerSide = static_cast<int>(std::cbrt(numPoints));
        points = generateGridPoints(pointsPerSide, min, max);
    } else if (distributionType == "spiral") {
        points = generateSpiralPoints(numPoints, min, max);
    } else {
        std::cerr << "Invalid distribution type: " << distributionType << std::endl;
        printUsage();
        return 1;
    }

    // Create and populate the octree
    std::unique_ptr<OctreeNode> classicOctree;
    std::unique_ptr<OctreeHashMap> hashmapOctree;
    std::unique_ptr<OctreeMorton> mortonOctree;

    if (treeType == "classic") {
        classicOctree = std::make_unique<OctreeNode>(min, max);
    } else if (treeType == "hashmap") {
        hashmapOctree = std::make_unique<OctreeHashMap>(min, max);
    } else if (treeType == "morton") {
        mortonOctree = std::make_unique<OctreeMorton>(min, max);
    } else {
        std::cerr << "Invalid tree type: " << treeType << std::endl;
        printUsage();
        return 1;
    }

    // Measure build time
    auto start = std::chrono::high_resolution_clock::now();
    
    // Insert points into the appropriate octree
    if (treeType == "classic") {
        for (const auto& p : points) {
            classicOctree->insert(p);
        }
    } else if (treeType == "hashmap") {
        for (const auto& p : points) {
            hashmapOctree->insert(p);
        }
    } else if (treeType == "morton") {
        for (const auto& p : points) {
            mortonOctree->insert(p);
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Print statistics
    std::cout << "\nBuild time: " << duration.count() << " ms" << std::endl;
    
    if (treeType == "classic") {
        classicOctree->printStatistics();
    } else if (treeType == "hashmap") {
        hashmapOctree->printStatistics();
    } else if (treeType == "morton") {
        mortonOctree->printStatistics();
    }

    // Export to VTK for visualization
    std::string filename = "octree_" + distributionType + ".vtk";
    if (treeType == "classic") {
        classicOctree->exportToVTK(filename);
    } else if (treeType == "hashmap") {
        hashmapOctree->exportToVTK(filename);
    } else if (treeType == "morton") {
        mortonOctree->exportToVTK(filename);
    }

    return 0;
}
