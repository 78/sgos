#include <sgos.h>
#include <error.h>
#include "debug.h"

#define errno (*__errno())
extern int* __errno (void);
extern double BIGX;
extern double SMALLX;
double BIGX;
double SMALLX;
typedef const union
{
  long l;
  float f;
} ufloat;
typedef union
{
  float value;
  uint32_t word;
} ieee_float_shape_type;
ufloat  z_infinity_f;

/* Get a 32 bit int from a float.  */

#define GET_FLOAT_WORD(i,d)					\
do {								\
  ieee_float_shape_type gf_u;					\
  gf_u.value = (d);						\
  (i) = gf_u.word;						\
} while (0)

/* Set a float from a 32 bit int.  */
#define SET_FLOAT_WORD(d,i)					\
do {								\
  ieee_float_shape_type sf_u;					\
  sf_u.word = (i);						\
  (d) = sf_u.value;						\
} while (0)

float infinityf()
{
	float x;
	SET_FLOAT_WORD(x,0x7f800000);
	return x;
}

double infinity()
{
	return (double) infinityf();
}

float powf (float x, float y)
{
	NOT_IMPLEMENTED();
	return x;
}

#ifdef _DOUBLE_IS_32BITS

double pow (double x, double y)
{
	NOT_IMPLEMENTED();
	return (double) powf ((float) x, (float) y);
}

#endif /* defined(_DOUBLE_IS_32BITS) */ 
