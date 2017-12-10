#ifndef SKT_ITERATOR_H
#define SKT_ITERATOR_H

#include<cstddef>

struct input_iterator_tag{};
struct output_iterator_tag{};
struct forword_iterator_tag:public input_iterator_tag{};
struct bidirectional_iterator_tag:public forword_iterator_tag{};
struct random_iterator_tag:public bidirectional_iterator_tag{};

template<class Category,class T,class Distance = ptrdiff_t,class Pointer = T*,class Reference =T&>
struct iterator{
	typedef Category iterator_category;
	typedef T value_type;
	typedef Distance difference_type;
	typedef Pointer pointer;
	typedef Reference reference;
};
template<class iterator>
struct iterator_traits{
	typedef typename iterator::iterator_category iterator_category;
	typedef typename iterator::value_type value_type;
	typedef typename iterator::difference_type difference_type;
	typedef typename iterator::pointer pointer;
	typedef typename iteratro::reference reference;
};
template<class T>
struct iterator_traits<T*>{
	typedef typename random_iteratro_tag iterator_category;
	typedef typename T value_type;
	typedef typename T& reference;
	typedef typename ptrdiff_t difference_type;
	typedef typename T* pointer;
};
template<class T>
struct iterator_traits<const T*>
{
	typedef random_iterator_tag iterator_category;
	typedef T value_type;
	typedef const T* pointer;
	typedef const T& reference;
	typedef ptrdiff_t difference_category;
};

#endif
