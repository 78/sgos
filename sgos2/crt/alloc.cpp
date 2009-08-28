
extern "C" void* malloc(unsigned int );
extern "C" void free(void*p);

void * operator new (unsigned int size)
{
    return (void*)malloc(size);
}
 
//overload the operator "new[]"
void * operator new[] (unsigned int size)
{
    return (void*)malloc(size);
}
 
//overload the operator "delete"
void operator delete (void * p)
{
	free(p);
}
 
//overload the operator "delete[]"
void operator delete[] (void * p)
{
	free(p);
}
