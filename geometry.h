#include <iostream>
#include <cstring>
#include <cmath>
#include <vector>
#include <initializer_list>

bool _eq(double a, double b){
    return (std::abs(a-b)<10e-7);
}

struct Point{
    double x;
    double y;
    Point()=default;
    Point(double x, double y): x(x), y(y){}
    bool operator ==(const Point& p)const{
        return (_eq(x, p.x) && _eq(y, p.y));
    }
    bool operator !=(const Point& p)const{
        return !(*this == p);
    }
    void rotate(Point center, double phi){
        double v_x = x - center.x;
        double v_y = y - center.y;
        double nx = v_x*cos(phi*M_PI/180)-v_y*sin(phi*M_PI/180);
        double ny = v_x*sin(phi*M_PI/180)+v_y*cos(phi*M_PI/180);
        x = nx;
        y = ny;
    }

};

class Line{
public:
    double a;
    double b;
    double c;
    ~Line()=default;
    Line (const Point& a, const Point& b){
        this->a = b.y - a.y;
        this->b = a.x-b.x;
        c = a.y*b.x - a.x*b.y;
    }
    Line(double a, double b, double c):a(a), b(b), c(c){}

    Line(double k, double b):a(k), b(-1), c(b){}
    Line() = default;
    Line (const Point& p, double k){
        a = -k;
        b = 1;
        c = k*p.x - p.y;
    }
    bool operator ==(const Line& p)const{
        if (b!=0){
            if(!_eq(-c/b,-p.c/p.b)) return false;
            if(!_eq((-c-a)/b,(-p.c-p.a)/p.b)) return false;
        }
        return true;
    }
    bool operator !=(const Line& p)const{
        return !(*this == p);
    }
};

class Shape {
public:
    virtual double perimeter()const = 0;
    virtual double area()const = 0;
    virtual ~Shape() = default;

    virtual bool isCongruentTo(const Shape&)const = 0;
    virtual bool isSimilarTo(const Shape&)const = 0;
    virtual bool containsPoint(Point point)const = 0;
    virtual bool operator==(const Shape&)const = 0;
    virtual bool operator!=(const Shape&)const = 0;

    virtual void reflex(Point) = 0;
    virtual void rotate(Point, double) = 0;
    virtual void scale(Point, double) = 0;
    virtual void reflex(Line&) = 0;
};

class Polygon: public Shape{
protected:
    std::vector<Point> vertices;
public:
    Polygon() = default;
    Polygon(const std::vector<Point>&v){
        for (unsigned long i = 0; i < v.size(); ++i){
            vertices.push_back(v[i]);
        }
    }
    Polygon(std::initializer_list<Point> &list){
        for (auto &elem : list)
        {
            vertices.push_back(elem);
        }
    }


    ~Polygon() = default;
    size_t verticesCount() const{
        return vertices.size();
    }
    const std::vector<Point> getVertices(){
        return vertices;
    }
    bool isConvex() const{
        if (verticesCount()==3) return true;
        bool neg = false, checked = false;
        double v1_x, v1_y, v2_x, v2_y;
        for (int i = 0; i < verticesCount(); ++i){
            if (i == 0){
                v1_x = vertices[i].x - vertices.back().x;
                v1_y = vertices[i].y - vertices.back().y;
                v2_x = vertices[i+1].x - vertices[i].x;
                v2_y = vertices[i+1].y - vertices[i].y;
            }
            else if (i == verticesCount()-1){
                v1_x = vertices[i].x - vertices[i-1].x;
                v1_y = vertices[i].y - vertices[i-1].y;
                v2_x = vertices[0].x - vertices[i].x;
                v2_y = vertices[0].y - vertices[i].y;
            }
            else{
                v1_x = vertices[i].x - vertices[i-1].x;
                v1_y = vertices[i].y - vertices[i-1].y;
                v2_x = vertices[i+1].x - vertices[i].x;
                v2_y = vertices[i+1].y - vertices[i].y;
            }
            if (!checked){
                if (v1_x*v2_y - v1_y*v2_x < 0 ){
                    neg = true;
                }else neg = false;
                checked = true;
            }else{
                if (v1_x*v2_y - v1_y*v2_x < 0 && !neg) return false;
                if (v1_x*v2_y - v1_y*v2_x > 0 && neg) return false;
            }
        }
        return true;
    }

