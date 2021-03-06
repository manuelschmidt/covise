/* This file is part of COVISE.

   You can use it under the terms of the GNU Lesser General Public License
   version 2.1 or later, see lgpl-2.1.txt.

 * License: LGPL 2+ */

/**************************************************************************
** ODD: OpenDRIVE Designer
**   Frank Naegele (c) 2010
**   <mail@f-naegele.de>
**   22.03.2010
**
**************************************************************************/

#include "objectobject.hpp"

#include "src/data/roadsystem/rsystemelementroad.hpp"

//####################//
// Constructors       //
//####################//

Object::Object(const QString &id, const QString &name, const QString &type, double s, double t, double zOffset,
               double validLength, ObjectOrientation orientation, double length, double width, double radius, double height,
               double hdg, double pitch, double roll, bool pole, double repeatS, double repeatLength, double repeatDistance,
               const QString &textureFile)
    : RoadSection(s)
    , id_(id)
    , name_(name)
{
    objectProps_.type = type;
    objectProps_.t = t;
    objectProps_.zOffset = zOffset;
    objectProps_.validLength = validLength;
    objectProps_.orientation = orientation;
    objectProps_.length = length;
    objectProps_.width = width;
    objectProps_.radius = radius;
    objectProps_.height = height;
    objectProps_.hdg = hdg;
    objectProps_.pitch = pitch;
    objectProps_.roll = roll;
    objectProps_.pole = pole;

    objectRepeat_.s = repeatS;
    objectRepeat_.length = repeatLength;
    objectRepeat_.distance = repeatDistance;

    userData_.modelFile = name;
    userData_.textureFile = textureFile;
}

/*!
* Returns the end coordinate of this section.
* In road coordinates [m].
*
*/
double
Object::getSEnd() const
{
    if (objectProps_.validLength < objectProps_.length)
    {
        return getSStart() + objectProps_.validLength;
    }
    else
    {
        return objectRepeat_.s + objectRepeat_.length;
    }
}


//##################//
// Observer Pattern //
//##################//

/*! \brief Called after all Observers have been notified.
*
* Resets the change flags to 0x0.
*/
void
Object::notificationDone()
{
    objectChanges_ = 0x0;
    RoadSection::notificationDone();
}

/*! \brief Add one or more change flags.
*
*/
void
Object::addObjectChanges(int changes)
{
    if (changes)
    {
        objectChanges_ |= changes;
        notifyObservers();
    }
}

//###################//
// Prototype Pattern //
//###################//

/*! \brief Creates and returns a deep copy clone of this object.
*
*/
Object *
Object::getClone()
{
    // Object //
    //
    Object *clone = new Object(id_, name_, objectProps_.type, getSStart(), objectProps_.t, objectProps_.zOffset, objectProps_.validLength, objectProps_.orientation,
                               objectProps_.length, objectProps_.width, objectProps_.radius, objectProps_.height, objectProps_.hdg, objectProps_.pitch, objectProps_.roll, objectProps_.pole,
                               objectRepeat_.s, objectRepeat_.length, objectRepeat_.distance, userData_.textureFile);

    return clone;
}

//###################//
// Visitor Pattern   //
//###################//

/*!
* Accepts a visitor for this section.
*
* \param visitor The visitor that will be visited.
*/
void
Object::accept(Visitor *visitor)
{
    visitor->visit(this);
}
