#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <cstring>
extern "C" {
    #include "ipasir.h"
}

using namespace std;

# define maxColors 10

struct Node {
    int value;
    int color;
    vector<int> adjacency;
    
};

struct Graph {
	long unsigned int longestAdjacency;
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
            while (graph.nodes.size() <= start) {
                Node node;
			    graph.nodes.push_back(node);
            }
		    graph.nodes[start].value = start;
            graph.nodes[start].color = 0;
            graph.nodes[start].adjacency.push_back(end);
        }
    } 
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



void getColoring(Graph &graph) {
    bool notSatisfiable = false;
    int color = 1;
    vector<int> everyNodeGetsColorClauses;
    vector<vector<int>> adjacencyHaveDiffColorClauses;
    vector<vector<int>> atMostOneClauses;

    
    while(!notSatisfiable) {
        for(Node node : graph.nodes) {
            int baseValue = node.value * maxColors;
            // Generate clauses for iteration
            // Every node gets a color
            everyNodeGetsColor(node, color, everyNodeGetsColorClauses);
            // Adjacent nodes have diff color
            adjacencyHaveDiffColor(node, color, adjacencyHaveDiffColorClauses);
            // At-most-one
            atMostOne(node, color, atMostOneClauses);

            //TODO  Check if vectors really change otherwise just return
        }
        
        color++;
    }
}
// DONE
void everyNodeGetsColor(Node node, int color, vector<int> clauses) {
    // To remove the existing 0 from the previous iteration
    clauses.pop_back();
    clauses.push_back((node.value * maxColors) + color);
    clauses.push_back(0);
}

void adjacencyHaveDiffColor(Node node, int color, vector<vector<int>> clauses) {
    vector<int> newClause;
    for(int adjaNode : node.adjacency) {
        newClause.push_back(-1 * ((node.value * maxColors) + color));
        newClause.push_back(-1 * ((adjaNode * maxColors) + color));
        newClause.push_back(0);
        clauses.push_back(newClause);
    }
}

void atMostOne(Node node, int color, vector<vector<int>> clauses) {
    vector<int> newClause;
    for(int i = 1; i < color; i++) {
        newClause.push_back(-1 * ((node.value * maxColors) + i));
        newClause.push_back(-1 * ((node.value * maxColors) + color));
        newClause.push_back(0);
    }
    clauses.push_back(newClause);
}

int main(int argc, char **argv) {
    // maximum range the algortihmn should search for a coloration
    const char* filename = argv[1];
    Graph graph = graphInit(filename);
    printGraph(graph);
    // longestAdjacency(&graph);
    void* solver = ipasir_init();
    printf("Min amount of colors needed is %li\n", graph.longestAdjacency + 1);
    // At the moment only a maximum amount of 10 colors is possible. Change if better idea.
    getColoring(graph);
}