
// gcc acism_banana_test.c -o acism_banana_test libacism.a

#include "msutil.h"
#include <assert.h>
#include "acism.h"

static int actual = 0;
static int
on_match(int strnum, int textpos, MEMREF const *pattv)
{
    (void)strnum, (void)textpos, (void)pattv;
    ++actual;
    printf("%9d %7d    '%.*s'\n", textpos, strnum, (int)pattv[strnum].len, pattv[strnum].ptr);
    return 0;
}


void test_AC(const char* id, const char* text, MEMREF* patterns, int patterns_count)
{
    ACISM* ac = acism_create(patterns, patterns_count);
    assert(ac);
    printf("-- Testing %s ------------------\n\n", id);
    printf("%s AC dump: ", id);
    acism_dump(ac, PS_STATS, stdout, patterns);
    printf("%s AC patterns:\n", id);
    for(int i = 0; i < patterns_count; i++)
        printf("\t%s\n", patterns[i].ptr);

    puts("");
    MEMREF t =
    {
        text, strlen(text)
    };

    printf("Now running on string '%s'\n", text);

    printf("%s AC run:\n", id);
    int state = 0;
    printf("%9s %7s    Pattern\n", "Pos", "Id");
    while(acism_more(ac, t, (ACISM_ACTION*)on_match, patterns, &state));
    acism_destroy(ac);
    puts("");
}


int
main(int argc, char **argv)
{
// this is strange.
// run on string "bananas", will find 1 "ana", 2 "na", 1 "banana" and NO "nana".
// is this a bug or it's just me which do not know Aho-Corasick well enough?

#define BUGGED_PATTERNS_COUNT 4

    MEMREF bugged_patterns[BUGGED_PATTERNS_COUNT] =
    {
        { "banana", strlen("banana")},
        { "nana", strlen("nana")},
        { "ana", strlen("ana")},
        { "na", strlen("na")},
    };

// this works as expected.
// run on string "bananas", will find 2 "na", 1 "banana" and 1 "nana".

#define CORRECT_PATTERNS_COUNT 3

    MEMREF correct_patterns[CORRECT_PATTERNS_COUNT] =
    {
        { "banana", strlen("banana")},
        { "nana", strlen("nana")},
        { "na", strlen("na")},
    };

    test_AC("uno", "bananas", bugged_patterns, BUGGED_PATTERNS_COUNT);
    test_AC("due", "bananas", correct_patterns, CORRECT_PATTERNS_COUNT);

    return 0;
}
