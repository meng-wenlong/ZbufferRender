//
//  tri_vec.h
//  ZBuffer
//
//  Created by 孟文龙 on 2021/11/12.
//

#ifndef tri_vec_h
#define tri_vec_h

#include <math.h>
#include <ostream>


#define     GLH_ZERO                double(0.0)
#define     GLH_EPSILON          double(10e-6)
#define        GLH_EPSILON_2        double(10e-12)
#define     equivalent(a,b)             (((a < b + GLH_EPSILON) &&\
                                                      (a > b - GLH_EPSILON)) ? true : false)
inline double lerp(double a, double b, float t)
{
    return a + t * (b - a);
}

inline double fmax(double a, double b) {
    return (a > b) ? a : b;
}

inline double fmin(double a, double b) {
    return (a < b) ? a : b;
}

inline bool isEqual(double a, double b, double tol = GLH_EPSILON)
{
    return fabs(a - b) < tol;
}

#ifndef M_PI
#define M_PI 3.14159f
#endif

#include <assert.h>

class vec3f {
public:
    union {
        struct {
            double x, y, z;
        };
        struct {
            double v[3];
        };
    };

    inline vec3f()
    {
        x = 0; y = 0; z = 0;
    }

    inline vec3f(const vec3f &v)
    {
        x = v.x;
        y = v.y;
        z = v.z;
    }

    inline vec3f(const double *v)
    {
        x = v[0];
        y = v[1];
        z = v[2];
    }

    inline vec3f(double x, double y, double z)
    {
        this->x = x;
        this->y = y;
        this->z = z;
    }

    inline double operator [] (int i) const { return v[i]; }
    inline double &operator [] (int i) { return v[i]; }

    inline vec3f &operator += (const vec3f &v) {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }

    inline vec3f &operator -= (const vec3f &v) {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
    }

    inline vec3f &operator *= (double t) {
        x *= t;
        y *= t;
        z *= t;
        return *this;
    }

    inline vec3f &operator /= (double t) {
        x /= t;
        y /= t;
        z /= t;
        return *this;
    }

    inline void negate() {
        x = -x;
        y = -y;
        z = -z;
    }

    inline vec3f operator - () const {
        return vec3f(-x, -y, -z);
    }

    inline vec3f operator+ (const vec3f &v) const
    {
        return vec3f(x + v.x, y + v.y, z + v.z);
    }

    inline vec3f operator- (const vec3f &v) const
    {
        return vec3f(x - v.x, y - v.y, z - v.z);
    }

    inline vec3f operator *(double t) const
    {
        return vec3f(x*t, y*t, z*t);
    }

    inline vec3f operator /(double t) const
    {
        return vec3f(x / t, y / t, z / t);
    }

    // cross product
    inline const vec3f cross(const vec3f &vec) const
    {
        return vec3f(y*vec.z - z * vec.y, z*vec.x - x * vec.z, x*vec.y - y * vec.x);
    }

    inline double dot(const vec3f &vec) const {
        return x * vec.x + y * vec.y + z * vec.z;
    }

    inline void normalize()
    {
        double sum = x * x + y * y + z * z;
        if (sum > GLH_EPSILON_2) {
            double base = double(1.0 / sqrt(sum));
            x *= base;
            y *= base;
            z *= base;
        }
    }

    inline double length() const {
        return double(sqrt(x*x + y * y + z * z));
    }

    inline vec3f getUnit() const {
        return (*this) / length();
    }

    inline bool isUnit() const {
        return isEqual(squareLength(), 1.f);
    }

    //! max(|x|,|y|,|z|)
    inline double infinityNorm() const
    {
        return fmax(fmax(fabs(x), fabs(y)), fabs(z));
    }

    inline vec3f & set_value(const double &vx, const double &vy, const double &vz)
    {
        x = vx; y = vy; z = vz; return *this;
    }

    inline bool equal_abs(const vec3f &other) {
        return x == other.x && y == other.y && z == other.z;
    }

    inline double squareLength() const {
        return x * x + y * y + z * z;
    }

    static vec3f zero() {
        return vec3f(0.f, 0.f, 0.f);
    }

    //! Named constructor: retrieve vector for nth axis
    static vec3f axis(int n) {
        assert(n < 3);
        switch (n) {
        case 0: {
            return xAxis();
        }
        case 1: {
            return yAxis();
        }
        case 2: {
            return zAxis();
        }
        }
        return vec3f();
    }

