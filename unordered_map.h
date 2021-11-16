#include <iostream>
#include <vector>
#include <functional>
#include <utility>
#include "fastallocator_list.h"

template <typename Key,
        typename Value,
        typename Hash = std::hash<Key>,
        typename Equal = std::equal_to<Key>,
        typename Alloc = std::allocator<std::pair<const Key, Value>>
        >
class UnorderedMap{
private:
    using NodeType = std::pair<const Key, Value>;

    using InnerList = List<NodeType, Alloc>;


    int sz;//сколько лежит
    double max_load_fact = 0.75;
    size_t max_siz = 275647562756;
    int array_size = 4;//размер data - внешнего массива
    Hash hash;
    Equal eq;

    int starth = -1;//первая использованная ячейка data
    int endh = -1;//последняя использованная ячейка data

    std::vector<int> order;//массив с указанием для каждой ячейки следующей за ней
    std::vector<int> rev_order;//массив с указанием для каждой ячейки предшествующей ей

    std::vector<InnerList, typename std::allocator_traits<Alloc>::template rebind_alloc<InnerList>> data;

public:


    UnorderedMap(int size = 4, const Hash& hash = Hash(), const Equal& eq = Equal(), const Alloc& alloc = Alloc()):
    array_size(size),
    hash(hash),
    eq(eq),
    order(std::vector<int>(size, -1)),
    rev_order(std::vector<int>(size, -1)),
    data(std::vector<InnerList, typename std::allocator_traits<Alloc>::template rebind_alloc<InnerList>>(size, alloc))
    {
        sz = 0;
    }

    UnorderedMap(const UnorderedMap& oth):
        sz(oth.sz),
        max_load_fact(oth.max_load_fact),
        array_size(oth.array_size),
        hash(oth.hash),
        eq(oth.eq),
        starth(oth.starth),
        endh(oth.endh),
        order(oth.order),
        rev_order(oth.rev_order),
        data(oth.data)
        {}
    UnorderedMap(UnorderedMap&& oth):
        hash(std::move(oth.hash)),
        eq(std::move(oth.eq)),
        order(std::move(oth.order)),
        rev_order(std::move(oth.rev_order)),
        data(std::move(oth.data))
        {
        oth.order.clear();
        oth.rev_order.clear();
        oth.data.clear();
        sz = oth.sz;
        oth.sz = 0;
        max_load_fact = oth.max_load_fact;
        oth.max_load_fact = 0.75;
        array_size = oth.array_size;
        oth.array_size = 4;
        starth = oth.starth;
        oth.starth = -1;
        endh = oth.endh;
        oth.endh = -1;
    }

    UnorderedMap& operator=(const UnorderedMap& oth){
        sz = oth.sz;
        max_load_fact = oth.max_load_fact;
        array_size = oth.array_size;
        hash = oth.hash;
        eq = oth.eq;
        starth = oth.starth;
        endh = oth.endh;
        order = oth.order;
        rev_order = oth.rev_order;
        data = oth.data;
        return *this;
    }

    UnorderedMap& operator=(UnorderedMap&& oth){
        hash = std::move(oth.hash);
        eq = std::move(oth.eq);
        order = std::move(oth.order);
        rev_order = std::move(oth.rev_order);
        data = std::move(oth.data);
        oth.order.clear();
        oth.rev_order.clear();
        oth.data.clear();
        sz = oth.sz;
        oth.sz = 0;
        max_load_fact = oth.max_load_fact;
        oth.max_load_fact = 0.75;
        array_size = oth.array_size;
        oth.array_size = 4;
        starth = oth.starth;
        oth.starth = -1;
        endh = oth.endh;
        oth.endh = -1;
        return *this;
    }

    void add_hash(int h){
        if(starth == -1){
            starth = h;
        }else{
            rev_order[h] = endh;
            order[endh] = h;
        }
        endh = h;
    }


    Value& operator[](const Key& key){
        if (array_size < 2*sz) rehash(2*array_size);
        int h = hash(key) % array_size;
        if (data[h].size() != 0){
            for (auto it = data[h].begin(); it != data[h].end(); ++it){
                if (eq(key, (*it).first)) return (*it).second;
            }
        }else add_hash(h);
        data[h].push_front({key, Value()});
        ++sz;
        return (*data[h].begin()).second;
    }
    Value& operator[](Key&& key){
        if (array_size < 2*sz) rehash(2*array_size);
        int h = hash(key) % array_size;
        if (data[h].size() != 0){
            for (auto it = data[h].begin(); it != data[h].end(); ++it){
                if (eq(key, (*it).first)) return (*it).second;
            }
        }else add_hash(h);
        data[h].push_front({std::move(key), Value()});
        ++sz;
        return (*data[h].begin()).second;
    }

