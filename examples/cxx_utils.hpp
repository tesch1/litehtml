// -*-  Mode: C++; c-basic-offset: 4 -*-
/**
 * c++ utility functions
 *
 * (c)2016 Michael Tesch. tesch1@gmail.com
 */
#pragma once

#include <algorithm>
#include <vector>
#include <string>

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

struct MatchSep {
    bool operator()(char ch) const { return ch == '/'; }
};

inline std::string cxx_basename(std::string pathname)
{
    // remove trailing slashes
    if (pathname.size() > 1 && *pathname.rbegin() == '/')
        pathname.erase(pathname.end() - 1);
    return std::string(std::find_if(pathname.rbegin(), pathname.rend(), MatchSep()).base(), pathname.end());
}

inline std::string cxx_basename(const char * cpathname)
{
    std::string pathname(cpathname);
    return cxx_basename(pathname);
}

inline std::string cxx_dirname(std::string source)
{
    if (source.size() > 1 && *source.rbegin() == '/')
        source.erase(source.end() - 1);
    while (source.size() > 1 && *source.rbegin() != '/')
        source.erase(source.end() - 1);
    if (source.size() > 1 && *source.rbegin() == '/')
        source.erase(source.end() - 1);
    return source;
}

inline bool cxx_ends_with(std::string const & value, std::string const & ending)
{
    if (ending.size() > value.size())
        return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

