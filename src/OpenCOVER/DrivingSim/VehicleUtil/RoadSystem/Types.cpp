/* This file is part of COVISE.

   You can use it under the terms of the GNU Lesser General Public License
   version 2.1 or later, see lgpl-2.1.txt.

 * License: LGPL 2+ */

#include "Types.h"

#include <cmath>
#include <iostream>
#include <sstream>

Polynom::Polynom(double s, double a, double b, double c, double d)
{
    start = s;

    this->a = a, this->b = b, this->c = c, this->d = d;
}

Vector2D Polynom::getPoint(double s)
{
    s -= start;
    return Vector2D(a + b * s + c * s * s + d * s * s * s, -atan(b + 2 * c * s + 3 * d * s * s));
}

double Polynom::getValue(double s)
{
    s -= start;
    return a + b * s + c * s * s + d * s * s * s;
}

double Polynom::getSlope(double s)
{
    s -= start;
    return (b + 2 * c * s + 3 * d * s * s);
}

double Polynom::getSlopeAngle(double s)
{
    s -= start;
    return atan(b + 2 * c * s + 3 * d * s * s);
}

double Polynom::getCurvature(double s)
{
    s -= start;
    return 6 * d * s + 2 * c;
}

void Polynom::getCoefficients(double &a, double &b, double &c, double &d)
{
    a = this->a;
    b = this->b;
    c = this->c;
    d = this->d;
}

void Polynom::accept(XodrWriteRoadSystemVisitor *visitor)
{
    visitor->visit(this);
}

double PlaneCurve::getLength()
{
    return length;
}

PlaneStraightLine::PlaneStraightLine(double s, double l, double xs, double ys, double hdg)
    : A(cos(hdg), -sin(hdg),
        sin(hdg), cos(hdg))
    , cs(xs,
         ys)
{
    start = s;
    length = l;

    //ax = cos(hdg);
    //ay = sin(hdg);
    //bx = xs;
    //by = ys;

    hdgs = hdg;
}

Vector3D PlaneStraightLine::getPoint(double s)
{
    s -= start;
    Vector2D cm(s, 0);
    return Vector3D(A * cm + cs, hdgs);
    //return Vector3D(ax*s+bx, ay*s+by, hdgs);
}

double PlaneStraightLine::getOrientation(double)
{
    return hdgs;
}

double PlaneStraightLine::getCurvature(double)
{
    return 0.0;
}

Vector2D PlaneStraightLine::getTangentVector(double)
{
    return A * Vector2D(1, 0);
}

Vector2D PlaneStraightLine::getNormalVector(double)
{
    //return A*Vector2D(0,1);
    return Vector2D(0, 0);
}

void PlaneStraightLine::accept(XodrWriteRoadSystemVisitor *visitor)
{
    visitor->visit(this);
}

PlaneArc::PlaneArc(double s, double l, double xstart, double ystart, double hdg, double r)
    : A(cos(hdg), sin(hdg),
        sin(hdg), -cos(hdg))
    , cs(xstart - r * sin(hdg),
         ystart + r * cos(hdg))
{
    start = s;
    length = l;

    this->r = r;
    //double sinhdg = sin(hdg);
    //double coshdg = cos(hdg);
    //xs = xstart - r*sinhdg;
    //ys = ystart + r*coshdg;
    hdgs = hdg;
}

Vector3D PlaneArc::getPoint(double s)
{
    s -= start;
    double t = s / r;
    //double sint = sin(t);
    //double cost = cos(t);
    Vector2D cm(r * sin(t),
                r * cos(t));
    return Vector3D(A * cm + cs, s / r + hdgs);
    //return Vector3D(xs+r*(coshdg*sint+sinhdg*cost), ys+r*(sinhdg*sint-coshdg*cost), s/r+hdgs);
}

double PlaneArc::getOrientation(double s)
{
    s -= start;
    return s / r + hdgs;
}

double PlaneArc::getCurvature(double)
{
    return 1 / r;
}

Vector2D PlaneArc::getTangentVector(double s)
{
    s -= start;
    Vector2D dcm(cos(s / r),
                 -sin(s / r));
    return A * dcm;
}

Vector2D PlaneArc::getNormalVector(double s)
{
    s -= start;
    Vector2D dcm(-1 / r * sin(s / r),
                 -1 / r * cos(s / r));
    return A * dcm;
}

void PlaneArc::accept(XodrWriteRoadSystemVisitor *visitor)
{
    visitor->visit(this);
}