    //! Named constructor: retrieve vector for x axis
    static vec3f xAxis() { return vec3f(1.f, 0.f, 0.f); }
    //! Named constructor: retrieve vector for y axis
    static vec3f yAxis() { return vec3f(0.f, 1.f, 0.f); }
    //! Named constructor: retrieve vector for z axis
    static vec3f zAxis() { return vec3f(0.f, 0.f, 1.f); }

};

inline vec3f operator * (double t, const vec3f &v) {
    return vec3f(v.x*t, v.y*t, v.z*t);
}

inline vec3f interp(const vec3f &a, const vec3f &b, double t)
{
    return a * (1 - t) + b * t;
}

inline vec3f vinterp(const vec3f &a, const vec3f &b, double t)
{
    return a * t + b * (1 - t);
}

inline vec3f interp(const vec3f &a, const vec3f &b, const vec3f &c, double u, double v, double w)
{
    return a * u + b * v + c * w;
}

inline double clamp(double f, double a, double b)
{
    return fmax(a, fmin(f, b));
}

inline double vdistance(const vec3f &a, const vec3f &b)
{
    return (a - b).length();
}


inline std::ostream& operator<<(std::ostream&os, const vec3f &v) {
    os << "(" << v.x << ", " << v.y << ", " << v.z << ")" << std::endl;
    return os;
}

inline void
vmin(vec3f &a, const vec3f &b)
{
    a.set_value(
        fmin(a[0], b[0]),
        fmin(a[1], b[1]),
        fmin(a[2], b[2]));
}

inline void
vmax(vec3f &a, const vec3f &b)
{
    a.set_value(
        fmax(a[0], b[0]),
        fmax(a[1], b[1]),
        fmax(a[2], b[2]));
}

inline vec3f lerp(const vec3f &a, const vec3f &b, float t)
{
    return a + t * (b - a);
}


inline double fmax(double a, double b, double c)
{
    double t = a;
    if (b > t) t = b;
    if (c > t) t = c;
    return t;
}

inline double fmin(double a, double b, double c)
{
    double t = a;
    if (b < t) t = b;
    if (c < t) t = c;
    return t;
}

class vec2f {
public:
    union {
        struct {
            double u, v;
        };
        struct {
            double w[2];
        };
    };
    
    inline vec2f()
    {
        u = 0; v = 0;
    }
    
    inline vec2f(const vec2f &w)
    {
        u = w.u;
        v = w.v;
    }
    
    inline vec2f(double u, double v)
    {
        this->u = u;
        this->v = v;
    }
    
    inline double operator [] (int i) const { return w[i]; }
    inline double &operator [] (int i) { return w[i]; }
    
    inline vec2f &operator += (const vec2f &w) {
        u += w.u;
        v += w.v;
        return *this;
    }
    
    inline vec2f &operator -= (const vec2f &w) {
        u -= w.u;
        v -= w.v;
        return *this;
    }
    
    inline vec2f &operator *= (double t) {
        u *= t;
        v *= t;
        return *this;
    }
    
    inline vec2f &operator /= (double t) {
        u /= t;
        v /= t;
        return *this;
    }
    
    inline void negate() {
        u = -u;
        v = -v;
    }
    
    inline vec2f operator - () const {
        return vec2f(-u, -v);
    }
    
    inline vec2f operator+ (const vec2f &w) const
    {
        return vec2f(u + w.u, v + w.v);
    }
    
    inline vec2f operator- (const vec2f &w) const
    {
        return vec2f(u - w.u, v - w.v);
    }
    
    inline vec2f operator *(double t) const
    {
        return vec2f(u*t, v*t);
    }
    
    inline vec2f operator /(double t) const
    {
        return vec2f(u / t, v / t);
    }
    
    // cross product
    inline double cross(const vec2f &vec) const
    {
        return u*vec.v -v*vec.u;
    }
    
    inline double dot(const vec2f &vec) const {
        return u * vec.u + v * vec.v;
    }
    
    inline void normalize()
    {
        double sum = u * u + v * v;
        if (sum > GLH_EPSILON_2) {
            double base = double(1.0 / sqrt(sum));
            u *= base;
            v *= base;
        }
    }
    
    inline double length() const {
        return double(sqrt(u*u + v*v));
    }
    
    inline vec2f getUnit() const {
        return (*this) / length();
    }
    
    inline bool isUnit() const {
        return isEqual(squareLength(), 1.f);
    }
    
    inline vec2f & set_value(const double &wu, const double &wv)
    {
        u = wu; v = wv; return *this;
    }
    
    inline double squareLength() const {
        return u * u + v * v;
    }
};

#endif /* tri_vec_h */