    double perimeter()const override{
        double ans = 0;
        for (int i = 0; i < verticesCount()-1; ++i){
            double delta1 = vertices[i].x - vertices[i+1].x;
            double delta2 = vertices[i].y - vertices[i+1].y;
            ans+=sqrt(delta1*delta1+delta2*delta2);
        }
        double delta1 = vertices[verticesCount()-1].x - vertices[0].x;
        double delta2 = vertices[verticesCount()-1].y - vertices[0].y;
        ans+=sqrt(delta1*delta1+delta2*delta2);
        return ans;
    }
    double area()const override{
        double ans = 0.0;
        for (int i = 0; i < verticesCount(); ++i){
            if (i == verticesCount()-1) ans+=(vertices[i].x*vertices[0].y);
            else ans+=(vertices[i].x*vertices[i+1].y);
        }
        for (int i = 0; i < verticesCount(); ++i){
            if (i == verticesCount()-1) ans-=(vertices[i].y*vertices[0].x);
            else ans-=(vertices[i].y*vertices[i+1].x);
        }
        if (ans<0) ans*=-1;
        return ans/2;
    }

    void reflex(Point p)override{
        for (int i = 0; i < verticesCount(); ++i){
            double v1_x = p.x - vertices[i].x;
            double v1_y = p.y - vertices[i].y;
            vertices[i].x+=2*v1_x;
            vertices[i].y+=2*v1_y;
        }
    }
    void scale(Point p, double k)override{
        for (int i = 0; i < verticesCount(); ++i){
            double v1_x = vertices[i].x - p.x;
            double v1_y = vertices[i].y - p.y;
            vertices[i].x = p.x + k*v1_x;
            vertices[i].y = p.y + k*v1_y;
        }
    }

    bool containsPoint(Point point)const override{
        bool result = false;
        int j = verticesCount() - 1;
        for (int i = 0; i < verticesCount(); i++) {
            if ( ((vertices[i].y < point.y && vertices[j].y >= point.y) || (vertices[j].y < point.y && vertices[i].y >= point.y)) &&
                 (vertices[i].x + (point.y - vertices[i].y) / (vertices[j].y - vertices[i].y) * (vertices[j].x - vertices[i].x) < point.x) )
                result = !result;
            j = i;
        }
        return result;
    }
    void reflex(Line& l)override{
        for (int i = 0; i < verticesCount(); ++i){
            double x = (vertices[i].x*l.b*l.b - l.a*l.b*vertices[i].y - l.a*l.c)/(l.a*l.a+l.b*l.b);
            double y = (-l.c - l.a*x)/l.b;
            double v_x = x - vertices[i].x;
            double v_y = y - vertices[i].y;
            vertices[i].x += 2*v_x;
            vertices[i].y += 2*v_y;
        }
    }
    void rotate(Point center, double phi)override{
        for (int i = 0; i < verticesCount(); ++i){
            vertices[i].rotate(center, phi);
        }
    }

    bool operator==(const Shape& sh)const override{
        const Polygon* tmp = dynamic_cast<const Polygon*>(&sh);
        if (tmp==NULL) return false;
        if (verticesCount()!=tmp->verticesCount()) return false;


        int index = -1;

        for (int i = 0; i<verticesCount(); ++i){
            if (tmp->vertices[i]==vertices[0]){
                index = i;
                break;
            }
        }


        bool b = true;
        for(int j = 0; j < verticesCount();++j){
            if (tmp->vertices[(j+index)%verticesCount()]!=vertices[j]) b = false;
        }
        if (b) return true;
        b = true;
        for(int j = verticesCount()-1; j >= 0;--j){
            if (tmp->vertices[(index-j+verticesCount())%verticesCount()]!=vertices[j]) b = false;
        }
        if (b) return true;
        return false;
    }

