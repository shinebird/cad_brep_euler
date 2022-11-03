// OpenGL Tessellation example: http://www.songho.ca/opengl/gl_tessellation.html

#include <cstddef>
#include <format>
#include <vector>

#ifdef __APPLE__
    #include <GLUT/glut.h>
#else
    #include <GL/freeglut.h>
#endif

#ifndef _WIN32
    #define __stdcall
#endif

#include <b-rep.hpp>
#include <iomanip>
#include <iostream>
#include <render.hpp>
#include <sstream>

using std::cerr;
using std::cout;
using std::endl;
using std::ends;
using std::stringstream;

#ifndef CALLBACK
    #define CALLBACK
#endif

// CALLBACK functions for GLU_TESS ////////////////////////////////////////////
// NOTE: must be declared with CALLBACK directive
void CALLBACK tess_begin_callback(GLenum which);
void CALLBACK tess_end_callback();
void CALLBACK tess_error_callback(GLenum errorCode);
void CALLBACK tess_vertex_callback(const GLvoid* data);
void CALLBACK tess_vertex_callback2(const GLvoid* data);
void CALLBACK tess_combine_callback(const GLdouble newVertex[3],
                                    const GLdouble* neighborVertex[4],
                                    const GLfloat neighborWeight[4],
                                    GLdouble** outData);

// GLUT CALLBACK functions ////////////////////////////////////////////////////
void display_callback();
void reshape_callback(int w, int h);
void timer_callback(int millisec);
void idle_callback();
void keyboard_callback(unsigned char key, int x, int y);
void mouse_callback(int button, int stat, int x, int y);
void mouse_motion_callback(int x, int y);

// function declarations //////////////////////////////////////////////////////
void init_GL();
int init_GLUT(int argc, char** argv);
bool init_shared_mem();
void clear_shared_mem();
void init_lights();
void set_camera(float posX, float posY, float posZ, float targetX,
                float targetY, float targetZ);
void draw_string(const char* str, int x, int y, float color[4], void* font);
void draw_string_3D(const char* str, float pos[3], float color[4], void* font);
void show_info();
const char* getPrimitiveType(GLenum type);
GLuint tessellate1();
GLuint tessellate2();
GLuint tessellate3();

// global variables ///////////////////////////////////////////////////////////
static void* font_ptr = GLUT_BITMAP_8_BY_13;
static bool mouse_left_down;
static bool mouse_right_down;
static float mouse_x, mouse_y;
static float camera_angle_x;
static float camera_angle_y;
static float camera_distance;
static int draw_mode = 0;
static GLuint list_id_1, list_id_2, list_id_3; // IDs of display lists
static GLdouble vertices[64][6]; // arrary to store newly created vertices
                                 // (x,y,z,r,g,b) by combine callback
static int vertexIndex =
    0; // array index for above array incremented inside combine callback

