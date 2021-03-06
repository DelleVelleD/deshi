#pragma once
#ifndef DESHI_TUPLE_H
#define DESHI_TUPLE_H

#include "../defines.h"
#include <stdlib.h>

template<class... T>
struct tuple {
	void* mem = 0;
	u32* sizes = 0;
	u32 total = 0;
	
	tuple(T... args) {
		const int numargs = sizeof...(T);
		const int uintsize = sizeof(0U);
		
		mem = malloc((sizeof(args) + ...));
		sizes = (u32*)malloc(numargs * uintsize);
		
		total = (sizeof(args) + ...);
		
		void* ptrs[numargs] = { (void*)&args... };
		u32 sa[numargs] = { sizeof(args)... };
		
		for (int i = 0, j = 0; i < numargs; i++, j += sa[i]) {
			memcpy((char*)mem + j, ptrs[i], sa[i]);
			memcpy(sizes + i, &sa[i], uintsize);
		}
	}
	
	~tuple() {
		free(realloc(mem, total));
		free(realloc(sizes, sizeof...(T) * sizeof(0U)));
	}
	
	template<class S>
		S* get(int i) {
		for (int o = 0, k = 0; o < i; k += *(sizes + o), o++) {
			if (o == i - 1) return (S*)((char*)mem + k);
		}
	}
	
};

//TODO(delle) make_pair function that doesnt require specifying the template

//base case, never instantiated
template<typename... Dummy> struct pair;

template<typename T, typename U>
struct pair<T,U> {
	T first;
	U second;
	
	pair(T first, U second) {
		this->first = first;
		this->second = second;
	}
};

template<typename T, typename U, typename V>
struct pair<T,U,V> {
	T first;
	U second;
	V third;
	
	pair(T first, U second, V third) {
		this->first = first;
		this->second = second;
		this->third = third;
	}
};

template<typename T, typename U, typename V, typename W>
struct pair<T,U,V,W> {
	T first;
	U second;
	V third;
	W fourth;
	
	pair(T first, U second, V third, W fourth) {
		this->first = first;
		this->second = second;
		this->third = third;
		this->fourth = fourth;
	}
};

template<typename T, typename U, typename V, typename W, typename X>
struct pair<T,U,V,W,X> {
	T first;
	U second;
	V third;
	W fourth;
	X fifth;
	
	pair(T first, U second, V third, W fourth, X fifth) {
		this->first = first;
		this->second = second;
		this->third = third;
		this->fourth = fourth;
		this->fifth = fifth;
	}
};

#endif //DESHI_TUPLE_H