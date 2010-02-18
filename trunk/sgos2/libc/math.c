

#define RAND_MAX 65536

static unsigned short rand_seed = 0;

/* srand:
 *  Seed initialization routine for rand() replacement.
 */
void srand(int seed)
{
   rand_seed = seed;
}

/* _rand:
 *  Simple rand() replacement with guaranteed randomness in the lower 16 bits.
 */
int rand(void)
{
   rand_seed = (rand_seed + 1) * 1103515245 + 12345;
   return rand_seed;
}

int pow(int a,int b)
{
	int rs=1;
	while(b--)
		rs=rs*a;
	return rs;
}
