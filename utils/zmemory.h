#pragma once


void* my_malloc(int size, char* file, int line);
void my_free(void*p);


#define zm_init() 
#define zm_uninit()

#define zm_malloc(size)			my_malloc(size,__FILE__,__LINE__)
#define zm_free(p)				my_free(p)

#define zm_unsafe_malloc(size)	my_malloc(size,__FILE__,__LINE__)
#define zm_unsafe_free(p)		my_free(p)
