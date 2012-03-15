#include <math.h>
#include <SFML/Window.hpp>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h> 
#endif

#include "TuringCommon/LibDefs.h"
#include "TuringCommon/RuntimeError.h"
#include "WindowManager.h"
#include "RGB.h"

#pragma mark Tesselation Callbacks
static void tcbBegin (GLenum prim) { 
    glBegin (prim); 
} 
static void tcbVertex (void *data) {
    GLdouble *castedData = (GLdouble *)data;
    GLdouble x = castedData[0];
    GLdouble y = castedData[1];
    glVertex2d (x,y); 
} 
static void tcbEnd () { 
    glEnd (); 
} 
static void tcbCombine (GLdouble c[3], void *d[4], GLfloat w[4], void **out) { 
    GLdouble *nv = (GLdouble *) malloc(sizeof(GLdouble)*3);
    nv[0] = c[0];
    nv[1] = c[1];
    nv[2] = c[2];
    *out = nv;  
}

#pragma mark Helpers

//! creates vertices for ovals and arcs. Should be wrapped in a glBegin if the desired type
static void MyMakeOvalVertices(TInt x, TInt y, TInt rx, TInt ry, TInt startAngle, TInt endAngle) {
    if (startAngle < 0 || startAngle > 360)
        TuringCommon::runtimeError("Start angle of arc out of range.");
    if (endAngle < 0 || endAngle > 360)
        TuringCommon::runtimeError("End angle of arc out of range.");
    
    for (int i=startAngle; i < endAngle; i++)
    {
        glVertex2f(x+(float)cos(i*M_PI/180.0)*rx,
                   y+(float)sin(i*M_PI/180.0)*ry);
    }
}

static GLUtesselator *MyNewTesselator() {
    GLUtesselator *tess = gluNewTess();
    gluTessProperty(tess, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_ODD);
    gluTessCallback (tess, GLU_TESS_BEGIN, (void (*)())tcbBegin); 
    gluTessCallback (tess, GLU_TESS_VERTEX, (void (*)())tcbVertex); 
    gluTessCallback (tess, GLU_TESS_END, (void (*)())tcbEnd);
    gluTessCallback (tess, GLU_TESS_COMBINE, (void (*)())tcbCombine);
    return tess;
}

#pragma mark External Functions

