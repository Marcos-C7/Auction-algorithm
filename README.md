# Auction algorithm
Implementation in C of the \epsilon-Scaling Auction Algorithm for the Assignment Problem
`[D. P. Bertsekas, Auction algorithms for network flow problems: A tutorial introduction, 
Computational optimization and applications, Vol. 1, pp. 7-66, 1992]`.

The Auction Algorithm solves the Assignment Problem, where given a Weighted Bipartite Graph the 
objective is to find a perfect matching of minimum weight (cost).
Since the current implementation does not include infeasibility detection the bipartite graph must have perfect matchings,
otherwise the program will fall into an infinite loop. As a consecuence of this, the graph must be balanded, i.e.
the number of vertices in each side of the graph must be equal.
The infeasibility detection can be easily addressed with minimum performance overhead and could be considered 
as a future feature.

Similar problems can be solved with this implementation using very efficient graph transformations. One of such problems is to
find a one-side perfect matching of minmum cost in an unbalanced bipartite graph, i.e. the matching must cover the 
smallest side of the graph.
Another problem that can be solved is to find a maximum-cardinality matching of minimum cost in a bipartite graph with no
(one-side) perfect matchings. For more information about these transformations contact `esocram@gmail.com`.

### Prorgam usage

The Auction Algorithm considers the left hand side vertices of the bipartite graph as Persons and the right hand side 
vertices as Objects. We will keep this terminology. Also all the edge costs (weights) must be integer.

The solver works through an interface given in the header file "AP_Tools.h" (for Assignment Problem Tools).
This interface has functions for loading a weighted bipartite graph, a function for solving the loaded instance
and a function for saving the resulting matching to a text file.
For now we will focus on the implementation given in "auction.c",
which makes use of the interface and automatically loads a weighted bipartite graph from a file, solves it and writes
the result in a file.
The source should compile with `gcc -O3 auction.c`, making sure that the header files are in hand for the compiler.

Once compiled the source `"auction.c"`, and assuming an executable called "auction.exe" was created, the usage of the program
via the console is as follows:
```
auction.exe weighted_graph_path
```
where weighted_graph_path is the path of the file that contains the weighted graph to be solved.

The program admits two different formats for the files, one in binary and one in plain text.
In both formats the Persons are considered to be represented by the set of consecutive integers {0,...,N-1} 
and the Objects are also represented by the set {0,...,N-1}, where N is the number of Persons which equals the number of Objects.
Since the sets are disjoint this does not give place to confusion. We will represent weighted edges by [u,v,c] and means that 
the person with index 'u' is connected to the object with index 'v' at cost 'c'. Examples of weighted edges are [0,2,7385], 
[2,0, 543] or [1,1,8192].

The program will automatically detect the file format based on the file extension. The extension for a text file is `.txt` 
and the extension for a binary file is `.wbg`. Make sure that the correct extension is used in your file.
Once the program finishes a correct execution, a new text file will be written containing the resulting minumum cost perfect matching.
The name of the file will be equal to the file name of the input instance appending "_matching" to it.
The first line of the file is the matching cost, the next line is the solving time in seconds, and the following lines contain
one weighted edge per line of the form `u,v,c` as described before.

### File formats

#### Text format
In this format the first line contains the number of persons, the second line contains the number of objects
and the following lines contain one weighted edge per line in the form "u,v,c", where 'u' is the index of the person, 'v' is
the index of the object and 'c' is the edge cost. There is only one condition that the edges must satisfy, the edges must be
grouped by the index of the person, i.e. all the edges adjacent to a person must be contiguous in the file, it doesn't matter 
the order of the neighbors, only that they be together. In other words, we place the adjacency lists of the persons one at a time.
An example of a valid file is the following:
```
persons: 3
objects: 3
0,0,32
0,1,54
1,2,12
2,2,30
2,1,25
```

#### Binary format
This format is similar to the previous one, except that this time we include information of the number of neighbors of each person
and we get rid of the indices of the persons, since we now take the first encountered adjacency list to be of person '0'
the next one to be of person '1' and so on.

All the data in the binary file is of type `int` and in Big-Endian byte order.
The first value in the file is the number of Persons followed by the number of Objecs. After this is the adjacency list of the
person with index '0'. Since we already know that the edges in the adjacency list are adjacent to person '0' we no longer need
to include this index in the respective edges. the adjacency list has the following structure: first the number of neighbors, 
say 'k_0', followed by 'k_0' pairs of values [index_of_neighbor_object, cost].
After the adjacency list of person '0' is the adjacency list of person '1', which is similar to the one just described, and 
so on until the adjacecy of person 'N-1'.

An ilustration of an example of a valid binary file is the following, which contains the same graph as the example of the text format:
```
3,3,[2,0,32,1,54],[1,2,12],[2,2,30,1,25]
```
just for ilustration purposes we enclosed each adjacency list in square brackets. Remember that all the values must be written in
Big-Endian order.

### About the interface

The main interface of the solver is very easy to use. There is a structure to report errors called `Error` and only contains 
the members `int code` and `char *msg`. It is fully documented in the header file `"error.h"`.

There is a structure called `AuctionSolver` that stores a Weighted Bipartite Graph and the necessary data structures for 
applying the Auction Algorithm. The first function that must be called on an 'AuctionSolver' instance is:
```
void	AS_Defaults(AuctionSolver *Instance);
```
this function initializes the values and pointers to safe default values, otherwise there will be lots of crashes.

There is a function to load a graph from a binary file to an 'AuctionSolver' instance:
```
Error	AS_Load_graph_Binary(AuctionSolver *Instance, char *file_path);
```

There is a function to load a graph from a text file to an 'AuctionSolver' instance:
```
Error	AS_Load_graph_Text(AuctionSolver *Instance, char *file_path);
```

Once a weighted graph has been loaded in an 'AuctionSolver' instance, to find a minimum cost perfect matching
via the \epsilon-Scaling Auction Algorithm we can use the following function:
```
Error	AS_Solve_Instance(AuctionSolver *Instance, double initial_epsilon, double alpha, double final_epsilon);
```
the resulting matching and its costs will be in the members `Instance.Matching` and `Instance.Matching_costs` representing that
object 'i' is assigned to person 'Instance.Matching[i]' at cost 'Instance.Matching_costs[i]'.
The solving time will be in `Instance.solving_time` and the matching cost will be in `Instance.matching_cost`.

Once a matching has ben found in an 'AuctionSolver' instance, we can save the matching to a text file with the function:
```
void	AS_Save_matching_Text(AuctionSolver *Instance, char *file_path);
```
the resulting file is structured as described in the section *Program usage*.

Once you are done with the 'AuctionSolver' instance, use the following function to free the dinamically allocated memory:
```
void	AS_Clear(AuctionSolver *Instance);
```
