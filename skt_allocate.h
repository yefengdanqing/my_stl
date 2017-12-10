#ifndef SKT_DEFAULT_ALLOCATE_H
#define SKT_DEFAULT_ALLOCATE_H
	
	
#include<new>
#include<stdlib.h>
#include<stddef.h>
#include<limits.h>
#include<iostream>
#include"algobase.h"


template<class T>
inline T* allocate(ptrdiff_t size,T*)
{
	T* tmp = (T*)(::operator new((size_t)(size*sizeof(T))));
	if(tmp ==0)
	{
		_STD::cerr<<"out of memory"<<_STD::endl;
	}
	return tmp;
}
template<class T> inline void deallocate(T* buf)
{
	if(buf !=0)
	{
		::operator delete(buf);
	}
}


template<typename T> class allocator
{
public:
	typedef T value_type;
	typedef T* pointer;
	typedef const T* const_pointer;
	typedef T& reference;
	typedef const T& const_reference;
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	pointer allocate(size_type n)
	{
		return ::allocate((difference_type)n,(pointer)0);
	}
	pointer address(reference x)
	{
		return (pointer)&x;
	}
	const_pointer const_address(reference x)
	{
		return (pointer)&x;
	}
	size_type init_page_size()
	{
		return max(size_type(1),4096/sizeof(T));
	}
	size_type max_size()
	{
		return max(size_type(1),UINT_MAX/sizeof(T));
	}
};

template<> class allocator<void>
{
public:
		typedef void* pointer;
};
#endif
