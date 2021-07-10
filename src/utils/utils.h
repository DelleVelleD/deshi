#pragma once
#ifndef UTILS_H
#define UTILS_H

#include "tuple.h"
#include "../defines.h"

#include <string>
#include <vector>

namespace Utils{
	
	
	///////////////////////////////
	//// FNV-1a hash functions ////
	///////////////////////////////
	
	
	u32 dataHash32(const void* data, size_t data_size, u32 seed = 2166136261); //32bit FNV_offset_basis
	u64 dataHash64(const void* data, size_t data_size, u64 seed = 14695981039346656037); //64bit FNV_offset_basis
	u32 stringHash32(const char* data, size_t data_size = 0, u32 seed = 2166136261); //32bit FNV_offset_basis
	u64 stringHash64(const char* data, size_t data_size = 0, u64 seed = 14695981039346656037); //64bit FNV_offset_basis
	
	
	///////////////////////////////
	//// std::string functions ////
	///////////////////////////////
	
	
	//returns a new string with the leading spaces removed
	std::string eatSpacesLeading(std::string text);
	
	//returns a new string with the trailing spaces removed
	std::string eatSpacesTrailing(std::string text);
	
	//returns a new string with the comments removed
	//NOTE all comment_characters are compared against to start a comment
	std::string eatComments(std::string text, const char* comment_characters);
	
	//separates a string by specified character
	std::vector<std::string> characterDelimit(std::string text, char character);
	
	//separates a string by specified character, ignores sequences of the character
	//eg: 1,,,,2,3 is the same as 1,2,3
	std::vector<std::string> characterDelimitIgnoreRepeat(std::string text, char character);
	
	//separates a string by spaces, ignores leading and trailing spaces
	std::vector<std::string> spaceDelimit(std::string text);
	
	//separates a string by spaces, ignores leading and trailing spaces
	//also ignores spaces between double quotes
	std::vector<std::string> spaceDelimitIgnoreStrings(std::string text);
	
	
	////////////////////////////
	//// c-string functions ////
	////////////////////////////
	
	
	//returns a pointer to the first character that is not a space
	char* skipSpacesLeading(const char* text, const char* text_stop = 0);
	
	//returns a pointer to the last character that is not a space
	char* skipSpacesTrailing(const char* text, const char* text_stop = 0);
	
	//returns a start-stop pointer pair to the characters which are not commented out
	pair<char*,char*> skipComments(const char* text, const char* comment_chararacters);
	pair<char*,char*> skipComments(const char* text, const char* text_stop, const char* comment_chararacters);
	
	//returns an array of start-stop pointer pairs to characters separated by the specified character
	//NOTE the caller is responsible for freeing the array this allocates
	pair<char*,char*>* characterDelimit(const char* text, char character);
	
	//returns an array of start-stop pointer pairs to characters separated by any number of the specified character
	//eg: 1,,,,2,3 is treated the same as 1,2,3
	//NOTE the caller is responsible for freeing the array this allocates
	pair<char*,char*>* characterDelimitIgnoreRepeat(const char* text, char character);
	
	//returns an array of the characters separated by spaces
	//ignores leading and trailing spaces
	//NOTE the caller is responsible for freeing the array this allocates
	pair<char*,char*>* spaceDelimit(const char* text);
	
	//returns an array of the characters separated by spaces
	//ignores leading, trailing, and double-quotes-encapsulated spaces 
	//eg: '1 2 "3 4" 5' creates 4 items: ["1", "2", "3 4", "5"]
	//NOTE the caller is responsible for freeing the array this allocates
	pair<char*,char*>* spaceDelimitIgnoreStrings(const char* text);
	
	
}; //namespace Utils


///////////////////////////////
//// FNV-1a hash functions ////
///////////////////////////////


//ref: https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
//ref: imgui.cpp ImHashData https://github.com/ocornut/imgui

inline u32 Utils::
dataHash32(const void* _data, size_t data_size, u32 seed){
	const u8* data = (const u8*)_data;
	while(data_size-- != 0){
		seed ^= *data++;
		seed *= 16777619; //32bit FNV_prime
	}
	return seed;
}

inline u64 Utils::
dataHash64(const void* _data, size_t data_size, u64 seed){
	const u8* data = (const u8*)_data;
	while(data_size-- != 0){
		seed ^= *data++;
		seed *= 1099511628211; //64bit FNV_prime
	}
	return seed;
}

