#pragma once
#ifndef DESHI_ANY_H
#define DESHI_ANY_H

struct Any{
	void*       value = 0;
	const char* type = 0;
	
	template <typename T>
		Any(const T& value){
		value = value;
		type = typeid(value).name;
	}
};

#endif //DESHI_ANY_H
