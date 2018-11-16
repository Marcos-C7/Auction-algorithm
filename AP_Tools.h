#ifndef __AP_TOOLS__
#define __AP_TOOLS__

#include <sys/types.h>
#include <sys/times.h>
#include <unistd.h>
#include "error.h"
#include "types.h"
#include "boundeddeque.h"

//******************************************************************************
// AUXILARY FUNCTIONS

// Allocates memory for an array using 'malloc', deleting the current memory allocated, if any.
// To detect memory currently allocated it tests for non NULL value of the pointer.
// The size is in bytes.
void		Allocate_array(char **array, int size)
{
	if (*array != NULL) free(*array);
	*array = NULL;
	*array = malloc(size);
}

// Frees the memory allocated to a pointer, and sets the pointer to NULL.
void		Delete_array(char **array)
{
	free(*array);
	*array = NULL;
}

// Given an int value, reverse its bytes.
void		Swap_endianness_int(int *v)
{
	int aux = *v;
	int size = sizeof(int);
	for(char i = 0; i < size; ++i) ((char*)v)[i] = ((char*)&aux)[size - i - 1];
}

// Function to compute time durations. Given two calls to 'My_time' storing the values
// in 'start_time' and 'end_time', the time duration in seconds is:
// 'time = (end_time - start_time) / sysconf(_SC_CLK_TCK)'.
unsigned	My_time()
{
	struct tms timer;
	times(&timer);
	return timer.tms_utime;
}

//******************************************************************************
// AUCTIONSOLVER STRUCTURE
/*
	Structure that contains all data related to an instance of the Assignment Problem
	and the data related to the Auction Algorithm.
*/
typedef struct
{
// DATA RELATED TO THE INSTANCE OF THE ASSIGNMENT PROBLEM.
	// The number of persons and the number of objects in the bipartite graph.
	int				num_persons;
	int				num_objects;
	// The maximum absolut cost.
	int				max_abs_cost;
	// The graph, i.e. the array of persons and their adjancency lists.
	Graph			Persons;
	
// DATA RELATED TO THE AUCTION ALGORITHM.
	// The solving time in seconds.
	double			solving_time;
	// The cost of the resulting matching after solving the instance.
	long int		matching_cost;
	// The array of prices of the objects.
	double			*Prices;
	// The array of mates of the objects: object 'i' is matched to person 'Matching[i]'.
	int				*Matching;
	// The array of costs of the matching edges: cost of edge between object 'i' and 
	// its mate 'Matching[i]' is Matching_costs[i].
	costType		*Matching_costs;
	// Deque for storing the unmatched persons.
	BoundedDeque	Unmatched_persons;
} AuctionSolver;

//******************************************************************************
// AUCTIONSOLVER METHODS

// Constructor: initializes the members to safe default values.
// This function always needs to be called on an 'AuctionSolver' instance before any other move.
void	AS_Defaults(AuctionSolver *Instance)
{
	Instance->num_persons = 0;
	Instance->num_objects = 0;
	Instance->matching_cost = 0;
	Instance->solving_time = 0;
	Instance->max_abs_cost = 0;
	Instance->Prices = NULL;
	Instance->Matching = NULL;
	Instance->Matching_costs = NULL;
	Instance->Persons = NULL;
	BD_Defaults(&Instance->Unmatched_persons);
}

// Safe destructor: free the memory and set everything to default values.
void	AS_Clear(AuctionSolver *Instance)
{
	Graph_Clear(&Instance->Persons, Instance->num_persons);
	Delete_array((char**)&Instance->Prices);
	Delete_array((char**)&Instance->Matching);
	Delete_array((char**)&Instance->Matching_costs);
	BD_Clear(&Instance->Unmatched_persons);
	Instance->num_persons = 0;
	Instance->num_objects = 0;
	Instance->matching_cost = 0;
	Instance->solving_time = 0;
	Instance->max_abs_cost = 0;
}