// DEBUG //
stringstream ss;
std::vector<render::point> points;

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
    init_shared_mem();

    // init GLUT and GL
    init_GLUT(argc, argv);
    init_GL();

    // perform tessellation and compile into display lists
    list_id_1 = tessellate1(); // a concave quad
    list_id_2 = tessellate2(); // a quad with a hole in it
    // listId3 = tessellate3(); // a self-intersecting star

    // the last GLUT call (LOOP)
    // window will be shown and display callback is triggered by events
    // NOTE: this call never return main().
    glutMainLoop(); /* Start GLUT event-processing loop */

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// tessellate a polygon with a hole and compile it into a display list
///////////////////////////////////////////////////////////////////////////////
GLuint tessellate1()
{
    GLuint id = glGenLists(1);          // create a display list
    if (!id)
        return id;                      // failed to create a list, return 0

    GLUtesselator* tess = gluNewTess(); // create a tessellator
    if (!tess)
        return 0; // failed to create tessellation object, return 0

    // define concave quad with a hole
    //  0--------3
    //  | 4----7 |
    //  | |    | |
    //  | 5----6 |
    //  1--------2
    // GLdouble quad2[8][3] = {{-2, 3, -1}, {-2, 0, -1}, {2, 0, -1}, {2, 3, -1},
    //                         {-1, 2, -1}, {-1, 1, -1}, {1, 1, -1}, {1, 2,
    //                         -1}};

    // register callback functions
    gluTessCallback(tess, GLU_TESS_BEGIN,
                    (void(__stdcall*)(void))tess_begin_callback);
    gluTessCallback(tess, GLU_TESS_END,
                    (void(__stdcall*)(void))tess_end_callback);
    gluTessCallback(tess, GLU_TESS_ERROR,
                    (void(__stdcall*)(void))tess_error_callback);
    gluTessCallback(tess, GLU_TESS_VERTEX,
                    (void(__stdcall*)())tess_vertex_callback);

    // tessellate and compile a concave quad into display list
    glNewList(id, GL_COMPILE);
    glColor3f(1, 1, 1);
    render::point points[8] = {{-1, 1.5, 0}, {-1, 0, 0}, {1, 0, 0}, {1, 1.5, 0},
                               {-0.5, 1, 0}, {-0.5, 0.5, 0}, {0.5, 0.5, 0}, {0.5, 1, 0}};
    brep::vertex* v = nullptr;
    brep::solid* solid = brep::mvfs(&points[0], &v);

    // mev kemr 生成的都是“上面”的环
    // mef 生成的是“下面”的环

    // outer loop
    brep::half_edge* he = brep::mev(v, &points[1], solid->sface->floops);
    he = brep::mev(he->endv, &points[2], solid->sface->floops);
    he = brep::mev(he->endv, &points[3], solid->sface->floops);
    auto outer_loop = brep::mef(v, he->endv, solid->sface->floops);

    // inner loop
    he = brep::mev(v, &points[4], solid->sface->floops);
    auto inner_loop_start_vertex = he->endv;
    he = brep::mev(he->endv, &points[5], solid->sface->floops);
    he = brep::mev(he->endv, &points[6], solid->sface->floops);
    he = brep::mev(he->endv, &points[7], solid->sface->floops);
    auto inner_loop =
        brep::mef(he->endv, inner_loop_start_vertex, solid->sface->floops);
    brep::kemr(v, inner_loop_start_vertex, solid->sface->floops);

    brep::kfmrh(outer_loop, inner_loop);
    // double direction[] = {0, 0, 1};
    // brep::sweep(solid->sface, direction, 3);

    auto solid_points = render::to_points(solid);

    for (auto& face_vector : solid_points)
    {
        gluTessBeginPolygon(tess, nullptr); // with NULL data
        for (std::size_t i = 0; i < face_vector.size(); i++)
        {
            // face_vector[0]为外环，其余视为内环
            auto& loop_vector = face_vector[i];
            gluTessBeginContour(tess);
            for (auto& point_data : loop_vector)
            {
                std::cout << point_data << std::endl;
                gluTessVertex(tess, point_data.GetCoord(),
                              (void*)point_data.GetCoord());
            }
            gluTessEndContour(tess);
            std::cout << std::endl;
        }
        gluTessEndPolygon(tess);
    }
    glEndList();

    gluDeleteTess(tess); // delete after tessellation

    // DEBUG //
    // print out actual GL calls that are performed
    cout << endl;
    cout << "2. Quad with a Hole\n";
    cout << "===================\n";
    cout << ss.str().c_str() << endl;
    ss.str(""); // clear string buffer

    return id;  // return handle ID of a display list
}

