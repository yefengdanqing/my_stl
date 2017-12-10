#include<unistd.h>
#include<string.h>
#ifndef SKT_STL_INTER_ALLOC_H
#define SKT_STL_INTER_ALLOC_H


/*#ifndef THROW_BAD_ALLOC
	#define THROW_BAD_ALLOC fprintf(ceer,"out of memory\n");exit(0)
	#include<stddef.h>
	#include<string.h>*/
#if 0
	#include<new>
	#define _THROW_BAD_ALLOC throw bad_alloc
#elif !defined(_THROW_BAD_ALLOC)
	#include<iostream>
	#define _THROW_BAD_ALLOC std::cerr<<"out of memory"<<std::endl;exit(0)
//#else !if 
#endif

#include<stdlib.h>
#include<stdio.h>



//template<int ints> void (*malloc_alloc_template<ints>::__malloc_all_oom_handler)()=0;

template<int ints> class malloc_alloc_template{
private:
	static void (*__malloc_alloc_oom_handler)();
	static void* oom_malloc(size_t size);
	static void* oom_realloc(void* p,size_t size);
public:
	static void* allocate(size_t size)
	{
		void* result =malloc(size);
		if(0 == result)
		{
			result = oom_malloc(size);
		}
		return result;
	}

	static void deallocate(void* buf)
	{
		free(buf);
	}
	static void* reallocate(void* p,size_t old_size,size_t new_size)
	{
		void* result=realloc(p,new_size);
		if(0==result)
			result = oom_realloc(p,new_size);
		return result;
	}
	static void (*set_new_handle(void (*f)()))()
	{
		void (*old)()=__malloc_alloc_oom_handler;
//		__malloc_oom_handler=f;
		__malloc_alloc_oom_handler=f;
		return old;
	}
};

template<int ints> void (*malloc_alloc_template<ints>::__malloc_alloc_oom_handler)()=0;
template<int ints> void* malloc_alloc_template<ints>::oom_malloc(size_t n)
{
	void* result;
	void (*my_malloc_handler)();
	for(;;){
	my_malloc_handler=__malloc_alloc_oom_handler;
	if(my_malloc_handler ==0)
		_THROW_BAD_ALLOC;

	(*my_malloc_handler)();
	result=malloc(n);
	if(result)
		return result;
	}
}
template<int ints> void* malloc_alloc_template<ints>::oom_realloc(void* p,size_t n)
{
	void* result;
	void (*my_malloc_handler)();
	for(;;) {
	my_malloc_handler = __malloc_alloc_oom_handler;
	if(my_malloc_handler == 0)
		_THROW_BAD_ALLOC;
	(*my_malloc_handler)();
	result = realloc(p,n);
	if(result)
		return result;
	}
}
typedef malloc_alloc_template<0> malloc_alloc;

enum{
	_ALIGN = 8
};
enum {
	_MAX_BYTES = 128
};
enum {
	_NFREELISTS = _MAX_BYTES/_ALIGN
};

template<bool threads,int ints> 
class default_alloc_template
{
	private:
		union obj
		{
		//	union obj* free_list_link;
			obj* free_list_link;
			char client[1];
		};
		static obj* volatile free_lists[_NFREELISTS];
		
		//for chunk_alloc
		static char* start_free;
		static char* end_free;
		static size_t heap_size;
	private:
		static size_t FREELIST_INDEX(size_t size)
		{
			return ((size+(_ALIGN-1))/_ALIGN -1);
		}
		static size_t round_up(size_t size)
		{
			return (size+(_ALIGN-1)& ~(_ALIGN-1));
		}
		static char* chunk_alloc(size_t& nodes,size_t size);
		static void* refill(size_t size);
	public:
		static void* allocate(size_t size);
		static void deallocate(void* p,size_t size);
		static void* reallocate(void* p ,size_t old_size,size_t new_size); 
};
template<bool threads,int ints> char* default_alloc_template<threads,ints>::start_free = 0;
template<bool threads,int ints> char* default_alloc_template<threads,ints>::end_free = 0;
template<bool threads,int ints> size_t default_alloc_template<threads,ints>::heap_size = 0;
template<bool threads,int ints> typename default_alloc_template<threads,ints>::obj* volatile default_alloc_template<threads,ints>::free_lists[_NFREELISTS]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//template<bool threads,int ints> default_alloc_template<threads,ints>::obj* volatile default_alloc_template<threads,ints>::free_lists[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

