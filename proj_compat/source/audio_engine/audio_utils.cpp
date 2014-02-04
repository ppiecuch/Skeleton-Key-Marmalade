///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2013 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    audio_utils.cpp
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for Allacrost utility code.
*** ***************************************************************************/

// Headers included for directory manipulation. Windows has its own way of
// dealing with directories, hence the need for conditional includes
#ifdef _WIN32
	#include <direct.h>
	#include <shlobj.h>
#else
	#include <dirent.h>
	#include <sys/types.h>
	#include <pwd.h>
#endif

#include <iconv.h>
#include <sys/stat.h>
#if defined(__APPLE__)
# include <mach/mach.h>
# include <mach/mach_time.h>
#elif defined(__linux)
# include <time.h>
#elif defined(WIN32)
# include <windows.h>
#endif

#include "audio_utils.h"

using namespace std;

namespace hoa_utils {

////////////////////////////////////////////////////////////////////////////////
///// getCurrentTime (in float seconds)
////////////////////////////////////////////////////////////////////////////////

double getCurrentTime() {
#if defined(__APPLE__)
    static mach_timebase_info_data_t timebaseInfo;
    
    if (timebaseInfo.denom == 0) {
        mach_timebase_info(&timebaseInfo);
    }
    return mach_absolute_time() * (double) timebaseInfo.numer / timebaseInfo.denom * 0.000000001;
    
#elif defined(WIN32)
    static LARGE_INTEGER frequency;
    LARGE_INTEGER currentTime;
    
    if (frequency.QuadPart == 0) {
        QueryPerformanceFrequency(&frequency);
    }
    QueryPerformanceCounter(&currentTime);
    
    return (double) currentTime.QuadPart / frequency.QuadPart;
    
#elif defined(__linux) || defined(__QNXNTO__) || defined(__ANDROID__)
    struct timespec currentTime;
    
    clock_gettime(CLOCK_MONOTONIC, &currentTime);
    return currentTime.tv_sec + currentTime.tv_nsec * 0.000000001;
#endif
}

////////////////////////////////////////////////////////////////////////////////
///// ustring Class
////////////////////////////////////////////////////////////////////////////////

const size_t ustring::npos = ~0;



ustring::ustring() {
	_str.push_back(0);
}



ustring::ustring(const uint16* s) {
	_str.clear();

	if (!s) {
		_str.push_back(0);
		return;
	}

	while (*s != 0) {
		_str.push_back(*s);
		++s;
	}

	_str.push_back(0);
}


// Return a substring starting at pos, continuing for n elements
ustring ustring::substr(size_t pos, size_t n) const
{
	size_t len = length();

	if (pos >= len)
		throw std::out_of_range("pos passed to substr() was too large");

	ustring s;
	while (n > 0 && pos < len) {
		s += _str[pos];
		++pos;
		--n;
	}

	return s;
}


// Concatenates string to another
ustring ustring::operator + (const ustring& s)
{
    ustring temp = *this;

	// nothing to do for empty string
	if (s.empty())
		return temp;

	// add first character of string into the null character spot
	temp._str[length()] = s[0];

	// add rest of characters afterward
	size_t len = s.length();
	for (size_t j = 1; j < len; ++j) {
		temp._str.push_back(s[j]);
	}

	// Finish off with a null character
	temp._str.push_back(0);

	return temp;
}


// Adds a character to end of this string
ustring& ustring::operator += (uint16 c) {
	_str[length()] = c;
	_str.push_back(0);

	return *this;
}


// Concatenate another string on to the end of this string
ustring& ustring::operator += (const ustring& s) {
	// nothing to do for empty string
	if (s.empty())
		return *this;

	// add first character of string into the null character spot
	_str[length()] = s[0];

	// add rest of characters afterward
	size_t len = s.length();
	for (size_t j = 1; j < len; ++j) {
		_str.push_back(s[j]);
	}

	// Finish off with a null character
	_str.push_back(0);

	return *this;
}


// Will assign the current string to this string
ustring& ustring::operator = (const ustring& s) {
	clear();
	operator += (s);

	return *this;
}



bool ustring::operator == (const ustring& s)
{
    size_t len = length();
    if (s.length() != len)
        return false;

    for (size_t j = 0; j < len; ++j) {
        if (_str[j] != s[j] )
            return false;
    }

    return true;
}


// Finds a character within a string, starting at pos. If nothing is found, npos is returned
size_t ustring::find(uint16 c, size_t pos) const {
	size_t len = length();

	for (size_t j = pos; j < len; ++j) {
		if (_str[j] == c)
			return j;
	}

	return npos;
}


// Finds a string within a string, starting at pos. If nothing is found, npos is returned
size_t ustring::find(const ustring& s, size_t pos) const {
	size_t len = length();
	size_t total_chars = s.length();
	size_t chars_found = 0;

	for (size_t j = pos; j < len; ++j) {
		if (_str[j] == s[chars_found]) {
			++chars_found;
			if (chars_found == total_chars) {
				return (j - chars_found + 1);
			}
		}
		else {
			chars_found = 0;
		}
	}

	return npos;
}

} // namespace utils