///////////////////////////////////////////////////////////////////////////////
// tessellate a polygon with a hole and compile it into a display list
///////////////////////////////////////////////////////////////////////////////
GLuint tessellate2()
{
    GLuint id = glGenLists(1);          // create a display list
    if (!id)
        return id;                      // failed to create a list, return 0

    GLUtesselator* tess = gluNewTess(); // create a tessellator
    if (!tess)
        return 0; // failed to create tessellation object, return 0

    // define concave quad with a hole
    //  0--------3
    //  | 4----7 |
    //  | |    | |
    //  | 5----6 |
    //  1--------2
    // GLdouble quad2[8][3] = {{-2, 3, -1}, {-2, 0, -1}, {2, 0, -1}, {2, 3, -1},
    //                         {-1, 2, -1}, {-1, 1, -1}, {1, 1, -1}, {1, 2,
    //                         -1}};

    // register callback functions
    gluTessCallback(tess, GLU_TESS_BEGIN,
                    (void(__stdcall*)(void))tess_begin_callback);
    gluTessCallback(tess, GLU_TESS_END,
                    (void(__stdcall*)(void))tess_end_callback);
    gluTessCallback(tess, GLU_TESS_ERROR,
                    (void(__stdcall*)(void))tess_error_callback);
    gluTessCallback(tess, GLU_TESS_VERTEX,
                    (void(__stdcall*)())tess_vertex_callback);

    // tessellate and compile a concave quad into display list
    glNewList(id, GL_COMPILE);
    glColor3f(1, 1, 1);
    render::point points[8] = {{-2, 3, 0}, {-2, 0, 0}, {2, 0, 0}, {2, 3, 0},
                               {-1, 2, 0}, {-1, 1, 0}, {1, 1, 0}, {1, 2, 0}};
    brep::vertex* v = nullptr;
    brep::solid* solid = brep::mvfs(&points[0], &v);

    // mev kemr 生成的都是“上面”的环
    // mef 生成的是“下面”的环

    // outer loop
    brep::half_edge* he = brep::mev(v, &points[1], solid->sface->floops);
    he = brep::mev(he->endv, &points[2], solid->sface->floops);
    he = brep::mev(he->endv, &points[3], solid->sface->floops);
    auto outer_loop = brep::mef(v, he->endv, solid->sface->floops);

    // inner loop
    he = brep::mev(v, &points[4], solid->sface->floops);
    auto inner_loop_start_vertex = he->endv;
    he = brep::mev(he->endv, &points[5], solid->sface->floops);
    he = brep::mev(he->endv, &points[6], solid->sface->floops);
    he = brep::mev(he->endv, &points[7], solid->sface->floops);
    auto inner_loop =
        brep::mef(he->endv, inner_loop_start_vertex, solid->sface->floops);
    brep::kemr(v, inner_loop_start_vertex, solid->sface->floops);

    brep::kfmrh(outer_loop, inner_loop);
    double direction[] = {0, 0, 1};
    brep::sweep(solid->sface, direction, 3);

    auto solid_points = render::to_points(solid);

    for (auto& face_vector : solid_points)
    {
        gluTessBeginPolygon(tess, nullptr); // with NULL data
        for (std::size_t i = 0; i < face_vector.size(); i++)
        {
            // face_vector[0]为外环，其余视为内环
            auto& loop_vector = face_vector[i];
            gluTessBeginContour(tess);
            for (auto& point_data : loop_vector)
            {
                gluTessVertex(tess, point_data.GetCoord(),
                              (void*)point_data.GetCoord());
            }
            gluTessEndContour(tess);
        }
        gluTessEndPolygon(tess);
    }
    glEndList();

    gluDeleteTess(tess); // delete after tessellation

    // DEBUG //
    // print out actual GL calls that are performed
    cout << endl;
    cout << "2. Quad with a Hole\n";
    cout << "===================\n";
    cout << ss.str().c_str() << endl;
    ss.str(""); // clear string buffer

    return id;  // return handle ID of a display list
}

///////////////////////////////////////////////////////////////////////////////
// convert enum of OpenGL primitive type to a string(char*)
// OpenGL supports only 10 primitive types.
///////////////////////////////////////////////////////////////////////////////
const char* getPrimitiveType(GLenum type)
{
    switch (type)
    {
    case 0x0000:
        return "GL_POINTS";
        break;
    case 0x0001:
        return "GL_LINES";
        break;
    case 0x0002:
        return "GL_LINE_LOOP";
        break;
    case 0x0003:
        return "GL_LINE_STRIP";
        break;
    case 0x0004:
        return "GL_TRIANGLES";
        break;
    case 0x0005:
        return "GL_TRIANGLE_STRIP";
        break;
    case 0x0006:
        return "GL_TRIANGLE_FAN";
        break;
    case 0x0007:
        return "GL_QUADS";
        break;
    case 0x0008:
        return "GL_QUAD_STRIP";
        break;
    case 0x0009:
        return "GL_POLYGON";
        break;
    }
}

///////////////////////////////////////////////////////////////////////////////
// initialize GLUT for windowing
///////////////////////////////////////////////////////////////////////////////
int init_GLUT(int argc, char** argv)
{
    // GLUT stuff for windowing
    // initialization openGL window.
    // it is called before any other GLUT routine
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL |
                        GLUT_MULTISAMPLE); // display mode

    glutInitWindowSize(400, 200);          // window size

    glutInitWindowPosition(100, 100);      // window location

    // finally, create a window with openGL context
    // Window will not displayed until glutMainLoop() is called
    // it returns a unique ID
    int handle = glutCreateWindow(argv[0]); // param is the title of window

    // register GLUT callback functions
    glutDisplayFunc(display_callback);
    glutTimerFunc(33, timer_callback, 33); // redraw only every given millisec
    // glutIdleFunc(idleCB);                       // redraw only every given
    // millisec
    glutReshapeFunc(reshape_callback);
    glutKeyboardFunc(keyboard_callback);
    glutMouseFunc(mouse_callback);
    glutMotionFunc(mouse_motion_callback);

    return handle;
}

