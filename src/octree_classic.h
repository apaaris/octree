#ifndef OCTREE_CLASSIC_H
#define OCTREE_CLASSIC_H

#include <iostream>
#include <vector>
#include <fstream>
#include <iomanip>
#include <string>
#include "point.h"

class OctreeNode {
public:
    // Bounding box: min and max coordinates
    Point min, max;
    // Child nodes (could be std::unique_ptr<OctreeNode>[8] or std::array)
    OctreeNode* children[8] = {nullptr};
    // Data (e.g., list of points or objects)
    std::vector<Point> points;

    OctreeNode(const Point& min, const Point& max) : min(min), max(max) {}

    // Destructor to clean up memory
    ~OctreeNode() {
        for (int i = 0; i < 8; ++i) {
            delete children[i];
        }
    }

    void insert(const Point& p) {
        // Check if point is within bounds
        if (!contains(p)) {
            std::cout << "Warning: Point (" << p.x << ", " << p.y << ", " << p.z 
                      << ") is outside node bounds" << std::endl;
            return;
        }

        if (isLeaf()) {
            points.push_back(p);
            // Optionally subdivide if too many points
            if (points.size() > 1) subdivide();
        } else {
            int idx = getOctant(p);
            if (children[idx] == nullptr) {
                // Create child node with new bounds
                Point childMin, childMax;
                calculateChildBounds(idx, childMin, childMax);
                children[idx] = new OctreeNode(childMin, childMax);
            }
            children[idx]->insert(p);
        }
    }

    bool contains(const Point& p) const {
        return p.x >= min.x && p.x <= max.x &&
               p.y >= min.y && p.y <= max.y &&
               p.z >= min.z && p.z <= max.z;
    }

    void calculateChildBounds(int octant, Point& childMin, Point& childMax) const {
        Point center = {
            (min.x + max.x) / 2.0f,
            (min.y + max.y) / 2.0f,
            (min.z + max.z) / 2.0f
        };

        childMin.x = (octant & 1) ? center.x : min.x;
        childMin.y = (octant & 2) ? center.y : min.y;
        childMin.z = (octant & 4) ? center.z : min.z;
        childMax.x = (octant & 1) ? max.x : center.x;
        childMax.y = (octant & 2) ? max.y : center.y;
        childMax.z = (octant & 4) ? max.z : center.z;
    }

    bool isLeaf() const { return children[0] == nullptr; }

    void subdivide() {
        // Calculate center of current node
        Point center = {
            (min.x + max.x) / 2.0f,
            (min.y + max.y) / 2.0f,
            (min.z + max.z) / 2.0f
        };

        // Create children for each octant
        for (int i = 0; i < 8; ++i) {
            Point childMin, childMax;
            calculateChildBounds(i, childMin, childMax);
            children[i] = new OctreeNode(childMin, childMax);
        }

        // Move points to appropriate child
        for (const auto& p : points) {
            int idx = getOctant(p);
            children[idx]->insert(p);
        }
        points.clear();  // Clear parent's points since they are moved to children
    }

    int getOctant(const Point& p) const {
        Point center = {
            (min.x + max.x) / 2.0f,
            (min.y + max.y) / 2.0f,
            (min.z + max.z) / 2.0f
        };
        int idx = 0;
        if (p.x > center.x) idx |= 1;
        if (p.y > center.y) idx |= 2;
        if (p.z > center.z) idx |= 4;
        return idx;
    }

    // Helper function to print the octree structure
    void print(int depth = 0) const {
        std::string indent(depth * 2, ' ');
        std::cout << indent << "Node bounds: (" << min.x << "," << min.y << "," << min.z 
                  << ") to (" << max.x << "," << max.y << "," << max.z << ")" << std::endl;
        std::cout << indent << "Points: " << points.size() << std::endl;
        
        if (!isLeaf()) {
            for (int i = 0; i < 8; ++i) {
                if (children[i] != nullptr) {
                    std::cout << indent << "Child " << i << ":" << std::endl;
                    children[i]->print(depth + 1);
                }
            }
        }
    }

    // Collect all points in the octree
    void collectAllPoints(std::vector<Point>& allPoints) const {
        // Add points from this node
        for (const auto& p : points) {
            allPoints.push_back(p);
        }
        
        // Recursively collect from children
        if (!isLeaf()) {
            for (int i = 0; i < 8; ++i) {
                if (children[i] != nullptr) {
                    children[i]->collectAllPoints(allPoints);
                }
            }
        }
    }

    // Collect all node bounding boxes for visualization
    void collectNodeBoxes(std::vector<std::pair<Point, Point>>& boxes, std::vector<int>& levels, int currentLevel = 0) const {
        // Add this node's bounding box
        boxes.push_back({min, max});
        levels.push_back(currentLevel);
        
        // Recursively collect from children
        if (!isLeaf()) {
            for (int i = 0; i < 8; ++i) {
                if (children[i] != nullptr) {
                    children[i]->collectNodeBoxes(boxes, levels, currentLevel + 1);
                }
            }
        }
    }

    // Export octree to VTK format for ParaView visualization
    void exportToVTK(const std::string& filename) const {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << filename << " for writing." << std::endl;
            return;
        }

        // Collect all points and boxes
        std::vector<Point> allPoints;
        std::vector<std::pair<Point, Point>> boxes;
        std::vector<int> levels;
        
