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
    int value;
    int color;
    vector<int> adjacency;
    
};

struct Graph {
    int amountOfNodes;
    // int longestAdjacency;
    vector<Node> nodes;
};


char* concatFilepath(const char* filename) {
    const char* base = "./inputs/";
    char* fullpath = (char*) malloc(sizeof(char) * 256);
    strcpy(fullpath, base);
    strcat(fullpath, filename);
    return fullpath;
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
            // TODO Refactor
            // adds 0 node to the graph, makes working with the graph easier. start value = graph[start] and not start - 1
            while (graph.nodes.size() <= start) {
                Node node;
			    graph.nodes.push_back(node);
            }
		    graph.nodes[start].value = start;
            graph.nodes[start].color = 0;
            graph.nodes[start].adjacency.push_back(end);
            // in case of coloring we have to generate a undirected graph, no matter what form the original graph has
            while (graph.nodes.size() <= end) {
                Node node;
				graph.nodes.push_back(node);
			}
			graph.nodes[end].value = end;
            graph.nodes[end].color = 0;
            graph.nodes[end].adjacency.push_back(start);
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
/**
void longestAdjacency(Graph *graph) {
    long unsigned int max = 0;
    for (size_t i = 0; i < graph->nodes.size(); i++) {
		if(graph->nodes[i].adjacency.size() > max) {
            // printf("adjacence: %li\n", graph->adjacency[i].size());
            max = graph->nodes[i].adjacency.size();
            // printf("inner max is %li\n", max);
        }
	}
    graph->longestAdjacency = max;
}
*/
void everyNodeGetsColor(int key, vector<int> &clauses) {
    // To remove the existing 0 from the previous iteration
    if(!clauses.empty()) {
        clauses.pop_back();
    }
    clauses.push_back(key);
    clauses.push_back(0);
}

void adjacencyHaveDiffColor(vector<int> adjacency, int maxNodes, int key, int color, vector<vector<int>> &clauses) {
    vector<int> newClause;
    for(int adjaNode : adjacency) {
        newClause.push_back(-1 * key);
        newClause.push_back(-1 * (adjaNode + ((color - 1) * maxNodes)));
        newClause.push_back(0);
        clauses.push_back(newClause);
    }
}

void atMostOne(int maxNodes, int key, int color, vector<vector<int>> &clauses) {
    for(int i = 1; i < color; i++) {
        vector<int> newClause;
        newClause.push_back(-1 * key);
        newClause.push_back(-1 * (key - (i * maxNodes)));
        newClause.push_back(0);
        clauses.push_back(newClause);
    }
}

void getColoring(Graph &graph) {
    // key, node, color
    map<int, vector<int>> variables;
    bool notSatisfiable = false;
    int color = 1;
    int key = 1;
    // nodes.size() - 1 because we add the 0 node to the graph so we have to ignore it here.
    // if no coloration was found or if the max amount of colors is not reached we can search for a new coloration
    while(!notSatisfiable && color <= 4) {
        for(size_t i = 1; i < graph.nodes.size(); i++) {
            variables.insert({key, vector<int>{graph.nodes[i].value, color}});
            // required to simulate the first ending 0 of the first clause
            vector<int> everyNodeGetsColorClauses;
            vector<vector<int>> adjacencyHaveDiffColorClauses;
            vector<vector<int>> atMostOneClauses;
            // Generate clauses for iteration
            // Every node gets a color
            everyNodeGetsColor(key, everyNodeGetsColorClauses);
            // Adjacent nodes have diff color
            adjacencyHaveDiffColor(graph.nodes[i].adjacency, graph.amountOfNodes, key, color, adjacencyHaveDiffColorClauses);
            // At-most-one
            atMostOne(graph.amountOfNodes, key, color, atMostOneClauses);
            // TODO Add clauses to ipasir
            key++;
        }
        color++;
    }
    printf("---");
}

int main(int argc, char **argv) {
    // maximum range the algortihmn should search for a coloration
    const char* filename = argv[1];
    Graph graph = graphInit(filename);
    printGraph(graph);
    // longestAdjacency(&graph);
    // void* solver = ipasir_init();
    // At the moment only a maximum amount of 10 colors is possible. Change if better idea.
    getColoring(graph);
}