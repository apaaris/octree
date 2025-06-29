#ifndef OCTREE_HASHMAP_H
#define OCTREE_HASHMAP_H

#include <iostream>
#include <vector>
#include <unordered_map>
#include <memory>
#include <fstream>
#include <iomanip>
#include <string>
#include "point.h"

class OctreeHashMapNode {
public:
    // Bounding box: min and max coordinates
    Point min, max;
    
    // Child nodes stored in hashmap with octant as key
    std::unordered_map<int, std::unique_ptr<OctreeHashMapNode>> children;
    
    // Data (list of points)
    std::vector<Point> points;
    
    // Maximum points per leaf before subdivision
    static const size_t MAX_POINTS_PER_LEAF = 1;

    OctreeHashMapNode(const Point& min, const Point& max) : min(min), max(max) {}

    void insert(const Point& p) {
        // Check if point is within bounds
        if (!contains(p)) {
            std::cout << "Warning: Point (" << p.x << ", " << p.y << ", " << p.z 
                      << ") is outside node bounds" << std::endl;
            return;
        }

        if (isLeaf()) {
            points.push_back(p);
            // Subdivide if too many points
            if (points.size() > MAX_POINTS_PER_LEAF) {
                subdivide();
            }
        } else {
            int octant = getOctant(p);
            
            // Create child node if it doesn't exist
            if (children.find(octant) == children.end()) {
                Point childMin, childMax;
                calculateChildBounds(octant, childMin, childMax);
                children[octant] = std::make_unique<OctreeHashMapNode>(childMin, childMax);
            }
            
            children[octant]->insert(p);
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

    bool isLeaf() const { 
        return children.empty(); 
    }

    void subdivide() {
        // Create children for each octant that has points
        std::unordered_map<int, std::vector<Point>> octantPoints;
        
        // Distribute points to octants
        for (const auto& p : points) {
            int octant = getOctant(p);
            octantPoints[octant].push_back(p);
        }
        
        // Create child nodes and insert points
        for (const auto& [octant, octantPointList] : octantPoints) {
            Point childMin, childMax;
            calculateChildBounds(octant, childMin, childMax);
            children[octant] = std::make_unique<OctreeHashMapNode>(childMin, childMax);
            
            for (const auto& p : octantPointList) {
                children[octant]->insert(p);
            }
        }
        
        // Clear parent's points since they are moved to children
        points.clear();
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
        std::cout << indent << "Active children: " << children.size() << std::endl;
        
        if (!isLeaf()) {
            for (const auto& [octant, child] : children) {
                std::cout << indent << "Child octant " << octant << ":" << std::endl;
                child->print(depth + 1);
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
        for (const auto& [octant, child] : children) {
            child->collectAllPoints(allPoints);
        }
    }

    // Collect all node bounding boxes for visualization
    void collectNodeBoxes(std::vector<std::pair<Point, Point>>& boxes, std::vector<int>& levels, int currentLevel = 0) const {
        // Add this node's bounding box
        boxes.push_back({min, max});
        levels.push_back(currentLevel);
        
        // Recursively collect from children
        for (const auto& [octant, child] : children) {
            child->collectNodeBoxes(boxes, levels, currentLevel + 1);
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
        file << "Octree Visualization (HashMap Implementation)\n";
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
        for (size_t i = 0; i < allPoints.size(); ++i) {
            file << "1 " << i << "\n";
        }
        
        // Write box cells (hexahedrons)
        int baseIdx = allPoints.size();
        for (size_t i = 0; i < boxes.size(); ++i) {
            int start = baseIdx + i * 8;
            file << "8 " << start << " " << (start+1) << " " << (start+2) << " " << (start+3)
                 << " " << (start+4) << " " << (start+5) << " " << (start+6) << " " << (start+7) << "\n";
        }

        // Write cell types
        file << "\nCELL_TYPES " << totalCells << "\n";
        
        // Point cells (VTK_VERTEX = 1)
        for (size_t i = 0; i < allPoints.size(); ++i) {
            file << "1\n";
        }
        
        // Hexahedron cells (VTK_HEXAHEDRON = 12)
        for (size_t i = 0; i < boxes.size(); ++i) {
            file << "12\n";
        }

        // Write cell data (octree levels for coloring)
        file << "\nCELL_DATA " << totalCells << "\n";
        file << "SCALARS OctreeLevel int 1\n";
        file << "LOOKUP_TABLE default\n";
        
        // Points are at level -1 (to distinguish from boxes)
        for (size_t i = 0; i < allPoints.size(); ++i) {
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

    // Query methods
    std::vector<Point> rangeQuery(const Point& queryMin, const Point& queryMax) const {
        std::vector<Point> result;
        rangeQueryRecursive(queryMin, queryMax, result);
        return result;
    }

    void rangeQueryRecursive(const Point& queryMin, const Point& queryMax, std::vector<Point>& result) const {
        // Check if this node's bounding box intersects with query range
        if (!boxIntersects(queryMin, queryMax)) {
            return;
        }

        // Check points in this node
        for (const auto& p : points) {
            if (p.x >= queryMin.x && p.x <= queryMax.x &&
                p.y >= queryMin.y && p.y <= queryMax.y &&
                p.z >= queryMin.z && p.z <= queryMax.z) {
                result.push_back(p);
            }
        }

        // Recursively search children
        for (const auto& [octant, child] : children) {
            child->rangeQueryRecursive(queryMin, queryMax, result);
        }
    }

    bool boxIntersects(const Point& queryMin, const Point& queryMax) const {
        return !(max.x < queryMin.x || min.x > queryMax.x ||
                 max.y < queryMin.y || min.y > queryMax.y ||
                 max.z < queryMin.z || min.z > queryMax.z);
    }

    // Get statistics about the octree
    void getStatistics(int& totalNodes, int& leafNodes, int& totalPoints, int& maxDepth, int currentDepth = 0) const {
        totalNodes++;
        totalPoints += points.size();
        maxDepth = std::max(maxDepth, currentDepth);

        if (isLeaf()) {
            leafNodes++;
        } else {
            for (const auto& [octant, child] : children) {
                child->getStatistics(totalNodes, leafNodes, totalPoints, maxDepth, currentDepth + 1);
            }
        }
    }

    void printStatistics() const {
        int totalNodes = 0, leafNodes = 0, totalPoints = 0, maxDepth = 0;
        getStatistics(totalNodes, leafNodes, totalPoints, maxDepth);
        
        std::cout << "=== Octree Statistics ===" << std::endl;
        std::cout << "Total nodes: " << totalNodes << std::endl;
        std::cout << "Leaf nodes: " << leafNodes << std::endl;
        std::cout << "Internal nodes: " << (totalNodes - leafNodes) << std::endl;
        std::cout << "Total points: " << totalPoints << std::endl;
        std::cout << "Maximum depth: " << maxDepth << std::endl;
        std::cout << "Average points per leaf: " << (leafNodes > 0 ? (float)totalPoints / leafNodes : 0) << std::endl;
    }
};

// Convenience typedef for the hashmap octree
using OctreeHashMap = OctreeHashMapNode;

#endif // OCTREE_HASHMAP_H 