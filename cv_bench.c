/*
clang -Ofast -ocv.exe -DCVECTOR cv_bench.c
clang -Ofast -ouv.exe cv_bench.c

gcc -Ofast -ocv -DCVECTOR cv_bench.c
gcc -Ofast -ouv cv_bench.c

cl /O2 /Fecv -DCVECTOR cv_bench.c
cl /O2 /Feuv cv_bench.c

measure execution times of both exe on your env
*/

#ifdef CVECTOR
#include "cvector.h"
#else
#include "utarray.h"
#endif


int main(void)
{
#ifdef CVECTOR
    cvector_t v;
    cvector_t* pv = &v;
#else
    UT_array* nums;
#endif
    size_t i, j;
    int x = 0;

#define MAIN_LOOP 4000
#define INNER_LOOP 500000
#define SKIP_STEP 10000

#ifdef CVECTOR
    cvector_init(pv, sizeof(int), 1U, CVECTOR_DATA);
#else
    utarray_new(nums,&ut_int_icd);
#endif

    for (i = 0; i < MAIN_LOOP; i++, x = 0) {

        for (j = 0; j < INNER_LOOP; j++, x++) {
#ifdef CVECTOR
            cvector_push_back(pv, &x);
#else
            utarray_push_back(nums, &x);
#endif
        }

        for (j = 0; j < INNER_LOOP; j += SKIP_STEP, x++) {
#ifdef CVECTOR
            cvector_insert(pv, j, &x);
#else
            utarray_insert(nums, &x, j);
#endif
        }

        for (j = 0; j < INNER_LOOP; j += SKIP_STEP, x++) {
#ifdef CVECTOR
            cvector_erase(pv, j, 1U);
#else
            utarray_erase(nums, j, 1U);
#endif
        }

#ifdef CVECTOR
        cvector_clear(pv);
#else
        utarray_clear(nums);
#endif
    }

#ifdef CVECTOR
    cvector_destroy(pv);
#else
    utarray_free(nums);
#endif

    return 0;
}
