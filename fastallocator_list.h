#include <iterator>
#include <vector>
#include <forward_list>
#include <memory>
#include <stack>

template<size_t ChunkSize>
class FixedAllocator{
    struct Block{
        int POOL_SIZE = 5000;
        int ind = 0;
        int8_t* free;
        std::vector<int8_t*> data;

        Block(){
            ind = 0;
            data.push_back(new int8_t[POOL_SIZE*ChunkSize]);
            free = data.back();
        }

        void addBlock(){
            data.push_back(new int8_t[POOL_SIZE*ChunkSize]);
            free = data.back();
            ind = 0;
        }
        ~Block(){
            for (auto x: data) delete[]x;
            data.clear();
        }
    };
    std::stack<int8_t*> freeblocks;

    std::shared_ptr<Block> memory;
public:
    FixedAllocator(){
        memory = std::make_shared<Block>();
    }

    void* allocate(size_t){
        if (!freeblocks.empty()){
            auto p = freeblocks.top();
            freeblocks.pop();
            return p;
        }
        if (memory->ind >= memory->POOL_SIZE) memory->addBlock();
        auto beg = static_cast<void*>(memory->free);
        memory->free = memory->free + ChunkSize;
        ++memory->ind;
        return beg;
    }

    void deallocate(void* ptr){
        freeblocks.push(static_cast<int8_t*>(ptr));
    }

    FixedAllocator& operator=(const FixedAllocator& alloc){
        memory = alloc.memory;
        return *this;
    }

    FixedAllocator(const FixedAllocator&) = delete;

    ~FixedAllocator(){
        memory = nullptr;
    }
};
template<size_t T, size_t U>
bool operator ==(const FixedAllocator<T>&, const FixedAllocator<U>&){
    return (T==U);
}

template<size_t T, size_t U>
bool operator !=(const FixedAllocator<T>&, const FixedAllocator<U>&){
    return !(T==U);
}


template<typename T>
class FastAllocator{
public:
    FixedAllocator<24> alloc_24_bytes;
    using value_type = T;

    template<class U>
    using rebind = FastAllocator<U>;

    FastAllocator() = default;

    FastAllocator(const FastAllocator& other){
        alloc_24_bytes =  other.alloc_24_bytes;
    }

    template<typename M>
    FastAllocator<T>(const FastAllocator<M>& other){
        alloc_24_bytes =  other.alloc_24_bytes;
    }

    ~FastAllocator()=default;

    FastAllocator& operator=(const FastAllocator& other){
        alloc_24_bytes = other.alloc_24_bytes;
        return *this;
    }

    T* allocate(size_t size);

    void deallocate(T* ptr, size_t size);
};

template <typename T>
bool operator == (const FastAllocator<T>& first, const FastAllocator<T>& second){
    return (first.alloc_24_bytes == second.alloc_24_bytes);
}

template <typename T>
bool operator != (const FastAllocator<T>& first, const FastAllocator<T>& second){
    return (first.alloc_24_bytes != second.alloc_24_bytes);
}

template <typename T, typename U>
bool operator == (const FastAllocator<T>& first, const FastAllocator<U>& second){
    return true;
}

template <typename T, typename U>
bool operator != (const FastAllocator<T>& first, const FastAllocator<U>& second){
    return false;
}

template<typename T>
T *FastAllocator<T>::allocate(size_t size) {
    if (size*sizeof(T) <= 24){
        return reinterpret_cast<T*>(alloc_24_bytes.allocate(size));
    }else{
        return static_cast<T*>(::operator new(sizeof(T)*size));
    }
}

template<typename T>
void FastAllocator<T>::deallocate(T *ptr, size_t size) {
    if (size*sizeof(T) <= 24){
        alloc_24_bytes.deallocate(ptr);
    }else{
        operator delete(ptr);
    }
}


template<typename T, typename Allocator = std::allocator<T>>
class List{
    struct Node{
        Node* prev;
        Node* next;
        T value;
        Node(const T& val):value(val){}
        Node(){};
        ~Node() = default;
    };
    Node* im;
    using RebindAlloc = typename std::allocator_traits<Allocator>::template rebind_alloc<Node>;
    RebindAlloc alloc;
    using AllocTraits = std::allocator_traits<RebindAlloc>;
    Allocator allocT;
    size_t sz = 0;

