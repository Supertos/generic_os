/* Supertos Industries
	Generic OS Kernel Random function
*/
#include "Random.h"

#include <stdint.h>
#include <stddef.h>

typedef struct Seed Seed;
struct Seed {
	size_t x;
	size_t y;
	size_t z;
	size_t w;
};

struct Seed SEED = {.x = 1565656567876664, .y = 5486757654245657683, .z = 94587956787643467, .w = INT64_MAX };


double Random() {
  size_t t = SEED.x^(SEED.x<<11);
  SEED.x = SEED.y;
  SEED.y = SEED.z;
  SEED.z = SEED.w;
  SEED.w = (SEED.w^(SEED.w>>19)) ^ (t^(t>>8));
  return (double)SEED.w / (double)(~((size_t)0));
}