PlaneClothoid::PlaneClothoid(double s, double l, double xstart, double ystart, double hdg, double cs, double ce)
    : A(0.0)
    , cs(xstart, ystart)
{
    sqrtpi = sqrt(M_PI);

    start = s;
    length = l;

    double cdelta = ce - cs;
    if (cdelta > 0)
    {
        ax = ay = sqrt(l / cdelta);
        ts = cs * ax / sqrtpi;
    }
    else if (cdelta < 0)
    {
        ax = sqrt(-l / cdelta);
        ay = -ax;
        ts = cs * ay / sqrtpi;
    }
    else
    {
        ax = ay = 0;
        ts = 0;
    }

    //double xsc, ysc;
    //integrateClothoid(0, ts, xsc, ysc);
    //xs = xstart - xsc;
    //ys = ystart - ysc;
    //xs = xstart;
    //ys = ystart;

    double chdg = ay / ax * M_PI * ts * ts * 0.5;
    double beta = hdg - chdg;
    hdgs = beta;
    double sinbeta = sin(beta);
    double cosbeta = cos(beta);
    A(0, 0) = cosbeta;
    A(0, 1) = -sin(beta);
    A(1, 0) = sinbeta;
    A(1, 1) = cos(beta);

    double ts2 = pow(ts, 2);
    tsn[0] = ts;
    for (int i = 1; i < 9; ++i)
    {
        tsn[i] = tsn[i - 1] * ts2;
    }
    fca8 = 39. * pow(M_PI, 8) / (6843432960.);
    fca7 = 11. * pow(M_PI, 7) / (106444800.);
    fca6 = 11424. * pow(M_PI, 6) / (6843432960.);
    fca5 = 2520. * pow(M_PI, 5) / (106444800.);
    fca4 = 1980160. * pow(M_PI, 4) / (6843432960.);
    fca3 = 316800. * pow(M_PI, 3) / (106444800.);
    fca2 = 171085824. * pow(M_PI, 2) / (6843432960.);
    fca1 = 17740800. * M_PI / (106444800.);
    fca0 = 6843432960. / (6843432960.);
}

Vector3D PlaneClothoid::getPoint(double s)
{
    s -= start;
    double t = s / (ax * sqrtpi) + ts;
    double x, y;
    //integrateClothoid(t, x, y);
    approximateClothoid(t, x, y);

    Vector2D cm(x, y);

    //return Vector3D(xs + x*cosbeta-y*sinbeta, ys + x*sinbeta+y*cosbeta, hdgs + ay/ax*0.5*M_PI*t*t);
    return Vector3D(A * cm + cs, hdgs + ay / ax * 0.5 * M_PI * t * t);
}

double PlaneClothoid::getOrientation(double s)
{
    s -= start;
    double t = s / (ax * sqrtpi) + ts;

    //std::cerr << "Clothoid hdgs: " << hdgs << ", hdg: " << -ay/ax*0.5*M_PI*t*t << ", t: " << t << std::endl;
    return hdgs + ay / ax * 0.5 * M_PI * t * t;
}

double PlaneClothoid::getCurvature(double s)
{
    s -= start;
    double t = s / (ax * sqrtpi) + ts;

    return t * sqrtpi / ay;
}

Vector2D PlaneClothoid::getTangentVector(double s)
{
    s -= start;
    double t = s / (ax * sqrtpi) + ts;

    //return A*Vector2D(sin(M_PI*t*t*0.5), ay/ax*cos(M_PI*t*t*0.5));
    return A * Vector2D(cos(M_PI * t * t * 0.5), ay / ax * sin(M_PI * t * t * 0.5));
}

Vector2D PlaneClothoid::getNormalVector(double s)
{
    s -= start;
    double t = s / (ax * sqrtpi) + ts;

    //return A*Vector2D(M_PI*t*cos(M_PI*t*t*0.5), -ay/ax*M_PI*t*sin(M_PI*t*t*0.5));
    return A * Vector2D(-1 / ax * M_PI * t * sin(M_PI * t * t * 0.5), ay / (ax * ax) * M_PI * t * cos(M_PI * t * t * 0.5));
}

void PlaneClothoid::integrateClothoid(double t, double &x, double &y)
{
    x = 0;
    y = 0;

    double h = 0.0001;

    //std::cout << "t-ts: " << t-ts << std::endl;
    double n = ceil((t - ts) / h);

    h = (t - ts) / n;

    if (h == h)
    {
        for (int i = 1; i < (n - 1); ++i)
        {
            x += cos(M_PI * 0.5 * pow(ts + i * h, 2));
            y += sin(M_PI * 0.5 * pow(ts + i * h, 2));
        }
        x += (cos(M_PI * ts) + cos(M_PI * t)) * 0.5;
        y += (sin(M_PI * ts) + sin(M_PI * t)) * 0.5;
        x *= ax * sqrtpi * h;
        y *= ay * sqrtpi * h;
    }
}

