#ifndef _XFLANN_HHEAP_
#define _XFLANN_HHEAP_
#include <iostream>
#include <cassert>
#include <vector>
namespace ucoslam{
//heap having at the top the minimum element
template<typename T, class comparison=std::less<T>,typename Container=std::vector<T>>
class Heap{
  public:
    Container array;
    comparison comp;
public:
    Heap( ){}

    void reset(){
        array.reset();
    }
    void reserve(int size){array.reserve(size);}

    //returns true if the element has been added and false if not (the heap is full)
    inline void push (const T &val)
    {
        array.push_back(val);
        down ( array.size()-1) ;
    }


    inline T pop ( ){
        T res=array[0];
        std::swap(array[0],array[array.size()-1]);
        array.pop_back();
        if (array.size() > 1)  up (0) ;
        return res;
    }
    inline size_t size()const {return array.size();}

    inline bool empty()const {return array.size()==0;}
    const T& top()const{return array[0];}
private:


    inline void
    down ( size_t index)
    {
        if (index == 0) return  ;

        size_t parentIndex =(index - 1) / 2;

        if ( comp(array[ index], array[ parentIndex])) {
            std::swap (array[ index], array[parentIndex]) ;
            down (parentIndex) ;
        }
    }

    inline void up (size_t index)
    {
        size_t leftIndex  =   2 * index + 1;
        size_t rightIndex = 2 * index + 2  ;

        /* no childer: stop */
        if (leftIndex >= array.size()) return ;

        /* only left childer: easy */
        if (rightIndex >= array.size()) {
            if (comp(array[leftIndex], array [ index]) ) {
                std::swap (array[index], array[leftIndex]) ;
            }
            return ;
        }

        /* both childern */
        {
            if ( comp(array[  leftIndex] , array[ rightIndex])) {
                /* swap with left */
                if ( comp(array[leftIndex], array [index]) ) {
                    std::swap (array [index], array[leftIndex]) ;
                    up ( leftIndex) ;
                }
            } else {
                /* swap with right */
                if ( comp(array[rightIndex], array[ index])  ) {
                    std::swap (array [index], array[rightIndex]) ;
                    up ( rightIndex) ;
                }
            }
        }
    }


};



}


#endif
