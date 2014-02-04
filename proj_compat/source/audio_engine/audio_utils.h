///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2013 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************(
*** \file    audio_utils.h
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Header file for Allacrost utility code.
***
*** This code includes various utility functions that are used across different
*** parts of the code base. This file is included in every header file in the
*** Allacrost source tree.
***
*** \note Use the following macros for OS-dependent code.
***   - Windows    #ifdef _WIN32
***   - Mac OS X   #ifdef __MACH__
***   - OpenDarwin #ifdef __MACH__
***   - Linux      #ifdef __linux__
***   - FreeBSD    #ifdef __FreeBSD__
***   - Solaris    #ifdef SOLARIS
***   - BeOS       #ifdef __BEOS__
***
*** \note Use the following macros for compiler-dependent code.
***   - MSVC       #ifdef _MSC_VER
***   - g++        #ifdef __GNUC__
***
*** \note Use the following statements to determine system endianess.
***   - Big endian      if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
***   - Little endian   if (SDL_BYTEORDER == SDL_LITTLE_ENDIAN)
***
*** \note Use the following integer types throughout the entire Allacrost code.
***   - int32
***   - uint32
***   - int16
***   - uint16
***   - int8
***   - uint8
***
*** \note Use the following string types througout the entire Allacrost code.
***   - ustring   Unicode strings, meant only for text to be rendered on the screen.
***   - string    Standard C++ strings, used for all text that is not to be rendered to the screen.
***   - char*     Acceptable, but use strings instead wherever possible.
*** ***************************************************************************/

#ifndef __AUDIO_UTILS_HEADER__
#define __AUDIO_UTILS_HEADER__

#include <cstdlib>
#include <cmath>
#include <cstring> // For C string manipulation functions like strcmp

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <stack>
#include <stdexcept>
#include <sstream>

#include <unistd.h>

#include <stdint.h> // Using the C header, because the C++ header, <cstdint> is only available in ISO C++0x

#ifdef __MACH__
	#include <OpenAL/al.h>
	#include <OpenAL/alc.h>
#elif __QNXNTO__
	#include <AL/al.h>
#else
	#include "al.h"
	#include "alc.h"
#endif

#ifdef _WIN32
	// Even though Allacrost is platform independent, OpenGL on Windows requires windows.h to be included
	#include <windows.h>
	// Case-insensitive string compare is called stricmp in Windows and strcasecmp everywhere else
	#ifndef strcasecmp
	#define strcasecmp stricmp
	#endif
#endif


/** \brief Forces the application to abort with an error
*** \param message An error message string to report
*** This macro throws an Exception which if unhandled, will be caught at the end of the main game loop.
*** Therefore the application will not terminate immediately, but instead will wait until the main game
*** loop reaches the end of the current iteration.
**/
#define ERROR_AND_ABORT(message)  (std::cerr<<message<<__FILE__<<__LINE__<<__FUNCTION__,exit(1))

/** \name Print Message Helper Macros
*** These macros assist programmers with writing debug, warning, or error messages that are to be printed to
*** a user's terminal. They are formatted as follows: `MSGTYPE:FILE:FUNCTION:LINE: `. To use the macro, all
*** that is needed is to add `<< "print message" << std::endl;` after the macro name.
**/
//@{
#define PRINT_DEBUG std::cout << "DEBUG:" << __FILE__ << ":" << __FUNCTION__ << ":" << __LINE__ << ": "
#define PRINT_WARNING std::cerr << "WARNING:" << __FILE__ << ":" << __FUNCTION__ << ":" << __LINE__ << ": "
#define PRINT_ERROR std::cerr << "ERROR:" << __FILE__ << ":" << __FUNCTION__ << ":" << __LINE__ << ": "
//@}