inline u32 Utils::
stringHash32(const char* _data, size_t data_size, u32 seed){
	const u8* data = (const u8*)_data;
	if(data_size){
		while(data_size-- != 0){
			seed ^= *data++;
			seed *= 16777619; //32bit FNV_prime
		}
	}else{
		while(u8 c = *data++){
			seed ^= *data++;
			seed *= 16777619; //32bit FNV_prime
		}
	}
	return seed;
}

inline u64 Utils::
stringHash64(const char* _data, size_t data_size, u64 seed){
	const u8* data = (const u8*)_data;
	if(data_size){
		while(data_size-- != 0){
			seed ^= *data++;
			seed *= 1099511628211; //64bit FNV_prime
		}
	}else{
		while(u8 c = *data++){
			seed ^= *data++;
			seed *= 1099511628211; //64bit FNV_prime
		}
	}
	return seed;
}


///////////////////////////////
//// std::string functions ////
///////////////////////////////


inline std::string Utils::
eatSpacesLeading(std::string text){
	size_t idx = text.find_first_not_of(' ');
	return (idx != -1) ? text.substr(idx) : "";
}

inline std::string Utils::
eatSpacesTrailing(std::string text){
	size_t idx = text.find_last_not_of(' ');
	return (idx != -1) ? text.substr(0, idx+1) : "";
}

inline std::string Utils::
eatComments(std::string text, const char* comment_characters){
	size_t idx = text.find_first_of(comment_characters);
	return (idx != -1) ? text.substr(0, idx) : text;
}

inline std::vector<std::string> Utils::
characterDelimit(std::string text, char character){
	std::vector<std::string> out;
	
	int prev = 0;
	for(int i=0; i < text.size(); ++i){
		if(text[i] == character){
			out.push_back(text.substr(prev, i-prev));
    		prev = i+1;
		}
	}
	out.push_back(text.substr(prev, -1));
	
	return out;
}

inline std::vector<std::string> Utils::
characterDelimitIgnoreRepeat(std::string text, char character){
	std::vector<std::string> out;
	
	int prev = 0;
	for(int i=0; i < text.size(); ++i){
		if(text[i] == character){
			out.push_back(text.substr(prev, i-prev));
			while(text[i+1] == ' ') ++i;
    		prev = i+1;
		}
	}
	out.push_back(text.substr(prev, -1));
	
	return out;
}

inline std::vector<std::string> Utils::
spaceDelimit(std::string text){
	std::vector<std::string> out;
	text = eatSpacesLeading(text);
	text = eatSpacesTrailing(text);
	
	int prev = 0;
	for(int i=0; i < text.size(); ++i){
		if(text[i] == ' '){
			out.push_back(text.substr(prev, i-prev));
			while(text[i+1] == ' ') ++i;
    		prev = i+1;
		}
	}
	out.push_back(text.substr(prev, -1));
	
	return out;
}

inline std::vector<std::string> Utils::
spaceDelimitIgnoreStrings(std::string text){
	std::vector<std::string> out;
	text = eatSpacesLeading(text);
	text = eatSpacesTrailing(text);
	
	size_t prev = 0, end_quote = 0;
	forI(text.size()){
		if(text[i] == ' '){
			out.push_back(text.substr(prev, i-prev));
			while(text[i+1] == ' ') ++i;
			prev = i+1;
    		while(text[prev] == '\"'){
    		    end_quote = text.find_first_of('\"', prev+1);
    		    if(end_quote != -1){
    		        out.push_back(text.substr(prev+1, end_quote-prev-1));
    		        i = end_quote+1;
					if(i >= text.size()) return out;
    		        prev = i+1;
    		    }else{
					Assert(!"Opening quote did not have a closing quote in the text");
    		        return std::vector<std::string>();
    		    }
    		}
		}
	}
	out.push_back(text.substr(prev, -1));
	
	return out;
}

/*
inline char* Utils::
skipSpacesLeading(const char* text, const char* text_stop){

}

inline char* Utils::
skipSpacesTrailing(const char* text, const char* text_stop){

}

inline pair<char*,char*> Utils::
skipComments(const char* text, const char* comment_chararacters){

}

inline pair<char*,char*> Utils::
skipComments(const char* text, const char* text_stop, const char* comment_chararacters){

}

inline pair<char*,char*>* Utils::
characterDelimit(const char* text, char character){

}

inline pair<char*,char*>* Utils::
characterDelimitIgnoreRepeat(const char* text, char character){

}

inline pair<char*,char*>* Utils::
spaceDelimit(const char* text){

}

inline pair<char*,char*>* Utils::
spaceDelimitIgnoreStrings(const char* text){

}
*/

#endif //UTILS_H
