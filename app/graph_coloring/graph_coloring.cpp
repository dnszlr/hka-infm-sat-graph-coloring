#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <map>
#include <iostream>
#include <cstring>
extern "C" {
    #include "ipasir.h"
}

using namespace std;

struct Node {
    int value = 0;
    int color = 0;
    vector<int> adjacency;
    
};

struct Graph {
    int amountOfNodes;
    vector<Node> nodes;
};


char* concatFilepath(const char* filename) {
    const char* base = "./inputs/";
    char* fullpath = (char*) malloc(sizeof(char) * 256);
    strcpy(fullpath, base);
    strcat(fullpath, filename);
    return fullpath;
}

void createNodes(int node, int neighbour, Graph &graph) {
    size_t max = node > neighbour ? node : neighbour;
    // adds 0 node to the graph, makes working with the graph easier. start value = graph[start] and not [start - 1]
    while (graph.nodes.size() <= max) {
        Node node;
	    graph.nodes.push_back(node);
	}
    // Node
	graph.nodes[node].value = node;
    graph.nodes[node].adjacency.push_back(neighbour);
    // Neighbour node
    graph.nodes[neighbour].value = neighbour;
}


Graph graphInit(const char* filename) {
    // TODO Only for Debug reasons, remove if no more needed.
    if(filename == NULL) {
        filename = (char*) "test.txt";
        printf("test.txt was used because no Filename was passed to the program\n");
    }
    filename = concatFilepath(filename);
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
		printf("File '%s' cannot be opened\n", filename);
		exit(1);
	}
    Graph graph;
    int current = 0;

    while (current != EOF) {

        current = fgetc(file);
        // Select the type of the currently viewed row:
        // Comment or Problem row, could be made more safe with more time.
		if (current == 'c' || current == 'p') {
			// skip this line
			while(current != '\n' && current != EOF) {
				current = fgetc(file);
			}
			continue;
		}
        // Edge row
        if(current == 'e') {
            char edge[256];
			int i = 0;
            // EOF is nessecary in case of no line break at the end of the file.
			while (current != '\n' && current != EOF) {
				edge[i] = current;
				current = fgetc(file);
                i++;
			}
			edge[i] = 0;
            size_t start, end;
            sscanf(edge, "e %lu %lu", &start , &end);
            createNodes(start, end, graph);
            // in case of coloring we have to generate a undirected graph, no matter what form the original graph has
            // createNodes(end, start, graph);
        }
    }
    // Since we push back Node 0 we have to ignore this one in the counting
    graph.amountOfNodes = graph.nodes.size() - 1; 
    fclose(file);
    return graph;
}

void printGraph(const Graph &graph) {
    printf("Graph:\n");
	for (size_t i = 0; i < graph.nodes.size(); i++) {
		for (size_t j = 0; j < graph.nodes[i].adjacency.size(); j++) {
			printf("e %lu %d\n", i, graph.nodes[i].adjacency[j]);
		}
	}
}
// Man muss die alten Klauseln erweitern
void everyNodeGetsColor(int maxNodes, int key, int color, vector<vector<int>> &clauses) {
    // Find the origin value for a given key
    size_t origin = (key - ((color - 1) * maxNodes)) - 1;
    while(clauses.size() <= origin) {
        clauses.push_back(vector<int>());
    }
    // To remove the existing 0 from the previous iteration
    if(!clauses[origin].empty()) {
        clauses[origin].pop_back();
        clauses[origin].pop_back();
    }
    clauses[origin].push_back(key);
    // Assumption
    clauses[origin].push_back(key + (50 * maxNodes));
    clauses[origin].push_back(0);
}
// Man muss einfach neue Klauseln erstellen und dem Solver übergeben
void adjacencyHaveDiffColor(vector<int> adjacency, int maxNodes, int key, int color, vector<vector<int>> &clauses) {
    for(int adjaNode : adjacency) {
        vector<int> newClause;
        newClause.push_back(-1 * key);
        newClause.push_back(-1 * (adjaNode + ((color - 1) * maxNodes)));
        newClause.push_back(0);
        clauses.push_back(newClause);
    }
}
// Stimmt so, lediglich einfach immer ein neuen Vector erstellen?
void atMostOne(int maxNodes, int key, int color, vector<vector<int>> &clauses) {
    for(int i = 1; i < color; i++) {
        vector<int> newClause;
        newClause.push_back(-1 * (key - (i * maxNodes)));
        newClause.push_back(-1 * key);
        newClause.push_back(0);
        clauses.push_back(newClause);
    }
}

void addClausesToSolver(vector<vector<int>> clauses, void * solver) {
    for(vector<int> clause : clauses) {
        for(int index : clause) {
            // printf("%i ", index);
            ipasir_add(solver, index);
        }
        // printf("\n");
    }
}

void getAssumption(void * solver, vector<vector<int>> everyNodeGetsColorClauses) {
    for(vector<int> clause : everyNodeGetsColorClauses) {
        // Not clause.size() - 1 because last element is always the zero
        ipasir_assume(solver, -1 * (clause[clause.size() - 2]));
        // printf("Assumption is: %i: \n", -1 * (clause[clause.size() - 2]));
    }
}


void getColoring(Graph &graph, void * solver) {
    // map <key, <node, color>>
    map<int, vector<int>> variables;
    bool satisfiable = false;
    int color = 1;
    int key = 1;
    vector<vector<int>> everyNodeGetsColorClauses;
    vector<vector<int>> adjacencyHaveDiffColorClauses;
        vector<vector<int>> atMostOneClauses;
    // if no coloration was found or if the max amount of colors is not reached we can search for a new coloration
    // TODO Remove 4, only for dev reasons while no solver is included
    while(!satisfiable) {
        printf("Color is %i: \n", color);
        // Vectors hold the clauses for each color iteration
        for(size_t i = 1; i < graph.nodes.size(); i++) {
            variables.insert({key, vector<int>{graph.nodes[i].value, color}});
            // Generate clauses for iteration
            // Every node gets a color
            everyNodeGetsColor(graph.amountOfNodes, key, color, everyNodeGetsColorClauses);
            // Adjacent nodes have diff color
            adjacencyHaveDiffColor(graph.nodes[i].adjacency, graph.amountOfNodes, key, color, adjacencyHaveDiffColorClauses);
            // At-most-one
            atMostOne(graph.amountOfNodes, key, color, atMostOneClauses);
            key++;
        }
        addClausesToSolver(everyNodeGetsColorClauses, solver);
        addClausesToSolver(adjacencyHaveDiffColorClauses, solver);
        addClausesToSolver(atMostOneClauses, solver);
        getAssumption(solver, everyNodeGetsColorClauses);
        int result = ipasir_solve(solver);
        printf("The result of solve is %i: \n", result);
        satisfiable = result == 10;
        color++;
    }
    printf("---");
}

int main(int argc, char **argv) {
    // maximum range the algortihmn should search for a coloration
    const char* filename = argv[1];
    Graph graph = graphInit(filename);
    printGraph(graph);
    void* solver = ipasir_init();
    // At the moment only a maximum amount of 10 colors is possible. Change if better idea.
    getColoring(graph, solver);
    // getColoring(graph);
}