#pragma once
#include "redirection.hpp"
#include <iostream>
#include <vector>
#include <fstream>

using namespace std;

struct RedirectionGuard
{
    streambuf *savedCout = nullptr;
    streambuf *savedCin = nullptr;
    streambuf *savedCerr = nullptr;
    ofstream outFile;
    ofstream errFile;
    ofstream inFile;

    void apply(const vector<Redirection> &redirections)
    {
        for (const auto &r : redirections)
        {
            switch (r.type)
            {
            case RedirectionType::STDOUT_OVERWRITE:
                outFile.open(r.file, ios::out | ios::trunc);
                savedCout = cout.rdbuf(outFile.rdbuf());
                break;
            case RedirectionType::STDOUT_APPEND:
                outFile.open(r.file, ios::out | ios::app);
                savedCout = cout.rdbuf(outFile.rdbuf());
                break;
            case RedirectionType::STDIN_FILE:
                inFile.open(r.file);
                savedCin = cin.rdbuf(inFile.rdbuf());
                break;
            case RedirectionType::STDERR_FILE:
                errFile.open(r.file, ios::out | ios::trunc);
                savedCerr = cerr.rdbuf(errFile.rdbuf());
                break;
            case RedirectionType::STDERR_TO_STDOUT:
                savedCerr = cerr.rdbuf(cout.rdbuf());
                break;
            case RedirectionType::BOTH_FILE:
                outFile.open(r.file, ios::out | ios::trunc);
                savedCout = cout.rdbuf(outFile.rdbuf());
                savedCerr = cerr.rdbuf(outFile.rdbuf());
                break;
            }
        }
    }

    void restore()
    {
        if (savedCout)
        {
            cout.rdbuf(savedCout);
            savedCout = nullptr;
        }
        if (savedCerr)
        {
            cerr.rdbuf(savedCerr);
            savedCerr = nullptr;
        }
        if (savedCin)
        {
            cin.rdbuf(savedCin);
            savedCin = nullptr;
        }
        if (outFile.is_open())
            outFile.close();
        if (errFile.is_open())
            errFile.close();
        if (inFile.is_open())
            inFile.close();
    }

    ~RedirectionGuard()
    {
        restore();
    }
};