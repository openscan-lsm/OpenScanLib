/*
 * We do not have good unit tests yet; this is just a quick setup so that there
 * is a place to add regression tests.
 *
 * Make sure to switch to a full-featured unit testing framework (perhaps
 * Unity, which has the advantage of being in pure C) before adding more than a
 * few more tests.
 */


/*
 * MinUnit - http://www.jera.com/techinfo/jtns/jtn002.html
 *
 * License for MinUnit:
 * You may use the code in this tech note for any purpose, with the
 * understanding that it comes with NO WARRANTY.
 */
#define mu_assert(message, test) do { if (!(test)) return message; } while (0)
#define mu_run_test(test) do { char *message = test(); tests_run++; \
                               if (message) return message; } while (0)
extern int tests_run;
/* End of MinUnit */


#include <stdio.h>

#include "OpenScanLibPrivate.h"


static char *test_NumRange_Intersection(void) {
	OScInternal_NumRange *bigRange =
		OScInternal_NumRange_CreateContinuous(1e-6, 1e6);
	OScInternal_NumRange *smallRange =
		OScInternal_NumRange_CreateContinuous(0.5, 40.0);
	OScInternal_NumRange *overlap =
		OScInternal_NumRange_Intersection(smallRange, bigRange);

	mu_assert("continuous range expected", !OScInternal_NumRange_IsDiscrete(overlap));
	mu_assert("correct minimum expected", OScInternal_NumRange_Min(overlap) == 0.5);
	mu_assert("correct maximum expected", OScInternal_NumRange_Max(overlap) == 40.0);

	return NULL;
}


static char *all_tests(void) {
	mu_run_test(test_NumRange_Intersection);

	return NULL;
}


int tests_run;

int main() {
	char *result = all_tests();

	if (result != NULL)
		printf("%s\n", result);
	else
		printf("ALL TESTS PASSED\n");

	printf("Tests run: %d\n", tests_run);

	return result != NULL;
}
