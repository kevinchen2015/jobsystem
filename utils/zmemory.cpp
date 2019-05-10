
#include "zmemory.h"
#include <memory.h>
#include <stdlib.h>

void on_malloc(void* p, int size, char* file, int line)
{
	//do nothing
}

void on_free(void* p)
{
	//do nothing
}

void* my_malloc(int size, char* file, int line)
{
	void* p = malloc(size);
	on_malloc(p, size, file, line);
	return p;
}

void my_free(void*p)
{
	on_free(p);
	free(p);
}


