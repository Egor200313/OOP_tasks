#include <iostream>
#include <vector>
#include <iterator>
#include <typeinfo>
#include <type_traits>


template<typename T>
class Deque{
protected:
    int chunk_size = 5;
    std::vector<T*> storage;
    int len;
    size_t capacity = 1;
    int start_o;
    int start_i;
    int end_o;
    int end_i;

public:

    template <bool IsConst>
    class common_iterator{
    public:
        int index_o, index_i;
        int iMax;
        std::conditional_t<IsConst, const T*, T*> ptr;
        std::conditional_t<IsConst, T*const*, T**> outptr;
    public:
        using iterator_category = std::random_access_iterator_tag;
        using difference_type = int;
        using value_type = T;
        using pointer = T*;
        using reference = T&;
        common_iterator(int a, int b,int m, std::conditional_t<IsConst, const T*, T*> ptr, std::conditional_t<IsConst, T*const*, T**> p): index_o(a), index_i(b),ptr(ptr), outptr(p){
            if (m < 0) iMax = 0;
            else iMax = m;
        }
        common_iterator(const common_iterator& iter) = default;

        std::conditional_t<IsConst, const T&, T&> operator*()const{
            return *ptr;
        }


        std::conditional_t<IsConst, const T*, T*> operator->()const{
            return ptr;
        }

        common_iterator& operator++(){
            if (index_i == iMax){
                outptr++;
                index_o++;
                index_i = 0;
            }
            else{
                index_i++;
            }
            ptr = *outptr+index_i;
            return *this;
        }

        common_iterator& operator+(int n){
            int new_out = (n+index_i)/(iMax+1);
            outptr+=new_out;
            index_o+=new_out;
            index_i = (n+index_i)%(iMax+1);
            ptr = *outptr+index_i;
            return *this;
        }
        common_iterator& operator-(int n){
            if (n <= static_cast<int>(index_i)){
                index_i-=n;
                --ptr;
            }else{
                int m = iMax+1;
                int delta_y = (n-index_i-1)/m + 1;
                int new_in = ((index_i-n)%m+m)%m;
                index_o -= delta_y;
                outptr = outptr - delta_y;
                ptr = *outptr + new_in;
                index_i = new_in;
            }
            return *this;
        }

        int operator-(common_iterator& iter){
            int res = 0;
            res += (iMax+1)*(index_o-iter.index_o);
            res += index_i-iter.index_i;
            return res;
        }
        common_iterator& operator--(){
            if (1 <= index_i){
                index_i--;
                ptr = ptr - 1;
            }else{
                --outptr;
                ptr = *outptr + iMax;
                index_o--;
                index_i = iMax;
            }
            return *this;
        }

        bool operator <= (common_iterator iter)const{
            if (index_o < iter.index_o) return true;
            if (index_o > iter.index_o) return false;
            return (index_i <= iter.index_i);
        }
        bool operator < (common_iterator iter)const{
            return (*this <= iter && *this != iter);
        }
        bool operator >= (common_iterator iter)const{
            return (iter <= *this);
        }
        bool operator > (common_iterator iter)const{
            return (iter < *this);
        }
        bool operator == (common_iterator iter)const{
            return ((index_i == iter.index_i) && (index_o == iter.index_o));
        }
        bool operator != (common_iterator iter)const{
            return !(*this == iter);
        }
    };


    using iterator = common_iterator<false>;
    using const_iterator = common_iterator<true>;

    //Constructors
    Deque(){
        for (int i = 0; i < 2; ++i) storage.push_back(reinterpret_cast<T*> (new int8_t[sizeof(T)*chunk_size]));
        start_i = 2;
        end_i = 2;
        start_o = 0;
        end_o = 0;
        capacity = 2;
        len = 0;
    }//default constructor

    Deque(int n): Deque(n, T()){}//int constructor