/** \name Print Message Helper Macros With Conditional
*** \param var Any type of variable that can be used to evaluate a true/false condition
*** These macros perform the exact same function as the previous set of print message macros, but these include a conditional
*** parameter. If the parameter is true the message will be printed and if it is false, no message will be printed. Note that
*** the if statement is not enclosed in brackets, so the programmer is not required to add a terminating bracket after they
*** append their print message.
*** \note There is no error conditional macro because detected errors should always be printed when they are discovered
**/
//@{
#define IF_PRINT_DEBUG(var) if (var) std::cout << "DEBUG:" << __FILE__ << ":" << __FUNCTION__ << ":" << __LINE__ << ": "
#define IF_PRINT_WARNING(var) if (var) std::cerr << "WARNING:" << __FILE__ << ":" << __FUNCTION__ << ":" << __LINE__ << ": "
//@}

/** \name Allacrost Integer Types
*** \brief These are the integer types used throughout the Allacrost source code.
*** These types are created by redefining the ANSI C types.
*** Use of the standard int, long, etc. is forbidden in Allacrost source code! Don't attempt to use any
*** 64-bit types either, since a large population of PCs in our target audience are not a 64-bit
*** architecture.
**/
//@{
typedef int32_t   int32;
typedef uint32_t  uint32;
typedef int16_t   int16;
typedef uint16_t  uint16;
typedef int8_t    int8;
typedef uint8_t   uint8;
//@}

//! Contains utility code used across the entire source code
namespace hoa_utils {

double getCurrentTime();

/**	\brief Converts data to string
*** \param s String to be filled
*** \param data Data to convert to string
***
*** This function converts any type of data to a string. It will only work if the data type has the
*** sstring::operator<< implemented.
**/
template<class T>
void DataToString(std::string &s, const T &data)
{
	std::ostringstream stream;
	stream << data;
	s = stream.str();
}


/** ****************************************************************************
*** \brief Implements unicode strings with uint16 as the character type
***
*** This class functions identically to the std::string class provided in the C++
*** standard library. The critical difference is that each character is 2 bytes
*** (16 bits) wide instead of 1 byte (8 bits) wide so that it may implement the
*** full unicode character set.
***
*** \note This class intentionally ignores the code standard convention for class
*** names because the class objects are to be used as if they were a standard C++ type.
***
*** \note This class does not implement a destructor because the only data member
*** (a std::vector) will automatically destroy itself when the no-arg destructor is invoked.
***
*** \note The member functions of this class are not documented because they function
*** in the exact same manner that the C++ string class does.
***
*** \note There are some libstdc++ compatability problems with simply defining
*** basic_string<uint16>, so this class is a custom version of it.
***
*** \note Currently not all functionality of basic_string has been implemented, but
*** instead only the functions that we typically use in Allacrost. If you need a
*** basic_string function available that isn't already implemented in this class,
*** go ahead and add it yourself.
***
*** \note This class does not use wchar_t because it has poor compatibility.
*** ***************************************************************************/
class ustring {
public:
	ustring();

	ustring(const uint16*);

	static const size_t npos;

	void clear()
		{ _str.clear(); _str.push_back(0); }

	bool empty() const
		{ return _str.size() <= 1; }

	size_t length() const
		// We assume that there is always a null terminating character, hence the -1 subtracted from the size
		{ return _str.size() - 1; }

	size_t size() const
		{ return length(); }

	const uint16* c_str() const
		{ return &_str[0]; }

	size_t find(uint16 c, size_t pos = 0) const;

	size_t find(const ustring& s, size_t pos = 0) const;

	ustring substr(size_t pos = 0, size_t n = npos) const;

	ustring operator + (const ustring& s);

	ustring& operator += (uint16 c);

	ustring& operator += (const ustring& s);

	ustring& operator = (const ustring& s);

	bool operator == (const ustring& s);

	uint16& operator [] (size_t pos)
		{ return _str[pos]; }