template<bool threads,int ints> void* default_alloc_template<threads,ints>::allocate(size_t size)
{
	void* result;
	if(size>_MAX_BYTES)
	{
		return (malloc_alloc::allocate(size));
	}
	obj* volatile *my_free_list = free_lists + FREELIST_INDEX(size);
	obj* ret;
	ret = *my_free_list;
	if(ret == 0)
	{
		result = refill(round_up(size));
		return result;
	}
	*my_free_list = ret->free_list_link;
	result=ret;
	return result;
}
template<bool threads,int ints> void default_alloc_template<threads,ints>::deallocate(void* buf,size_t size)
{
	obj* volatile *my_free_list = free_lists+FREELIST_INDEX(size);
	obj* p =(obj*)buf;
	if(size>_MAX_BYTES)
	{
		deallocate(buf,size);
		return;
	}
	p->free_list_link=*my_free_list;
	*my_free_list =p;
}
template<bool threads,int ints> void* default_alloc_template<threads,ints>::refill(size_t size)
{
	size_t nodes= 20;
	void* result;
	char* ret=chunk_alloc(nodes,size);
	result = ret;
	if(nodes == 1)
	{
		return result;
	}
	obj* current,*next;
	obj* volatile *my_free_list;
	my_free_list = free_lists+FREELIST_INDEX(size);
	*my_free_list=(obj*)(ret+size);
	next=(obj*)(ret+size);
	int i =0;
	for(i=1;;i++)
	{
		current=next;
		next=next+size;
		if(i==nodes-1)
		{
			current->free_list_link=0;
			break;
		}else
		{
			current->free_list_link=next;
		}
	}
	return result;
}

template<bool threads,int ints> char* default_alloc_template<threads,ints>::chunk_alloc(size_t& nodes,size_t size)
{	
	char* result;
	size_t tatol_bytes=nodes*size;
	size_t free_tatol_bytes=end_free-start_free;
	if(free_tatol_bytes>tatol_bytes)
	{
		result=start_free;
		start_free+=tatol_bytes;
		return result;
		
	}else if(free_tatol_bytes>size)
	{
		nodes = free_tatol_bytes/size;
		tatol_bytes=nodes*size;
		result = start_free;
		start_free+=tatol_bytes;
		return result;
	}else
	{
		size_t bytes_to_get=2*tatol_bytes+round_up(heap_size>>4);
		if(free_tatol_bytes>0)
		{

			obj* volatile * my_free_list = free_lists+FREELIST_INDEX(free_tatol_bytes);
			((obj*)start_free)->free_list_link=*my_free_list;
			*my_free_list = (obj*)start_free;
		}
		start_free = (char*)malloc(bytes_to_get);
		if(start_free==0)
		{
			int i=0;
			obj* volatile *my_free_list,*p;
			for(i=size;i<_MAX_BYTES;i+=_ALIGN)
			{
				my_free_list = free_lists+FREELIST_INDEX(i);
				p =*my_free_list;
				if(p!=0)
				{
					*my_free_list =p->free_list_link;
					start_free =(char*)p;
					end_free = start_free+i;
					return chunk_alloc(nodes,size);
				}
			}
			end_free =0;
			start_free = (char*)malloc_alloc::allocate(bytes_to_get);
		}
		heap_size+=bytes_to_get;
		end_free =start_free+bytes_to_get;
		return (chunk_alloc(nodes,size));
	}
}
template<bool threads,int ints>
void* default_alloc_template<threads,ints>::reallocate(void* buf,size_t old_size,size_t new_size)
{
	if(old_size>_MAX_BYTES && new_size>_MAX_BYTES)
	{
		return malloc_alloc::allocate(new_size);
	}
	//if(old_size == new_size) return buf;
	if(round_up(old_size) == round_up(new_size)) return buf;
	void* result = allocate(new_size);
	size_t copy_size = old_size>new_size?new_size:old_size;
	memcpy(result,buf,copy_size);
	deallocate(buf,old_size);
	return result;
}
#ifdef __USE_MALLOC
	typedef malloc_alloc alloc;
#else
	typedef default_alloc_template<false,0> alloc; 
//interface 
template<class T,class Alloc>
class simple_alloc
{
	public:
		static T* allocate()
		{
			return (T*)alloc::allocate(sizeof(T));
		}
		static T* allocate(size_t n)
		{
			return n==0?0:(T*)allocate(n*sizeof(T));
		}
		static void allocate(T* p)
		{
			alloc::allocate(p,sizeof(T));
		}
		static void allocate(T* p,sizeof n)
		{
			if(n!=0)
			{
				alloc::allocate(p,sizeof(T)*n);
			}
		}
};
#endif
#endif
