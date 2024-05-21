// * <Kerem Er>

#include <iostream>         
#include <vector>
#include <fstream>
#include <sstream>       
#include <stack>
#include <queue>
#include <chrono>       // For time measurement

#include <algorithm> // For sort() algorithm

using namespace std;
using namespace std::chrono;

vector<vector<bool>> visited; // Global variable to track visited planets

void initVisited(const vector<vector<int>>& map) {
    visited.assign(map.size(), vector<bool>(map[0].size(), false));
}

// Makes the map circular
pair<int, int> getWrappedIndices(int row, int col, const vector<vector<int>>& map) {
    int wrappedRow = (row + map.size()) % map.size();
    int wrappedCol = (col + map[0].size()) % map[0].size();
    return {wrappedRow, wrappedCol};
}

bool colonyComparator(const pair<int, int>& a, const pair<int, int>& b) {
    if (a.first == b.first) { // If resources are equal
        return a.second < b.second;
    }
    return a.first > b.first; // Otherwise, compare by size
}

/**
 * Perform Depth-First Search (DFS) traversal on a map.
 * 
 * @param map The map represented as a 2D vector of integers.
 * @param row The starting row index for the DFS traversal.
 * @param col The starting column index for the DFS traversal.
 * @param resource The resource value to search for during the traversal.
 * @return The size of the colony found during the DFS traversal.
 */
int dfs(vector<vector<int>>& map, int row, int col, int resource) {

    // Handles the map boundaries
    pair<int, int> wrapped = getWrappedIndices(row, col, map);
    row = wrapped.first;
    col = wrapped.second;

    // Mark planets with visited vector
    if (visited[row][col] || map[row][col] != resource) return 0;
    visited[row][col] = true;

    // Calculating the size of the colony while traversing
    int size = 1;
    size += dfs(map, row - 1, col, resource); // Up
    size += dfs(map, row + 1, col, resource); // Down
    size += dfs(map, row, col - 1, resource); // left
    size += dfs(map, row, col + 1, resource); // Right

    return size;

}


/**
 * Perform Breadth-First Search (BFS) traversal on a map.
 * 
 * @param map The map represented as a 2D vector of integers.
 * @param row The starting row index for the BFS traversal.
 * @param col The starting column index for the BFS traversal.
 * @param resource The resource value to search for during the traversal.
 * @return The size of the colony found during the BFS traversal.
 */
int bfs(vector<vector<int>>& map, int row, int col, int resource) {

    // Initializing a queue to manage the planets
    queue<pair<int, int>> q;
    pair<int, int> wrapped = getWrappedIndices(row, col, map); // Handling boundaries

    // If already visited or doesnt have the same resource
    if (visited[wrapped.first][wrapped.second] || map[wrapped.first][wrapped.second] != resource) return 0;

    // Marking the starting cell visited
    q.push(wrapped);
    visited[wrapped.first][wrapped.second] = true;
    int size = 0;

    // Until there are no more planets to visit 
    while (!q.empty()) {
        pair<int, int> front = q.front(); q.pop(); // Get the planet at the front of the queue and remove it from the queue
        size++;

        // Calculates the wrapped planets
        vector<pair<int, int>> neighbors = {
            getWrappedIndices(front.first - 1, front.second, map),
            getWrappedIndices(front.first + 1, front.second, map),
            getWrappedIndices(front.first, front.second - 1, map),
            getWrappedIndices(front.first, front.second + 1, map)
        };

        // Itarates each neighbor
        for (auto& neighbor : neighbors) {
            int nr = neighbor.first, nc = neighbor.second;
            if (!visited[nr][nc] && map[nr][nc] == resource) { // If a neighbor is not visited and has the same target resource
                visited[nr][nc] = true; // Marks as visited
                q.push(neighbor); 
            }
        }
    }

    return size;

}

/**
 * Finds the top-k largest colonies in a given map.
 *
 * @param map The map represented as a 2D vector of integers.
 * @param useDFS A boolean flag indicating whether to use Depth-First Search (DFS) or Breadth-First Search (BFS) algorithm.
 * @param k The number of top colonies to find.
 * @return A vector of pairs representing the size and resource type of the top-k largest colonies.
 */
vector<pair<int, int>> top_k_largest_colonies(vector<vector<int>>& map, bool useDFS, unsigned int k) {

    auto start = high_resolution_clock::now();      // Start measuring time
    
    if (map.empty() || map[0].empty()) {
        return {}; // Return an empty vector if the map is empty
    }

    initVisited(map); // Initialize the visited vector
    vector<pair<int, int>> colonySizes; // Define a vector to hold the sizes and resource types

    // Itarates every cell
    for (size_t i = 0; i < map.size(); ++i) {
        for (size_t j = 0; j < map[i].size(); ++j) {
            if (!visited[i][j] && map[i][j] > 0) {
                int resourceType = map[i][j]; // Store the resource type before it gets marked as visited
                int size = (useDFS) ? dfs(map, i, j, resourceType) : bfs(map, i, j, resourceType);
                colonySizes.emplace_back(size, resourceType);
            }
        }
    }

    // Sort the colonies by size in descending order and keep the top k
    sort(colonySizes.begin(), colonySizes.end(), colonyComparator);
    if (colonySizes.size() > k) colonySizes.resize(k); // Resizes the colonies to only top-k of them

    auto stop = high_resolution_clock::now();                       // Stop measuring time
    auto duration = duration_cast<nanoseconds>(stop - start);       // Calculate the duration
    cout << "Time taken: " << duration.count() << " nanoseconds" << endl;

    return colonySizes;

}

// Main function
int main(int argc, char* argv[]) {

    // Check the command line arguments
    if (argc != 4) {

        // It should be: <1 or 0> <k> <filename.txt>
        cerr << "Usage: " << argv[0] << " <1 or 0> <k> <filename.txt>" << endl;
        return 1;
    }

    // Get the arguments
    bool useDFS = (atoi(argv[1]) == 1);
    int k = atoi(argv[2]);

    // Read the map from the text file
    string filename = argv[3];
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Unable to open file." << endl;
        return 1;
    }

    vector<vector<int>> sector_map;
    string line;
    while (getline(file, line)) {
        istringstream iss(line);
        vector<int> row;
        int num;
        while (iss >> num) {
            row.push_back(num);
        }
        sector_map.push_back(row);
    }
    file.close();   

    // Find the largest colony
    vector<pair<int, int>> result = top_k_largest_colonies(sector_map, useDFS, k);

    // Print the result
    cout << "Algorithm: " << (useDFS ? "DFS" : "BFS") << endl;
    cout << "Map: " << filename << endl;

    // If no colonies are found
    if (result.empty()) {
        cout << "No colonies found." << endl;
        return 0;
    }

    // Print the top-k colonies
    for (size_t i = 0; i < result.size(); ++i) {
        cout << "Colony " << i + 1 << ": Size = " << result[i].first << ", Resource Type = " << result[i].second << endl;
    }

    return 0;
}