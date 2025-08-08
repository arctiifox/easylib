#pragma once
#include <iostream>
#include <string>

// print (linux ver.)
extern "C" void print_lx(const char* str, long len);
// print (windows ver.)
extern "C" void print_wn(const char* str, long len);

// print fast
inline void print_f(const char* str, const long& len) {
    #if defined(_WIN32) || defined(_WIN64)
        print_wn(str, len);
    #else
        print_lx(str, len);
    #endif
}

// print
void print(const std::string& str) {
    const char* str2 = str.c_str();
    const size_t len = str.length();

    print_f(std::move(str2), std::move(len));
}