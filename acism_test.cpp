
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
    for(int i = 0; i <= text.len; i++)
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
            putchar('_');
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
    int patternsCount = 0;
    MEMREF* patterns = new MEMREF[test.patternsAndResults.size()];
    for(int i = 0; i < test.patternsAndResults.size(); i++)
    {
        // if the expected result is 0, I must NOT add the pattern..
        if(test.patternsAndResults[i].count > 0)
        {
            patterns[patternsCount].ptr = test.patternsAndResults[i].pattern.c_str();
            patterns[patternsCount].len = test.patternsAndResults[i].pattern.length();
            patternsCount++;
        }
    }

    // build the AC:
    ACISM* ac = acism_create(patterns, patternsCount);
    assert(ac);
    cout << "-- Testing " << test.id << " ----------------------------------------------------------------" << endl << endl;
    cout << "Scanning string: <" << test.textToScan << ">" << endl;
    cout << "Expected results: " << endl;
    for(int i = 0; i < test.patternsAndResults.size(); i++)
    {
        cout << setw(15) << test.patternsAndResults[i].pattern << ": " <<  setw(5) << test.patternsAndResults[i].count << endl;
    }

    cout << endl;
    acism_dump(ac, PS_STATS, stdout, patterns);
    cout << test.id << " AC patterns:" << endl;
    for(int i = 0; i < patternsCount; i++)
        cout << setw(10) << i << setw(15) << patterns[i].ptr << endl;

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
    test.id = "banana1";
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

    test.id = "banana2";
    test.patternsAndResults =
    {
        {"banana", 1},
        {"nana", 1},
        {"ana", 0},
        {"na", 2},
    };
    testAC(test);

// this will fail

    test.id = "firefox";
    // test.textToScan = "***/2.----Firefox/2.0";
    test.textToScan = "/2. Firefox/2.0";
    test.patternsAndResults =
    {
        {"/2.", 2},
        {"Firefox/2.0", 1},
    };
    testAC(test);

    return 0;
}
