#include <stdio.h>
#include "types.h"
#include "AP_Tools.h"


int		main()
{
	AuctionSolver Solver;
	
	// This function always needs to be called on an 'AuctionSolver' to initialize to safe values.
	AS_Defaults(&Solver);
	
	// Load a graph from a file.
	AS_Load_graph(&Solver, "graph_aux.wbg");
	// Solve the intance.
	AS_Solve_Instance(&Solver, (double)Solver.max_abs_cost, 7.0, 0);
	// Get the matching cost and the solution.
	printf("\nCost:%u\nTime:%.5f\n", Solver.matching_cost, Solver.solving_time);
	
	// Free the memory.
	AS_Clear(&Solver);
	
	return 0;
}