void PlaneClothoid::approximateClothoid(double t, double &x, double &y)
{
    double t2 = pow(t, 2);
    double tn[9];
    tn[0] = t;
    for (int i = 1; i < 9; ++i)
    {
        tn[i] = tn[i - 1] * t2;
    }

    x = ax * sqrtpi * (fca8 * (tn[8] - tsn[8]) - fca6 * (tn[6] - tsn[6]) + fca4 * (tn[4] - tsn[4]) - fca2 * (tn[2] - tsn[2]) + fca0 * (tn[0] - tsn[0]));
    y = ay * sqrtpi * (fca7 * (tn[7] - tsn[7]) + fca5 * (tn[5] - tsn[5]) - fca3 * (tn[3] - tsn[3]) + fca1 * (tn[1] - tsn[1]));
}

void PlaneClothoid::accept(XodrWriteRoadSystemVisitor *visitor)
{
    visitor->visit(this);
}

PlanePolynom::PlanePolynom(double s, double l, double xstart, double ystart, double hdg, double a, double b, double c, double d)
    : A(cos(hdg), -sin(hdg),
        sin(hdg), cos(hdg))
    , cs(xstart,
         ystart)
{
    start = s;
    length = l;

    hdgs = hdg;
    //sinhdg = sin(hdg);
    //coshdg = cos(hdg);
    //xs = xstart;
    //ys = ystart;

    this->a = a, this->b = b, this->c = c, this->d = d;
}

Vector3D PlanePolynom::getPoint(double s)
{
    s -= start;
    double t = getT(s);

    double u = t;
    double v = (a + b * t + c * t * t + d * t * t * t);

    Vector2D cm(u, v);
    //return Vector3D( u*coshdg-v*sinhdg+xs, u*sinhdg+v*coshdg+ys, atan(3*d*t*t+2*c*t+b)+hdgs);
    return Vector3D(A * cm + cs, atan(3 * d * t * t + 2 * c * t + b) + hdgs);
}

double PlanePolynom::getOrientation(double s)
{
    s -= start;
    double t = getT(s);
    return atan(3 * d * t * t + 2 * c * t + b) + hdgs;
}

double PlanePolynom::getCurvature(double s)
{
    s -= start;
    double t = getT(s);
    double dv = b + 2 * c * t + 3 * d * t * t;
    double ddv = 2 * c + 6 * d * t;
    return ddv / pow((1 + dv * dv), 1.5);
}

Vector2D PlanePolynom::getTangentVector(double s)
{
    s -= start;
    double t = getT(s);
    return A * Vector2D(1, 3 * d * t * t + 2 * c * t + b);
}

Vector2D PlanePolynom::getNormalVector(double s)
{
    s -= start;
    double t = getT(s);
    return A * Vector2D(-(3 * d * t * t + 2 * c * t + b), 1);
}

void PlanePolynom::getCoefficients(double &a, double &b, double &c, double &d)
{
    a = this->a;
    b = this->b;
    c = this->c;
    d = this->d;
}

double PlanePolynom::getT(double s)
{
    /*	int n=100;
   double h=s/n;
   double t=0;
   double dt=0;
   for(int i=0; i<n; ++i) {
      dt = 1/sqrt(pow((3*d*t*t+2*c*t+b),2)+1)*h;
      t += dt;
   }
	return t; */

    //Cut taylor series approximation (1-degree) of arc length integral, solved with Newton-Raphson for t with respect to s
    double t = s;
    for (int i = 0; i < 5; ++i)
    {
        //       double f = t*sqrt(pow(((3*d*pow(t,2))/4+c*t+b),2)+1)-s;
        //       double df = sqrt(pow(((3*d*pow(t,2))/4+c*t+b),2)+1)+(t*((3*d*t)/2+c)*((3*d*pow(t,2))/4+c*t+b))/sqrt(pow(((3*d*pow(t,2))/4+c*t+b),2)+1);

        // New code with integration //
        //
        double f = getCurveLength(0.0, t) - s;
        double df = sqrt(1.0 + (b + 2.0 * c * t + 3.0 * d * t * t) * (b + 2.0 * c * t + 3.0 * d * t * t));

        t -= f / df;
    }
    return t;
}

