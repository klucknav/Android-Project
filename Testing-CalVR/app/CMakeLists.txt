# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -Wall")
set(OSG_CPP_EXCEPTIONS_AVAILABLE "Yes")


SET(KERNEL_LIBRARIES
    cvrKernel                     # For cvr, Kernel > Menu > Util
    cvrMenu                       # The order of Input, Config, Collaborative is unsure (current order works)
    cvrUtil                       #
    cvrInput                      #
    cvrConfig                     #
    cvrCollaborative              #

    mxml                          # mxml must be included before cvr
    log                           #

    osgPresentation               # For osg, the dependency relations:
    osgWidget                     # osgPresentation: https://github.com/WeichenLiu/CalVR_for_Android/blob/master/Note/osgPresentation.png
    osgUI                         # osgdb_osg: https://github.com/WeichenLiu/CalVR_for_Android/blob/master/Note/osgdb_osg.png
    osgManipulator                # osgdb_serializers_osg:
    osgAnimation                  # https://github.com/WeichenLiu/CalVR_for_Android/blob/master/Note/osgdb_serializers_osg.png
    osgViewer                     # (These three are the "roots" of the trees)
    osgVolume                     #
    osgSim                        #
    osgShadow                     #
    osgTerrain                    #
    osgParticle                   #
    osgFX                         #
    z                             #
    osgText                       #
    osgGA                         #
    osgdb_tiff
    #osgdb_rgb
    osgdb_png
    osgdb_jpeg
    osgdb_osg                     #
    osgdb_serializers_osg         #
    osgDB                         #
    osgUtil                       #
    osg                           #
    OpenThreads                   #

    tiff
    png
    jpeg
    #zlib
    EGL                           #
    GLESv2                        #
)


# Path to external projects:
set(EXT_PROJ_DIR "${CMAKE_SOURCE_DIR}/../../..")


# build the dependencies (tiff, png, jpeg)
include(CMakeLists-dependencies.txt)

# build OSG, Mini-XML, and CalVR
#include(CMakeLists-buildOSG.txt)
#include(CMakeLists-buildMXML.txt)
#include(CMakeLists-buildCalVR.txt)

# static build OSG, Mini-XML, and CalVR
include(CMakeLists-staticOSG.txt)
include(CMakeLists-staticMXML.txt)
include(CMakeLists-staticCalVR.txt)

# build the plugin(s)
include(CMakeLists-buildCalVR_plugin.txt)



## import the arcore library ##
add_library(arcore SHARED IMPORTED)
set_target_properties(arcore PROPERTIES IMPORTED_LOCATION ${ARCORE_LIBPATH}/${ANDROID_ABI}/libarcore_sdk_c.so)


# AR-OSG activity
include(CMakeLists-arBasic.txt)

# CalVR activity
include(CMakeLists-simpleCalVR.txt)