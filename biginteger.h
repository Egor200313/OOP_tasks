#include <iostream>
#include <vector>
#include <string>

class BigInteger{
private:
    std::vector<int> chunks;//storage of digits
    const int base = 10;
    bool is_neg = false;//control of negativity
public:
    BigInteger(){
        chunks.push_back(0);
        is_neg = false;
    }
    BigInteger(int integer){
        if (integer > 0) is_neg=false;
        else{
            is_neg=true;
            integer *= (-1);
        }
        if (integer == 0){
            chunks.push_back(0);
            is_neg = false;
        }
        while(integer){
            chunks.push_back(integer%base);
            integer /= base;
        }
    }
    BigInteger(const BigInteger& copy) {
        chunks=copy.chunks;
        is_neg=copy.is_neg;
    }

    BigInteger& operator= (BigInteger b){
        chunks = b.chunks;
        is_neg = b.is_neg;
        return *this;
    }

    bool operator < (const BigInteger &bg)const{
        if (is_neg && !bg.is_neg) return true;
        if (!is_neg && bg.is_neg) return false;
        if (!is_neg){//comparison negative numbers
            if (chunks.size() < bg.chunks.size()) return true;
            if (chunks.size() > bg.chunks.size()) return false;
            for (int i = static_cast<int>(chunks.size()-1); i>=0; --i){
                if (chunks[i] < bg.chunks[i]) return true;
                if (chunks[i] > bg.chunks[i]) return false;
            }
            return false;
        }
        else{//comparison positive numbers
            if (chunks.size() > bg.chunks.size()) return true;
            if (chunks.size() < bg.chunks.size()) return false;
            for (int i = static_cast<int>(chunks.size()-1); i>=0; --i){
                if (chunks[i] > bg.chunks[i]) return true;
                if (chunks[i] < bg.chunks[i]) return false;
            }
            return false;
        }
    }
    bool operator == (const BigInteger &bg)const{
        if (is_neg != bg.is_neg) return false;
        if (chunks.size() != bg.chunks.size()) return false;
        for (size_t i = 0; i < chunks.size(); ++i){
            if (chunks[i] != bg.chunks[i]) return false;
        }
        return true;
    }
    bool operator > (const BigInteger &bg)const {
        return (!(*this < bg)&&!(*this == bg));
    }

    bool operator <= (const BigInteger &bg)const{
        return (*this < bg || *this == bg);
    }
    bool operator >= (const BigInteger &bg)const{
        return bg <= *this;
    }
    bool operator!=(const BigInteger &bg)const{
        return !(*this == bg);
    }

    explicit operator int()const{//promotion to integer
        int res = 0;
        int tmp = 1;
        for (size_t i = 0; i < chunks.size(); ++i){
            res += chunks[i] * tmp;
            tmp *= base;
        }
        if (is_neg) return -res;
        else return res;
    }

    explicit operator bool()const{//promotion to boolean
        if (chunks.size() == 1 && chunks[0] == 0) return false;
        else return true;
    }