/*! Calculates the length [m] of a curve segment.
*
* Uses a Gauss-Legendre integration (n=5). Should be good enough.
*/
double PlanePolynom::getCurveLength(double from, double to)
{
    double factor = (to - from) / 2.0; // new length = 2.0
    double deltaX = (to + from) / 2.0; // center
    return (0.236926885056189 * g(-0.906179845938664, factor, deltaX)
            + 0.478628670499366 * g(-0.538469310105683, factor, deltaX)
            + 0.568888888888889 * g(0.0, factor, deltaX)
            + 0.478628670499366 * g(0.538469310105683, factor, deltaX)
            + 0.236926885056189 * g(0.906179845938664, factor, deltaX)) * factor;
}

/*! Support function for Gauss-Legendre integration.
*
* GLI is in interval [-1.0, 1.0], so a factor and an offset is needed.
* The calculation is based on the curve length integral(sqrt(1+f')).
*/
double PlanePolynom::g(double x, double factor, double delta)
{
    x = x * factor + delta;
    return sqrt(1.0 + (b + 2.0 * c * x + 3.0 * d * x * x) * (b + 2.0 * c * x + 3.0 * d * x * x));
}

void PlanePolynom::accept(XodrWriteRoadSystemVisitor *visitor)
{
    visitor->visit(this);
}

SuperelevationPolynom::SuperelevationPolynom(double s, double a, double b, double c, double d)
{
    start = s;

    this->a = a, this->b = b, this->c = c, this->d = d;
}

double SuperelevationPolynom::getAngle(double s, double)
{
    s -= start;
    return a + b * s + c * s * s + d * s * s * s;
}

void SuperelevationPolynom::accept(XodrWriteRoadSystemVisitor *visitor)
{
    visitor->visit(this);
}

void SuperelevationPolynom::getCoefficients(double &a, double &b, double &c, double &d)
{
    a = this->a;
    b = this->b;
    c = this->c;
    d = this->d;
}

CrossfallPolynom::CrossfallPolynom(double s, double a, double b, double c, double d, double lf, double rf)
{
    start = s;

    this->a = a, this->b = b, this->c = c, this->d = d;
    leftFactor = lf, rightFactor = rf;
}

double CrossfallPolynom::getAngle(double s, double t)
{
    s -= start;
    double absval = a + b * s + c * s * s + d * s * s * s;
    return (t < 0) ? rightFactor * absval : leftFactor * absval;
}

double CrossfallPolynom::getLeftFallFactor()
{
    return leftFactor;
}

double CrossfallPolynom::getRightFallFactor()
{
    return rightFactor;
}

void CrossfallPolynom::getCoefficients(double &a, double &b, double &c, double &d)
{
    a = this->a;
    b = this->b;
    c = this->c;
    d = this->d;
}

void CrossfallPolynom::accept(XodrWriteRoadSystemVisitor *visitor)
{
    visitor->visit(this);
}

std::ostream &operator<<(std::ostream &os, const Vector3D &vec)
{
    os << "( " << vec[0] << " \t" << vec[1] << " \t" << vec[2] << " \t)" << std::endl;
    return os;
}

std::ostream &operator<<(std::ostream &os, const Matrix2D2D &mat)
{
    for (int i = 0; i < 2; ++i)
    {
        os << "( ";
        for (int j = 0; j < 2; ++j)
        {
            os << "\t" << mat(i, j);
        }
        os << "\t )" << std::endl;
    }

    return os;
}

std::ostream &operator<<(std::ostream &os, const Matrix3D3D &mat)
{
    for (int i = 0; i < 3; ++i)
    {
        os << "( ";
        for (int j = 0; j < 3; ++j)
        {
            os << "\t" << mat(i, j);
        }
        os << "\t )" << std::endl;
    }

    return os;
}

std::ostream &operator<<(std::ostream &os, const Quaternion &quat)
{
    os << "Hallllooooooooooooooooo ";
    os << "(" << quat.w() << ", [" << quat.x() << ", " << quat.y() << ", " << quat.z() << "])" << std::endl;

    return os;
}

std::ostream &operator<<(std::ostream &os, const Transform &trans)
{
    os << "Translation: " << trans.v() << "Rotation: " << trans.q();

    return os;
}

void Transform::print() const
{
    std::cout << "Translation: " << _v << "Rotation: "
              << "(" << _q.w() << ", [" << _q.x() << ", " << _q.y() << ", " << _q.z() << "])" << std::endl;
}