    void link(Node* A, Node* B, Node* C){
        C->prev = A;
        C->next = B;
        A->next = C;
        B->prev = C;
    }

    void cut(Node* A, Node* B, Node* C){
        A->next = B;
        B->prev = A;
        AllocTraits::destroy(alloc, C);
        alloc.deallocate(C, 1);
    }

public:
    template <bool IsConst>
    class common_iterator{
    public:
        std::conditional_t<IsConst, const Node*, Node*> ptr;
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = int;
        using value_type = std::conditional_t<IsConst, const T, T>;
        using pointer = std::conditional_t<IsConst, const Node*, Node*>;
        using reference = std::conditional_t<IsConst, const T&, T&>;

        common_iterator(std::conditional_t<IsConst, const Node*, Node*> ptr): ptr(ptr){}// pointer

        common_iterator(const common_iterator<false>& iter){
            ptr = iter.ptr;
        }

        std::conditional_t<IsConst, const T&, T&> operator*()const{// reference
            return ptr->value;
        }

        std::conditional_t<IsConst, const Node*, Node*> operator->()const{// pointer
            return ptr;
        }

        common_iterator& operator++(){
            ptr = ptr->next;
            return *this;
        }
        common_iterator& operator--(){
            ptr = ptr->prev;
            return *this;
        }

        common_iterator operator++(int){
            common_iterator copy = *this;
            ptr = ptr->next;
            return copy;
        }

        common_iterator operator--(int){
            common_iterator copy = *this;
            ptr = ptr->prev;
            return copy;
        }

        bool operator==(const common_iterator& it){
            return ptr == it.ptr;
        }

        bool operator!=(const common_iterator& it){
            return ptr != it.ptr;
        }
    };

    using iterator = common_iterator<false>;
    using const_iterator = common_iterator<true>;

    explicit List(const Allocator& n_alloc = Allocator()):alloc(n_alloc){
        allocT = n_alloc;
        im = alloc.allocate(1);
    }

    List(size_t count, const T& value, const Allocator& n_alloc = Allocator()):alloc(n_alloc){
        allocT = n_alloc;
        im = alloc.allocate(1);
        sz = 0;
        for (size_t i = 0; i < count; ++i) push_front(value);
    }

    List(size_t count, const Allocator& n_alloc = Allocator()):alloc(n_alloc) {
        allocT = n_alloc;
        im = alloc.allocate(1);
        Node* head = alloc.allocate(1);
        AllocTraits::construct(alloc, head);
        head->prev = im;
        im->prev = head;
        im->next = head;
        head->next = im;
        sz = count;
        for (size_t i = 0; i < count-1; ++i) {
            Node* new_node = alloc.allocate(1);
            AllocTraits::construct(alloc, new_node);
            link(im, im->next, new_node);
        }
    }

    List(const List& lst){
        while (sz) pop_back();

        alloc = lst.alloc;
        allocT = AllocTraits::select_on_container_copy_construction(lst.allocT);
        im = alloc.allocate(1);
        Node* head = alloc.allocate(1);
        AllocTraits::construct(alloc, head, lst.im->next->value);
        head->prev = im;
        im->prev = head;
        im->next = head;
        head->next = im;
        sz = lst.sz;
        Node* cur = lst.im->next->next;
        for (size_t i = 1; i < lst.sz; ++i) {
            Node* new_node = alloc.allocate(1);
            AllocTraits::construct(alloc, new_node, cur->value);
            cur=cur->next;
            new_node->prev = im->prev;
            new_node->next = im;
            im->prev->next = new_node;
            im->prev = new_node;
        }
    }

