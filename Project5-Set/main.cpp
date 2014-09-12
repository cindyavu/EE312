#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include "Set.h"

/* Globals */
unsigned time();
int const number_of_tests = 1000;
int const maximum_set_size = 100;

volatile int bogus_value; // used to ensure compiler does not optimize away our func calls

Set* setA;
Set* setB;
Set* setAPrime; // almost exactly the same as A

int is_mem_x;

int sizes[] = { 100, 400, 1600, 6400, 25600 };
#define num_times (sizeof(sizes) / sizeof(int))

Set setsA[num_times];
Set setsAPrime[num_times];
Set setsB[num_times];
double times[num_times];

/* NOTE: the order of these strings MUST MATCH EXACTLY the order of the Scales enum */
char* scaling_strings[] = {
	"sub linear (YAHOO!)",
	"linear",
	"super linear, yuck."
};

enum Tests {
	IS_MEMBER = 0,
	INSERT = 1,
	REMOVE = 2,
	IS_EQUAL_TO = 3,
	IS_SUBSET_OF = 4,
	UNION_IN = 5,
	INTERSECT_FROM = 6,
	SUBTRACT_FROM = 7,

	NUM_TESTS /* MUST BE LAST */
};

typedef enum Scales {
	SUB_LINEAR,
	LINEAR,
	SUPER_LINEAR
} Scales;

typedef void(*Function)(void);
double timeFunction(Function f);


void randomSet(Set* s);
void visualTests(void);
void testTime(void);
/* Support function for timing checks*/
void isMemberTimeFun(void);
void isEqualTimeFun(void);
void isSubsetTimeFun(void);
void unionTimeFun(void);
void intersectTimeFun(void);
void subtractTimeFun(void);
bool checkLinear(double times[], int sizes[], int first, int last);
bool checkSubLinear(double times[], int sizes[], int first, int last);
Scales determineScaling(Function fun);
void createRandomSetN(int n, Set* a, Set* b);

void main (void) {
	visualTests();
	testTime();
	printf("The tests are over\n"); 
}

void showOutput(const char* str, Set* set) {
	printf(str);
	displaySet(set);
	printf("\n");
}
void randomSet(Set* s) {
	Set t;
	int n = rand() % maximum_set_size + 1;
	int i;

	createEmptySet(&t);
	for (i = 0; i < n; i += 1) {
		insertSet(&t, rand() % maximum_set_size);
	}
	assignSet(s, &t);
	destroySet(&t);
}
void visualTests() {
	Set s;
	Set t; 
	int i;
	createEmptySet(&s);
	showOutput("The set constructed with the default constructor: ", &s);

	for (i = 0; i < 10; i += 1) {
	  insertSet(&s, i);
	}
	showOutput("The set should be {0, ..., 9}:  ", &s);
	
	// test Insert() and Contains() with '<<'
	for (i = 0; i < 10; i += 1) {
		insertSet(&s, i);
	}
	showOutput("The set should be {0, ..., 9}:  ", &s);

	createCopySet(&t, &s);
	showOutput("The copy of s constructed with the copy constructor = ", &t);

	randomSet(&t);
	showOutput("The random set generated equals = ", &t);

	printf("The visual tests are over\n"); 
	destroySet(&s);
	destroySet(&t);
}



void testTime(void) {
	Scales behavior[NUM_TESTS];
	int k;
	int x;
	for (k = 0; k < num_times; k += 1) {
		printf("creating a random set with %d elements...", sizes[k]);
		fflush(stdout);
		createRandomSetN(sizes[k], &setsA[k], &setsB[k]);
		x = 10 * sizes[k];
		createCopySet(&setsAPrime[k], &setsA[k]);
		insertSet(&setsA[k], x);
		insertSet(&setsAPrime[k], x + 1);
		insertSet(&setsB[k], x);
		printf("done\n");
	}

	printf("checking scaling of isMember\n");
	behavior[IS_MEMBER] = determineScaling(&isMemberTimeFun);
	printf("isMember's scaling appears to be: %s\n", scaling_strings[behavior[IS_MEMBER]]);

	printf("checking scaling of isEqualTo\n");
	behavior[IS_EQUAL_TO] = determineScaling(&isEqualTimeFun);
	printf("isEqualTo's scaling appears to be: %s\n", scaling_strings[behavior[IS_EQUAL_TO]]);

	printf("checking scaling of isSubsetOf\n");
	behavior[IS_SUBSET_OF] = determineScaling(&isSubsetTimeFun);
	printf("isSubsetOf's scaling appears to be: %s\n", scaling_strings[behavior[IS_SUBSET_OF]]);

	printf("checking scaling of unionIn\n");
	behavior[UNION_IN] = determineScaling(&unionTimeFun);
	printf("unionIn's scaling appears to be: %s\n", scaling_strings[behavior[UNION_IN]]);

	printf("checking scaling of intersectFrom\n");
	behavior[INTERSECT_FROM] = determineScaling(&intersectTimeFun);
	printf("intersectFrom's scaling appears to be: %s\n", scaling_strings[behavior[INTERSECT_FROM]]);

	printf("checking scaling of subtractFrom\n");
	behavior[SUBTRACT_FROM] = determineScaling(&subtractTimeFun);
	printf("subtractFrom's scaling appears to be: %s\n", scaling_strings[behavior[SUBTRACT_FROM]]);

	printf("Performance Summary:\n");
	printf("isMemberSet\t\t%s\n", scaling_strings[behavior[IS_MEMBER]]);
	printf("isEqualToSet\t\t%s\n", scaling_strings[behavior[IS_EQUAL_TO]]);
	printf("isSubsetOf\t\t%s\n", scaling_strings[behavior[IS_SUBSET_OF]]);
	printf("unionInSet\t\t%s\n", scaling_strings[behavior[UNION_IN]]);
	printf("intersectFromSet\t%s\n", scaling_strings[behavior[INTERSECT_FROM]]);
	printf("subtractFromSet\t\t%s\n", scaling_strings[behavior[SUBTRACT_FROM]]);
}