///////////////////////////////////////////////////////////////////////////////
// initialize OpenGL
// disable unused features
///////////////////////////////////////////////////////////////////////////////
void init_GL()
{
    glShadeModel(GL_SMOOTH); // shading mathod: GL_SMOOTH or GL_FLAT
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4); // 4-byte pixel alignment

    // enable /disable features
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    // glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    // glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);

    // track material ambient and diffuse from surface color, call it before
    // glEnable(GL_COLOR_MATERIAL)
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);

    glClearColor(0, 0, 0, 0); // background color
    glClearStencil(0);        // clear stencil buffer
    glClearDepth(1.0f);       // 0 is near, 1 is far
    glDepthFunc(GL_LEQUAL);

    init_lights();
    set_camera(-5, 0, 5, 0, 0, 0);
}

///////////////////////////////////////////////////////////////////////////////
// write 2d text using GLUT
// The projection matrix must be set to orthogonal before call this function.
///////////////////////////////////////////////////////////////////////////////
void draw_string(const char* str, int x, int y, float color[4], void* font)
{
    glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT); // lighting and color mask
    glDisable(GL_LIGHTING); // need to disable lighting for proper text color

    glColor4fv(color);      // set text color
    glRasterPos2i(x, y);    // place text position

    // loop all characters in the string
    while (*str)
    {
        glutBitmapCharacter(font, *str);
        ++str;
    }

    glEnable(GL_LIGHTING);
    glPopAttrib();
}

///////////////////////////////////////////////////////////////////////////////
// draw a string in 3D space
///////////////////////////////////////////////////////////////////////////////
void draw_string_3D(const char* str, float pos[3], float color[4], void* font)
{
    glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT); // lighting and color mask
    glDisable(GL_LIGHTING); // need to disable lighting for proper text color

    glColor4fv(color);      // set text color
    glRasterPos3fv(pos);    // place text position

    // loop all characters in the string
    while (*str)
    {
        glutBitmapCharacter(font, *str);
        ++str;
    }

    glEnable(GL_LIGHTING);
    glPopAttrib();
}

///////////////////////////////////////////////////////////////////////////////
// initialize global variables
///////////////////////////////////////////////////////////////////////////////
bool init_shared_mem()
{
    mouse_left_down = mouse_right_down = false;
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// clean up shared memory
///////////////////////////////////////////////////////////////////////////////
void clear_shared_mem() {}

///////////////////////////////////////////////////////////////////////////////
// initialize lights
///////////////////////////////////////////////////////////////////////////////
void init_lights()
{
    // set up light colors (ambient, diffuse, specular)
    GLfloat lightKa[] = {.2f, .2f, .2f, 1.0f}; // ambient light
    GLfloat lightKd[] = {.7f, .7f, .7f, 1.0f}; // diffuse light
    GLfloat lightKs[] = {1, 1, 1, 1};          // specular light
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightKa);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightKd);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightKs);

    // position the light
    float lightPos[4] = {0, 0, 20, 1}; // positional light
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    glEnable(GL_LIGHT0); // MUST enable each light source after configuration
}

///////////////////////////////////////////////////////////////////////////////
// set camera position and lookat direction
///////////////////////////////////////////////////////////////////////////////
void set_camera(float posX, float posY, float posZ, float targetX,
                float targetY, float targetZ)
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(posX, posY, posZ, targetX, targetY, targetZ, 0, 1,
              0); // eye(x,y,z), focal(x,y,z), up(x,y,z)
}