// Load a graph from the file in 'file_path' onto the AuctionSolver instance.
// The file format must be binary.
// The members that will be modified are: 'num_persons, num_objects, max_abs_cost, Persons'.
Error	AS_Load_graph_Binary(AuctionSolver *Instance, char *file_path)
{
	Error		error = {0, ""};
	FILE		*graph_file;
	int			size_int = sizeof(int);
	int			num_neighbors;
	int			neighbor;
	int			cost;
	
	
	// Open the graph file and check for errors.
	graph_file = fopen(file_path, "rb");
	if(graph_file == 0) printf("\nGraph file does not exists: '%s'\n", file_path);
	
	// Get the number of persons and the number of objects of the graph.
	fread(&Instance->num_persons, size_int, 1, graph_file);
	Swap_endianness_int(&Instance->num_persons);
	fread(&Instance->num_objects, size_int, 1, graph_file);
	Swap_endianness_int(&Instance->num_objects);
	
	// Make the allocation of the memory. Initialized to zero for safe clear in case of future memory allocation error.
	Graph_Allocate(&Instance->Persons, Instance->num_persons);
	if (Instance->Persons == NULL)
	{
		Instance->num_persons = 0;
		Instance->num_objects = 0;
		Error_Set(&error, -1, "Error: no memory for the allocation of a person in 'AS_Load_graph_Binary'");
		return error;
	}
	
	// Load the arcs.
	for(int person_i = 0; person_i < Instance->num_persons; ++person_i)
	{
		// Read the number of neighbors of the person.
		fread(&num_neighbors, size_int, 1, graph_file);
		Swap_endianness_int(&num_neighbors);
		// Try to allocate memory for the Person and check for errors.
		error = Person_Allocate(&Instance->Persons[person_i], num_neighbors);
		if (error.code == -1)
		{
			AS_Clear(Instance);
			strcat(error.msg, " In 'AS_Load_graph_Binary'");
			return error;
		}
		
		// Fill the neighbors and costs arrays of the person.
		for(int neighbor_j = 0; neighbor_j < num_neighbors; ++neighbor_j)
		{
			// Get the neighbor index and the edge cost.
			fread(&neighbor, size_int, 1, graph_file);
			Swap_endianness_int(&neighbor);
			fread(&cost, size_int, 1, graph_file);
			Swap_endianness_int(&cost);
			// Put them in the respective arrays.
			Instance->Persons[person_i].neighbors[neighbor_j] = neighbor;
			Instance->Persons[person_i].costs[neighbor_j] = (costType)cost;
			
			// Update the maximum absolute cost.
			if ((int)abs(cost) > Instance->max_abs_cost) Instance->max_abs_cost = (int)abs(cost);
		}
	}
	
	fclose(graph_file);
	
	Error_Set(&error, 1, "OK");
	return error;
}