    const Value& at(const Key& key)const{
        int h = hash(key) % array_size;
        if (data[h].size() != 0){
            for (auto it = data[h].begin(); it != data[h].end(); ++it){
                if (eq(key, (*it).first)) return (*it).second;
            }
        }
        throw std::out_of_range("No such element");
    }
    Value& at(const Key& key){
        int h = hash(key) % array_size;
        if (data[h].size() != 0){
            for (auto it = data[h].begin(); it != data[h].end(); ++it){
                if (eq(key, (*it).first)) return (*it).second;
            }
        }
        throw std::out_of_range("No such element");
    }
    int size() const{
        return sz;
    }

    template<bool IsConst>
    class common_iterator{
    public:
        using itertype = std::conditional_t<IsConst, typename InnerList::const_iterator, typename InnerList::iterator>;
        using data_ptr_t = std::conditional_t<IsConst, const std::vector<InnerList, typename std::allocator_traits<Alloc>
                ::template rebind_alloc<InnerList>>*, std::vector<InnerList, typename std::allocator_traits<Alloc>
                        ::template rebind_alloc<InnerList>>*>;
        using order_ptr_t = std::conditional_t<IsConst, const std::vector<int>*, std::vector<int>*>;

        int hash;
        order_ptr_t order_ptr;
        data_ptr_t data_ptr;
        itertype iter;

        using value_type = NodeType;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;
        using pointer = std::conditional<IsConst, const NodeType*, NodeType*>;
        using reference = std::conditional_t<IsConst, const NodeType&, NodeType&>;


        using data_ref_t = std::conditional_t<IsConst, const std::vector<InnerList, typename std::allocator_traits<Alloc>::
                template rebind_alloc<InnerList>>&, std::vector<InnerList, typename std::allocator_traits<Alloc>::
                        template rebind_alloc<InnerList>>&>;
        using order_ref_t = std::conditional_t<IsConst, const std::vector<int>&, std::vector<int>&>;

        common_iterator(data_ref_t data, order_ref_t order, int hash, bool beg): hash(hash), order_ptr(&order), data_ptr(&data){
            (beg)? iter = data.at(hash).begin(): iter = data.at(hash).end();
        }

        common_iterator(itertype it, data_ref_t data, order_ref_t order, int hash): hash(hash), order_ptr(&order), data_ptr(&data), iter(it){}

        common_iterator& operator++(){
            ++iter;
            if (iter == data_ptr->at(hash).end() && order_ptr->at(hash) != -1){
                iter = data_ptr->at(order_ptr->at(hash)).begin();
                hash = order_ptr->at(hash);
            }
            return *this;
        }

        common_iterator operator++(int){
            common_iterator copy = *this;
            ++*this;
            return copy;
        }

        std::conditional_t<IsConst,const NodeType*,NodeType*> operator->(){
            return iter.operator->();
        }

        reference operator*(){
            return *iter;
        }

        bool operator==(const common_iterator& other){
            return iter == other.iter;
        }

        bool operator!=(const common_iterator& other){
            return iter != other.iter;
        }
    };

    using iterator = common_iterator<false>;
    using const_iterator = common_iterator<true>;

    iterator begin(){
        int h;
        if (starth == -1) h = 0;
        else h = starth;
        iterator beg(data, order, h, true);
        return beg;
    }

    const_iterator begin() const{
        int h;
        if (starth == -1) h = 0;
        else h = starth;
        const_iterator beg(data, order, h, true);
        return beg;
    }

    const_iterator cbegin() const{
        int h;
        if (starth == -1) h = 0;
        else h = starth;
        const_iterator beg(data, order, h, true);
        return beg;
    }

    iterator end(){
        int h;
        if (endh == -1) h = 0;
        else h = endh;
        iterator end(data, order, h, false);
        return end;
    }

    const_iterator end()const{
        int h;
        if (endh == -1) h = 0;
        else h = endh;
        const_iterator end(data, order, h, false);
        return end;
    }

