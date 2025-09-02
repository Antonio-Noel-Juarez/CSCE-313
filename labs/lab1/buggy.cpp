#include <iostream>
#include <cstring>
#include <cstdio>

struct Point {
    int x, y;

    Point () : x(), y() {}
    Point (int _x, int _y) : x(_x), y(_y) {}
};

class Shape {
    int vertices;
    Point** points;
public:
    Shape (int _vertices) {
        this->vertices = _vertices;
        this->points = new Point*[this->vertices+1];

	for (int i = 0; i <= this->vertices; i++) {
      	    this->points[i] = new Point();
	}
    }

    ~Shape () {
	for(int i = 0; i <= this->vertices; i++) {
	    delete this->points[i];
	}

	delete[] this->points;
    }

    void addPoints (Point* points) {
        for (int i = 0; i <= vertices; i++) {
            memcpy(this->points[i], &points[i%vertices], sizeof(Point));
        }
    }

    double area () {
        int temp = 0;
        for (int i = 0; i < this->vertices; i++) {
            // FIXME: there are two methods to access members of pointers
	    //        use one to fix lhs and the other to fix rhs
	    int lhs = this->points[i]->x * this->points[i+1]->y;
            int rhs = this->points[i+1]->x * this->points[i]->y;
            temp += (lhs - rhs);
        }
        double area = abs(temp)/2.0;
        return area;
    }
};

int main () {
    // FIXME: create the following points using the three different methods
    //        of defining structs:
    //          tri1 = (0, 0)
    //          tri2 = (1, 2)
    //          tri3 = (2, 0)

    Point tri1;
    tri1.x = 0;
    tri1.y = 0;

    Point tri2 = {1, 2};
   
    Point tri3(2, 0);

    // adding points to tri
    Point triPts[3] = {tri1, tri2, tri3};
    Shape* tri = new Shape(3);
    tri->addPoints(triPts);

    // FIXME: create the following points using your preferred struct
    //        definition:
    //          quad1 = (0, 0)
    //          quad2 = (0, 2)
    //          quad3 = (2, 2)
    //          quad4 = (2, 0)
    
    Point quad1 = {0, 0};
    Point quad2 = {0, 2};
    Point quad3 = {2, 2};
    Point quad4 = {2, 0};

    // adding points to quad
    Point quadPts[4] = {quad1, quad2, quad3, quad4};
    Shape* quad = new Shape(4);
    quad->addPoints(quadPts);

    // FIXME: print out area of tri and area of quad
    printf("%f\n", tri->area());
    printf("%f\n", quad->area());

    delete tri;
    delete quad;
}