// Load a graph from the file in 'file_path' onto the AuctionSolver instance.
// The file format must be text.
// The members that will be modified are: 'num_persons, num_objects, max_abs_cost, Persons'.
Error	AS_Load_graph_Text(AuctionSolver *Instance, char *file_path)
{
	Error		error;
	FILE		*graph_file;
	char		line[200];
	char		aux_str[100];
	int			*aux_neighbors = NULL;
	costType	*aux_costs = NULL;
	// The index of a person, of an object and the cost between them.
	int			person_i;
	int			object_j;
	int			cost;
	// For counting the number of neighbors of a person.
	int			num_neighbors;
	// To detect when we have reached the adjacency list of a different person.
	int			aux_person;
	
	
	// For safe string processing.
	for (int i = 0; i < 200; ++i) line[i] = '\0';
	for (int i = 0; i < 100; ++i) aux_str[i] = '\0';
	
	// Open the text file.
	graph_file = fopen(file_path, "r");
	
	// Read the number of persons and the number of objects.
	fgets(line, 200, graph_file);
	sscanf(line, "%s %d", aux_str, &Instance->num_persons);
	fgets(line, 200, graph_file);
	sscanf(line, "%s %d", aux_str, &Instance->num_objects);
	
	// Make the allocation of the memory. Initialized to zero for safe clear in case of future memory allocation error.
	Graph_Allocate(&Instance->Persons, Instance->num_persons);
	if (Instance->Persons == NULL)
	{
		Instance->num_persons = 0;
		Instance->num_objects = 0;
		Error_Set(&error, -1, "Error: no memory for the allocation of a person in 'AS_Load_graph_Text'");
		return error;
	}
	
	// Allocate auxiliary space for an adjacency list.
	Allocate_array((char**)&aux_neighbors, Instance->num_objects * sizeof(int));
	Allocate_array((char**)&aux_costs, Instance->num_objects * sizeof(costType));
	if (aux_neighbors ==  NULL || aux_costs == NULL)
	{
		AS_Clear(Instance);
		Delete_array((char**) aux_neighbors);
		Delete_array((char**) aux_costs);
		Error_Set(&error, -1, "Error: no memory for the allocation of a person in 'AS_Load_graph_Text'");
		return error;
	}
	
	// Work apart the first line (edge).
	fgets(line, 200, graph_file);
	for(int i = 0; line[i] != '\0'; ++i)
		if (line[i] == ',' || line[i] == '\r' || line[i] == '\n') line[i] = ' ';
	sscanf(line, "%d %d %d", &person_i, &object_j, &cost);
	aux_neighbors[0] = object_j;
	aux_costs[0] = cost;
	
	num_neighbors = 1;
	aux_person = person_i;
	// Read line by line (i.e. edge by edge).
	// We will read one adjacency list at a time in 'aux_neighbors' and 'aux_costs' and dump it in the corresponding person.
	while(fgets(line, 200, graph_file) != NULL)
	{
		// From the line get the person index, the object index, and the edge cost.
		for(int i = 0; line[i] != '\0'; ++i)
			if (line[i] == ',' || line[i] == '\r' || line[i] == '\n') line[i] = ' ';
		sscanf(line, "%d %d %d", &person_i, &object_j, &cost);
		
		// If we have reached the adjacency list of a different person, save the current one and start the next.
		if (aux_person != person_i)
		{
			// Since we now know the size of the adjacency list, allocate space.
			error = Person_Allocate(&Instance->Persons[aux_person], num_neighbors);
			if (error.code == -1)
			{
				AS_Clear(Instance);
				strcat(error.msg, " In 'AS_Load_graph_Text'");
				return error;
			}
			// Dump the current adjacency list into the person 'person_i'.
			for (int neighbor_j = 0; neighbor_j < num_neighbors; ++neighbor_j)
			{
				Instance->Persons[aux_person].neighbors[neighbor_j] = aux_neighbors[neighbor_j];
				Instance->Persons[aux_person].costs[neighbor_j] = aux_costs[neighbor_j];
			}
			
			aux_person = person_i;
			num_neighbors = 0;
		}
		
		aux_neighbors[num_neighbors] = object_j;
		aux_costs[num_neighbors] = (costType) cost;
		++num_neighbors;
	}
	// The last person wont be created inside the loop, so we create it here.
	error = Person_Allocate(&Instance->Persons[aux_person], num_neighbors);
	if (error.code == -1)
	{
		AS_Clear(Instance);
		strcat(error.msg, " In 'AS_Load_graph_Text'");
		return error;
	}
	for (int neighbor_j = 0; neighbor_j < num_neighbors; ++neighbor_j)
	{
		Instance->Persons[aux_person].neighbors[neighbor_j] = aux_neighbors[neighbor_j];
		Instance->Persons[aux_person].costs[neighbor_j] = aux_costs[neighbor_j];
	}
	
	// Free the auxiliary adjacency list.
	Delete_array((char**) &aux_neighbors);
	Delete_array((char**) &aux_costs);
	fclose(graph_file);
}

// Save the resulting matching and its cost to a text file.
void	AS_Save_matching_Text(AuctionSolver *Instance, char *file_path)
{
	FILE	*output_file;
	
	// Save the matching cost and the solving time.
	output_file = fopen(file_path, "w");
	fprintf(output_file, "cost %.0f\n", (double)Instance->matching_cost);
	fprintf(output_file, "time %f\n", Instance->solving_time);
	// Save the matching edge by edge, space separated as: "person_index object_index cost"
	for (int object_i = 0; object_i < Instance->num_objects; ++object_i)
		fprintf(output_file, "%d,%d,%.0lf\n", Instance->Matching[object_i], object_i, Instance->Matching_costs[object_i]);
	
	fclose(output_file);
}

