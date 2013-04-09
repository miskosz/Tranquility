/**
 * Gets graphs in plantri ascii_code on input.
 * Outputs sql INSERT statement inserting the graph with computed triangulations
 * into a database.
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <climits>

#include <vector>
#include <sstream>
#include <iostream>
#include <map>
#include <algorithm>
using namespace std;

typedef unsigned char byte;

struct STRUCT_SETTINGS {
	bool debug, help, sql, sizes_only, graph;
	char *token;
	int progress;
} settings;

#define MAX_VERTICES 30

// the graph
// 3-connected planar Eulerian triangulation
byte nVertices;
/// every edge is listed twice, (3v-6) edges, (v-1) separating commas, \0
char ascii_code[7*MAX_VERTICES];
vector< vector<byte> > graph;
vector<byte> color;

// matrices
// nBlackFaces = nWhiteFaces = nVertices-2
int gem[MAX_VERTICES][2*MAX_VERTICES];
int faces[2][MAX_VERTICES][MAX_VERTICES];
byte faces_arr[2][MAX_VERTICES][3];
char faces_str[2][4*MAX_VERTICES];

// sql inserts
class SqlClass {
	int counter;
	char graph_insert_comma, triangulation_insert_comma;
	char id_graph[50];

public:
	stringstream graph_insert, triangulation_insert;
	char *token;
	
	SqlClass() {
		counter = 0;
		graph_insert <<
			"INSERT INTO graph (id_graph, nvertices, ascii_code, black_faces,"
			" white_faces, last_computed) VALUES\n";
		triangulation_insert <<
			"INSERT INTO triangulation (id_graph, vertex_color, size,"
			" degenerate, coordinates) VALUES\n";	
		graph_insert_comma = ' ';
		triangulation_insert_comma = ' ';
		token = (char *)"";
	}
	
	void add_graph() {
		// make up id_graph
		sprintf(id_graph, "%d%s%d", nVertices, token, counter++);

		// compose insert statement
		graph_insert << graph_insert_comma << "('" << id_graph << "', "
			<< (int)nVertices << ", '" << ascii_code << "', '" << faces_str[0]
			<< "', '" << faces_str[1] << "', now())" << endl;
		if (graph_insert_comma != ',') graph_insert_comma = ',';
	}

	void add_triangulation(int p_color, int i, int *solution, bool degenerate) {
		// prepare insert
		triangulation_insert << triangulation_insert_comma << "('" << id_graph
			<< "', " << (p_color ? "'white', " : "'black', ") << gem[i][i]
			<< ", " << degenerate << ", '" << solution[0];
		for (int j = 1; j < nVertices; j++) {
			triangulation_insert << " " << solution[j];
		}
		triangulation_insert << "')\n";
		if (triangulation_insert_comma != ',') triangulation_insert_comma = ',';
	}	
} sql;

// solution container
class SolutionHandler {
	map<int, pair<int, string> > solutions;
	stringstream output;
	
public:

	bool is_better(int i) {
		int size = gem[i][i];
		
		// if we have don't have a solution or existing solution is worse
		return solutions.find(size) == solutions.end() ||
			solutions[size].first > nVertices;
	}

	void save_if_better(int p_color, int i, int *solution, bool degenerate) {
		int size = gem[i][i];
		int n = nVertices;
		
		// check if better
		if (degenerate || !is_better(i)) {
			return;
		}
		
		// clear the stringstream
		output.str(string());
		
		// put in the data
		output << size << ' ' << n;
		if (!settings.sizes_only) {	
			output << ' ';
			if (settings.graph) {
				output << ascii_code << ' ';
			}
			output <<
				faces_str[p_color^1] << ' ' <<
				solution[0];
			for (int j = 1; j < nVertices; j++) {
				output << ',' << solution[j];
			}
		}
		
		// save
		solutions[size] = make_pair(n, output.str());
	}
	
	void print() {
		map<int, pair<int, string> >::iterator it;
		for (it = solutions.begin(); it != solutions.end(); ++it) {
			printf("%s\n", it->second.second.c_str());
		}
	}
} solution_handler;

// The graph is 3-colorable. Colors the graph using dfs.
// Takes on input the vertex to be colored, its already
// colored neighbor and a color to use.
void color3(byte v, byte colored_neighbor, byte p_color) {
	// color the vertex v
	if (color[v] != (byte)-1) {
		return;
	}
	color[v] = p_color;

	// find the index of the colored neighbor
	int i = 0;
	while (graph[v][i] != colored_neighbor) i++;

	// color the neighbors of v alternately
	byte col[2];
	col[0] = color[colored_neighbor];
	col[1] = 3 - col[0] - p_color;
	int n = graph[v].size();
	for (int j = 1; j < n; j++) {
		color3(graph[v][(i+j)%n], v, col[j%2]);
	}
}

void color3() {
	color.clear();
	color = vector<byte>(nVertices, (byte)-1);

	// assign color such that 0 has color 0 and 1 has color 1
	color[0] = 0;
	color3(1, 0, 1);
}

// Constructs matrices for faces.
// Every column is a vertex, every row is an incidence vector for faces,
// thus the matrix has nVertices columns and nVertices-2 rows.
// To the "right" from an edge with end colors 0->1 is a black face,
// to the "right" from an edge with end colors 0->2 is a white face.
void get_face_matrices() {
	memset(faces, 0, sizeof(faces));
	memset(faces_arr, 0, sizeof(faces_arr));
	int face_index[2] = {0,0};

	for (int v = 0; v < nVertices; v++) {
		if (color[v] != 0) continue;
		
		byte face_color;
		if (color[graph[v][0]] == 1) face_color = 0;
		else face_color = 1;

		int n = graph[v].size();
		for (int i = 0; i < n; i++) {
			// set matrix values
			faces[face_color][face_index[face_color]][v] = 1;
			faces[face_color][face_index[face_color]][graph[v][i]] = 1;
			faces[face_color][face_index[face_color]][graph[v][(i+1)%n]] = 1;

			// store face vertices in an array
			faces_arr[face_color][face_index[face_color]][0] = v;
			faces_arr[face_color][face_index[face_color]][1+face_color] = graph[v][i];
			faces_arr[face_color][face_index[face_color]][2-face_color] = graph[v][(i+1)%n];

			face_index[face_color]++;
			face_color ^= 1;
		}
	}
	
	// parse into an ascii_code string for database
	for (int col = 0; col < 2; col++) {
		for (int j = 0; j < nVertices-2; j++) {
			for (int k = 0; k < 3; k++) {
				faces_str[col][4*j + k] = faces_arr[col][j][k] + 'a';
			}
			faces_str[col][4*j + 3] = ',';
		}
		faces_str[col][4*(nVertices-2) -1] = '\0';
	}
}

int gcd(int a, int b) {
	return b == 0 ? a : gcd(b, a%b);
}

void divide_by_gcd(int n) {
	// divide rows by gcd
	for (int j = 0; j < n; j++) {
		
		int g = 0;
		for (int i = 0; i < 2*n; i++)
			g = gcd(g, abs(gem[j][i]));
		if (gem[j][j] < 0) g *= -1;
		
		if (g != 1 && g != 0) {
			if (settings.debug) fprintf(stderr, "Dividing row %d by %d.\n", j, g);
			for (int i = 0; i < 2*n; i++)
				gem[j][i] /= g;
		}
	}

}

int gauss_elimination(int n) {
	int buffer[2*MAX_VERTICES];

	// for all rows
	for (int j = 0; j < n; j++) {

		// make sure gem[j][j] != 0
		if (gem[j][j] == 0) {
			if (settings.debug) fprintf(stderr, "Swapping row %d.\n", j);
			int k = j+1;
			while (k < n && gem[k][j] == 0) k++;
			if (k == n) {
				fprintf(stderr, "The system of equations is not regular!\n");
				return 1;
			}
			// swap the rows
			memcpy(buffer, gem[j], sizeof(buffer));
			memcpy(gem[j], gem[k], sizeof(buffer));
			memcpy(gem[k], buffer, sizeof(buffer));
		}

		// eliminate j-th column
		for (int k = 0; k < n; k++) {
			if (k == j || gem[k][j] == 0) continue;
			for (int i = 0; i < j; i++)
				gem[k][i] *= gem[j][j];
			for (int i = j+1; i < 2*n; i++)
				gem[k][i] = gem[j][j]*gem[k][i] - gem[k][j]*gem[j][i];
			gem[k][j] = 0;
		}

		divide_by_gcd(n);
	}

	if (settings.debug) {
		printf("\nGaussovka:\n");
		for (int i = 0; i < nVertices-2; i++) {
			for (int j = 0; j < 2*nVertices-4; j++)
				printf("%2d ", gem[i][j]);
			printf("\n");
		}
		printf("\n");
	}

	return 0;
}

int find_solution(int p_color) {
	// prepare matrix for Gauss elimination method
	memset(gem, 0, sizeof(gem));

	// the left side is a transpose of the face matrix whithout the first two columns
	for (int i = 0; i < nVertices-2; i++)
		for (int j = 0; j < nVertices-2; j++)
			gem[i][j] = faces[p_color][j][i+2];

	// the right side is the identity matrix
	for (int i = 0; i < nVertices-2; i++)
		gem[i][i+nVertices-2] = 1;

	// eliminate
	gauss_elimination(nVertices-2);

	// adjust and print the solution
	int solution[MAX_VERTICES];

	for (int i = 0; i < nVertices-2; i++) {
		// cutting off unnecessary checks
		if (!settings.sql && !solution_handler.is_better(i)) {
			continue;
		}
	
		// copy to int array
		solution[0] = solution[1] = 0;
		memcpy(solution+2, &gem[i][nVertices-2], (nVertices-2)*sizeof(int));
		
		// verify correctness of the solution and determine if degenerate
		for (int j = 0; j < nVertices-2; j++) {
			if (j != i && solution[faces_arr[p_color][j][0]] + solution[faces_arr[p_color][j][1]] + solution[faces_arr[p_color][j][2]] != 0) {
				fprintf(stderr, "Error: Gauss elimination method failed. (Probably overflow issues.) j = %d\n", j);
				return 1;
			}
		}
		
		// is degenerate if multiple vertices coincide
		bool degenerate = false;
		map<pair<int, int>, bool> in_solution;
		for (int j = 0; j < nVertices-2; j++) {
			pair<int, int> point(solution[faces_arr[p_color][j][0]], solution[faces_arr[p_color][j][1]]);
			if (in_solution[point]) {
				degenerate = true;
				break;
			}
			in_solution[point] = true;
		}
		
		// make (0,0,-n) bottom left corner
		int shift[3];
		shift[0] = solution[faces_arr[p_color][i][0]];
		shift[1] = solution[faces_arr[p_color][i][1]];
		shift[2] = -shift[0] - shift[1];
		for (int j = 0; j < nVertices; j++)
			solution[j] -= shift[color[j]];
		
		// sql
		if (settings.sql) {
			sql.add_triangulation(p_color, i, solution, degenerate);
		} else {
			solution_handler.save_if_better(p_color, i, solution, degenerate);
		}
	}
	
	return 0;
}

int load_graph() {
	// read the number of vertices
	if (scanf(" %d", &nVertices) != 1) {
		// if fails, it must be EOF
		return 1;
	}
	if (nVertices > MAX_VERTICES) {
		fprintf(
			stderr,
			"The program does not support more than %d vertices.\n",
			MAX_VERTICES
		);
		return 2;
	}

	// read ascii_code of the graph
	scanf(" %s", ascii_code);
	
	// check that vertex 1 is the first neighbor of 0
	if (ascii_code[0] != 'b') {
		fprintf(
            stderr,
            "Error: Data corruption - the first neighbor of 'a' must be 'b'.\n"
        );
		return 2;
	}
	
	// construct the graph, numbering the vertices from 0
	graph.clear();
	graph = vector< vector<byte> >(nVertices);
	int pos = -1;
	for (int v = 0; v < nVertices; v++) {
		for (pos++; ascii_code[pos] != ',' && ascii_code[pos] != '\0'; pos++) {
			graph[v].push_back(ascii_code[pos]-'a');
		}
	}
	
	return 0;
}

int main(int argc, char *argv[]) {
	
	// extract command line arguments
	for (int i = 0; i < argc; i++) {
		string s(argv[i]);
		if (s == "-debug") {
			settings.debug = true;
		} else if (s == "-h" || s == "-help" || s == "--help") {
			settings.help = true;
		} else if (s == "-sql") {
			settings.sql = true;
			i++;
			if (i >= argc || argv[i][0] == '-') {
				settings.help = true;
			} else {
				sql.token = argv[i];
			}
		} else if (s == "-sizes-only") {
			settings.sizes_only = true;
		} else if (s == "-graph") {
			settings.graph = true;
		} else if (s == "-progress") {
			i++;
			if (i >= argc || argv[i][0] == '-') {
				settings.help = true;
			} else {
				settings.progress = atoi(argv[i]);
			}
		}
	}
	
	// help message
	if (settings.help) {
		fprintf(stderr, "USAGE: tranquility [-progress num] [-graph] [-sql graph_token] [-h] [-debug]\n"
			"Takes plantri graphs on input. Outputs one minimal triangulation\n"
			"for every triangle size possible to construct from input graphs.\n"
			"  -progress num    - prints out progress info every 'num' graphs\n"
			"  -graph           - includes graph on output\n"
			"  -sql graph_token - outputs all triangulations as sql inserts with\n"
			"                     graph_id containig 'graph_token'\n"
			"  -h               - prints out this message\n"
			"  -debug           - prints out debugging information\n"
		);
		return 1;
	}

	// debug init message
	if (settings.debug) {
		printf("Tranquility started.\n");
	}
	
	int progress = 0;
	
	// while possible, load a graph
	while (load_graph() == 0) {

		// progress info
		if (settings.progress && ++progress % settings.progress == 0) {
			fprintf(stderr, "%d graphs already processed.\n", progress);
		}
	
		// the graph is 3-colorable, assign colors
		color3();
	
		// construct the matrices for faces
		get_face_matrices();
		
		// prepare graph insert
		if (settings.debug) {
			printf("Black faces: %s\nWhite faces: %s\n", faces_str[0], faces_str[1]);
		}

		if (settings.sql) {
			sql.add_graph();
		}

		// find solutions
		for (int col = 0; col < 2; col++) {
			find_solution(col);
		}

		// debugging information
		if (settings.debug) {
			for (int v = 0; v < nVertices; v++) {
				printf("Vertex %d | Color %d | Neighbors ", v, color[v]);
				for (int neigh = 0; neigh < (int)graph[v].size(); neigh++)
					printf("%d ", graph[v][neigh]);
				printf("\n");
			}
		}
	}
	
	if (settings.sql) {
		// output to stdout
		printf("%s;\n", sql.graph_insert.str().c_str());
		printf("%s;\n", sql.triangulation_insert.str().c_str());
	} else {
		solution_handler.print();
	}
	
	return 0;
}