    bool operator!=(const Shape& sh)const override{
        return !(*this == sh);
    }

    bool isCongruentTo(const Shape& sh)const override{
        const Polygon* tmp = dynamic_cast<const Polygon*>(&sh);
        if (tmp==NULL) return false;
        if (verticesCount()!=tmp->verticesCount()) return false;
        std::vector<double>lines1, lines2;
        double v_x, v_y, v1_x, v1_y;
        for (int i = 0; i < verticesCount();++i){
            if (i==verticesCount()-1){
                v_x = tmp->vertices[0].x - tmp->vertices[i].x;
                v_y = tmp->vertices[0].y - tmp->vertices[i].y;
                v1_x = vertices[0].x - vertices[i].x;
                v1_y = vertices[0].y - vertices[i].y;
            }
            else{
                v_x = tmp->vertices[i+1].x - tmp->vertices[i].x;
                v_y = tmp->vertices[i+1].y - tmp->vertices[i].y;
                v1_x = vertices[i+1].x - vertices[i].x;
                v1_y = vertices[i+1].y - vertices[i].y;
            }
            double v_len = sqrt(v_x*v_x+v_y*v_y);
            double v1_len = sqrt(v1_x*v1_x+v1_y*v1_y);
            lines1.push_back(v_len);
            lines2.push_back(v1_len);
        }
        std::vector<int>first_i;
        for (int i = 0; i<verticesCount(); ++i){
            if (_eq(lines2[i],lines1[0])) first_i.push_back(i);
        }

        for (size_t i = 0; i < first_i.size();++i){
            bool b = true;
            for(int j = 0; j < verticesCount();++j){
                if (!_eq(lines2[(j+first_i[i])%verticesCount()],lines1[j])) b = false;
            }
            if (b) return true;
            b = true;

            for(int j = verticesCount()-1; j >= 0;--j){
                if (!_eq(lines2[(first_i[i]-j+verticesCount())%verticesCount()],lines1[j])) b = false;
            }
            if (b) return true;
        }
        return false;
    }
    bool isSimilarTo(const Shape& sh)const override{
        const Polygon* tmp = dynamic_cast<const Polygon*>(&sh);
        if (tmp==NULL) return false;
        if (verticesCount()!=tmp->verticesCount()) return false;
        double delta = perimeter()/tmp->perimeter();
        if (_eq(delta*delta*tmp->area(),area())) return true;
        else return false;
    }
};

class Ellipse: public Shape{
protected:
    double a;// большая полуось эллипса
    double b;// меньшая полуось эллипса
    Point focus1;
    Point focus2;
public:
    Ellipse()=default;
    Ellipse(const Point& f1, const Point& f2, double l){
        focus1 = Point(f1.x, f1.y);
        focus2 = Point(f2.x, f2.y);
        a = l/2;
        double delta_x = f2.x - f1.x;
        double delta_y = f2.y - f1.y;
        double c = sqrt(delta_x*delta_x + delta_y*delta_y);
        b = sqrt(a*a - c*c/4);
    }