    BigInteger& operator+=(const BigInteger& other) {
        if (is_neg==other.is_neg){//addition if signs are equal
            for (size_t i = 0; i < std::max(chunks.size(), other.chunks.size()); i++){
                if (i >= chunks.size()) chunks.push_back(0);
                if (other.chunks.size() > i) chunks[i] += other.chunks[i];
            }
            for (size_t i = 0; i < std::max(chunks.size(), other.chunks.size()) - 1; i++) {
                if (chunks[i] >= 10) {
                    chunks[i] -= 10;
                    if (i + 1 == chunks.size()) chunks.push_back(0);
                    chunks[i + 1]++;
                }
            }
        }
        else{
            is_neg = !is_neg;
            *this -= other;//subtraction if signs are not equal
            is_neg = !is_neg;
        }
        int im = chunks[chunks.size()-1];
        while (im != 0){
            chunks.push_back(0);
            chunks[chunks.size() - 2] = im % 10;
            im /= 10;
        }
        while (chunks.back() == 0 && chunks.size() > 1) chunks.pop_back();//deletion leading zeros
        return *this;
    }
    BigInteger& operator-=(const BigInteger& other) {
        if (!other) return *this;
        if (*this == 0){
            if (other) other.is_neg = !other.is_neg;
            *this = other;
            return *this;
        }
        if ((*this >= other && !other.is_neg) || (other.is_neg && *this<other)){//subtracting more from less
            if (other.is_neg && *this < other) is_neg = true;
            else is_neg = false;
            for (size_t i = 0; i < std::max(chunks.size(), other.chunks.size()); i++) {
                if (i >= chunks.size()) chunks.push_back(0);
                if (other.chunks.size() > i) chunks[i] -= other.chunks[i];
            }
            for (size_t i = 0; i < std::max(chunks.size(), other.chunks.size()) - 1; i++) {
                if (chunks[i] < 0) {
                    chunks[i] += 10;
                    if (i + 1 == chunks.size()) chunks.push_back(0);
                    chunks[i + 1]--;
                }
            }
            if (chunks.back() < 0) chunks.back() *= -1;
        }
        else if ((*this >= other && this->is_neg) || (!this->is_neg && *this < other)){//reversed order of subtracting
            if (*this >= other && this->is_neg) is_neg = false;
            else is_neg = true;
            for (size_t i = 0; i < std::max(chunks.size(), other.chunks.size()); i++) {
                if (i >= chunks.size()) chunks.push_back(0);
                if (other.chunks.size() > i) chunks[i] = other.chunks[i] - chunks[i];
            }
            for (size_t i = 0; i < chunks.size() - 1; ++i) {
                if (chunks[i] < 0) {
                    chunks[i] += 10;
                    if (i + 1 == chunks.size()) chunks.push_back(0);
                    chunks[i + 1]--;
                }
            }
            if (chunks.back() < 0) chunks.back() *= -1;
        }
        else{
            is_neg = !is_neg;
            *this += other;//addition if signs are equal
            is_neg = !is_neg;
        }
        while (chunks.back() == 0 && chunks.size() > 1) chunks.pop_back();//deletion leading zeros
        return *this;
    }
    BigInteger& operator/= (const BigInteger&);
    BigInteger& operator*= (const BigInteger&);
    BigInteger& operator%= (const BigInteger&);

    BigInteger& operator--(){
        *this -= 1;
        return *this;
    }
    BigInteger& operator++(){
        *this += 1;
        return *this;
    }
    BigInteger operator--(int){
        BigInteger copy = *this;
        --*this;
        return copy;
    }

    BigInteger operator++(int){
        BigInteger copy = *this;
        ++*this;
        return copy;
    }
    BigInteger operator-(){
        BigInteger copy = *this;
        if (copy == 0) return copy;
        copy.is_neg =! copy.is_neg;
        return copy;
    }
    std::string toString() const{
        std::string tmp = "";
        if (is_neg) tmp.push_back('-');
        for (size_t i = chunks.size(); i > 0; --i){
            char x = '0'+ chunks[i - 1];
            tmp.push_back(x);
        }
        return tmp;
    }
    friend std::ostream& operator<< (std::ostream &out, const BigInteger &bg);
    friend std::istream& operator>> (std::istream &in, BigInteger &bg);
};
BigInteger operator+ (const BigInteger&, const BigInteger&);
BigInteger operator- (const BigInteger&, const BigInteger&);
BigInteger operator* (const BigInteger&, const BigInteger&);
BigInteger operator/ (const BigInteger&, const BigInteger&);
BigInteger operator% (const BigInteger&, const BigInteger&);

std::ostream& operator<< (std::ostream &out, const BigInteger &bg){
    out << bg.toString();
    return out;
}
std::istream& operator>> (std::istream &in, BigInteger &bg){
    std::string s;
    in >> s;
    bool b = false;
    bg = 0;
    size_t start = 0;
    if (s.size() > 0 && s[0] == '-'){
        b = true;
        ++start;
    }
    bg.chunks.resize(s.size() - start);
    for (size_t i = 0; i < bg.chunks.size(); ++i) {
        bg.chunks[i] = s[s.size() - i] - '0';
    }
    if (b) bg *= -1;
    return in;
}


