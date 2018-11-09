#ifndef __TYPES__
#define __TYPES__

#include <stdlib.h>
#include <string.h>
#include "error.h"

//*****************************************************************************************
// GLOBAL CONSTANTS

// Value used to denote that a vertex is not matched.
const int UNMATCHED = 2147483647;
// A big number.
const double INFINITY = 1000000000.0;
// The type for the edge costs.
typedef int	costType;

//*****************************************************************************************
// PERSON STRUCTURE
/*
	Structure that contains the necessary information of a Person Vertex.
	This information is the number of neighbors that the person has, 
	an array for the indices of the neighbors, and an array for the edge
	costs of the person with its neighbors in the same order.
*/
typedef struct
{
	// Number of neighbors of the person.
	int			num_neighbors;
	// Array of neighbors indices (the indices are in the range {0, ... , num_objects - 1}.
	int			*neighbors;
	// Array of edge costs with the neighbors, in the same order as the 'neighbors' array.
	// i.e. this person has an edge with object neighbor[i] with cost costs[i].
	costType	*costs;
} Person;

//***********************************
// PERSON METHODS

// Safe destructor: free the memory and set members to default values.
void	Person_Clear(Person *P)
{
	if (P->num_neighbors > 0)
	{
		free((char*) P->neighbors);
		free((char*) P->costs);
	}
	P->num_neighbors = 0;
	P->neighbors = NULL;
	P->costs = NULL;
}

// Safe allocate arrays for neighbors and costs of size 'num_neighbors' each.
// The member 'num_neighbors' of the person will be set to the correct value.
// In case of error, an Error instance will be returned.
Error	Person_Allocate(Person *P, int num_neighbors)
{
	Error error = {-1, "Error: no memory for the allocation of a person in 'Person_Allocate'"};
	
	// Clear the person just in case.
	Person_Clear(P);
	// Only try to allocate space for >= 0 number of neighbors.
	if (num_neighbors >= 0)
	{
		// Try to allocate the neighbors.
		P->neighbors = (int*) malloc(num_neighbors * sizeof(int));
		if (P->neighbors == NULL) return error;
		// Try to allocate the costs.
		P->costs = (int*) malloc(num_neighbors * sizeof(costType));
		if (P->costs == NULL)
		{
			free((char *) P->neighbors);
			P->neighbors = NULL;
			return error;
		}
		P->num_neighbors = num_neighbors;
		
		error.code = 1;
		strcpy(error.msg, "OK");
		return error;
	}
	// Error if negative number of neighbors.
	strcpy(error.msg, "Error: can't allocate space for a negative number of neighbors of a person in 'Person_Allocate'");
	return error;
}

//*****************************************************************************************
// GRAPH STRUCTURE
/*
	Since the Auction algorithm only works with the adjacency lists of the Persons,
	then we will only store such adjacency lists. A Graph structure is simply a
	pointer to an array of Person structures.
*/
typedef Person* Graph;

//***********************************
// GRAPH METHODS

// Safe destructor: free the memory allocated for each person in the Graph and the Graph (array) itself.
// The number of elements in the array is needed ('num_persons' argument).
// Also set members to default values.
void	Graph_Clear(Graph *Persons, int num_persons)
{
	for(int person_i = 0; person_i < num_persons; ++person_i)
		Person_Clear(&(*Persons)[person_i]);
	free((char*) *Persons);
	*Persons = NULL;
}

// Safe allocate the array of Person structures for 'num_persons' elements.
// Every byte initialized to zero for safe memory free in case of errors.
// If the memory was not allocated the pointer will point to NULL.
void	Graph_Allocate(Graph *Persons, int num_persons)
{
	if (*Persons != NULL) free((char*) *Persons);
	*Persons = NULL;
	*Persons = (Graph) calloc(num_persons * sizeof(Person), 1);
}

#endif