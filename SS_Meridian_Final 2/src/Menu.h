#ifndef SS_MERIDIAN_MENU_H
#define SS_MERIDIAN_MENU_H

// Implementation header: include only through iMain.cpp (one host translation unit).
// Original menu artwork, layout, hover behavior and audio; shared-host adapters.
namespace menu {
static int requestedMission=0;
static bool keyPressed[512]={false};
#ifndef _WIN32
typedef unsigned long DWORD;
const int MAX_PATH=260;
#define _snprintf snprintf
static DWORD GetTickCount(){return (DWORD)glutGet(GLUT_ELAPSED_TIME);}
#endif
static void iSetColor(float r,float g,float b){glColor3f(r/255,g/255,b/255);}
static void iText(float x,float y,char* s,void* font){glDisable(GL_TEXTURE_2D);glRasterPos2f(x,y);for(;*s;++s)glutBitmapCharacter(font,*s);}
static void iFilledRectangle(float x,float y,float w,float h){glRectf(x,y,x+w,y+h);}
static void iClear(){glClear(GL_COLOR_BUFFER_BIT);}
const int SCREEN_W = 1280;
const int SCREEN_H = 720;

// -------------------- Application states --------------------
enum AppState {
    STATE_MAIN_MENU,
    STATE_MISSION_SELECT,
    STATE_CONTROLS,
    STATE_SETTINGS,
    STATE_CREDITS
    // Add your gameplay states here, e.g.:
    // STATE_PLAYING,
    // STATE_PAUSED,
    // STATE_GAME_OVER,
    // STATE_MISSION_COMPLETE
};

// -------------------- Data structures --------------------
struct Texture {
    unsigned int id;
    int w, h;
    float u,v;
    bool valid;
    Texture() : id(0), w(0), h(0), u(1), v(1), valid(false) {}
};

struct RectF {
    float x, y, w, h;
    RectF() : x(0), y(0), w(0), h(0) {}
    RectF(float _x, float _y, float _w, float _h) : x(_x), y(_y), w(_w), h(_h) {}
};

// -------------------- Global state --------------------
AppState gState = STATE_MAIN_MENU;
bool gAudioEnabled = true;
int gMenuHover = -1;
int gPrevMenuHover = -1;
float gMenuPulse = 0.0f;
float gMenuDripOffset = 0.0f;
int gMissionHover = -1;
int gPrevMissionHover = -1;

unsigned char gPrevKeys[512] = {0};

// Subtitle / notification
DWORD gSubtitleUntil = 0;
char gSubtitle[512] = "";

// -------------------- Textures --------------------
Texture texMenuBg, texMenuTitle, texMenuDrips, texMenuHoverPalm, texMenuButton;
Texture texMissionPanel, texMissionCard, texMissionCardSelected, texMissionComplete;

// -------------------- Path utilities --------------------
char gExeDir[MAX_PATH] = {0};
bool gFrameworkReady = false;

void initExeDir() { strcpy(gExeDir, "."); }

void makeFullPath(const char* relativePath, char* outPath, int outSize) {
    if (!relativePath || !outPath || outSize <= 0) return;
    if ((strlen(relativePath) > 2 && relativePath[1] == ':') ||
        relativePath[0] == '\\' || relativePath[0] == '/') {
        strncpy(outPath, relativePath, outSize - 1);
        outPath[outSize - 1] = 0;
        return;
    }
    _snprintf(outPath, outSize - 1, "%s/%s", gExeDir, relativePath);
    outPath[outSize - 1] = 0;
}

bool fileExistsFull(const char* fullPath) {
    FILE* f = fopen(fullPath, "rb");
    if (!f) return false;
    fclose(f);
    return true;
}

// -------------------- Texture loading --------------------
Texture loadTextureSafe(const char* relativePath) {
    Texture t;
    char path[MAX_PATH * 2];
    makeFullPath(relativePath, path, sizeof(path));
    if (!fileExistsFull(path)) throw std::runtime_error(std::string("Missing menu asset: ")+path);

    int w = 0, h = 0, bpp = 0;
    unsigned char* data = stbi_load(path, &w, &h, &bpp, 4);
    if (!data || w <= 0 || h <= 0) {
        if (data) stbi_image_free(data);
        throw std::runtime_error(std::string("Cannot decode menu asset: ")+path);
    }

    int tw=1,th=1;while(tw<w)tw*=2;while(th<h)th*=2;
    GLint limit=0;glGetIntegerv(GL_MAX_TEXTURE_SIZE,&limit);
    if(tw>limit||th>limit){stbi_image_free(data);throw std::runtime_error("Menu texture exceeds GPU limit");}
    std::vector<unsigned char> backing(tw*th*4,0);
    for(int y=0;y<h;++y)std::copy(data+y*w*4,data+(y+1)*w*4,backing.begin()+y*tw*4);
    t.u=float(w)/tw;t.v=float(h)/th;
    glGenTextures(1, &t.id);
    glBindTexture(GL_TEXTURE_2D, t.id);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tw, th, 0, GL_RGBA, GL_UNSIGNED_BYTE, &backing[0]);
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);

    if(glGetError()!=GL_NO_ERROR){glDeleteTextures(1,&t.id);throw std::runtime_error(std::string("Menu texture upload failed: ")+path);}
    t.w = w; t.h = h; t.valid = (t.id != 0);
    return t;
}

