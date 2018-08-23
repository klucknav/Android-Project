#ifndef TESTING_GLDRAWABLE_H
#define TESTING_GLDRAWABLE_H

#include <osg/Drawable>
#include <osg/Geode>
#include <stack>
#include <android/asset_manager.h>
#include "util.h"

class glDrawable: public osg::Drawable {
public:
    osg::Geode* createDrawableNode(){

        osg::Geode* glNode = new osg::Geode;
        glNode->addDrawable(this);
        setUseDisplayList(false);
        return glNode;
    }
};

#endif //TESTING_GLDRAWABLE_H