BigInteger& BigInteger::operator /=(const BigInteger& right) {
    if (right == 0) return *this;
    BigInteger b = right;
    if (b == 1) return *this;
    if (b == -1){
        is_neg = !is_neg;
        return *this;
    }
    b.is_neg = false;
    BigInteger res, cur = 0;
    res.chunks.resize(chunks.size());
    for (long i = chunks.size() - 1; i >= 0; --i) {
        cur = cur * 10 + chunks[i];
        while (cur.chunks.back() == 0 && cur.chunks.size() > 1) cur.chunks.pop_back();
        int l = 0, r = 11;
        while (l + 1 < r) {
            int m = (l + r) / 2;
            BigInteger t = b;
            t *= BigInteger(m);
            if (t <= cur) {
                l = m;
            }
            else r = m;
        }

        res.chunks[i] = l;
        BigInteger tmp = b;
        tmp *= l;
        while (tmp.chunks.back() == 0 && tmp.chunks.size() > 1) tmp.chunks.pop_back();
        cur -= tmp;
    }

    res.is_neg = is_neg != right.is_neg;
    int im = res.chunks[res.chunks.size() - 1];
    while (im != 0){
        res.chunks.push_back(0);
        res.chunks[res.chunks.size()-2] = im % 10;
        im /= 10;
    }
    while (res.chunks.back() == 0 && res.chunks.size() > 1) res.chunks.pop_back();
    if (res.chunks.size() == 1 && res.chunks[0] == 0) res.is_neg = false;
    *this = res;
    return *this;
}

BigInteger& BigInteger::operator*=(const BigInteger &bg){
    BigInteger tmp = BigInteger(0);
    if (bg == 0){
        *this = tmp;
        return *this;
    }
    for (size_t i = 0; i < chunks.size();++i){
        int im = 0;
        for(size_t j = 0; j < bg.chunks.size() || im != 0; ++j){
            int res = im;
            if (j < bg.chunks.size()) res = chunks[i] * bg.chunks[j] + im;
            if (i + j >= tmp.chunks.size()) tmp.chunks.push_back(0);
            tmp.chunks[i + j] += res % 10;
            im = res / 10;
            if (tmp.chunks[i+j] > 9){
                im += tmp.chunks[i+j] / 10;
                tmp.chunks[i+j] %= 10;
            }
        }
    }
    chunks = tmp.chunks;
    if (is_neg == bg.is_neg) is_neg = false;
    else is_neg = true;
    int im = chunks[chunks.size()-1];
    while (im != 0){
        chunks.push_back(0);
        chunks[chunks.size()-2] = im % 10;
        im /= 10;
    }
    while (chunks.back() == 0 && chunks.size() > 1) chunks.pop_back();
    return *this;
}
BigInteger& BigInteger::operator%=(const BigInteger &bg){
    *this -= ((*this / bg) * bg);
    return *this;
}
BigInteger operator+ (const BigInteger &a, const BigInteger &b){
    BigInteger copy = a;
    copy += b;
    return copy;
}
BigInteger operator- (const BigInteger &a, const BigInteger &b){
    BigInteger copy = a;
    copy -= b;
    return copy;
}
BigInteger operator* (const BigInteger &a, const BigInteger &b){
    BigInteger copy = a;
    copy *= b;
    return copy;
}

BigInteger operator/ (const BigInteger &a, const BigInteger &b){
    BigInteger copy = a;
    copy /= b;
    return copy;
}
BigInteger operator% (const BigInteger &a, const BigInteger &b) {
    BigInteger copy = a;
    copy %= b;
    return copy;
}