// Displays the graph contained in the 'AuctionSolver' instance.
// It shows the number of persons and objects; and the adjacency lists of the persons as
// pairs (neighbor, cost).
// Only use on very small instances, to see that the instance is being loaded correctly.
void	AS_Display_Instance(AuctionSolver *Instance)
{
	int			num_neighbors;
	int			neighbor;
	costType	cost;
	
	printf("------------------------------------------------------------\n");
	printf("Num_Persons=%d, Num_Objects=%d\n", Instance->num_objects, Instance->num_objects);
	for(int person_i = 0; person_i < Instance->num_persons; ++person_i)
	{
		num_neighbors = Instance->Persons[person_i].num_neighbors;
		printf("--------------\n");
		printf("Num_Neighbors of p%d:%d\n", person_i, num_neighbors);
		for(int neighbor_j = 0; neighbor_j < num_neighbors; ++neighbor_j)
		{
			neighbor = Instance->Persons[person_i].neighbors[neighbor_j];
			cost = Instance->Persons[person_i].costs[neighbor_j];
			printf("(%d,%d) ", neighbor, (int)cost);
		}
		printf("\n");
	}
	printf("------------------------------------------------------------\n");
}

// Given the index 'I' of an unmatched person, this function finds the index of the object that has the best_reduced_cost,
// the edge cost with this object, and the value of 'gamma' which is the difference 'second_best_reduced_cost - best_reduced_cost'.
// The first is returned as a regular return value and the other two are returned by reference.
int		AS_Find_best_object(AuctionSolver *Instance, int I, double *gamma, costType *cost_of_best_object)
{
	//The smallest and the second smallest losses with very big values.
	double		best_reduced_cost = _INFINITY;
	double		second_best_reduced_cost = _INFINITY;
	double		auxiliary_reduced_cost = _INFINITY;
	
	//The variable we will return.
	int			best_object = UNMATCHED;
	//Pointer to the person "I".
	Person*		person_ptr = Instance->Persons + I;
	
	
	//See if the person has only one neighbor.
	if(person_ptr->num_neighbors == 1)
	{
		best_object = person_ptr->neighbors[0];
		*cost_of_best_object = person_ptr->costs[0];
		//Assign a value to "best_reduced_cost" so that gamma==1000000.0.
		best_reduced_cost -= 1000000.0;
	}
	else
	{
		// Find the first two best reduced costs.
		best_reduced_cost = person_ptr->costs[0] - Instance->Prices[person_ptr->neighbors[0]];
		auxiliary_reduced_cost = person_ptr->costs[1] - Instance->Prices[person_ptr->neighbors[1]];
		if (best_reduced_cost <= auxiliary_reduced_cost)
		{
			second_best_reduced_cost = auxiliary_reduced_cost;
			best_object = person_ptr->neighbors[0];
			*cost_of_best_object = person_ptr->costs[0];
		}
		else
		{
			second_best_reduced_cost = best_reduced_cost;
			best_reduced_cost = auxiliary_reduced_cost;
			best_object = person_ptr->neighbors[1];
			*cost_of_best_object = person_ptr->costs[1];
		}
		
		//Scan the rest of neighbours.
		for(int neighs_i = 2; neighs_i < person_ptr->num_neighbors; ++neighs_i)
		{
			//Get the reduced cost of the edge.
			auxiliary_reduced_cost = person_ptr->costs[neighs_i] - Instance->Prices[person_ptr->neighbors[neighs_i]];
			
			//Update the smallest and second smallest reduced costs.
			if(auxiliary_reduced_cost < best_reduced_cost)
			{
				second_best_reduced_cost = best_reduced_cost;
				best_reduced_cost = auxiliary_reduced_cost;
				//Keep track of the best object and its cost.
				best_object = person_ptr->neighbors[neighs_i];
				*cost_of_best_object = person_ptr->costs[neighs_i];
			}
			else if(auxiliary_reduced_cost < second_best_reduced_cost)
				second_best_reduced_cost = auxiliary_reduced_cost;
		}
	}
	
	//Compute the bidding difference "gamma".
	*gamma = second_best_reduced_cost - best_reduced_cost;
	
	//Return the index of the best object.
	return best_object;
}