    ~Ellipse() = default;
    std::pair<Point,Point> focuses() const{
        return std::make_pair(focus1, focus2);
    }
    std::pair<Line, Line> directrices() const{
        double delta_x = focus2.x - focus1.x;
        double delta_y = focus2.y - focus1.y;
        double c = sqrt(delta_x*delta_x + delta_y*delta_y);
        double v_x = delta_x;
        double v_y = delta_y;
        double v_len = sqrt(v_x*v_x + v_y*v_y);
        double l = (a*a/c) - c;
        double k = (v_len + l)/v_len;
        v_x*=k;
        v_y*=k;
        Point h1 = Point(v_x, v_y);
        Line f = Line(focus1, focus2);
        double v_n_x = f.a;
        double v_n_y = f.b;
        v_x+=v_n_x;
        v_y+=v_n_y;
        Point h2 = Point(v_x, v_y);
        Line dir1 = Line(h1, h2);
        v_x-=v_n_x;
        v_y-=v_n_y;
        v_x*=-1;
        v_y*=-1;
        v_x+=focus2.x;
        v_y+=focus2.y;
        Point t = Point(v_x, v_y);
        double c1 = -dir1.a*t.x - dir1.b*t.y;
        Line dir2= Line(dir1.a, dir1.b, c1);
        return std::make_pair(dir1, dir2);
    }
    double eccentricity() const{
        double delta_x = focus2.x - focus1.x;
        double delta_y = focus2.y - focus1.y;
        double c = sqrt(delta_x*delta_x + delta_y*delta_y);
        c/=2;
        return c/a;
    }

    Point center() const{
        double delta_x = focus2.x + focus1.x;
        double delta_y = focus2.y + focus1.y;
        return Point(delta_x/2, delta_y/2);
    }

    double perimeter()const override{
        double x = sqrt((3*a+b)*(a+3*b));
        return M_PI*(3*(a+b)-x);
    }
    double area()const override{
        return M_PI*a*b;
    }

    bool containsPoint(Point point)const override{
        if (point==focus1 || point==focus2) return true;
        double v1_x = point.x - focus1.x;
        double v1_y = point.y - focus1.y;
        double v2_x = point.x - focus2.x;
        double v2_y = point.y - focus2.y;
        double len1 = sqrt(v1_x*v1_x+v1_y*v1_y);
        double len2 = sqrt(v2_x*v2_x+v2_y*v2_y);
        if (len1+len2>2*a) return false;
        return true;
    }

    void reflex(Point p)override{
        double v1_x = p.x - focus1.x;
        double v1_y = p.y - focus1.y;
        focus1.x+=2*v1_x;
        focus1.y+=2*v1_y;
        v1_x = p.x - focus2.x;
        v1_y = p.y - focus2.y;
        focus2.x+=2*v1_x;
        focus2.y+=2*v1_y;
    }
    void reflex(Line& l)override{
        double x = (focus1.x * l.b*l.b - l.a*l.b*focus1.y - l.a*l.c)/(l.a*l.a+l.b*l.b);
        double y = (-l.c - l.a*x)/l.b;
        double v_x = x - focus1.x;
        double v_y = y - focus1.y;
        focus1.x += 2*v_x;
        focus1.y += 2*v_y;
        x = (focus2.x*l.b*l.b - l.a*l.b*focus2.y - l.a*l.c)/(l.a*l.a+l.b*l.b);
        y = (-l.c - l.a*x)/l.b;
        v_x = x - focus2.x;
        v_y = y - focus2.y;
        focus2.x += 2*v_x;
        focus2.y += 2*v_y;
    }

    void scale(Point p, double k)override{
        double v1_x = focus1.x - p.x;
        double v1_y = focus1.y - p.y;
        focus1.x = p.x + k*v1_x;
        focus1.y = p.y + k*v1_y;
        v1_x = focus2.x - p.x;
        v1_y = focus2.y - p.y;
        focus2.x = p.x + k*v1_x;
        focus2.y = p.y + k*v1_y;
        a *=k;
        double delta_x = focus2.x - focus1.x;
        double delta_y = focus2.y - focus1.y;
        double c = sqrt(delta_x*delta_x + delta_y*delta_y);
        b = sqrt(a*a - c*c/4);
    }

    void rotate(Point center, double phi)override{
        focus1.rotate(center, phi);
        focus2.rotate(center, phi);
    }