extern "C" {
    //! different from other Cls calls because it uses pallete colour 0
    void Turing_StdlibSFML_Draw_Cls() {
        WinMan->curWin()->Win.Clear(sf::Color(TuringPalette[0][0],TuringPalette[0][1],TuringPalette[0][2]));
    }
    void Turing_StdlibSFML_Draw_Dot(TInt x, TInt y, TInt colour) {
        glColor3ubv((GLubyte *)getRGBColourFromNum(colour));
        
        glBegin(GL_POINTS);
        glVertex2i(x, y);
        glEnd();
        
        WinMan->updateCurWin();
	}
    void Turing_StdlibSFML_Draw_Line(TInt x1, TInt y1, TInt x2, TInt y2, TInt colour) {
        glColor3ubv((GLubyte *)getRGBColourFromNum(colour));
        
        glBegin(GL_LINES);
        glVertex2i(x1, y1);
        glVertex2i(x2, y2);
        glEnd();
        
        WinMan->updateCurWin();
	}
    void Turing_StdlibSFML_Draw_ThickLine(TInt x1, TInt y1, TInt x2, TInt y2, TInt width, TInt colour) {
        glColor3ubv((GLubyte *)getRGBColourFromNum(colour));
        
        glLineWidth(width);
        
        glBegin(GL_LINES);
        glVertex2i(x1, y1);
        glVertex2i(x2, y2);
        glEnd();
        
        glLineWidth(1);
        
        WinMan->updateCurWin();
	}
    //! uses GLUT tesselation to draw any concave or convex polygon using triangles
    void Turing_StdlibSFML_Draw_FillPolygon(TIntArray *x, TIntArray *y, TInt n, TInt colour) {
        glColor3ubv((GLubyte *)getRGBColourFromNum(colour));
        
        if (x->length < n || y->length < n) {
            TuringCommon::runtimeError("Not enough points in polygon array.");
        }
        
        std::vector<GLdouble*> points;
        for (int i = 0; i < n; ++i) {
            GLdouble *coords = new GLdouble[3];
            coords[0] = x->data[i];
            coords[1] = y->data[i];
            coords[2] = 0; // no Z coordinate
            points.push_back(coords);
        } 
        
        
        GLUtesselator *tess = MyNewTesselator();
        gluTessBeginPolygon(tess,NULL);
        gluTessBeginContour(tess);        
            for (unsigned int i = 0; i < points.size(); ++i) {
                gluTessVertex(tess, points[i], points[i]);
            }        
        gluTessEndContour(tess);
        gluTessEndPolygon(tess);
        gluDeleteTess(tess);
        
        // free all the points
        for (unsigned int i = 0; i < points.size(); ++i) {
            delete points[i];
        }   
        
        WinMan->updateCurWin();
	}
    void Turing_StdlibSFML_Draw_Polygon(TIntArray *x, TIntArray *y, TInt n, TInt colour) {
        glColor3ubv((GLubyte *)getRGBColourFromNum(colour));
        
        if (x->length < n || y->length < n) {
            TuringCommon::runtimeError("Not enough points in polygon array.");
        }
        
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < n; ++i) {
            glVertex2i(x->data[i], y->data[i]);
        }
        glEnd();
        
        WinMan->updateCurWin();
	}  
    void Turing_StdlibSFML_Draw_Box(TInt x1, TInt y1, TInt x2, TInt y2, TInt colour) {
        glColor3ubv((GLubyte *)getRGBColourFromNum(colour));
        
        glBegin(GL_LINE_LOOP);
        glVertex2i(x1, y1); glVertex2i(x2, y1); 
        glVertex2i(x2, y2); glVertex2i(x1, y2);
        glEnd();
        
        WinMan->updateCurWin();
	}    
    void Turing_StdlibSFML_Draw_FillBox(TInt x1, TInt y1, TInt x2, TInt y2, TInt colour) {
        glColor3ubv((GLubyte *)getRGBColourFromNum(colour));
        
        glBegin(GL_TRIANGLE_STRIP);
        glVertex2i(x1, y1); glVertex2i(x1, y2); 
        glVertex2i(x2, y1); glVertex2i(x2, y2);
        glEnd();
        
        WinMan->updateCurWin();
	}
    void Turing_StdlibSFML_Draw_Oval(TInt x, TInt y, TInt rx, TInt ry, TInt colour) {
        glColor3ubv((GLubyte *)getRGBColourFromNum(colour));
        
        glBegin(GL_LINE_LOOP);
        
        MyMakeOvalVertices(x, y, rx, ry, 0, 360);
        
        glEnd();
        
        WinMan->updateCurWin();
	}
    void Turing_StdlibSFML_Draw_FillOval(TInt x, TInt y, TInt rx, TInt ry, TInt colour) {
        glColor3ubv((GLubyte *)getRGBColourFromNum(colour));
        
        glBegin(GL_TRIANGLE_FAN);
        MyMakeOvalVertices(x, y, rx, ry, 0, 360);
        glEnd();
        
        WinMan->updateCurWin();
	} 
    void Turing_StdlibSFML_Draw_Arc(TInt x, TInt y, TInt rx, TInt ry, 
                                    TInt startAngle,TInt endAngle, TInt colour) {
        glColor3ubv((GLubyte *)getRGBColourFromNum(colour));
        
        glBegin(GL_LINES);
        
        MyMakeOvalVertices(x, y, rx, ry, startAngle, endAngle);
        
        glEnd();
        
        WinMan->updateCurWin();
	}
    void Turing_StdlibSFML_Draw_FillArc(TInt x, TInt y, TInt rx, TInt ry, 
                                    TInt startAngle,TInt endAngle, TInt colour) {
        glColor3ubv((GLubyte *)getRGBColourFromNum(colour));        
        glBegin(GL_TRIANGLE_FAN);        
        MyMakeOvalVertices(x, y, rx, ry, startAngle, endAngle);        
        glEnd();        
        WinMan->updateCurWin();
	}
}