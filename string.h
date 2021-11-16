#include <iostream>
#include <cstring>

class String{
private:
    size_t len = 0;
    char* storage = nullptr;
    size_t capacity = 2;
public:
    String(size_t n, char c) {
        len = n;
        if (n != 0) capacity = n * 2;
        storage = new char[capacity];
        memset(storage, c, n);
    }

    String(const char* a) {
        len = strlen(a);
        if (len != 0) capacity = len * 2;
        storage = new char[capacity];
        memcpy(storage, a, len);
    }

    String(): len(0), storage(nullptr), capacity(0){}

    size_t length() const{
        return len;
    }

    ~String(){
        delete[] storage;
    }

    void push_back(char c) {
        if (len == 0) {
            if (capacity > 0){
                ++len;
            }else{
                char* t = storage;
                delete []t;
                ++len;
                capacity = 2;
                storage = new char[2];
            }
            storage[0] = c;
        }
        else {
            if (len+1 > capacity) {
                capacity *= 2;
                char* tmp = new char[capacity];
                memcpy(tmp, storage, len);
                char* t = storage;
                storage = tmp;
                delete[] t;
            }
            storage[len] = c;
            ++len;
        }
    }
    void pop_back() {
        if (len>0) --len;
        if (4*len <= capacity && len > 0){
            capacity /= 2;
            char* tmp = new char[capacity];
            memcpy(tmp, storage, len);
            char* t = storage;
            storage = tmp;
            delete[] t;
        }
    }

    const char& front() const{
        return storage[0];
    }
    const char& back() const{
        return storage[len - 1];
    }

    char& front(){
        return storage[0];
    }
    char& back(){
        return storage[len - 1];
    }

    void clear(){
        char* t = storage;
        delete [] t;
        len = 0;
        capacity = 2;
        storage = new char[capacity];
    }

    String substr(size_t start, size_t count) const{
        String tmp;
        memcpy(tmp.storage, storage + start, count);
        return tmp;
    }

    bool empty()const{
        return len == 0;
    }

    //copy constructor
    String(const String& copy) {
        capacity = copy.len * 2;
        if (copy.len == 0) capacity = 2;
        storage = new char[capacity];
        len = copy.len;
        memcpy(storage, copy.storage, len);
    }

    String (const char c){
        len = 1;
        capacity = 4;
        storage = new char[capacity];
        *storage = c;
    }

    void swap(String& str){
        std::swap(len, str.len);
        std::swap(storage, str.storage);
        std::swap(capacity,str.capacity);
    }

    //=operator
    String& operator= (String str){
        swap(str);
        return *this;
    }

    String& operator+=(const String& str){
        if (capacity < len + str.len){
            char* tmp = new char[len];
            memcpy(tmp, storage, len);
            char* t = storage;
            delete[]t;
            capacity = len + str.len;
            storage = new char[capacity];
            memcpy(storage, tmp, len);
        }
        memcpy(storage+len, str.storage, str.len);
        len += str.len;
        return *this;
    }

    char& operator[](const size_t& index){
        return storage[index];
    }

    const char& operator[](const size_t& index) const{
        return storage[index];
    }

    size_t find(const String& substring) const{
        for (size_t i = 0; i < len - substring.length() + 1;++i){
            if (memcmp(storage + i, substring.storage, substring.length()) == 0) return i;
        }
        return len;
    }

    friend bool operator==(const String& str, const String& str1){
        if (strcmp(str1.storage, str.storage) == 0) return true;
        else return false;
    }

    size_t rfind(const String& substring) const{
        for (long i = len - substring.length(); i >= 0; --i){
            if (memcmp(storage+i, substring.storage, substring.length()) == 0) return static_cast<size_t>(i);
        }
        return len;
    }



    friend std::ostream& operator<< (std::ostream &out, const String &str);
    friend std::istream& operator>> (std::istream &in, String &str);
};

std::ostream& operator<< (std::ostream &out, const String &str){
    for (size_t i = 0; i < str.length(); ++i){
        out << str.storage[i];
    }
    return out;
}
std::istream& operator>> (std::istream &in, String &str) {
    char temp;
    str.clear();
    temp = in.get();
    while (isspace(temp)){
        temp = in.get();
    }
    while (!in.eof() && !isspace(temp)) {
        str.push_back(temp);
        temp = in.get();
    }
    return in;
}

String operator+ (const String& a, const String& b) {
    String copy = a;
    copy += b;
    return copy;
}