// Given 'epsilon', find a matching and prices that satisfy the epsilon-CS condition.
void	AS_eOpt_Matching(AuctionSolver *Instance, double epsilon)
{
	//A person.
	int			I = 0;
	//The bidding difference.
	double		gamma = 0.0;
	//The best option for a person.
	int			best_object = 0;
	//The cost of the best object.
	costType	cost_of_best_object = 0;
	
	//While we have unmatched persons.
	while(Instance->Unmatched_persons.length > 0)
	{
		//Get an unmatched person "I".
		BD_Pop_Back(&Instance->Unmatched_persons, &I);
		
		//Get the best option for person "I", the bidding increment and the cost of the chosen edge.
		best_object = AS_Find_best_object(Instance, I, &gamma, &cost_of_best_object);
		
		//If the chosen object is matched, then insert its matching in the unmatched list.
		if(Instance->Matching[best_object] != UNMATCHED)
			BD_Push_Back(&Instance->Unmatched_persons, Instance->Matching[best_object]);
		
		//Update the matching and the costs of the matching.
		Instance->Matching[best_object] = I;
		Instance->Matching_costs[best_object] = cost_of_best_object;
		
		//Update (decrease) the price of the object.
		Instance->Prices[best_object] -= (gamma + epsilon);
	}
}

// Find a matching and prices that satisfy the epsilon-CS condition with 'epsilon=final_epsilon'.
// This is done via the Auction Algorithm, with scaling phases that go from 'epsilon=initial_epsilon'
// down to 'epsilon=final_epsilon', with a scaling factor of 'alpha'.
Error	AS_Solve_Instance(AuctionSolver *Instance, double initial_epsilon, double alpha, double final_epsilon)
{
	//To get the time.
	unsigned		start_time;
	//Error tracker.
	Error			error = {0, ""};
	//The epsilon.
	double			epsilon;
	int				full_reset = 1;
	
	//Check for some posible errors.
	if(Instance->num_persons <= 0 || Instance->num_persons != Instance->num_objects)
	{
		Error_Set(&error, -1, "Error: The graph can not be empty or unbalanced");
		return error;
	}
	if(initial_epsilon < 0 || final_epsilon < 0)
	{
		Error_Set(&error, -1, "Error: The initial and final values for epsilon can not be negative.");
		return error;
	}
	
	//MEMORY ALLOCATION.
	//Allocate memory for the price of each object.
	Allocate_array((char**)&Instance->Prices, Instance->num_objects * sizeof(double));
	//Allocate memory for the matching vector of the objects.
	Allocate_array((char**)&Instance->Matching, Instance->num_objects * sizeof(double));
	//Allocate memory for the cost of the resulting matching.
	Allocate_array((char**)&Instance->Matching_costs, Instance->num_objects * sizeof(double));
	//Allocate memory for the unmatched persons list.
	BD_AllocateMemory(&Instance->Unmatched_persons, Instance->num_persons);
	
	if (Instance->Prices == NULL || Instance->Matching == NULL || Instance->Matching_costs == NULL || Instance->Unmatched_persons.container == NULL)
	{
		Error_Set(&error, -1, "Error: no memory for the allocation of the auxiliary structures in 'AS_Solve_Instance'.");
		AS_Clear(Instance);
		return error;
	}
	
	for(int object_i = 0; object_i < Instance->num_objects; ++object_i)
	{
		Instance->Prices[object_i] = 0;
		Instance->Matching_costs[object_i] = 0;
	}
	
	epsilon = initial_epsilon;
	//If no limit for epsilon was passed, use an optimal value.
	if(final_epsilon == 0) final_epsilon = 1.0 / (Instance->num_persons + 2.0);
	
	//Take the beginning time.
	start_time = My_time();
	
	//Make sure that the list of unmatched persons is empty.
	BD_Reset(&Instance->Unmatched_persons);
	do
	{
		//Reduce epsilon.
		epsilon /= alpha;
		if(epsilon < final_epsilon) epsilon = final_epsilon;
		
		//Discard the matching, and add all persons to the list of unmatched persons.
		for(int object_i = 0; object_i < Instance->num_objects; ++object_i)
			Instance->Matching[object_i] = UNMATCHED;
		for(int person_i = 0; person_i < Instance->num_persons; ++person_i)
			BD_Push_Back(&Instance->Unmatched_persons, person_i);
		
		//Find an epsilon-optimal matching.
		AS_eOpt_Matching(Instance, epsilon);
	}
	while(epsilon > final_epsilon);
	
	//Get the solving time.
	Instance->solving_time = (double) (My_time() - start_time) / sysconf(_SC_CLK_TCK);
	
	Instance->matching_cost = 0;
	for(int object_i = 0; object_i < Instance->num_objects; ++object_i)
		Instance->matching_cost += Instance->Matching_costs[object_i];
}


#endif