class Rational{
private:
    BigInteger numerator, denominator;
    BigInteger nod(BigInteger a, BigInteger b){
        while (a && b){
            if (a > b) a %= b;
            else b %= a;
        }
        return (a)? a: b;
    }
public:
    Rational(int x, int y):numerator(x), denominator(y){}
    Rational(const BigInteger& f, const BigInteger& t = 1){
        numerator = f;
        denominator = t;
    }
    Rational(int x):numerator(x), denominator(1){}
    Rational():numerator(0), denominator(1){}
    ~Rational(){}
    BigInteger modul(BigInteger b)const{
        if (b >= 0) return b;
        else return b *= -1;
    }
    void reduce(){
        if (denominator<0){
            numerator *= -1;
            denominator *= -1;
        }
        BigInteger n = nod(modul(numerator), modul(denominator));
        numerator /= n;
        denominator /= n;
    }
    Rational& operator +=(const Rational &f){
        numerator *= f.denominator;
        numerator += (f.numerator*denominator);
        denominator *= f.denominator;
        reduce();
        return *this;
    }
    Rational& operator -=(const Rational &f){
        if (f.numerator == 0) return *this;
        numerator *= f.denominator;
        numerator -= (f.numerator*denominator);
        denominator *= f.denominator;
        reduce();
        return *this;
    }
    Rational& operator *=(const Rational &f){
        numerator *= f.numerator;
        denominator *= f.denominator;
        reduce();
        return *this;
    }
    Rational& operator /=(const Rational &f){
        numerator *= f.denominator;
        denominator *= f.numerator;
        reduce();
        return *this;
    }

    bool operator==(const Rational &x)const{
        return (x.numerator==numerator && x.denominator==denominator);
    }
    bool operator!=(const Rational &x)const{
        return !(*this==x);
    }
    bool operator <=(const Rational &x)const{
        return numerator*x.denominator <= denominator*x.numerator;
    }
    bool operator >=(const Rational &x)const{
        return x <= *this;
    }
    bool operator <(const Rational &x)const{
        return (*this <= x && *this != x);
    }
    bool operator >(const Rational &x)const{
        return x < *this;
    }
    std::string toString(){
        std::string ans = numerator.toString();
        if (denominator != 1) ans = ans + '/' + denominator.toString();
        return ans;
    }
    std::string asDecimal(size_t presicion)const;
    explicit operator double()const;
    Rational operator -()const{
        Rational copy = *this;
        copy.numerator *= -1;
        copy.reduce();
        return copy;
    }
};

Rational operator +(const Rational &a, const Rational& b){
    Rational x = a;
    x += b;
    return x;
}
Rational operator -(const Rational &a, const Rational& b){
    Rational x = a;
    x -= b;
    return x;
}
Rational operator *(const Rational &a, const Rational& b){
    Rational x = a;
    x *= b;
    return x;
}
Rational operator /(const Rational &a, const Rational& b){
    Rational x = a;
    x /= b;
    return x;
}
std::string Rational::asDecimal(size_t presicion = 0)const{
    bool neg = false;
    if (numerator < 0) neg = true;
    BigInteger t = modul(numerator) / modul(denominator);
    std::string tmp = t.toString();
    size_t before_comma = tmp.size();
    std::string zeros = "";
    BigInteger copy = modul(numerator);
    for(size_t i = 0; i < presicion; ++i){
        if (copy < modul(denominator)) zeros = zeros + "0";
        BigInteger r = BigInteger(10);
        copy *= 10;
    }
    BigInteger completed = copy/denominator;
    std::string ans;
    tmp = completed.toString();
    if (zeros.size()>0){
        ans = zeros+tmp;
        if (presicion>0) ans.insert(1, ".");
    }else{
        ans = tmp;
        if (before_comma<ans.size()) ans.insert(before_comma, ".");
    }
    if (neg) ans = "-" + ans;
    return ans;
}

Rational::operator double()const{
    std::string tmp = asDecimal(16);
    int sign = 1;
    size_t i = 0;
    if (tmp[0] == '-'){
        sign = -1;
        ++i;
    }
    double res = 0.0;
    while (i < tmp.size() && tmp[i] != '.'){
        res = 10*res + (tmp[i] - '0');
    }
    double k = 0.1;
    while (i < tmp.size()){
        res += k * (tmp[i] - '0');
        k /= 10;
    }
    return res * sign;
}