double timeFunction(Function f) {
	int reps = 100;
	double start = 0;
	double end = 0;
	volatile int k;
	while ((end - start) < 6) {
		assert(reps < 1000000000); // avoid overflow
		start = time(0);
		for (k = 0; k < reps; k += 1) {
			f();
		}
		end = time(0);
		reps *= 2;
	}

	return (end - start) / reps;
}


void isMemberTimeFun(void) {
	if (isMemberSet(setA, is_mem_x)) {
		is_mem_x ^= 1;
		is_mem_x <<= 1;
	}
	else {
		is_mem_x >>= 1;
		is_mem_x ^= 2;
	}
}

void isEqualTimeFun(void) {
	bogus_value = isEqualToSet(setA, setAPrime);
}

void isSubsetTimeFun(void) {
	bogus_value = isSubsetOf(setA, setAPrime);
}

void unionTimeFun(void) {
	Set t;
	createCopySet(&t, setA);
	unionInSet(&t, setB);
	destroySet(&t);
}

void intersectTimeFun(void) {
	Set t;
	createCopySet(&t, setA);
	intersectFromSet(&t, setB);
	destroySet(&t);
}

void subtractTimeFun(void) {
	Set t;
	createCopySet(&t, setA);
	subtractFromSet(&t, setB);
	destroySet(&t);
}

bool checkLinear(double times[], int sizes[], int first, int last) {
	double time_ratio = times[last] / times[first];
	double size_ratio = sizes[last] / sizes[first];

	if (time_ratio < size_ratio * 1.25) {
		return true;
	}
	else {
		return false;
	}
}

bool checkSubLinear(double times[], int sizes[], int first, int last) {
	double time_ratio = times[last] / times[first];
	double size_ratio = sizes[last] / sizes[first];

	if (time_ratio < size_ratio * 0.5) {
		return true;
	}
	else {
		return false;
	}
}

Scales determineScaling(Function fun) {
	bool linear;
	bool sublinear;

	unsigned trial = 0;
	for (; trial < num_times; trial += 1) {
		if (trial > 0 && ((times[trial - 1] * sizes[trial])	> 10)) { break; }
		setA = &setsA[trial];
		setB = &setsB[trial];
		setAPrime = &setsAPrime[trial];
		times[trial] = timeFunction(fun);
		printf("time taken for size %d was %g\n",
			sizes[trial], times[trial]);
	}

	if (trial < 3) {
		printf("I'm sorry, your program is way too inefficient.\n");
		printf("you'll need to find a way-faster computer to test it on\n");
		return SUPER_LINEAR;
	}
	linear = checkLinear(times, sizes, 0, trial - 1)
		|| checkLinear(times, sizes, 1, trial - 1);
	sublinear = checkSubLinear(times, sizes, 0, trial - 1)
		|| checkSubLinear(times, sizes, 1, trial - 1);

	if (sublinear) { return SUB_LINEAR; }
	else if (linear) { return LINEAR; }
	else { return SUPER_LINEAR; }
}

/*
* create two sets with n random elements
* the sets should have 50% of the elements in common
*/
void createRandomSetN(int n, Set* a, Set* b) {
	int x;
	int last_size = 0;
	createEmptySet(a);
	while (a->len < n) {
		if (a->len != last_size) {
			last_size = a->len;
			if (last_size % 1000 == 0) {
				printf("%d..", last_size);
				fflush(stdout);
			}
		}
		x = 2 * (5 * n - (rand() % (10 * n)));
		if (isMemberSet(a, x)) { continue; } // try another number
		/* a will have only even elements */
		insertSet(a, x);
		if ((rand() % 2) == 0) {
			insertSet(b, x);
		}
		else {
			insertSet(b, x + 1); // an odd value, can't be in a
		}
	}
	assert(a->len == b->len);
}