    List& operator = (const List& lst){
        while (sz != 0) pop_back();
        if (AllocTraits::propagate_on_container_copy_assignment::value){
            if (allocT != lst.allocT) allocT = lst.allocT;

        }
        im = alloc.allocate(1);
        Node *head = alloc.allocate(1);
        AllocTraits::construct(alloc, head, *lst.begin());
        head->prev = im;
        im->prev = head;
        im->next = head;
        head->next = im;
        sz = lst.sz;

        Node* cur = lst.im->next->next;
        for (size_t i = 1; i < lst.sz; ++i) {
            Node* new_node = alloc.allocate(1);
            AllocTraits::construct(alloc, new_node, cur->value);
            cur=cur->next;
            new_node->prev = im->prev;
            new_node->next = im;
            im->prev->next = new_node;
            im->prev = new_node;
        }
        return *this;
    }

    ~List(){
        for (int i = 0; i < static_cast<int>(sz); ++i) pop_back();
        AllocTraits::destroy(alloc, im);
        alloc.deallocate(im, 1);
    }

    Allocator& get_allocator(){
        return allocT;
    }

    size_t size()const{return sz;}

    void push_back(const T& item){
        Node* new_back = alloc.allocate(1);
        AllocTraits::construct(alloc, new_back, item);
        if (sz == 0){
            im->next = new_back;
            im->prev = new_back;
            new_back->next = im;
            new_back->prev = im;

        }else{
            link(im->prev,im, new_back);
        }
        ++sz;
    }
    void pop_back() {
        if (sz == 1) {
            --sz;
            AllocTraits::destroy(alloc, im->next);
            alloc.deallocate(im->next, 1);
            im->next = nullptr;
            im->prev = nullptr;
        } else {
            Node* new_prev = im->prev->prev;
            im->prev->prev->next = im;
            AllocTraits::destroy(alloc, im->prev);
            alloc.deallocate(im->prev, 1);
            im->prev = new_prev;
            --sz;
        }
    }
    void push_front(const T& item){
        Node* new_front = alloc.allocate(1);
        AllocTraits::construct(alloc, new_front, item);
        if (sz == 0){
            im->next = new_front;
            im->prev = new_front;
            new_front->prev = im;
            new_front->next = im;
        }else{
            link(im, im->next, new_front);
        }
        ++sz;
    }
    void pop_front() {
        if (sz == 1) {
            --sz;
            AllocTraits::destroy(alloc, im->next);
            alloc.deallocate(im->next, 1);
            im->prev = nullptr;
            im->next = nullptr;
        } else {
            im->next->next->prev = im;
            Node* new_next = im->next->next;
            AllocTraits::destroy(alloc, im->next);
            alloc.deallocate(im->next, 1);
            im->next = new_next;
            --sz;
        }
    }

    void insert(iterator iter, const T& item){
        if (sz == 0) push_back(item);
        else{
            Node* new_node = alloc.allocate(1);
            AllocTraits::construct(alloc, new_node, item);
            link(iter.ptr->prev, iter.ptr, new_node);
        }
        sz++;
    }
    void erase(iterator iter){
        if (sz == 1) pop_back();
        else{
            iter.ptr->next->prev = iter.ptr->prev;
            iter.ptr->prev->next = iter.ptr->next;
            AllocTraits::destroy(alloc, iter.ptr);
            alloc.deallocate(iter.ptr, 1);
            --sz;
        }
    }

    void insert(const_iterator iter, const T& item){
        if (sz == 0) push_back(item);
        else{
            Node* new_node = alloc.allocate(1);
            AllocTraits::construct(alloc, new_node, item);
            link(iter.ptr->prev, const_cast<Node*>(iter.ptr), new_node);
        }
        sz++;
    }
    void erase(const_iterator iter){
        if (sz == 1) pop_back();
        else{
            Node* cur = const_cast<Node*>(iter.ptr);
            cur->next->prev = cur->prev;
            cur->prev->next = cur->next;
            AllocTraits::destroy(alloc, cur);
            alloc.deallocate(cur, 1);
            --sz;
        }
    }


    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    iterator begin(){
        return iterator(im->next);
    }
    const_iterator begin() const{
        return const_iterator(im->next);
    }
    const_iterator cbegin() const{
        return const_iterator(im->next);
    }
    iterator end(){
        return iterator(im);
    }
    const_iterator end()const{
        return const_iterator(im);
    }
    const_iterator cend() const{
        return const_iterator(im);
    }
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
};