    const_iterator cend()const{
        int h;
        if (endh == -1) h = 0;
        else h = endh;
        const_iterator end(data, order, h, false);
        return end;
    }


    iterator find(const Key& key){
        int h = hash(key)%array_size;
        if (data[h].size() == 0) return end();
        else{
            for (auto it = data[h].begin();it != data[h].end(); ++it){
                if (eq((*it).first, key)) return iterator(it, data, order, h);
            }
        }
        return end();
    }
    const_iterator find(const Key& key)const{
        int h = hash(key)%array_size;
        if (data[h].empty()) return cend();
        else{
            for (auto it = data[h].begin();it != data[h].end(); ++it){
                if (eq((*it).first, key)) return const_iterator(it, data, order, h);
            }
        }
        return cend();
    }

    void erase(iterator map_it){
        int h = map_it.hash % array_size;
        data[h].erase(map_it.iter);
        --sz;
        if (data[h].size() == 0){//переставляем порядок если цепочка стала пустой
            int next = order[h];
            int prev = rev_order[h];
            if (prev != -1){
                order[prev] = next;
            }else starth = next;
            if (next != -1){
                rev_order[next] = prev;
            }
            if (endh == h) endh = prev;
            order[h] = -1;
            rev_order[h] = -1;
        }
    }

    void erase(iterator beg, iterator end){
        while (beg != end) erase(beg++);
    }

    std::pair<iterator, bool> insert(const NodeType& node){
        if (array_size < 2*sz) rehash(2*array_size);
        int h = hash(node.first) % array_size;
        if (data[h].size() != 0){
            for (auto it = data[h].begin(); it != data[h].end(); ++it){
                if (eq(node.first, (*it).first)) return {iterator(it, data, order, h), false};
            }
        }else add_hash(h);
        data[h].push_front(node);
        ++sz;
        return {iterator(data[h].begin(), data, order, h), true};
    }

    template<typename Type>
    std::pair<iterator, bool> insert(Type&& node){
        if (array_size < 2*sz) rehash(2*array_size);
        int h = hash(node.first) % array_size;
        if (data[h].size() != 0){
            for (auto it = data[h].begin(); it != data[h].end(); ++it){
                if (eq(node.first, (*it).first)) return {iterator(it, data, order, h), false};
            }
        }else add_hash(h);
        data[h].push_front(std::forward<Type>(node));
        ++sz;
        return {iterator(data[h].begin(), data, order, h), true};
    }

    template<typename Iter>
    void insert(Iter beg, Iter end){
        while (beg != end) insert(*beg++);
    }

    template<typename... Args>
    std::pair<iterator, bool> emplace(Args... args){
        if (array_size < 2*sz) rehash(2*array_size);
        auto node = data[1].create(std::forward<Args>(args)...);
        auto itt = find(node->value.first);
        if (itt != end()) return {itt, false};
        else{
            int h = hash(node->value.first)%array_size;
            if (data[h].size() != 0){
                data[h].push_front(node);
                return {iterator(data[h].begin(), data, order, h), true};
            }else add_hash(h);
            data[h].push_front(node);
            ++sz;
            return {iterator(data[h].begin(), data, order, h), true};
        }
    }

    size_t max_size(){ return max_siz; }
    double max_load_factor(){ return max_load_fact; }
    double load_factor(){ return sz/array_size; }


    void rehash(int new_size){
        if (load_factor() > max_load_fact){
            decltype(data) new_data(new_size, data.get_allocator());
            decltype(order) new_order(new_size, -1);
            decltype(rev_order) new_rev_order(new_size, -1);

            int nexth = starth;
            starth = -1;
            endh = -1;
            while(nexth != -1){
                auto s = data[nexth].size();
                for (size_t i = 0; i < s; ++i){
                    auto it = data[nexth].begin();
                    int h = hash((*it).first) % new_size;
                    data[nexth].exchange(new_data[h]);
                    if (starth == -1) starth = h;
                    else{
                        if (new_data[h].size() == 1){
                            new_order[endh] = h;
                            new_rev_order[h] = endh;
                        }
                    }
                    endh = h;
                }
                nexth = order[nexth];
            }
            data = std::move(new_data);
            order = std::move(new_order);
            rev_order = std::move(new_rev_order);

            array_size = new_size;
        }
    }

    void reserve(size_t new_size){
        rehash(int(new_size/max_load_fact));
    }
};