///////////////////////////////////////////////////////////////////////////////
// display info messages
///////////////////////////////////////////////////////////////////////////////
void show_info()
{
    // backup current model-view matrix
    glPushMatrix();   // save current modelview matrix
    glLoadIdentity(); // reset modelview matrix

    // set to 2D orthogonal projection
    glMatrixMode(GL_PROJECTION); // switch to projection matrix
    glPushMatrix();              // save current projection matrix
    glLoadIdentity();            // reset projection matrix
    gluOrtho2D(0, 400, 0, 200);  // set to orthogonal projection

    float color[4] = {1, 1, 1, 1};

    stringstream ss;
    ss << std::fixed << std::setprecision(3);

    if (draw_mode == 0)
        ss << "Draw Mode: Fill" << ends;
    else if (draw_mode == 1)
        ss << "Draw Mode: Wireframe" << ends;
    else
        ss << "Draw Mode: Points" << ends;
    draw_string(ss.str().c_str(), 1, 186, color, font_ptr);
    ss.str("");

    ss << "Press 'D' to switch drawing mode." << ends;
    draw_string(ss.str().c_str(), 1, 2, color, font_ptr);
    ss.str("");

    // unset floating format
    ss << std::resetiosflags(std::ios_base::fixed | std::ios_base::floatfield);

    // restore projection matrix
    glPopMatrix(); // restore to previous projection matrix

    // restore modelview matrix
    glMatrixMode(GL_MODELVIEW); // switch to modelview matrix
    glPopMatrix();              // restore to previous modelview matrix
}

//=============================================================================
// CALLBACKS
//=============================================================================

void display_callback()
{
    // clear buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // save the initial ModelView matrix before modifying ModelView matrix
    glPushMatrix();

    // tramsform camera
    glTranslatef(0, 0, camera_distance);
    glRotatef(camera_angle_x, 1, 0, 0); // pitch
    glRotatef(camera_angle_y, 0, 1, 0); // heading

    // draw meshes
    glTranslatef(-4, -1.5f, 0);
    glCallList(list_id_1);

    glTranslatef(4, 0, 0);
    glCallList(list_id_2);

    glTranslatef(4, 0, 0);
    glCallList(list_id_3);

    // draw info messages
    show_info();

    glPopMatrix();

    glutSwapBuffers();
}

void reshape_callback(int w, int h)
{
    // set viewport to be the entire window
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);

    // set perspective viewing frustum
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // glFrustum(-aspectRatio, aspectRatio, -1, 1, 1, 100);
    gluPerspective(60.0, (double)(w) / h, 1.0,
                   1000.0); // FOV, AspectRatio, NearClip, FarClip

    // switch to modelview matrix in order to set scene
    glMatrixMode(GL_MODELVIEW);
}

void timer_callback(int millisec)
{
    glutTimerFunc(millisec, timer_callback, millisec);
    glutPostRedisplay();
}

void idle_callback() { glutPostRedisplay(); }

void keyboard_callback(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 27: // ESCAPE
        clear_shared_mem();
        exit(0);

    case ' ':
        break;

    case 'd': // switch rendering modes (fill -> wire -> point)
    case 'D':
        draw_mode = ++draw_mode % 3;
        if (draw_mode == 0) // fill mode
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
        }
        else if (draw_mode == 1) // wireframe mode
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
        }
        else // point mode
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
        }
        break;

    default:;
    }

    glutPostRedisplay();
}

void mouse_callback(int button, int state, int x, int y)
{
    mouse_x = x;
    mouse_y = y;

    if (button == GLUT_LEFT_BUTTON)
    {
        if (state == GLUT_DOWN)
        {
            mouse_left_down = true;
        }
        else if (state == GLUT_UP)
            mouse_left_down = false;
    }

    else if (button == GLUT_RIGHT_BUTTON)
    {
        if (state == GLUT_DOWN)
        {
            mouse_right_down = true;
        }
        else if (state == GLUT_UP)
            mouse_right_down = false;
    }
}

void mouse_motion_callback(int x, int y)
{
    if (mouse_left_down)
    {
        camera_angle_y += (x - mouse_x);
        camera_angle_x += (y - mouse_y);
        mouse_x = x;
        mouse_y = y;
    }
    if (mouse_right_down)
    {
        camera_distance += (y - mouse_y) * 0.2f;
        mouse_y = y;
    }

    // glutPostRedisplay();
}

///////////////////////////////////////////////////////////////////////////////
// GLU_TESS CALLBACKS
///////////////////////////////////////////////////////////////////////////////
void CALLBACK tess_begin_callback(GLenum which)
{
    glBegin(which);
    points.clear();

    // DEBUG //
    ss << "glBegin(" << getPrimitiveType(which) << ");\n";
}

void CALLBACK tess_end_callback()
{
    // glNormal3dv(render::get_normal(points[0].coord, points[1].coord,
    // points[2].coord));
    auto normal_ptr = render::get_normal(points[0].coord, points[1].coord,
                                             points[2].coord);
    for (auto& point: points)
    {
        glNormal3dv(normal_ptr.get());
        glVertex3dv(point.coord);
        // glNormal3dv(normal_ptr);
    }
    

    glEnd();
    points.clear();

    // DEBUG //
    ss << "glEnd();\n";
}

