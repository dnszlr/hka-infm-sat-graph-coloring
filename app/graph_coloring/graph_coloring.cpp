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
    // adds 0 node to the graph, makes working with the graph easier. node value = graph[node] and not [node - 1]
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
        filename = (char*) "small.txt";
        printf("small.txt was used because no Filename was passed to the program\n");
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

// TODO Maybe make a function that calculates the original key to the keys from current iteration
// decode
// key - ((colorIteration - 1) * (maxNodeValue * 2))
// encode
// nodeValue + ((colorIteration - 1) * (maxNodeValue * 2))

void everyNodeGetsColor(int maxNodes, int key, int color, vector<vector<int>> &clauses) {
    // Find the origin value for a given key
    size_t origin = (key - ((color - 1) * (maxNodes * 2))) - 1;
    while(clauses.size() <= origin) {
        clauses.push_back(vector<int>());
    }
    // To remove the existing 0 and assumption from the previous iteration
    if(!clauses[origin].empty()) {
        clauses[origin].pop_back();
        clauses[origin].pop_back();
    }
    clauses[origin].push_back(key);
    // Assumption variable (used to remove a this clause in a future iteration)
    // printf("key+max %i \n", key + maxNodes);
    clauses[origin].push_back(key + maxNodes);
    clauses[origin].push_back(0);
}

void adjacencyHaveDiffColor(vector<int> adjacency, int maxNodes, int key, int color, vector<vector<int>> &clauses) {
    int doubleMaxNodes = maxNodes * 2;
    for(int adjaNode : adjacency) {
        vector<int> newClause;
        newClause.push_back(-1 * key);
        newClause.push_back(-1 * (adjaNode + ((color - 1) * doubleMaxNodes)));
        newClause.push_back(0);
        clauses.push_back(newClause);
    }
}

void atMostOne(int maxNodes, int key, int color, vector<vector<int>> &clauses) {
    int doubleMaxNodes = maxNodes * 2;
    for(int i = 1; i < color; i++) {
        vector<int> newClause;
        newClause.push_back(-1 * (key - (i * doubleMaxNodes)));
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


map<int, vector<int>> getColoring(Graph &graph, void * solver) {
    // map <key, <node, color>>
    map<int, vector<int>> variables;
    bool satisfiable = false;
    int color = 1;
    int key = 1;
    vector<vector<int>> everyNodeGetsColorClauses;
    vector<vector<int>> adjacencyHaveDiffColorClauses;
    vector<vector<int>> atMostOneClauses;
    // if no coloration was found we continue searching
    while(!satisfiable) {
        printf("Color is %i: \n", color);
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
        color = satisfiable ? color : color + 1;
        // printf("key before %i \n", key);
        // All keys used for assumtions have to be skipped
        key = key + graph.amountOfNodes;
        // printf("key after %i \n", key);
    }
    return variables;
}

// If needed more colors can be added, just helps with the representation
string colorDecoding(int colorValue) {
    string color;
    switch(colorValue) {
        case 1:
            color = "Yellow";
            break;
        case 2:
            color = "Red";
            break;
        case 3:
            color = "Green";
            break;
        case 4:
            color = "Purple";
            break;
        case 5:
            color = "Blue";
            break;
        case 6:
            color = "Maroon";
            break;
        case 7:
            color = "Lime";
            break;
        case 8:
            color = "Gold";
            break;
        case 9:
            color = "Aqua";
            break;
        case 10:
            color = "Teal";
            break;
        case 11:
            color = "Olive";
            break;
        case 12:
            color = "Navy";
            break;
        case 13:
            color = "Violet";
            break;
        case 14:
            color = "Fuchsia";
            break;
        case 15:
            color = "Indigo";
            break;
        case 16:
            color = "Cyan";
            break;
        case 17:
            color = "Tomato";
            break;
        case 18:
            color = "Turquoise";
            break;
        case 19:
            color = "White";
            break;
        case 20:
            color = "Black";
            break;
        default:
            color = "Ups, something went wrong!";
    }
    return color;
}

void printOutResult(map<int, vector<int>> variables, void * solver) {
    for(auto entry : variables) {
        int value = ipasir_val(solver, entry.first);
        if(value > 0) {
            printf("Node %i has color %s\n", entry.second[0], colorDecoding(entry.second[1]).c_str());
        }
    }
}

int main(int argc, char **argv) {
    const char* filename = argv[1];
    Graph graph = graphInit(filename);
    printGraph(graph);
    void* solver = ipasir_init();
    map<int, vector<int>> variables = getColoring(graph, solver);
    printOutResult(variables, solver);
}