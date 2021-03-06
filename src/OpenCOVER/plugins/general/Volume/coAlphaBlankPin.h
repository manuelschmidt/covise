/* This file is part of COVISE.

   You can use it under the terms of the GNU Lesser General Public License
   version 2.1 or later, see lgpl-2.1.txt.

 * License: LGPL 2+ */

//-*-c++-*-
#ifndef CO_ALPHA_BLANK_PIN_H
#define CO_ALPHA_BLANK_PIN_H
#include "coPin.h"
#include "sys/types.h"

#include <osg/Group>
#include <osg/Geometry>
#include <osg/Array>

class coAlphaBlankPin : public coPin
{
public:
    coAlphaBlankPin(osg::Group *root, float Height, float Width, vvTFSkip *myPin);
    virtual ~coAlphaBlankPin();
    virtual void setPos(float x);
    void setWidth(float m);
    float w1, w2, w3;

protected:
    osg::ref_ptr<osg::Vec4Array> graphColor;
    osg::ref_ptr<osg::Vec3Array> graphCoord;
    osg::ref_ptr<osg::Vec3Array> graphNormal;
    osg::ref_ptr<osg::Geode> graphGeode;
    osg::ref_ptr<osg::Geometry> geometry;

    void adjustGraph();
    void createGraphLists();
    osg::ref_ptr<osg::Geode> createGraphGeode();
};
#endif
