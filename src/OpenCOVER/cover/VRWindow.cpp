/* This file is part of COVISE.

   You can use it under the terms of the GNU Lesser General Public License
   version 2.1 or later, see lgpl-2.1.txt.

 * License: LGPL 2+ */


/************************************************************************
 *									*
 *          								*
 *                            (C) 1996					*
 *              Computer Centre University of Stuttgart			*
 *                         Allmandring 30				*
 *                       D-70550 Stuttgart				*
 *                            Germany					*
 *									*
 *									*
 *	File			VRWindow.C 				*
 *									*
 *	Description		window class for opening up to 		*
 *				4 stereo windows			*
 *									*
 *	Author			Uwe Woessner				*
 *									*
 *	Date			20.07.2004				*
 *									*
 ************************************************************************/

#include <util/common.h>
#include "VRWindow.h"

#include <config/CoviseConfig.h>
#include "coVRConfig.h"
#include "OpenCOVER.h"

#include "coVRPluginSupport.h"
#include <osgViewer/GraphicsWindow>
#if defined(WIN32)
#include <osgViewer/api/Win32/GraphicsWindowWin32>
#elif defined(__APPLE__) && !defined(USE_X11)
#include <osgViewer/api/Cocoa/GraphicsWindowCocoa>
#else
#include <osgViewer/api/X11/GraphicsWindowX11>
#endif

using namespace vrui;
using namespace opencover;

VRWindow *VRWindow::instance()
{
    static VRWindow *singleton = NULL;
    if (!singleton)
        singleton = new VRWindow;
    return singleton;
}

VRWindow::VRWindow()
    : origVSize(NULL)
    , origHSize(NULL)
    , _firstTimeEmbedded(false)
    , _eventReceiver(NULL)
{
    if (cover->debugLevel(2))
    {
        fprintf(stderr, "\nnew VRWindow\n");
    }
    origVSize = NULL;
    origHSize = NULL;
}

VRWindow::~VRWindow()
{
    if (cover->debugLevel(2))
    {
        fprintf(stderr, "\ndelete VRWindow\n");
    }
    delete[] origVSize;
    delete[] origHSize;
}

bool
VRWindow::config()
{
    if (cover->debugLevel(3))
    {
        fprintf(stderr, "\nVRWindow::config: %d windows\n", coVRConfig::instance()->numWindows());
    }

    origVSize = new int[coVRConfig::instance()->numWindows()];
    origHSize = new int[coVRConfig::instance()->numWindows()];
    for (int i = 0; i < coVRConfig::instance()->numWindows(); i++)
    {
        origVSize[i] = -1;
        origHSize[i] = -1;
        if (!createWin(i))
            return false;
        //XXX ia->addRenderSurface(cover->windows[i].rs);
    }

    return true;
}

void
VRWindow::update()
{
    if (coVRConfig::instance()->numWindows() <= 0 || coVRConfig::instance()->numScreens() <= 0)
        return;
    // resize windows, ignore in multiple windows mode
    if (coVRConfig::instance()->numWindows() > 1)
        return;

    static int oldWidth = -1, oldHeight = -1;
    static int origWidth = -1, origHeight = -1;
    //static int oldX=-1;
    static float Aspect = 1;
    int currentW, currentH;
    const osg::GraphicsContext::Traits *traits = NULL;
    if (coVRConfig::instance()->windows[0].context)
        traits = coVRConfig::instance()->windows[0].context->getTraits();
    if (!traits)
        return;
    /*
   int currentX = traits->x;
   if(currentX!=oldX)
   {
       cerr<< "X pos: " << currentX << endl;
   }
   */
    float initialWidth = coVRConfig::instance()->screens[0].hsize;
    float initialHeight = coVRConfig::instance()->screens[0].vsize;

    currentW = traits->width;
    currentH = traits->height;

    if (oldWidth == -1)
    {
        oldWidth = currentW;
        oldHeight = currentH;
        origWidth = currentW;
        origHeight = currentH;
        if (coVRConfig::instance()->screens[0].hsize <= 0)
        {
            Aspect = 1;
        }
        else
        {
            Aspect = (initialHeight / initialWidth);
        }
    }
    if (oldWidth != currentW || oldHeight != currentH || _firstTimeEmbedded)
    {
        if ((OpenCOVER::instance()->parentWindow) && (coVRConfig::instance()->windows[0].embedded))
        {
            float width, height;
            width = (float)origHSize[0];
            height = (float)origVSize[0];
			if(width >0 && height > 0)
			{
                float vsize = coVRConfig::instance()->screens[0].hsize * (height / width);
                coVRConfig::instance()->screens[0].vsize = vsize;
			}
            _firstTimeEmbedded = false;
        }
        else if (origWidth != 0)
            coVRConfig::instance()->screens[0].hsize = (((initialHeight / (Aspect)) / origWidth) * currentW) * origHeight / currentH;
        // wir aendern hier die Breite um das Seitenverhaeltnis konstant zu halten.
        coVRConfig::instance()->windows[0].sx = currentW;
        coVRConfig::instance()->windows[0].sy = currentH;
        oldWidth = currentW;
        oldHeight = currentH;
    }
}

