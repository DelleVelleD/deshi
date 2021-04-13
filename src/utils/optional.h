#pragma once
#ifndef DESHI_OPTIONAL_H
#define DESHI_OPTIONAL_H

template <typename T>
struct Optional {
	T value;
	
	Optional() {
		memset(&value, 0xFFFFFFFF, sizeof(T));
	};
	
	Optional(T value) {
		this->value = value;
	}
	
	bool test(){
		T b; memset(&b, 0xFFFFFFFF, sizeof(T));
		return (memcmp(&value, &b, sizeof(T)))? true : false;
	}
	
	inline void reset(){
		memset(&value, 0xFFFFFFFF, sizeof(T));
	}
	
	inline bool operator==(const Optional& rhs) const{
		return value == rhs.value;
	}
	
	inline bool operator!=(const Optional& rhs) const{
		return value != rhs.value;
	}
};

#endif //DESHI_OPTIONAL_H