void CALLBACK tess_vertex_callback(const GLvoid* data)
{
    // cast back to double type
    const GLdouble* ptr = (const GLdouble*)data;

    // glVertex3dv(ptr);
    // DEBUG //
    ss << "  glVertex3d(" << *ptr << ", " << *(ptr + 1) << ", " << *(ptr + 2)
       << ");\n";
    points.emplace_back(render::point{const_cast<double*>(ptr)});
    // if (points.size() >= 3)
    // {
    //     auto normal_ptr = render::get_normal(points[0].coord, points[1].coord,
    //                                          points[2].coord);
    //     glNormal3dv(normal_ptr);
    //     ss << std::format("  glNormal3d({}, {}, {})", normal_ptr[0],
    //                       normal_ptr[1], normal_ptr[2])
    //        << std::endl;
    // }
}

///////////////////////////////////////////////////////////////////////////////
// draw a vertex with color
///////////////////////////////////////////////////////////////////////////////
void CALLBACK tess_vertex_callback2(const GLvoid* data)
{
    // cast back to double type
    const GLdouble* ptr = (const GLdouble*)data;

    glColor3dv(ptr + 3);
    glVertex3dv(ptr);

    // DEBUG //
    ss << "  glColor3d(" << *(ptr + 3) << ", " << *(ptr + 4) << ", "
       << *(ptr + 5) << ");\n";
    ss << "  glVertex3d(" << *ptr << ", " << *(ptr + 1) << ", " << *(ptr + 2)
       << ");\n";
}

///////////////////////////////////////////////////////////////////////////////
// Combine callback is used to create a new vertex where edges intersect.
// In this function, copy the vertex data into local array and compute the
// color of the vertex. And send it back to tessellator, so tessellator pass it
// to vertex callback function.
//
// newVertex: the intersect point which tessellator creates for us
// neighborVertex[4]: 4 neighbor vertices to cause intersection (given from 3rd
// param of gluTessVertex() neighborWeight[4]: 4 interpolation weights of 4
// neighbor vertices outData: the vertex data to return to tessellator
///////////////////////////////////////////////////////////////////////////////
void CALLBACK tess_combine_callback(const GLdouble newVertex[3],
                                    const GLdouble* neighborVertex[4],
                                    const GLfloat neighborWeight[4],
                                    GLdouble** outData)
{
    // copy new intersect vertex to local array
    // Because newVertex is temporal and cannot be hold by tessellator until
    // next vertex callback called, it must be copied to the safe place in the
    // app. Once gluTessEndPolygon() called, then you can safly deallocate the
    // array.
    vertices[vertexIndex][0] = newVertex[0];
    vertices[vertexIndex][1] = newVertex[1];
    vertices[vertexIndex][2] = newVertex[2];

    // compute vertex color with given weights and colors of 4 neighbors
    // the neighborVertex[4] must hold required info, in this case, color.
    // neighborVertex was actually the third param of gluTessVertex() and is
    // passed into here to compute the color of the intersect vertex.
    vertices[vertexIndex][3] = neighborWeight[0] * neighborVertex[0][3] + // red
                               neighborWeight[1] * neighborVertex[1][3] +
                               neighborWeight[2] * neighborVertex[2][3] +
                               neighborWeight[3] * neighborVertex[3][3];
    vertices[vertexIndex][4] =
        neighborWeight[0] * neighborVertex[0][4] + // green
        neighborWeight[1] * neighborVertex[1][4] +
        neighborWeight[2] * neighborVertex[2][4] +
        neighborWeight[3] * neighborVertex[3][4];
    vertices[vertexIndex][5] =
        neighborWeight[0] * neighborVertex[0][5] + // blue
        neighborWeight[1] * neighborVertex[1][5] +
        neighborWeight[2] * neighborVertex[2][5] +
        neighborWeight[3] * neighborVertex[3][5];

    // return output data (vertex coords and others)
    *outData =
        vertices[vertexIndex]; // assign the address of new intersect vertex

    ++vertexIndex;             // increase index for next vertex
}

void CALLBACK tess_error_callback(GLenum errorCode)
{
    const GLubyte* errorStr;
    std::cout << errorCode << std::endl;
    errorStr = gluErrorString(errorCode);
    cerr << "[ERROR]: " << errorStr << endl;
}
