#pragma once
#include "redirection.hpp"
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <memory>

using namespace std;

struct RedirectionGuard
{
    streambuf *savedCout = nullptr;
    streambuf *savedCin = nullptr;
    streambuf *savedCerr = nullptr;
    ofstream outFile;
    ofstream errFile;
    unique_ptr<istringstream> inBuf;

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
            case RedirectionType::STDIN_FILE: {
                ifstream f(r.file);
                if (f.is_open()) {
                    ostringstream oss;
                    oss << f.rdbuf();
                    inBuf = make_unique<istringstream>(oss.str());
                    savedCin = cin.rdbuf(inBuf->rdbuf());
                } else {
                    cerr << "bash: " << r.file << ": No such file or directory\n";
                }
                break;
            }
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
        inBuf.reset();
    }

    ~RedirectionGuard()
    {
        restore();
    }
};