// -------------------- Drawing helpers --------------------
void drawTextureAlpha(const Texture& t, float x, float y, float w, float h,
                      float alpha, bool additive) {
    if (!t.valid || alpha <= 0.001f) return;
    glDisable(GL_ALPHA_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, additive ? GL_ONE : GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, t.id);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glColor4f(1.0f, 1.0f, 1.0f, alpha);
    glBegin(GL_QUADS);
        glTexCoord2f(0, t.v); glVertex2f(x, y);
        glTexCoord2f(t.u, t.v); glVertex2f(x + w, y);
        glTexCoord2f(t.u, 0); glVertex2f(x + w, y + h);
        glTexCoord2f(0, 0); glVertex2f(x, y + h);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
    glEnable(GL_ALPHA_TEST);
    glColor4f(1, 1, 1, 1);
}

void drawTexture(const Texture& t, float x, float y, float w, float h) {
    drawTextureAlpha(t, x, y, w, h, 1.0f, false);
}

void fillAlpha(float x, float y, float w, float h,
               float r, float g, float b, float a) {
    glDisable(GL_ALPHA_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(r, g, b, a);
    glBegin(GL_QUADS);
        glVertex2f(x, y); glVertex2f(x + w, y);
        glVertex2f(x + w, y + h); glVertex2f(x, y + h);
    glEnd();
    glDisable(GL_BLEND);
    glEnable(GL_ALPHA_TEST);
    glColor4f(1, 1, 1, 1);
}

bool pointInRect(float px, float py, const RectF& r) {
    return px >= r.x && px <= r.x + r.w && py >= r.y && py <= r.y + r.h;
}

void drawCenteredText(float y, const char* text, void* font) {
    int len = (int)strlen(text);
    float approx = (font == GLUT_BITMAP_TIMES_ROMAN_24 ? 12.0f : 8.0f) * len;
    iText((SCREEN_W - approx) * 0.5f, y, (char*)text, font);
}

void setSubtitle(const char* s, DWORD ms) {
    strncpy(gSubtitle, s, sizeof(gSubtitle) - 1);
    gSubtitle[sizeof(gSubtitle) - 1] = 0;
    gSubtitleUntil = GetTickCount() + ms;
}

bool keyEdge(unsigned char k) {
    return keyPressed[k] && !gPrevKeys[k];
}

// -------------------- Audio --------------------
#ifdef _WIN32
void closeAlias(const char* alias) {
    char cmd[128];
    sprintf(cmd, "stop %s", alias);  mciSendStringA(cmd, NULL, 0, NULL);
    sprintf(cmd, "close %s", alias); mciSendStringA(cmd, NULL, 0, NULL);
}

void stopAllAudio() {
    closeAlias("bgm"); closeAlias("fx");
}

void playMenuSound(const char* filename, const char* alias) {
    if (!gAudioEnabled) return;
    closeAlias(alias);
    char rel[512], path[MAX_PATH * 2], cmd[MAX_PATH * 2 + 128];
    _snprintf(rel, sizeof(rel) - 1, "assets/audio/%s", filename);
    rel[sizeof(rel) - 1] = 0;
    makeFullPath(rel, path, sizeof(path));
    if (!fileExistsFull(path)) return;
    _snprintf(cmd, sizeof(cmd) - 1, "open \"%s\" type waveaudio alias %s", path, alias);
    cmd[sizeof(cmd) - 1] = 0;
    mciSendStringA(cmd, NULL, 0, NULL);
    char playCmd[128];
    _snprintf(playCmd, sizeof(playCmd) - 1, "play %s from 0", alias);
    playCmd[sizeof(playCmd) - 1] = 0;
    mciSendStringA(playCmd, NULL, 0, NULL);
}

#else
void stopAllAudio() {}
void playMenuSound(const char*, const char*) {}
#endif

void playHoverSound() { playMenuSound("menu_hover.wav", "fx"); }
void playClickSound() { playMenuSound("menu_click.wav", "fx"); }
void playBackSound()  { playMenuSound("menu_back.wav",  "fx"); }

// -------------------- Load textures --------------------
void loadMenuTextures() {
    texMenuBg            = loadTextureSafe("assets/menu/menu_background.png");
    texMenuTitle         = loadTextureSafe("assets/menu/menu_title.png");
    texMenuDrips         = loadTextureSafe("assets/menu/menu_title_drips.png");
    texMenuHoverPalm     = loadTextureSafe("assets/menu/menu_hover_palm.png");
    texMenuButton        = loadTextureSafe("assets/menu/menu_button_normal.png");
    texMissionPanel      = loadTextureSafe("assets/menu/mission_select_panel.png");
    texMissionCard       = loadTextureSafe("assets/menu/mission_card_normal.png");
    texMissionCardSelected = loadTextureSafe("assets/menu/mission_card_selected.png");
    texMissionComplete   = loadTextureSafe("assets/menu/mission_complete_overlay.png");
}

// Mission launch request is consumed by the shared host.
void launchMission(int missionNum) { requestedMission = missionNum; }

// -------------------- Layout --------------------
RectF menuButton(int i)  { return RectF(840, 410 - i * 58, 310, 44); }
RectF missionCard(int i) { return RectF(320.0f + i * 230.0f, 220, 180, 330); }

// ==================== DRAW FUNCTIONS ====================

void drawMainMenu() {
    // Background
    if (texMenuBg.valid) drawTexture(texMenuBg, 0, 0, 1280, 720);
    else { iSetColor(238, 235, 228); iFilledRectangle(0, 0, 1280, 720); }

    // Animated title
    drawTextureAlpha(texMenuTitle, 650, 555, 520, 125, 0.96f, false);
    gMenuPulse = 0.76f + 0.18f * sinf(GetTickCount() * 0.0025f);
    gMenuDripOffset = fmodf(GetTickCount() * 0.012f, 90.0f);
    drawTextureAlpha(texMenuDrips, 770, 350 - gMenuDripOffset, 180, 310, gMenuPulse, false);

    // Buttons
    const char* labels[6] = {
        "START GAME", "MISSION SELECT", "CONTROLS",
        "SETTINGS",   "CREDITS",        "EXIT"
    };
    for (int i = 0; i < 6; i++) {
        RectF r = menuButton(i);
        drawTextureAlpha(texMenuButton, r.x - 12, r.y - 10, r.w + 24, r.h + 20, 0.34f, false);
        if (gMenuHover == i)
            drawTextureAlpha(texMenuHoverPalm, r.x - 30, r.y - 22,
                             r.w + 60, r.h + 46, 0.72f, false);
        iSetColor(35, 30, 30);
        iText(r.x + 42, r.y + 14, (char*)labels[i], GLUT_BITMAP_TIMES_ROMAN_24);
    }

    // Tagline
    iSetColor(80, 75, 72);
    iText(838, 88, (char*)"A fixed-camera 2D survival-horror game", GLUT_BITMAP_8_BY_13);
}

void drawMissionSelect() {
    drawTexture(texMenuBg, 0, 0, 1280, 720);
    // The supplied panel bakes in five empty cards. Use the original individual
    // card textures below so exactly three cards are visible; no artwork edits.
    iSetColor(35, 30, 30);
    drawCenteredText(645, "MISSION SELECT", GLUT_BITMAP_TIMES_ROMAN_24);

    for (int i = 0; i < 3; i++) {
        RectF card = missionCard(i);
        bool hovered = (gMissionHover == i);
        Texture& c = hovered ? texMissionCardSelected : texMissionCard;
        fillAlpha(card.x, card.y, card.w, card.h, 1, 1, 1, .55f);
        drawTextureAlpha(c, card.x, card.y, card.w, card.h,
                         hovered ? 0.98f : 0.65f, false);

        // Mission number
        char b[64];
        sprintf(b, "MISSION %d", i + 1);
        iSetColor(hovered ? 70 : 125, hovered ? 20 : 120, hovered ? 20 : 115);
        iText(card.x + 38, 370, b, GLUT_BITMAP_HELVETICA_18);
    }

    fillAlpha(28, 25, 560, 38, 1, 1, 1, .80f);
    iSetColor(40, 35, 35);
    iText(42, 40, (char*)"[ESC] BACK     Click a mission card to play.",
          GLUT_BITMAP_HELVETICA_18);
}

void drawControls() {
    drawTexture(texMenuBg, 0, 0, 1280, 720);
    fillAlpha(280, 120, 720, 500, 1, 1, 1, 0.80f);
    iSetColor(45, 35, 35);
    drawCenteredText(570, "CONTROLS", GLUT_BITMAP_TIMES_ROMAN_24);

    const char* lines[] = {
        "MISSION 1: WASD move; ENTER interact",
        "SPACE attack; E hide; mouse buttons",
        "MISSION 2: A / D move and dodge",
        "SPACE fire; R reload; mouse buttons",
        "MISSION 3: A / D move; W jump",
        "J kick; K weapon; L fire; SHIFT guard",
        "ENTER confirm; mouse buttons",
        "ENDING: Hold W to walk; ENTER continue",
        "F11 fullscreen / windowed",
        "ESC back to menu"
    };
    for (int i = 0; i < 10; i++)
        iText(395, 515 - i * 38, (char*)lines[i], GLUT_BITMAP_HELVETICA_18);
    iText(45, 40, (char*)"[ESC] BACK", GLUT_BITMAP_HELVETICA_18);
}

void drawSettings() {
    drawTexture(texMenuBg, 0, 0, 1280, 720);
    fillAlpha(385, 210, 510, 300, 1, 1, 1, 0.82f);
    iSetColor(45, 35, 35);
    drawCenteredText(470, "SETTINGS", GLUT_BITMAP_TIMES_ROMAN_24);
    iText(500, 380, (char*)"MASTER AUDIO", GLUT_BITMAP_HELVETICA_18);
    iSetColor(gAudioEnabled ? 35 : 120, gAudioEnabled ? 100 : 35, 35);
    iText(690, 380, (char*)(gAudioEnabled ? "ON" : "OFF"), GLUT_BITMAP_HELVETICA_18);
    iSetColor(45, 35, 35);
    iText(500, 325, (char*)"Click ON/OFF area to toggle", GLUT_BITMAP_8_BY_13);
    iText(45, 40, (char*)"[ESC] BACK", GLUT_BITMAP_HELVETICA_18);
}

void drawCredits() {
    drawTexture(texMenuBg, 0, 0, 1280, 720);
    fillAlpha(260, 120, 760, 490, 1, 1, 1, 0.82f);
    iSetColor(45, 35, 35);
    drawCenteredText(565, "CREDITS", GLUT_BITMAP_TIMES_ROMAN_24);
    drawCenteredText(485, "SS MERIDIAN", GLUT_BITMAP_HELVETICA_18);
    drawCenteredText(445, "University CSE Game Project", GLUT_BITMAP_HELVETICA_18);
    drawCenteredText(390, "Built with iGraphics / OpenGL / GLUT", GLUT_BITMAP_8_BY_13);
    iText(45, 40, (char*)"[ESC] BACK", GLUT_BITMAP_HELVETICA_18);
}

void drawSubtitle() {
    if (GetTickCount() < gSubtitleUntil) {
        fillAlpha(185, 25, 910, 58, 0, 0, 0, 0.72f);
        iSetColor(245, 245, 245);
        drawCenteredText(49, gSubtitle, GLUT_BITMAP_HELVETICA_18);
    }
}

// ==================== iGRAPHICS CALLBACKS ====================

void draw() {
    iClear();
    switch (gState) {
        case STATE_MAIN_MENU:      drawMainMenu(); break;
        case STATE_MISSION_SELECT: drawMissionSelect(); break;
        case STATE_CONTROLS:       drawControls(); break;
        case STATE_SETTINGS:       drawSettings(); break;
        case STATE_CREDITS:        drawCredits(); break;
        // Add your gameplay draw cases here:
        // case STATE_PLAYING: drawGameplay(); break;
    }
    drawSubtitle();
}

void iMouseMove(int mx, int my) {}

void iPassiveMouseMove(int mx, int my) {
    if (gState == STATE_MAIN_MENU) {
        gMenuHover = -1;
        for (int i = 0; i < 6; i++) {
            if (pointInRect((float)mx, (float)my, menuButton(i))) {
                gMenuHover = i;
                break;
            }
        }
        if (gMenuHover != -1 && gMenuHover != gPrevMenuHover)
            playHoverSound();
        gPrevMenuHover = gMenuHover;
    }
    else if (gState == STATE_MISSION_SELECT) {
        gMissionHover = -1;
        for (int i = 0; i < 3; i++) {
            if (pointInRect((float)mx, (float)my, missionCard(i))) {
                gMissionHover = i;
                break;
            }
        }
        if (gMissionHover != -1 && gMissionHover != gPrevMissionHover)
            playHoverSound();
        gPrevMissionHover = gMissionHover;
    }
}

void iMouse(int button, int state, int mx, int my) {
    if (button != GLUT_LEFT_BUTTON || state != GLUT_DOWN) return;

    if (gState == STATE_MAIN_MENU) {
        for (int i = 0; i < 6; i++) {
            if (pointInRect((float)mx, (float)my, menuButton(i))) {
                playClickSound();
                switch (i) {
                    case 0: launchMission(1); break;               // START GAME
                    case 1: gState = STATE_MISSION_SELECT;         // MISSION SELECT
                            gMissionHover = gPrevMissionHover = -1;
                            break;
                    case 2: gState = STATE_CONTROLS; break;        // CONTROLS
                    case 3: gState = STATE_SETTINGS; break;        // SETTINGS
                    case 4: gState = STATE_CREDITS; break;         // CREDITS
                    case 5: stopAllAudio(); exit(0); break;        // EXIT
                }
                return;
            }
        }
    }
    else if (gState == STATE_MISSION_SELECT) {
        for (int i = 0; i < 3; i++) {
            if (pointInRect((float)mx, (float)my, missionCard(i))) {
                playClickSound();
                launchMission(i + 1);
                return;
            }
        }
    }
    else if (gState == STATE_SETTINGS) {
        if (mx > 650 && mx < 800 && my > 345 && my < 420) {
            playClickSound();
            gAudioEnabled = !gAudioEnabled;
            if (!gAudioEnabled) stopAllAudio();
        }
    }
}

void fixedUpdate() {
    if (!gFrameworkReady) return;

    // ESC handling
    if (gState == STATE_MISSION_SELECT || gState == STATE_CONTROLS ||
        gState == STATE_SETTINGS || gState == STATE_CREDITS) {
        if (keyEdge(27)) {
            playBackSound();
            gState = STATE_MAIN_MENU;
            gMenuHover = gPrevMenuHover = -1;
        }
    }
    else if (gState == STATE_MAIN_MENU) {
        if (keyEdge(27)) { stopAllAudio(); exit(0); }
    }

    // Snapshot key state for edge detection
    for (int i = 0; i < 512; i++)
        gPrevKeys[i] = (unsigned char)(keyPressed[i] ? 1 : 0);
}


void release(){
 Texture* all[]={&texMenuBg,&texMenuTitle,&texMenuDrips,&texMenuHoverPalm,&texMenuButton,&texMissionPanel,&texMissionCard,&texMissionCardSelected,&texMissionComplete};
 for(int i=0;i<9;++i){if(all[i]->id)glDeleteTextures(1,&all[i]->id);*all[i]=Texture();}
 stopAllAudio();
}
void initialize(){initExeDir();loadMenuTextures();gFrameworkReady=true;gState=STATE_MAIN_MENU;gMenuHover=gPrevMenuHover=gMissionHover=gPrevMissionHover=-1;requestedMission=0;}
} // namespace menu

#endif // SS_MERIDIAN_MENU_H