    bool operator==(const Shape& sh)const override{
        const Ellipse* tmp = dynamic_cast<const Ellipse*>(&sh);
        if (tmp==NULL) return false;
        if (_eq(tmp->a,a) && _eq(tmp->b,b) && (((tmp->focus1 == focus1) && (tmp->focus2 == focus2) )|| ((tmp->focus1 == focus2) && (tmp->focus2 == focus1)))) return true;
        return false;
    }
    bool operator!=(const Shape& sh)const override{
        return !(*this == sh);
    }
    bool isCongruentTo(const Shape& sh)const override{
        const Ellipse* tmp = dynamic_cast<const Ellipse*>(&sh);
        if (tmp==NULL) return false;
        double c1 = (focus1.x-focus2.x)*(focus1.x-focus2.x)+(focus1.y-focus2.y)*(focus1.y-focus2.y);
        double c2 = (tmp->focus1.x-tmp->focus2.x)*(tmp->focus1.x-tmp->focus2.x)
                +(tmp->focus1.y-tmp->focus2.y)*(tmp->focus1.y-tmp->focus2.y);

        if (_eq(c1,c2)&&_eq(a, tmp->a)) return true;
        return false;
    }
    bool isSimilarTo(const Shape& sh)const override{
        const Ellipse* tmp = dynamic_cast<const Ellipse*>(&sh);
        if (tmp==NULL) return false;
        if (_eq(eccentricity(),tmp->eccentricity())) return true;
        else return false;
    }
};

class Circle: public Ellipse{
public:
    Circle(Point a, double r = 0){
        this->a = b = r;
        focus1 = focus2 = Point(a.x, a.y);
    }
    ~Circle() = default;
    double radius() const{
        return a;
    }
    double area()const override{
        return M_PI*a*a;
    }
    double perimeter()const override{
        return 2*M_PI*a;
    }
    Point Center() const{
        return focus1;
    }
};

class Rectangle: public Polygon {
public:
    Rectangle() = default;
    Rectangle(const std::vector<Point>&v){
        for (unsigned long i = 0; i < v.size(); ++i){
            vertices.push_back(v[i]);
        }
    }
    Rectangle(std::initializer_list<Point> &list){
        for (auto &elem : list)
        {
            vertices.push_back(elem);
        }
    }
    Rectangle(const Point& p1, const Point& p2, double k){
        vertices.push_back(Point(p1.x, p1.y));
        double v_x = p2.x - p1.x;
        double v_y = p2.y - p1.y;
        double c = sqrt(v_x*v_x + v_y*v_y);
        double b = sqrt((c*c)/(k*k + 1));
        double a = b*k;
        if (a < b) std::swap(a,b);
        double H_x = p1.x + v_x*b/c;
        double H_y = p1.y + v_y*b/c;
        Point tmp = Point(H_x, H_y);
        tmp.rotate(p1, acos(b/c)*180/M_PI);
        v_x = H_x - p1.x;
        v_y = H_y - p1.y;
        double x = v_x*(b/c)-v_y*(a/c);
        double y = v_x*(a/c)+v_y*(b/c);
        vertices.push_back(Point(p1.x+tmp.x,p1.y+tmp.y));
        vertices.push_back(Point(p2.x, p2.y));
        double x_c = (p1.x+p2.x)/2;
        double y_c = (p1.y+p2.y)/2;
        v_x = x_c - vertices[1].x;
        v_y = y_c - vertices[1].y;
        v_x*=2;
        v_y*=2;
        x = vertices[1].x+v_x;
        y = vertices[1].y+v_y;
        vertices.push_back(Point(x,y));
    }
    ~Rectangle() = default;
    Point center() const{
        double x_c = (vertices[0].x+vertices[2].x)/2;
        double y_c = (vertices[0].y+vertices[2].y)/2;
        return Point(x_c, y_c);
    }
    std::pair<Line, Line> diagonals() const{
        return std::make_pair(Line(vertices[0], vertices[2]), Line(vertices[1], vertices[3]));
    }

};

