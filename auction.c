#include <stdio.h>
#include "types.h"
#include "AP_Tools.h"


int		main(int argc, char *argv[])
{
	AuctionSolver Solver;
	
	// This function always needs to be called on an 'AuctionSolver' to initialize to safe values.
	AS_Defaults(&Solver);
	
	// Load a graph from a file.
	AS_Load_graph(&Solver, argv[1]);
	// Solve the intance.
	AS_Solve_Instance(&Solver, (double)Solver.max_abs_cost, 7.0, 0);
	// Show the matching cost and the solving time.
	//printf("\nCost:%u\nTime:%.5f\n", Solver.matching_cost, Solver.solving_time);
	
	// Save the matching to a file.
	AS_Save_Matching_Text(&Solver, "Graphs/matching.txt");
	
	// Free the memory.
	AS_Clear(&Solver);
	
	return 0;
}
