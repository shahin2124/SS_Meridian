// Sole production entry point, rendering dispatcher and framework callbacks.
#include "m3/Platform.h"
#include "GameHost.h"

void iDraw()
{
    using namespace host;
    try {
        glDisable(GL_SCISSOR_TEST);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_ALPHA_TEST);
        glDisable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1, 1, 1, 1);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        if (active == 0) {
            projection(1280, 720, false);
            menu::draw();
        }
        if (active == 1) {
            projection(Meridian::WIDTH, Meridian::HEIGHT, true);
            renderer1->paint(game1.draw(input1));
        }
        if (active == 2) mission2::draw();
        if (active == 3) renderer3.draw(game3, width, height);

        glutSwapBuffers();
        framePresented = true;
    } catch (const std::exception& e) {
        fatal(e.what());
    }
}

// Preserve GLUT coordinates and key-up delivery; route to the active mission.
void iKeyboard(unsigned char key, int x, int y) { host::keyDown(key, x, y); }
void iKeyboardUp(unsigned char key, int x, int y) { host::keyUp(key, x, y); }
void iSpecialKeyboard(int key, int x, int y) { host::special(key, x, y); }
void iMouse(int button, int state, int x, int y) { host::mouse(button, state, x, y); }
void iMouseMove(int x, int y) { host::motion(x, y); }
void iMouseDrag(int x, int y) { host::motion(x, y); }

namespace host {
static void timer(int)
{
    int now = glutGet(GLUT_ELAPSED_TIME);
    double elapsed = std::min(.1, (now - lastTime) / 1000.0);
    lastTime = now;
    try { advance(elapsed); }
    catch (const std::exception& e) { fatal(e.what()); }
    glutPostRedisplay();
    glutTimerFunc(8, timer, 0);
}
}

int main(int argc, char** argv)
{
#ifdef _WIN32
    SetProcessDPIAware();
    char path[MAX_PATH];
    DWORD n = GetModuleFileNameA(0, path, MAX_PATH);
    if (n && n < MAX_PATH) {
        char* end = std::strrchr(path, '\\');
        if (end) { *end = 0; _chdir(path); }
    }
#endif
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(1280, 720);
    glutCreateWindow("SS Meridian");
    std::atexit(host::release);

    glutDisplayFunc(iDraw);
    glutReshapeFunc(host::reshape);
    glutKeyboardFunc(iKeyboard);
    glutKeyboardUpFunc(iKeyboardUp);
    glutSpecialFunc(iSpecialKeyboard);
    glutIgnoreKeyRepeat(1);
    glutMouseFunc(iMouse);
    glutPassiveMotionFunc(iMouseMove);
    glutMotionFunc(iMouseDrag);
    glutVisibilityFunc(host::visibility);

    try { host::launch(0, true); }
    catch (const std::exception& e) { host::fatal(e.what()); }
    glutTimerFunc(8, host::timer, 0);
    glutMainLoop();
    return 0;
}
