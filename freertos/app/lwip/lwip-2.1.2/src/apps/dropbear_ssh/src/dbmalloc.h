#ifndef DBMALLOC_H_
#define DBMALLOC_H_

#include "options.h"
#include <stdint.h>
#include <stdlib.h>

void * m_malloc(size_t size);
void * m_calloc(size_t nmemb, size_t size);
void * m_strdup(const char * str);
void * m_realloc(void* ptr, size_t size);
void * m_realloc_ltm(void* ptr, size_t oldsize, size_t newsize);
void m_free_direct(void* ptr);

#if DROPBEAR_TRACKING_MALLOC
void m_malloc_set_epoch(unsigned int epoch);
void m_malloc_free_epoch(unsigned int epoch, int dofree);
#endif

#define m_free(X) do { if (X != NULL) { m_free_direct(X); (X) = NULL; } } while (0)


#endif /* DBMALLOC_H_ */
