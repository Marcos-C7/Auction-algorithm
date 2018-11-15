#include <stdio.h>
#include <string.h>
#include "types.h"
#include "AP_Tools.h"

// Find the index of the last dot in 'file_path', and puts the file extension (whithout the dot) in 'extension'.
// The last dot index is returned.
int	Get_file_extension(const char *file_path, char *extension)
{
	int		dot_index = 0;
	int		path_len = 0;
	
	path_len = strlen(file_path);
	
	// Find the index of the last dot.
	for(int i = path_len - 1; i >= 0; --i)
	{
		if (file_path[i] == '.')
		{
			dot_index = i;
			break;
		}
	}
	
	// Copy the extension including the terminating '\0' character.
	for(int i = dot_index + 1; i <= path_len; ++i)
		extension[i - dot_index - 1] = file_path[i];
	
	return dot_index;
}

int		main(int argc, char *argv[])
{
	AuctionSolver	Solver;
	char			file_path[500];
	char			file_extension[50];
	int				dot_index = 0;
	
	
	// Make sure a file path of an Assignment Problem instance was sent.
	if (argc < 2)
	{
		printf("Usage: auction.exe file_path\n");
		printf("The file can be in binary format '.wbg' or text format '.txt'\n");
		return 0;
	}
	
	// This function always needs to be called on an 'AuctionSolver' to initialize to safe values.
	AS_Defaults(&Solver);
	
	// Load a graph from the file, depending on the file extension.
	strcpy(file_path, argv[1]);
	dot_index = Get_file_extension(file_path, file_extension);
	if (strcmp(file_extension, "txt") == 0) AS_Load_graph_Text(&Solver, file_path);
	else AS_Load_graph_Binary(&Solver, file_path);
	
	// Solve the intance.
	AS_Solve_Instance(&Solver, (double)Solver.max_abs_cost, 7.0, 0);
	
	// Show the matching cost and the solving time.
	printf("\nMatching cost: %u\nSolving time: %.5f sec\n", Solver.matching_cost, Solver.solving_time);
	
	// Save the matching to a file in the same place as the input file, but terminating with '_matching.txt'.
	file_path[dot_index] = '\0';
	strcat(file_path, "_matching.txt");
	AS_Save_matching_Text(&Solver, file_path);
	
	//AS_Display_Instance(&Solver);
	
	// Free the memory.
	AS_Clear(&Solver);
	
	return 0;
}
