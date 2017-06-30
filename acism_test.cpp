
// g++ acism_test.cpp --std=c++11 -o acism_test libacism.a

#include <map>
#include <vector>
#include <cassert>
#include <string>
#include <iostream>
#include <iomanip>
using namespace std;

#include "msutil.h"
#include "acism.h"

/*
struct Test
{
    string id;
    MEMREF text;
    MEMREF* patterns = nullptr;
    int patternsCount = 0;
    map<string, int> expectedResults;
};
*/

struct Test
{
    string id;
    string textToScan;
    struct PatternAndCount
    {
        string pattern;
        int count;
    };

    // map<string, int> patternsAndResults;
    vector<PatternAndCount> patternsAndResults;
};


struct ACData
{
    MEMREF text;
    MEMREF* patterns = nullptr;
    map<string, int> matches;
};

// static int actual = 0;
static int
on_match(int strnum, int textpos, ACData* acData)
{
    // (void)strnum, (void)textpos, (void)pattv;
    MEMREF text = acData->text;
    MEMREF* pattv = acData->patterns;

    // note: textpos is the position of the END of the match
    // ++actual;
    // printf("%9d %7d    '%.*s'\n", textpos, strnum, (int)pattv[strnum].len, pattv[strnum].ptr);
    cout << string(45, '.') << endl;
    cout << setw(15) << textpos << setw(15) << strnum << setw(15) << pattv[strnum].ptr << endl;
    puts(text.ptr);
    for(int i = 0; i < text.len; i++)
    {
        if(i == (textpos - pattv[strnum].len))
        {
            putchar('B');
        }
        else if(i == textpos)
        {
            putchar('E');
        }
        else if((textpos - pattv[strnum].len) < i && i < textpos)
        {
            putchar('-');
        }
        else
        {
            putchar(' ');
        }
    }
    puts("");

    acData->matches[pattv[strnum].ptr]++;

    return 0;
}

/*
void test_AC(const char* id, const char* text, MEMREF* patterns, int patterns_count)
{
    ACISM* ac = acism_create(patterns, patterns_count);
    assert(ac);
    printf("-- Testing %s ------------------\n\n", id);
    printf("%s AC dump: ", id);
    acism_dump(ac, PS_STATS, stdout, patterns);
    printf("%s AC patterns:\n", id);
    for(int i = 0; i < patterns_count; i++)
        printf("\t%d %s\n", i, patterns[i].ptr);

    puts("");
    MEMREF t =
    {
        text, strlen(text)
    };

    Test test;
    test.text = t;
    test.patterns = patterns;

    printf("Now running on string '%s'\n", text);
    puts("");

    // printf("%s AC run:\n", id);
    int state = 0;
    printf("%9s %7s    Pattern\n", "EndPos", "Id");
    while(acism_more(ac, t, (ACISM_ACTION*)on_match, &test, &state));
    acism_destroy(ac);
    puts("");
}
*/

void testAC(const Test& test)
{
    // create the patterns array
    int patternsCount = test.patternsAndResults.size();
    MEMREF* patterns = new MEMREF[patternsCount];
    for(int i = 0; i < patternsCount; i++)
    {
        patterns[i].ptr = test.patternsAndResults[i].pattern.c_str();
        patterns[i].len = test.patternsAndResults[i].pattern.length();
    }

    // build the AC:
    ACISM* ac = acism_create(patterns, patternsCount);
    assert(ac);
    cout << "-- Testing " << test.id << " ------------------" << endl << endl;
    cout << test.id << " AC dump: ";
    acism_dump(ac, PS_STATS, stdout, patterns);
    cout << test.id << " AC patterns:" << endl;
    for(int i = 0; i < patternsCount; i++)
        cout << "\t" << i << "\t" << patterns[i].ptr << endl;

    // build the memref text to scan
    puts("");
    MEMREF t =
    {
        test.textToScan.c_str(), test.textToScan.length()
    };

    ACData acData;
    acData.text = t;
    acData.patterns = patterns;

    cout << "Now running on string <" << acData.text.ptr << ">" << endl;
    puts("");

    int state = 0;
    cout << setw(15) << "EndPos" << setw(15) << "Id" << setw(15) << "Pattern" << endl;
    while(acism_more(ac, t, (ACISM_ACTION*)on_match, &acData, &state));
    acism_destroy(ac);
    puts("");

    // check test results
    int errorsCount = 0;
    for(int i = 0; i < patternsCount; i++)
    {
        string pattern = test.patternsAndResults[i].pattern;
        int expectedMatches = test.patternsAndResults[i].count;
        int realMatches = acData.matches[pattern];
        if(expectedMatches != realMatches)
        {
            cout << "Mismatch on pattern <" << pattern << ">: expected: " << expectedMatches << ", real: " << realMatches << endl;
            errorsCount++;
        }
    }

    delete[] patterns;

    assert(!errorsCount);
}

int
main(int argc, char **argv)
{
// this is strange.
// run on string "bananas", will find 1 "ana", 2 "na", 1 "banana" and NO "nana".
// is this a bug or it's just me which do not know Aho-Corasick well enough?

/*
    const int uno_patterns_count = 4;
    MEMREF uno_patterns[uno_patterns_count] =
    {
        { "banana", strlen("banana")},
        { "nana", strlen("nana")},
        { "ana", strlen("ana")},
        { "na", strlen("na")},
    };
*/

    // vector<string> uno_patterns =

/*
    map<string, int> unoPatternsAndResults =
    {
        {"banana", 1},
        {"nana", 1},
        {"ana", 2},
        {"na", 2},
    };
*/

    Test uno_test;
    uno_test.id = "uno";
    uno_test.textToScan = "bananas";
    // uno_test.patterns = uno_patterns;
    // uno_test.patternsAndResults = unoPatternsAndResults;
    uno_test.patternsAndResults =
    {
        {"banana", 1},
        {"nana", 2},
        {"ana", 2},
        {"na", 2},
    };

    // test_AC("uno", "bananas", uno_patterns, uno_patterns_count);

    testAC(uno_test);


// this works as expected.
// run on string "bananas", will find 2 "na", 1 "banana" and 1 "nana".

/*
    const int due_patterns_count = 3;
    MEMREF due_patterns[due_patterns_count] =
    {
        { "banana", strlen("banana")},
        { "nana", strlen("nana")},
        { "na", strlen("na")},
    };

    map<string, int> due_expected_results =
    {
        {"banana", 1},
        {"nana", 1},
        {"ana", 0},
        {"na", 2},
    };


    test_AC("due", "bananas", due_patterns, due_patterns_count);
    */


    return 0;
}