class Square : public Rectangle {
public:
    Square() = default;
    Square(const std::vector<Point>&v): Polygon(v){}
    Square(std::initializer_list<Point> &list): Polygon(list){}
    Square(const Point& p1, const Point& p2){
        double k = 1;
        vertices.push_back(Point(p1.x, p1.y));
        double v_x = p2.x - p1.x;
        double v_y = p2.y - p1.y;
        double c = sqrt(v_x*v_x + v_y*v_y);
        double b = sqrt((c*c)/(k*k + 1));
        double a = b*k;
        if (a < b) std::swap(a,b);
        double H_x = p1.x + v_x*b/c;
        double H_y = p1.y + v_y*b/c;
        Point tmp = Point(H_x, H_y);
        tmp.rotate(p1, acos(b/c)*180/M_PI);
        v_x = H_x - p1.x;
        v_y = H_y - p1.y;
        double x = v_x*(b/c)-v_y*(a/c);
        double y = v_x*(a/c)+v_y*(b/c);
        vertices.push_back(Point(p1.x+tmp.x,p1.y+tmp.y));
        vertices.push_back(Point(p2.x, p2.y));
        double x_c = (p1.x+p2.x)/2;
        double y_c = (p1.y+p2.y)/2;
        v_x = x_c - vertices[1].x;
        v_y = y_c - vertices[1].y;
        v_x*=2;
        v_y*=2;
        x = vertices[1].x+v_x;
        y = vertices[1].y+v_y;
        vertices.push_back(Point(x,y));
    }

    ~Square() = default;
    Circle circumscribedCircle() const{
        double delta_x = vertices[0].x-vertices[2].x;
        double delta_y = vertices[0].y-vertices[2].y;
        double r = sqrt(delta_x*delta_x + delta_y*delta_y)/2;
        return Circle(center(), r);
    }
    Circle inscribedCircle() const{
        double delta_x = vertices[0].x-vertices[1].x;
        double delta_y = vertices[0].y-vertices[1].y;
        double r = sqrt(delta_x*delta_x + delta_y*delta_y)/2;
        return Circle(center(), r);
    }
};

class Triangle: public Polygon {
public:
    Triangle(const std::vector<Point>&v): Polygon(v){}
    Triangle(std::initializer_list<Point> &list): Polygon(list){}
    Triangle(const Point& p1, const Point& p2, const Point& p3){
        vertices.push_back(Point(p1.x, p1.y));
        vertices.push_back(Point(p2.x, p2.y));
        vertices.push_back(Point(p3.x, p3.y));
    }

