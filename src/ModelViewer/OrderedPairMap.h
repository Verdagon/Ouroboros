#ifndef ORDEREDPAIRMAP_H_
#define ORDEREDPAIRMAP_H_

#include <cassert>

template<typename OrderedPair, typename Dim1Getter, typename Dim2Getter, typename T>
class OrderedPairMap {
    Dim1Getter dim1;
    Dim2Getter dim2;
public:
    const OrderedPair size;
private:
    T *values;
    
    inline int indexForTileLocation(const OrderedPair &pair) const {
        assert(dim1(pair) >= 0);
        assert(dim1(pair) < dim1(size));
        assert(dim2(pair) >= 0);
        assert(dim2(pair) < dim2(size));
        return dim1(pair) * dim2(size) + dim2(pair);
    }
    
    inline OrderedPair createOrderedPair(int dim1val, int dim2val) {
        OrderedPair result;
        dim1(result) = dim1val;
        dim2(result) = dim2val;
        return result;
    }
    
public:
    OrderedPairMap(const OrderedPair &size_, const T &defaultValue = T(), const Dim1Getter &dim1_ = Dim1Getter(), const Dim2Getter &dim2_ = Dim2Getter()) :
    dim1(dim1_),
    dim2(dim2_),
    size(size_) {
        values = new T[dim1(size) * dim2(size)];
        for (int i = 0; i < dim1(size) * dim2(size); i++)
            values[i] = defaultValue;
    }
    
    inline const OrderedPair &getSize() const { return size; }
    
    
    inline const T &operator[](const OrderedPair &pair) const {
        return values[indexForTileLocation(pair)];
    }
    
    inline T &operator[](const OrderedPair &pair) {
        return values[indexForTileLocation(pair)];
    }
    
    class iterator {
        OrderedPairMap *map;
        OrderedPair currentPair;
        
    public:
        iterator(OrderedPairMap *map_, const OrderedPair &currentPair_) :
        map(map_),
        currentPair(currentPair_) { }
        
        inline bool operator==(const iterator &that) const {
            return map == that.map && currentPair == that.currentPair;
        }
        
        inline bool operator!=(const iterator &that) const { return !operator==(that); }
        
        inline void operator++(int) {
            assert(map->dim1(currentPair) < map->dim1(map->size));
            assert(map->dim2(currentPair) < map->dim2(map->size));
            
            map->dim1(currentPair)++;
            if (map->dim1(currentPair) == map->dim1(map->size)) {
                map->dim1(currentPair) = 0;
                map->dim2(currentPair)++;
            }
        }
        
        OrderedPair operator*() const { return currentPair; }
    };
    
    friend class iterator;
    
    inline iterator begin() {
        return iterator(this, createOrderedPair(0, 0));
    }
    
    inline iterator end() {
        return iterator(this, createOrderedPair(dim1(size), 0));
    }
};

#endif
