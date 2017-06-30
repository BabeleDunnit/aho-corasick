
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

struct Test
{
    string id;
    string textToScan;

    struct PatternAndCount
    {
        string pattern;
        int count;
    };

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
    MEMREF text = acData->text;
    MEMREF* pattv = acData->patterns;

    // note: textpos is the position of the END of the match
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
// NOTE: fixed by elimination of prune-backlinks

    Test test;
    test.id = "uno";
    test.textToScan = "bananas";
    // uno_test.patterns = uno_patterns;
    // uno_test.patternsAndResults = unoPatternsAndResults;
    test.patternsAndResults =
    {
        {"banana", 1},
        {"nana", 1},
        {"ana", 2},
        {"na", 2},
    };

    testAC(test);


// this works as expected.
// run on string "bananas", will find 2 "na", 1 "banana" and 1 "nana".

    test.id = "due";
    test.patternsAndResults =
    {
        {"banana", 1},
        {"nana", 1},
        {"ana", 0},
        {"na", 2},
    };


    testAC(test);

    return 0;
}
