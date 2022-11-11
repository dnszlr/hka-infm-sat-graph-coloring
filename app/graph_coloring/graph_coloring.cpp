#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <cstring>
extern "C" {
    #include "ipasir.h"
}

using namespace std;

/**
- Aufgabe: file:///D:/Studium/Master/MI2/SAT%20Solving/%C3%9Cbungsbl%C3%A4tter/Blatt_1.pdf
Ideen:
- Dann immer eine Farbe (Farbe = Integer) hinzufügen und den letzten noch möglichen Stand speichern
- Klauseln: file:///D:/Studium/Master/MI2/SAT%20Solving/Folien/l01.pdf Seite 21-22
- Wie am besten die vorhandenen Variablen verwalten?
**/

struct Graph {
	long unsigned int longestAdjacency;
    vector<vector<int>> adjacency;
};

char* concatFilepath(const char* filename) {
    const char* base = "./inputs/";
    char* fullpath = (char*) malloc(sizeof(char) * 1000);
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
            while (graph.adjacency.size() <= start) {
			    graph.adjacency.push_back(vector<int>());
            }
		    graph.adjacency[start].push_back(end);
        }
    } 
    fclose(file);
    return graph;
}

void printGraph(const Graph &graph) {
    printf("Graph:\n");
	for (size_t i = 0; i < graph.adjacency.size(); i++) {
		for (size_t j = 0; j < graph.adjacency[i].size(); j++) {
			printf("e %lu %d\n", i, graph.adjacency[i][j]);
		}
	}
}

void longestAdjacency(Graph *graph) {
    long unsigned int max = 0;
    for (size_t i = 0; i < graph->adjacency.size(); i++) {
		if(graph->adjacency[i].size() > max) {
            // printf("adjacence: %li\n", graph->adjacency[i].size());
            max = graph->adjacency[i].size();
            // printf("inner max is %li\n", max);
        }
	}
    graph->longestAdjacency = max;
}

int main(int argc, char **argv) {
    const char* filename = argv[1];
    Graph graph = graphInit(filename);
    printGraph(graph);
    // Min amount of colors needed is the longest Adjacency for all neighbours + 1 for the current node.
    longestAdjacency(&graph);
    printf("Min amount of colors needed is %li\n", graph.longestAdjacency + 1);
}