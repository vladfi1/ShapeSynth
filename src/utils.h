//
//  utils.h
//
// Copyright (c) 2013-2014 Melinos Averkiou <m.averkiou@cs.ucl.ac.uk>
//

#ifndef UTILS_H
#define UTILS_H

#include <assert.h>
#include <time.h>
#include <cmath>

template <typename T> T random(T min, T max)
{
    // This function assumes max > min, you may want
    // more robust error checking for a non-debug build
    assert(max > min);
    T random = ((T) rand()) / (T) RAND_MAX;
    
    T range = max - min;
    return (random*range) + min;
}

// Template binary search in a vector
template <typename T> int binarySearch(const std::vector<T>& data, T num)
{
	int mid;
	int low=0;
	int high = data.size()-1;
    
	while(low<=high)
	{
		mid=(high + low)/2;
        
        T c = data.at(mid);
        
		if(c==num)
        {
			return mid;
		}
		else if(num >c)
        {
			low = mid + 1;
		}
        else
        {
			high = mid - 1;
        }
	}
    
	return high+1;
}



#endif
