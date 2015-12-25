//
// start of shared cmdlib stuff
// 

#include "stdafx.h"
#include "cmdlib.h"
#include "windows.h"






// rad end

#define MEM_BLOCKSIZE 4096
void* qblockmalloc(size_t nSize)
{
	void *b;
  // round up to threshold
  int nAllocSize = nSize % MEM_BLOCKSIZE;
  if ( nAllocSize > 0)
  {
    nSize += MEM_BLOCKSIZE - nAllocSize;
  }
	b = malloc(nSize + 1);
	memset (b, 0, nSize);
	return b;
}

void* qmalloc (size_t nSize)
{
	void *b;
	b = malloc(nSize + 1);
	memset (b, 0, nSize);
	return b;
}

void Error(const char *pFormat, ...)
{

}




void    StripExtension (char *path)
{
	int             length;

	length = strlen(path)-1;
	while (length > 0 && path[length] != '.')
	{
		length--;
		if (path[length] == '/')
			return;		// no extension
	}
	if (length)
		path[length] = 0;
}



void ExtractFileName (const char *path, char *dest)
{
	const char *src;

	src = path + strlen(path) - 1;

//
// back up until a \ or the start
//
	while (src != path && *(src-1) != '/' 
		 && *(src-1) != '\\' )
		src--;

	while (*src)
	{
		*dest++ = *src++;
	}
	*dest = 0;
}