    Deque(int n, const T& val){
        int vec_cap = static_cast<int>((2*n)/chunk_size+1);
        if (vec_cap < 3) vec_cap = 3;
        for (int i = 0; i < vec_cap; ++i) storage.push_back(reinterpret_cast<T*>(new int8_t[sizeof(T)*chunk_size]));
        start_i = 2;
        end_i = (2+len)%chunk_size;
        start_o = 0;
        end_o = 0;
        int cur_o = start_o;
        int cur_i = start_i;
        len = 0;
        int i = 0;
        try{
            for (; i < n; ++i){
                new (storage[cur_o]+cur_i) T(val);
                cur_i++;
                if (cur_i == chunk_size){
                    cur_i = 0;
                    cur_o++;
                }
                end_i++;
                //end_o = cur_o;
                if (end_i == chunk_size){
                    end_i = 0;
                    end_o++;
                }
                ++len;
            }
        }catch(...){
            for (int j = 0; j < i; ++j){
                (storage[cur_o]+cur_i)->~T();
                if (cur_i == 0){
                    cur_i = chunk_size-1;
                    cur_o--;
                }else cur_i--;
            }
            throw;
        }
        capacity = vec_cap;

    }//int, T constructor

    ~Deque(){
        for (size_t i = 0; i < capacity; ++i){
            delete storage[i];
        }
    }

    //copy constructor
    Deque(const Deque& deq){
        int l = static_cast<int>(deq.size());
        size_t cap = deq.get_capacity();
        storage.resize(cap);
        start_o = deq.start_o;
        len = 0;
        start_i = deq.start_i;
        end_o=deq.start_o;
        end_i = deq.start_i;
        capacity = deq.capacity;
        for (size_t i = 0; i < capacity; ++i) storage[i] = reinterpret_cast<T*>(new int8_t[sizeof(T)*chunk_size]);
        for (int i = 0; i < l; ++i){
            push_back(deq[i]);
        }
    }

    //= operator
    Deque& operator =(Deque deq){
        std::swap(start_i, deq.start_i);
        std::swap(start_o, deq.start_o);
        std::swap(end_i, deq.end_i);
        std::swap(end_o, deq.end_o);
        std::swap(storage, deq.storage);
        std::swap(capacity, deq.capacity);
        std::swap(len, deq.len);
        std::swap(chunk_size, deq.chunk_size);
        return *this;
    }

    //size method
    size_t size()const{
        return static_cast<size_t>(len);
    }

    size_t get_capacity()const{
        return capacity;
    }

    //indexing
    T& operator[](int index){
        return storage[(index+start_i)/chunk_size + start_o][(index+start_i)%chunk_size];
    }//without out_of_range check

    T& at(int index){
        int o = (index+start_i)/chunk_size + start_o;
        int i = (index+start_i)%chunk_size;
        if (o < start_o || o > end_o) throw std::out_of_range("...");
        else{
            if (o == start_o && i < start_i) throw std::out_of_range("...");
            if (o == end_o && i >= end_i) throw std::out_of_range("...");
        }
        return storage[o][i];
    }//with out_of_range check

    const T& operator[](int index)const{
        return storage[(index+start_i)/chunk_size + start_o][(index+start_i)%chunk_size];
    }//without out_of_range check
    const T& at(int index)const{
        int o = (index+start_i)/chunk_size + start_o;
        int i = (index+start_i)%chunk_size;
        if (o < start_o || o > end_o) throw std::out_of_range("...");
        else{
            if (o == start_o && i < start_i) throw std::out_of_range("...");
            if (o == end_o && i >= end_i) throw std::out_of_range("...");
        }
        return storage[(index+start_i)/chunk_size + start_o][(index+start_i)%chunk_size];
    }//with out_of_range check