	const uint16& operator [] (size_t pos) const
		{ return _str[pos]; }

private:
	//! \brief The structure containing the unicode string data.
	std::vector<uint16> _str;
}; // class ustring


/** ****************************************************************************
*** \brief Used for transforming a standard class into a singleton class
***
*** This is a templated abstract class which classes may derive from to become
*** singleton classes. To create a new singleton type class, follow the steps below.
*** It is assumed that the desired class is called "ClassName".
***
*** -# In the header file, define the class as follows: class ClassName : public hoa_utils::Singleton<ClassName>
*** -# Make hoa_utils::Singleton<ClassName> a friend of ClassName in the header file
*** -# Put the ClassName() constructor in the private section of the class, and the destructor in the public section
*** -# Define the following function in the public section of the class and implement it: bool SingletonInitialize()
*** -# In the source file, set the static template member like so: template<> ClassName* Singleton<ClassName>::_singleton_reference = NULL
***
*** With this done, your new class should be ready to go. To create and use a singleton class, do the following:
***
*** -# Call ClassName* SingletonCreate() to get a pointer to the new singleton class instance
*** -# After the singleton object has been created, next call bool SingletonInitialize() to initialize the class
*** -# If you ever need to retreive a pointer for a singleton you've created, just call the static method const ClassName* SingletonGetReference()
*** -# Call SingletonDestroy to destroy the class and its underlying singleton. You can then create and initialize the singleton once more if you'd like.
***
*** \note The creation steps listed above are the only way to properly construct a singleton class object. Don't try to circumvent it, and never attempt to
*** modify the protected static member Singleton::_singleton_reference directly, except as instructed above.
***
*** \note Sometimes singleton classes need to refer to each other to initialize themselves, particularly with game engine components. That is the purpose
*** of the SingletonInitialize() method, so that all the singleton objects can be created and then reference each other when this method is invoked. It
*** can be viewed as a helper function to the class constructor.
***
*** \note For engine singleton classes, SingletonCreate(), SingletonDestroy(), and SingletonInitialize()
*** should only be called in main.cpp before the main game loop. There may be qualified exceptions to this
*** practice, however.
***
*** \note Most of our singleton classes also define a pointer to their singleton object inside the
*** source file of the class. For example, the AudioEngine singleton contains the AudioManager class object
*** name inside the hoa_audio namespace. Therefore you do not need to call the SingletonGetReference()
*** function when this object is made available.
*** ***************************************************************************/
template<typename T> class Singleton {
protected:
	//! \brief A reference to the singleton class instance itself
	static T* _singleton_reference;

	Singleton()
		{}

	virtual ~Singleton()
		{}

public:
	//! \brief Creates and returns an instance of the singleton class
	static T* SingletonCreate() {
		if (_singleton_reference == NULL) {
			_singleton_reference = new T();
		}
		else {
			std::cerr << "UTILS WARNING: Singleton::SingletonCreate() was invoked when the class object was already instantiated" << std::endl;
		}
		return _singleton_reference;
	}

	//! \brief Destroys the singleton class instance
	static void SingletonDestroy() {
		if (_singleton_reference != NULL) {
			delete _singleton_reference;
		}
		else {
			std::cerr << "UTILS WARNING: Singleton::SingletonDestroy() was invoked when the class object was not instantiated" << std::endl;
		}
		_singleton_reference = NULL;
	}

	//! \brief Returns a pointer to the singleton class instance (or NULL if the class is not instantiated)
	static const T* SingletonGetReference()
		{ return _singleton_reference; }

	/** \brief A method for the inheriting class to implement, which initializes the class
	*** \return True if initialization was successful, false if it was not
	**/
	virtual bool SingletonInitialize() = 0;

private:
	Singleton(const Singleton &s);
	Singleton& operator=(const Singleton &s);
}; // template<typename T> class Singleton



//! \name String Utility Functions
//@{
/** \brief Converts an integer type into a standard string
*** \param T The integer type to convert to a string
*** \return A std::string containing the parameter in string form
**/
template <typename T>
std::string NumberToString(const T t)
{
	std::ostringstream text("");
	text << static_cast<int32>(t);
	return text.str();
}

} // namespace hoa_utils

#endif // __AUDIO_UTILS_HEADER__
