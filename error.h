#ifndef __ERROR__
#define __ERROR__

#include <string.h>

//*****************************************************************************************
// ERROR STRUCTURE
/*
	Structure used to handle errors. The member 'code' will be '1' when there is
	no error, and '-1' when there was an error. The message of the error should 
	be in the 'msg' member.
*/
typedef struct
{
	// The code: 1 for no error, and -1 for error.
	int		code;
	// The massage asociated to the error.
	char	msg[200];
} Error;

//***********************************
// ERROR METHODS

// Function for easy code and message setting.
// Put the 'code' and 'msg' in the 'error' instance.
void	Error_Set(Error *error, int code, const char *msg)
{
	error->code = code;
	strcpy(error->msg, msg);
}

//*****************************************************************************************

#endif