    ~Triangle() = default;
    Circle circumscribedCircle(){
        double d = 2*(vertices[0].x*(vertices[1].y - vertices[2].y)+vertices[1].x*(vertices[2].y-vertices[0].y)+vertices[2].x*(vertices[0].y-vertices[1].y));
        double A = pow(vertices[0].x, 2) + pow(vertices[0].y, 2);
        double B = pow(vertices[1].x, 2) + pow(vertices[1].y, 2);
        double C = pow(vertices[2].x, 2) + pow(vertices[2].y, 2);
        double x = (A*(vertices[1].y - vertices[2].y) + B*(vertices[2].y - vertices[0].y)+C*(vertices[0].y - vertices[1].y))/d;
        double y = -(A*(vertices[1].x - vertices[2].x) + B*(vertices[2].x - vertices[0].x)+C*(vertices[0].x - vertices[1].x))/d;
        double l1 = sqrt(pow(vertices[1].x-vertices[0].x, 2)+ pow(vertices[1].y - vertices[0].y, 2));
        double l2 = sqrt(pow(vertices[2].x-vertices[1].x, 2)+ pow(vertices[2].y - vertices[1].y, 2));
        double l3 = sqrt(pow(vertices[2].x-vertices[0].x, 2)+ pow(vertices[2].y - vertices[0].y, 2));
        double R = (l1*l2*l3)/(4*area());
        return Circle(Point(x,y), R);
    }
    Circle inscribedCircle(){
        double AB_l = sqrt(pow(vertices[1].x - vertices[0].x, 2) + pow(vertices[1].y - vertices[0].y, 2));
        double BC_l = sqrt(pow(vertices[2].x - vertices[1].x, 2) + pow(vertices[2].y - vertices[1].y, 2));
        double AC_l = sqrt(pow(vertices[2].x - vertices[0].x, 2) + pow(vertices[2].y - vertices[0].y, 2));
        double k1 = AB_l/(AC_l+AB_l);
        double AD_x = vertices[1].x - vertices[0].x + (vertices[2].x - vertices[1].x)*k1;
        double AD_y = vertices[1].y - vertices[0].y + (vertices[2].y - vertices[1].y)*k1;
        double k2 = AB_l/(BC_l+AB_l);
        double BH_x = vertices[0].x - vertices[1].x + (vertices[2].x - vertices[0].x) * k2;
        double BH_y = vertices[0].y - vertices[1].y + (vertices[2].y - vertices[0].y) * k2;
        double x_c = (BH_x*AD_x*(vertices[1].y - vertices[0].y)-AD_x*BH_y*vertices[1].x+BH_x*AD_y*vertices[0].x)/(BH_x*AD_y-AD_x*BH_y);
        double y_c = (BH_y*(x_c - vertices[1].x) + vertices[1].y*BH_x)/BH_x;
        double r = 2*area()/perimeter();
        return Circle(Point(x_c, y_c), r);
    }
    Point centroid(){
        double x1 = (vertices[0].x+vertices[1].x)/2;
        double y1 = (vertices[0].y+vertices[1].y)/2;
        double v_x = x1 - vertices[2].x;
        double v_y = y1 - vertices[2].y;
        v_x*=2;
        v_x/=3;
        v_y*=2;
        v_y/=3;
        return Point(vertices[2].x+v_x, vertices[2].y+v_y);
    }
    Point orthocenter(){
        double x1 = -(vertices[2].y - vertices[0].y)/(vertices[2].x - vertices[0].x);
        double y1 = 1.0;
        double x2 = -(vertices[2].y - vertices[1].y)/(vertices[2].x - vertices[1].x);
        double y2 = 1.0;
        double x = (x2*x1*(vertices[0].y - vertices[1].y)+vertices[1].x*x2*y1-vertices[0].x*y2*x1)/(x2*y1 - y2*x1);
        double y = (x1*vertices[1].y+y1*(x - vertices[1].x))/x1;
        return Point(x,y);
    }
    Line EulerLine(){
        double d = 2*(vertices[0].x*(vertices[1].y - vertices[2].y)+vertices[1].x*(vertices[2].y-vertices[0].y)+vertices[2].x*(vertices[0].y-vertices[1].y));
        double A = pow(vertices[0].x, 2) + pow(vertices[0].y, 2);
        double B = pow(vertices[1].x, 2) + pow(vertices[1].y, 2);
        double C = pow(vertices[2].x, 2) + pow(vertices[2].y, 2);
        double x = (A*(vertices[1].y - vertices[2].y) + B*(vertices[2].y - vertices[0].y)+C*(vertices[0].y - vertices[1].y))/d;
        double y = -(A*(vertices[1].x - vertices[2].x) + B*(vertices[2].x - vertices[0].x)+C*(vertices[0].x - vertices[1].x))/d;
        return Line(Point(x,y), orthocenter());
    }
    Circle ninePointsCircle(){
        double x = (orthocenter().x + circumscribedCircle().Center().x)/2;
        double y = (orthocenter().y + circumscribedCircle().Center().y)/2;
        return Circle(Point(x, y), circumscribedCircle().radius()/2);
    }

};