    //common_iterator methods
    iterator begin(){
        return iterator(start_o, start_i, chunk_size-1, &storage[start_o][start_i], &storage[start_o]);
    }
    const_iterator begin() const{
        return const_iterator(start_o, start_i, chunk_size-1, &storage[start_o][start_i], &storage[start_o]);
    }
    const_iterator cbegin() const{
        return const_iterator(start_o, start_i, chunk_size-1, &storage[start_o][start_i], &storage[end_o]);
    }
    iterator end(){
        return iterator(end_o, end_i, chunk_size-1, &storage[end_o][end_i], &storage[end_o]);
    }
    const_iterator end()const{
        return const_iterator(end_o, end_i, chunk_size-1, &storage[end_o][end_i], &storage[end_o]);
    }
    const_iterator cend() const{
        return const_iterator(end_o, end_i, chunk_size-1, &storage[end_o][end_i], &storage[end_o]);
    }



    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;


    //reverse_iterators methods
    reverse_iterator rbegin(){
        return reverse_iterator(end());
    }
    const_reverse_iterator rbegin()const{
        return const_reverse_iterator(end());
    }
    const_reverse_iterator crbegin() const{
        return const_reverse_iterator(cend());
    }
    reverse_iterator rend(){
        return reverse_iterator(begin());
    }
    const_reverse_iterator rend()const{
        return const_reverse_iterator(begin());
    }
    const_reverse_iterator crend() const{
        return const_reverse_iterator(cbegin());
    }


    //insert/erase
    void insert(iterator iter, const T& val){
        T last = val;
        T cur = val;
        for (auto it = iter; it != end(); ++it){
            last = *it;
            *it = cur;
            cur = last;
        }
        push_back(last);
    }//O(n)

    void erase(iterator& iter){
        int x = iter.index_i;
        int y = iter.index_o;
        while (y != end().index_o && x != end().index_i){
            int next_i = x+1;
            int next_o = y;
            if (next_i == chunk_size){
                next_i = 0;
                next_o++;
            }
            storage[y][x] = storage[next_o][next_i];
            y = next_o;
            x = next_i;
        }
        pop_back();
    }

    void extend(){
        std::vector<T*>new_vec(capacity*2);//new memory
        for (size_t i = 0; i < capacity*2; ++i) new_vec[i] = reinterpret_cast<T*>(new int8_t[sizeof(T)*chunk_size]);
        int free = static_cast<int>(capacity)*2;
        int tmp = end_o - start_o+1;
        int end = end_o - start_o;
        if (end_i == 0) tmp--;
        free -= tmp;
        free /= 2;
        for (int i = 0; i < tmp; ++i){//copy previous pointers
            new (&new_vec[free + i]) T(storage[start_o + i]);
        }
        start_o = free;
        end_o = free+end;
        storage = new_vec;
        capacity = static_cast<size_t>(storage.size());
    }

    //push/pop methods
    void push_back(T item){
        if (0 == end_i){//need next chunk
            if (end_o > static_cast<int>(capacity)-1){//no next chunk
                extend();
            }
            new(storage[end_o]) T(item);
            len++;
            end_i = 1;
            storage[end_o]->~T();

        }else{//don't need another chunk
            new(storage[end_o] + end_i) T(item);
            len++;
            end_i++;
            end_i%=chunk_size;
            if (end_i == 0) end_o++;
        }
    }
    void push_front(T item){
        if (len == 0) end_i++;
        if (end_i == chunk_size){
            end_i -= 1;
            end_o++;
        }
        if (start_i == 0){//need previous chunk
            if (start_o == 0){//no previous chunk
                extend();
            }

            new(storage[start_o-1] + chunk_size - 1) T(item);
            start_o--;
            len++;
            start_i = chunk_size-1;
        }else{//don't need previous chunk
            if (len == 0) new(storage[start_o] + start_i) T(item);
            else{
                new(storage[start_o] + start_i-1) T(item);
                start_i--;
            }
            len++;
        }
    }
    T pop_back(){
        if (end_i == 0){
            end_o--;
            end_i = chunk_size-1;
        }else end_i--;
        T res = storage[end_o][end_i];

        (storage[end_o]+end_i)->~T();
        --len;
        return res;
    }
    T pop_front(){
        T res = storage[start_o][start_i];
        (storage[start_o] + start_i)->~T();
        start_i++;
        if (start_i == chunk_size){
            start_i = 0;
            start_o++;
        }
        --len;
        return res;
    }
};
