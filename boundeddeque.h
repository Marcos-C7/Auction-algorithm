#ifndef _T_BOUNDED_DEQUE_
#define _T_BOUNDED_DEQUE_

#include "error.h"

//*****************************************************************************************
// BOUNDEDDEQUE STRUCTURE
/*
	Structure that simulates a deque (https://en.wikipedia.org/wiki/Double-ended_queue) using an array.
	Since the Deque is stored in an array, the maximum number of elements that the Deque can store is fixed,
	until a new maximum size is requested via 'BD_AllocateMemory' (this resets the Deque to empty).
*/
typedef struct
{
	// The maximum length that the deque.
	int		max_length;
	// The size of the deque.
	int		length;
	// The index of the first element of the deque in the container.
	int		begin;
	// The index of the last element of the deque in the container.
	int		end;
	// The array.
	int		*container;
} BoundedDeque;

//***********************************
// BOUNDEDDEQUE METHODS

// Constructor: initializes the members to safe default values.
// This function always needs to be called on a 'BoundedDeque' instance before any other move.
void	BD_Defaults(BoundedDeque *BL)
{
	BL->max_length = 0;
	BL->length = 0;
	BL->begin = 0;
	BL->end = 0;
	BL->container = NULL;
}

// Allocate memory for the maximum capacity of the Deque.
// In case of error, an Error instance will be returned.
Error	BD_AllocateMemory(BoundedDeque *BL, int size)
{
	Error	error = {1, "OK"};
	
	//Free the memory, if necessary.
	if(BL->container != NULL) 
	{
		free((char*) BL->container);
		BL->container = 0;
	}
	
	BL->max_length = 0;
	BL->length = 0;
	BL->begin = 0;
	BL->end = 0;
	
	//If the size is zero or less, return with the arrays destroyed.
	if(size > 0)
	{
		BL->container = (int*) malloc(size * sizeof(int));
		
		//If memory allocation failed, return the respective error.
		if(BL->container != NULL) BL->max_length = size;
		else
		{
			Error_Set(&error, -1, "Error: no memory for the allocation in 'BoundedDeque::allocateMemory'");
			return error;
		}
	}
	else if (size < 0)
	{
		Error_Set(&error, -1, "Error: can not allocate a negative amount of memory in BoundedDeque::allocateMemory");
		return error;
	}
	
	return error;
}

// Insert an element at the front of the Deque.
// If the Deque is already at max size, the element will not be inserted.
void    BD_Push_Front(BoundedDeque *BL, int data)
{  
	//If we have space for another element.
	if(BL->max_length > 0 && BL->length < BL->max_length)
	{
		//Consider the rotation in the array.
		if(BL->begin == 0) BL->begin = BL->max_length - 1;
		else --BL->begin;
		//Consider the case where this is the first element.
		if(BL->length == 0) BL->end = BL->begin;
		BL->container[BL->begin] = data;
		++BL->length;
	}
}

// Insert an element at the back of the Deque.
// If the Deque is already at max size, the element will not be inserted.
void	BD_Push_Back(BoundedDeque *BL, int data)
{
	//If we have space for another element.
	if(BL->max_length > 0 && BL->length < BL->max_length)
	{
		//Consider the rotation in the array.
		BL->end = (BL->end == BL->max_length - 1 ? 0 : BL->end + 1);
		//Consider the case where this is the first element.
		if (BL->length == 0) BL->begin = BL->end;
		BL->container[BL->end] = data;
		++BL->length;
	}
}

// Remove the front element of the Deque, returning such element in the reference argument 'data'.
// In case of error, an Error instance will be returned.
Error	BD_Pop_Front(BoundedDeque *BL, int *data)
{
	//If the list contains elements.
	if(BL->max_length > 0 && BL->length > 0)
	{
		Error error = {1, "OK"};
		//Save the position of the first element.
		int   aux_index = BL->begin;
		
		//Consider the rotation in the array.
		BL->begin = (BL->begin == BL->max_length - 1 ? 0 : BL->begin + 1);
		//Consider the case where this is the only element.
		if(BL->length == 1)
		{
			BL->begin = 0;
			BL->end = 0;
		}
		--BL->length;
		*data = BL->container[aux_index];
		return error;
	}
	else
	{
		Error error = {-1, "Error: The list is empty, in BoundedDeque::BD_Pop_Front"};
		return error;
	}
}

// Remove the back element of the Deque, returning such element in the reference argument 'data'.
// In case of error, an Error instance will be returned.
Error	BD_Pop_Back(BoundedDeque *BL, int *data)
{
	//If the list contains elements.
	if(BL->max_length > 0 && BL->length > 0)
	{
		Error error = {1, "OK"};
		//Save the position of the first element.
		int   aux_index = BL->end;
		//Consider the case where this is the only element.
		if(BL->length == 1)
		{
			BL->begin = 0;
			BL->end = 0;
		}
		else BL->end = (BL->end == 0 ? BL->max_length - 1 : BL->end - 1);
		--BL->length;
		*data = BL->container[aux_index];
		return error;
	}
	else
	{
		Error error = {-1, "Error: The list is empty, in BoundedDeque::BD_Pop_Back"};
		return error;
	}
}

// Remove the front element of the Deque.
void    BD_Delete_Front(BoundedDeque *BL)
{
	//If the list contains elements.
	if(BL->max_length > 0 && BL->length > 0)
	{
		//Consider the case where this is the only element.
		if(BL->length == 1)
		{
			BL->begin = 0;
			BL->end = 0;
		}
		//Take in account the rotation in the array.
		else BL->begin = (BL->begin == BL->max_length - 1 ? 0 : BL->begin + 1);
		--BL->length;
	}
}

// Remove the back element of the Deque
void    BD_Delete_Back(BoundedDeque *BL)
{
	//If the list contains elements.
	if(BL->max_length > 0 && BL->length > 0)
	{
		//Consider the case where this is the only element.
		if(BL->length == 1)
		{
			BL->begin = 0;
			BL->end = 0;
		}
		else
		//Take in account the rotation in the array.
		BL->end = (BL->end == 0 ? BL->max_length - 1 : BL->end - 1);
		--BL->length;
	}
}

// Empty the Deque.
void    BD_Reset(BoundedDeque *BL)
{
	BL->length = 0;
	BL->begin = 0;
	BL->end = 0;
}

// Safe destructor: free the memory and set everything to default values.
void    BD_Clear(BoundedDeque *BL)
{
	if(BL->container != NULL)
	{
		free((char*) BL->container);
		BL->container = NULL;
	}
	BL->max_length = 0;
	BL->length = 0;
	BL->begin = 0;
	BL->end = 0;
}

//*****************************************************************************************

#endif