        collectAllPoints(allPoints);
        collectNodeBoxes(boxes, levels);

        // Write VTK header
        file << "# vtk DataFile Version 3.0\n";
        file << "Octree Visualization\n";
        file << "ASCII\n";
        file << "DATASET UNSTRUCTURED_GRID\n\n";

        // Calculate total points: original points + 8 corners per box
        int totalPoints = allPoints.size() + boxes.size() * 8;
        
        file << "POINTS " << totalPoints << " float\n";
        
        // Write original points
        for (const auto& p : allPoints) {
            file << std::fixed << std::setprecision(6) << p.x << " " << p.y << " " << p.z << "\n";
        }
        
        // Write box corner points
        for (const auto& box : boxes) {
            const Point& minP = box.first;
            const Point& maxP = box.second;
            
            // 8 corners of the box
            file << minP.x << " " << minP.y << " " << minP.z << "\n";  // 0
            file << maxP.x << " " << minP.y << " " << minP.z << "\n";  // 1
            file << maxP.x << " " << maxP.y << " " << minP.z << "\n";  // 2
            file << minP.x << " " << maxP.y << " " << minP.z << "\n";  // 3
            file << minP.x << " " << minP.y << " " << maxP.z << "\n";  // 4
            file << maxP.x << " " << minP.y << " " << maxP.z << "\n";  // 5
            file << maxP.x << " " << maxP.y << " " << maxP.z << "\n";  // 6
            file << minP.x << " " << maxP.y << " " << maxP.z << "\n";  // 7
        }

        // Write cells (points as vertices + boxes as hexahedrons)
        int totalCells = allPoints.size() + boxes.size();
        int totalCellData = allPoints.size() * 2 + boxes.size() * 9; // 2 for points (1+1), 9 for hex (8+1)
        
        file << "\nCELLS " << totalCells << " " << totalCellData << "\n";
        
        // Write point cells (vertices)
        for (int i = 0; i < allPoints.size(); ++i) {
            file << "1 " << i << "\n";
        }
        
        // Write box cells (hexahedrons)
        int baseIdx = allPoints.size();
        for (int i = 0; i < boxes.size(); ++i) {
            int start = baseIdx + i * 8;
            file << "8 " << start << " " << (start+1) << " " << (start+2) << " " << (start+3)
                 << " " << (start+4) << " " << (start+5) << " " << (start+6) << " " << (start+7) << "\n";
        }

        // Write cell types
        file << "\nCELL_TYPES " << totalCells << "\n";
        
        // Point cells (VTK_VERTEX = 1)
        for (int i = 0; i < allPoints.size(); ++i) {
            file << "1\n";
        }
        
        // Hexahedron cells (VTK_HEXAHEDRON = 12)
        for (int i = 0; i < boxes.size(); ++i) {
            file << "12\n";
        }

        // Write cell data (octree levels for coloring)
        file << "\nCELL_DATA " << totalCells << "\n";
        file << "SCALARS OctreeLevel int 1\n";
        file << "LOOKUP_TABLE default\n";
        
        // Points are at level -1 (to distinguish from boxes)
        for (int i = 0; i < allPoints.size(); ++i) {
            file << "-1\n";
        }
        
        // Box levels
        for (int level : levels) {
            file << level << "\n";
        }

        file.close();
        std::cout << "Octree exported to " << filename << std::endl;
        std::cout << "Open this file in ParaView to visualize the octree structure!" << std::endl;
    }
    
    // Add missing methods for benchmark compatibility
    std::vector<Point> rangeQuery(const Point& queryMin, const Point& queryMax) const {
        std::vector<Point> result;
        std::vector<Point> allPoints;
        collectAllPoints(allPoints);
        
        for (const auto& p : allPoints) {
            if (p.x >= queryMin.x && p.x <= queryMax.x &&
                p.y >= queryMin.y && p.y <= queryMax.y &&
                p.z >= queryMin.z && p.z <= queryMax.z) {
                result.push_back(p);
            }
        }
        return result;
    }
    
    void getStatistics(int& totalNodes, int& leafNodes, int& totalPoints, int& maxDepth, int currentDepth = 0) const {
        totalNodes++;
        totalPoints += points.size();
        maxDepth = std::max(maxDepth, currentDepth);
        
        if (isLeaf()) {
            leafNodes++;
        } else {
            for (int i = 0; i < 8; ++i) {
                if (children[i] != nullptr) {
                    children[i]->getStatistics(totalNodes, leafNodes, totalPoints, maxDepth, currentDepth + 1);
                }
            }
        }
    }
    
    void printStatistics() const {
        int totalNodes = 0, leafNodes = 0, totalPoints = 0, maxDepth = 0;
        getStatistics(totalNodes, leafNodes, totalPoints, maxDepth);
        
        std::cout << "=== Classic Octree Statistics ===" << std::endl;
        std::cout << "Total nodes: " << totalNodes << std::endl;
        std::cout << "Leaf nodes: " << leafNodes << std::endl;
        std::cout << "Internal nodes: " << (totalNodes - leafNodes) << std::endl;
        std::cout << "Total points: " << totalPoints << std::endl;
        std::cout << "Maximum depth: " << maxDepth << std::endl;
        std::cout << "Average points per leaf: " << (leafNodes > 0 ? (float)totalPoints / leafNodes : 0) << std::endl;
    }
};

#endif // OCTREE_CLASSIC_H