/*************************************************************************/
bool
VRWindow::createWin(int i)
{
    if (cover->debugLevel(4))
    {
        fprintf(stderr, "VRWindow::createWin %d\n", i);
    }
    /*   int multisample_feature, stereo_feature;

      /// colle
      static int  frameBufAttr[64];
      int pos = 0;

      /// open a pfPipeWindow*/

    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
    traits->x = coVRConfig::instance()->windows[i].ox;
    traits->y = coVRConfig::instance()->windows[i].oy;
    traits->width = coVRConfig::instance()->windows[i].sx;
    traits->height = coVRConfig::instance()->windows[i].sy;
    traits->windowDecoration = coVRConfig::instance()->windows[i].decoration;
    if(traits->windowDecoration == false)
    {
        traits->overrideRedirect = true; 
    }
    traits->supportsResize = coVRConfig::instance()->windows[i].resize;
    traits->pbuffer = coVRConfig::instance()->windows[i].pbuffer;
    bool opengl3 = false;
    if(!coVRConfig::instance()->glVersion.empty())
    {
        traits->glContextVersion = coVRConfig::instance()->glVersion;
        const double ver = atof(coVRConfig::instance()->glVersion.c_str());
        if (ver >= 3.0)
            opengl3 = true;
        opengl3 = covise::coCoviseConfig::isOn("COVER.WindowConfig.OpenGL3", opengl3);
        if (cover->debugLevel(1))
        {
            std::cerr << "creating window " << i << " with GL context version " << coVRConfig::instance()->glVersion << ", OpenGL3=" << opengl3 << std::endl;
        }
    }
    traits->windowName = "OpenCOVER";

    if ((OpenCOVER::instance()->parentWindow) && (coVRConfig::instance()->windows[i].embedded))
    {
        traits->pbuffer = false;
        _eventReceiver = new EventReceiver(7878, 0);
        _firstTimeEmbedded = true;
#if defined(WIN32)
        traits->inheritedWindowData = new osgViewer::GraphicsWindowWin32::WindowData(OpenCOVER::instance()->parentWindow);
#elif defined(__APPLE__) && !defined(USE_X11)
        traits->inheritedWindowData = new osgViewer::GraphicsWindowCocoa::WindowData(OpenCOVER::instance()->parentWindow);
#else
        traits->inheritedWindowData = new osgViewer::GraphicsWindowX11::WindowData(OpenCOVER::instance()->parentWindow);
#endif
    }

    if (traits->inheritedWindowData != NULL)
        traits->windowDecoration = false;
    
    if(traits->windowDecoration == false)
    {
        traits->overrideRedirect = true; 
    }

    const int pipeNum = coVRConfig::instance()->windows[i].pipeNum;
    if (coVRConfig::instance()->useDisplayVariable() || coVRConfig::instance()->pipes[pipeNum].useDISPLAY)
    {
        char *display = NULL;
        
        traits->displayNum = 0;
        traits->screenNum = 0;
        char *disp = getenv("DISPLAY");
        if(disp)
        {
            display=new char[strlen(disp)+1];
            strcpy(display,disp);
            const char *host = display;
            char *p = strchr(display, ':');
            if (p)
            {
                *p = '\0';
                ++p;
                char *server = p;
                const char *screen = NULL;
                p = strchr(server, '.');
                if (p)
                {
                    *p = '\0';
                    ++p;
                    screen = p;
                }
                if (host && strlen(host) > 0)
                    traits->hostName = host;
                traits->displayNum = server ? atoi(server) : 0;
                traits->screenNum = screen ? atoi(screen) : 0;
            }
            delete[] display;
        }
    }
    else
    {
        const std::string &host = coVRConfig::instance()->pipes[pipeNum].x11DisplayHost;
        if (!host.empty())
            traits->hostName = host;
        traits->displayNum = coVRConfig::instance()->pipes[pipeNum].x11DisplayNum;
        traits->screenNum = coVRConfig::instance()->pipes[pipeNum].x11ScreenNum;

        // if possible, share graphics context with other windows
        for (int j=0; j<i; ++j)
        {
            const windowStruct &wj = coVRConfig::instance()->windows[j];
            const windowStruct &wi = coVRConfig::instance()->windows[i];
            if (wj.pipeNum == wi.pipeNum)
            {
                traits->sharedContext = wj.context;
                break;
            }
        }
    }
    //traits->alpha = 8;
    if (coVRConfig::instance()->m_stencil == true)
    {
        traits->stencil = coVRConfig::instance()->numStencilBits();
    }
    traits->doubleBuffer = true;
    traits->quadBufferStereo = coVRConfig::instance()->windows[i].stereo;

    if (coVRConfig::instance()->doMultisample())
    {
        traits->samples = coVRConfig::instance()->getMultisampleSamples();
        traits->sampleBuffers = coVRConfig::instance()->getMultisampleSampleBuffers();
    }

    coVRConfig::instance()->windows[i].context = osg::GraphicsContext::createGraphicsContext(traits.get());
    if (coVRConfig::instance()->windows[i].context == NULL)
    {
        cerr << "No valid Pixel Format found, trying without multisample Buffer" << endl;
        traits->sampleBuffers = 0;
        coVRConfig::instance()->windows[i].context = osg::GraphicsContext::createGraphicsContext(traits.get());
        if (coVRConfig::instance()->windows[i].context == NULL)
        {
            cerr << "No valid Pixel Format found, trying without stencil" << endl;
            traits->stencil = 0;
            coVRConfig::instance()->windows[i].context = osg::GraphicsContext::createGraphicsContext(traits.get());
            if (coVRConfig::instance()->windows[i].context == NULL)
            {
                cerr << "No valid Pixel Format found, trying without alpha" << endl;
                traits->alpha = 0;
                coVRConfig::instance()->windows[i].context = osg::GraphicsContext::createGraphicsContext(traits.get());
            }
        }
    }
    if (!coVRConfig::instance()->windows[i].context)
    {
        return false;
    }
    if (!coVRConfig::instance()->windows[i].context->valid())
    {
        cerr << "No valid GL context created" << endl;
        return false;
    }
    coVRConfig::instance()->windows[i].window = dynamic_cast<osgViewer::GraphicsWindow *>(coVRConfig::instance()->windows[i].context);

    if (opengl3)
    {
        // for non GL3/GL4 and non GLES2 platforms we need enable the osg_ uniforms that the shaders will use,
        // you don't need thse two lines on GL3/GL4 and GLES2 specific builds as these will be enable by default.
        coVRConfig::instance()->windows[i].context->getState()->setUseModelViewAndProjectionUniforms(true);
        //coVRConfig::instance()->windows[i].context->getState()->setUseVertexAttributeAliasing(true);
    }

#if 0
   // if the config file contains the keywords VisualID
   // we use directly the id to configure the framebuffer
   int visId=coVRConfig::instance()->windows[i].visualId;
   char entry[200];
   sprintf(&entry[0], "COVER.WindowConfig:%d.VisualID", i);
   if (visId == -1)
   {
     visId = covise::coCoviseConfig::getInt(entry, coVRConfig::instance()->windows[i].visualId);
#ifdef __sgi
     visId += 134*coVRConfig::instance()->windows[i].pipeNum;
#endif
   }
   else
     visId = coVRConfig::instance()->windows[i].visualId;

   if(visId != -1)
   {
      Producer::VisualChooser *visualChooser = coVRConfig::instance()->windows[i].rs->getVisualChooser();
      if(!visualChooser)
      {
         visualChooser = new Producer::VisualChooser;
         coVRConfig::instance()->windows[i].rs->setVisualChooser(visualChooser);
      }

      fprintf(stderr,"Configuring render surface %d with visual: dec:%d hex:%x\n",i, visId, visId);
      coVRConfig::instance()->windows[i].rs->getVisualChooser()->setVisualID(visId);
   }
#endif
    if (cover->debugLevel(4))
        fprintf(stderr, "VRWindow::createWin %d --- finished\n", i);
    //sleep(200);
    return true;
}
