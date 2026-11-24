// Smeagol's Precious - Mediterranean (C++ / raylib 5.x)
// Put step1.png (or .jpg) and step2.png next to the executable: your two walking photos.
// Menu: 1/2/3 difficulty, Enter start. Game: W/A/S/D move, Shift sprint (louder), mouse look,
// E = resist Gollum's outburst while Gollum prevails; F = final test at the lighthouse; Esc pause.
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <vector>
#include <cmath>
#include <string>
#include <algorithm>

struct Cfg { const char* name; float stride, spd, det; int hp; float sw; };
static const Cfg CFG[3] = {
    {"EASY",   1.15f, 1.8f, 13.f, 4, 0.80f},
    {"NORMAL", 0.85f, 2.6f, 19.f, 3, 1.00f},
    {"HARD",   0.60f, 3.4f, 27.f, 2, 1.25f}};

struct Col   { float x, z, r; };
struct Bld   { Vector3 p; float w, h; };
struct Tree  { Vector3 p; bool cypress; };
struct Enemy { Vector3 p; bool gamble; float blink, spin, stun, rage; };
struct MathSheet { const char* title; const char* lesson; const char* formula; const char* code; const char* exercise; const char* option[3]; int answer; };

// ---------------------------------------------------------------------------
// 12 sheets: each teaches a piece of math that a programmer uses every day.
// ---------------------------------------------------------------------------
static const MathSheet SHEETS[12] = {
 // 1 - distance
 {"1. GEOMETRY: DISTANCE",
  "Distance checks detect nearby objects, pickups and world boundaries.",
  "d = sqrt(dx*dx + dz*dz)",
  "float d = sqrtf(dx*dx + dz*dz);",
  "The player is 3 units east and 4 units north of a sheet. How far away is it?",
  {"4 units","5 units","7 units"}, 1},
 // 2 - direction
 {"2. TRIGONOMETRY: DIRECTION",
  "Sine and cosine turn a camera angle into a movement direction.",
  "forward = (sin(yaw), -cos(yaw))",
  "float fx = sinf(yaw), fz = -cosf(yaw);",
  "At yaw = 0, which way does the forward vector point in the X/Z plane?",
  {"(0, -1)","(1, 0)","(0, 1)"}, 0},
 // 3 - normalization
 {"3. VECTORS: NORMALIZATION",
  "Normalize a vector to keep its direction but make its length one.",
  "unit = (dx / d, dz / d)",
  "float ux = dx / d, uz = dz / d;",
  "Normalize the vector (3, 4). What is the result?",
  {"(3, 4)","(0.3, 0.4)","(0.6, 0.8)"}, 2},
 // 4 - delta time
 {"4. TIME: FRAME-RATE INDEPENDENCE",
  "Delta time makes movement independent of the frame rate.",
  "distance = speed * dt",
  "position += velocity * dt;",
  "A character moves at 4 units/second for 0.5 seconds. How far does it travel?",
  {"2 units","4 units","8 units"}, 0},
 // 5 - probability
 {"5. PROBABILITY: RANDOM EVENTS",
  "Random probabilities control uncertain outcomes in game logic.",
  "P(outcome) = favourable / total",
  "bool success = GetRandomValue(0, 1) == 1;",
  "A fair coin is tossed once. What is the probability of heads?",
  {"1/4","1/2","1"}, 1},
 // 6 - dot product
 {"6. VECTORS: DOT PRODUCT",
  "The dot product tells how aligned two vectors are. Used for field-of-view and lighting.",
  "dot = ax*bx + az*bz",
  "float dot = ax*bx + az*bz;  // >0 = same side",
  "What is the dot product of (1, 0) and (0, 1)?",
  {"0","1","-1"}, 0},
 // 7 - matrices
 {"7. LINEAR ALGEBRA: MATRICES",
  "A matrix rotates, scales and translates a 3D point in one operation.",
  "p' = M * p",
  "Matrix m = MatrixRotateY(angle);  // then Vector3Transform",
  "A rotation matrix rotates a point. What does a translation matrix do?",
  {"Rotates it","Moves it","Scales it"}, 1},
 // 8 - statistics
 {"8. STATISTICS: AVERAGES",
  "Averages smooth noisy data: FPS counters, AI reaction times, damage over time.",
  "avg = sum / n",
  "float avg = total / (float)count;",
  "Three damage rolls are 4, 6 and 8. What is their average?",
  {"4","6","9"}, 1},
 // 9 - logarithms
 {"9. LOGARITHMS: SCALING",
  "Logarithms compress large ranges. Used for XP curves, volume (dB), and zoom levels.",
  "y = log_b(x)  =>  b^y = x",
  "float lvl = logf(xp) / logf(1.5f);",
  "log2(8) equals which value?  (2^? = 8)",
  {"2","3","4"}, 1},
 // 10 - modulo / wrapping
 {"10. MODULO: WRAPPING",
  "Modulo wraps numbers into a range: compass angles, screen wrap, animation frames.",
  "wrap = ((x % n) + n) % n",
  "int frame = ((frame % 4) + 4) % 4;",
  "What is (7 % 3)?",
  {"1","2","3"}, 0},
 // 11 - interpolation / lerp
 {"11. INTERPOLATION: LERP & EASING",
  "Lerp blends two values by a factor t. Easing curves make motion feel natural.",
  "lerp(a, b, t) = a + (b - a) * t",
  "pos = Lerp(pos, target, t * dt);",
  "With lerp(0, 10, 0.25), what value is returned?",
  {"1.5","2.5","5"}, 1},
 // 12 - collision
 {"12. COLLISION: CIRCLE OVERLAP",
  "Two circles overlap when the distance between centres is less than the sum of radii.",
  "overlap  <=>  d < r1 + r2",
  "if (dist < r1 + r2) { resolve(); }",
  "Circle A has radius 2, circle B radius 3. They collide if the distance is less than...",
  {"5","6","1"}, 0}
};
static const int NSHEETS = 12;

enum State { MENU, PLAY, REST, WIN, LOSE };

static const char* QUIP[2][3] = {
    {"Gollum: No more numbers. Let us sit in the shade.",
     "Gollum: Quiet now... the shadows are listening.",
     "Gollum: Bah! These symbols make my head spin."},
    {"Smeagol: What's that glimmer? Curious, curious...",
     "Smeagol: Smeagol remembers warm hands, yes...",
     "Smeagol: Math is a door, and doors open!"}};

static unsigned rs = 7;
static float rnd() { rs = rs * 1664525u + 1013904223u; return (rs >> 8) / 16777216.0f; }

// ---------- synthesized sounds (no audio files needed) ----------
static Sound mkSound(std::vector<short>& b) {
    Wave w{};
    w.frameCount = (unsigned)b.size();
    w.sampleRate = 22050; w.sampleSize = 16; w.channels = 1;
    w.data = b.data();
    return LoadSoundFromWave(w);   // raylib copies the data
}
static Sound stepSound(float cut) {
    std::vector<short> b(3000);
    float prev = 0;
    for (size_t i = 0; i < b.size(); i++) {
        float t = i / 22050.f, n = GetRandomValue(-1000, 1000) / 1000.f;
        prev += cut * (n - prev);
        b[i] = (short)(prev * expf(-t * 30.f) * 20000.f);
    }
    return mkSound(b);
}
static Sound chimeSound() {
    std::vector<short> b(22050);
    const float f[3] = {660, 880, 1320};
    for (size_t i = 0; i < b.size(); i++) {
        float t = i / 22050.f, s = 0;
        for (int k = 0; k < 3; k++) {
            float tt = t - k * 0.1f;
            if (tt > 0) s += sinf(6.2832f * f[k] * tt) * expf(-tt * 6.f);
        }
        b[i] = (short)(s * 6000.f);
    }
    return mkSound(b);
}
static Sound growlSound() {
    std::vector<short> b(11025);
    float ph = 0;
    for (size_t i = 0; i < b.size(); i++) {
        float t = i / 22050.f, fq = 110.f * powf(45.f / 110.f, t / 0.5f);
        ph += fq / 22050.f; ph -= floorf(ph);
        b[i] = (short)((ph * 2 - 1) * expf(-t * 4.f) * 9000.f);
    }
    return mkSound(b);
}

// ---------- step photos (placeholder silhouette if file missing) ----------
static Texture2D placeholder(int pose) {
    Image im = GenImageColor(300, 420, BLANK);
    Color c = {43, 32, 24, 255};
    ImageDrawCircle(&im, 150, 110, 60, c);
    ImageDrawRectangle(&im, 100, 160, 100, 150, c);
    int a = pose ? 25 : -25;
    ImageDrawRectangle(&im, 115 + a, 300, 32, 115, c);
    ImageDrawRectangle(&im, 153 - a, 300, 32, 115, c);
    Texture2D t = LoadTextureFromImage(im);
    UnloadImage(im);
    return t;
}
static Texture2D loadStep(const char* base, int pose) {
    const char* ext[] = {".png", ".jpg", ".jpeg"};
    for (auto e : ext) {
        std::string f = std::string(base) + e;
        if (FileExists(f.c_str())) { Texture2D t = LoadTexture(f.c_str()); if (t.id) return t; }
    }
    return placeholder(pose);
}

// 12 sheet positions, scattered around the island, away from the lighthouse.
static const Vector3 SHEET_POS[NSHEETS] = {
    {-22, 1.1f, -10}, { 24, 1.1f, -18}, { -8, 1.1f,  24}, {-39, 1.1f, -28},
    { 39, 1.1f,  29}, {-48, 1.1f,  10}, { 14, 1.1f,  36}, { 46, 1.1f, -40},
    {-30, 1.1f, -45}, {  8, 1.1f, -30}, { 50, 1.1f,  -5}, {-14, 1.1f,  46}
};

int main() {
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(1280, 720, "Smeagol's Precious");
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);
    InitAudioDevice();

    // Put a TTF font named mathfont.ttf beside the executable.
    // If it is missing, raylib's built-in font is used.
    Font uiFont = GetFontDefault();
    bool ownsUiFont = false;
    if (FileExists("mathfont.ttf")) {
        Font candidate = LoadFontEx("mathfont.ttf", 40, nullptr, 0);
        if (candidate.texture.id != 0) { uiFont = candidate; ownsUiFont = true; }
    }
    auto drawText = [&](const char* text, float x, float y, float size, Color color) {
        DrawTextEx(uiFont, text, {x, y}, size, 1.0f, color);
    };

    Texture2D tex[2] = {loadStep("smeagol/step1", 0), loadStep("smeagol/step2", 1)};
    Sound st[2] = {stepSound(0.25f), stepSound(0.40f)};
    Sound sChime = chimeSound(), sGrowl = growlSound();

    // ---------- world ----------
    const Vector3 LH = {0, 0, -44};
    std::vector<Col> cols; std::vector<Bld> blds; std::vector<Tree> trees;
    for (int i = 0; i < 16; i++) {
        float a = rnd() * 6.28f, r = 10 + rnd() * 45, x = cosf(a) * r, z = sinf(a) * r;
        if (hypotf(x, z + 44) < 10) continue;
        float w = 3 + rnd() * 2.5f, h = 2.6f + rnd() * 1.5f;
        blds.push_back({{x, 0, z}, w, h});
        cols.push_back({x, z, w * 0.75f});
    }
    for (int i = 0; i < 40; i++) {
        float a = rnd() * 6.28f, r = 6 + rnd() * 58, x = cosf(a) * r, z = sinf(a) * r;
        if (hypotf(x, z + 44) < 8) continue;
        trees.push_back({{x, 0, z}, (i % 2) == 1});
        cols.push_back({x, z, 0.7f});
    }
    cols.push_back({LH.x, LH.z - 3, 2.6f});

    bool got[NSHEETS];
    for (bool& g : got) g = false;
    bool sheetOpen = false, examOpen = false;
    int activeSheet = -1, examIndex = 0, examScore = 0;

    // ---------- final test: 6 questions drawn from all 12 sheets ----------
    const int NEXAM = 6;
    const char* examQuestions[NEXAM] = {
        "FINAL TEST 1/6: A player moves 6 units east and 8 north. Distance?",
        "FINAL TEST 2/6: Why multiply velocity by delta time?",
        "FINAL TEST 3/6: A normalized direction vector has what length?",
        "FINAL TEST 4/6: Dot product of (2, 0) and (0, 5)?",
        "FINAL TEST 5/6: Three rolls are 3, 6 and 9. Their average?",
        "FINAL TEST 6/6: Circles of radius 2 and 4 collide when distance < ?"
    };
    const char* examOptions[NEXAM][3] = {
        {"10 units","14 units","48 units"},
        {"To depend on FPS","To get displacement","To change direction"},
        {"0","1","100"},
        {"0","10","-10"},
        {"5","6","9"},
        {"2","6","8"}
    };
    const int examAnswers[NEXAM] = {0, 1, 1, 0, 1, 1};

    // ---------- state ----------
    State state = MENU;
    int diff = 1, hp = 3, nGot = 0, foot = 0;
    float px = 0, pz = 38, yaw = 0, pitch = 0, stepAcc = 0, inv = 0, toastT = 0;
    // two souls: mode 1 = Smeagol, 0 = Gollum. stir = strength of the soul that is waiting.
    int mode = 1;
    float stir = 0, tDom = 0, bal = 1;
    // Gollum's humor: karma > 0 (jokes landed) -> gollumized Smeagol (gol);
    //                 karma < 0 (jokes backfired) -> stronger echoing Smeagol (ech).
    float jokeCd = 0, karma = 0, gol = 0, ech = 0, flash = 0, echoT = -1, scoff = 0, sealT = 0;
    std::string toast, endMsg;
    Enemy E[2];
    Camera3D cam = {};
    cam.up = {0, 1, 0}; cam.fovy = 70; cam.projection = CAMERA_PERSPECTIVE;

    auto say = [&](const char* s) { toast = s; toastT = 3.f; };
    auto reset = [&]() {
        px = 0; pz = 38; yaw = 0; pitch = 0; nGot = 0; hp = CFG[diff].hp; inv = 0; stepAcc = 0;
        mode = 1; stir = 0; tDom = 0; bal = 1; jokeCd = 0; karma = 0; gol = 0; ech = 0; flash = 0; echoT = -1;
        for (bool& g : got) g = false;
        sheetOpen = examOpen = false; activeSheet = -1;
        examIndex = examScore = 0;
        E[0] = {{-35, 0, 5}, false, 0, 0, 0, 0};
        E[1] = {{35, 0, -5}, true, 0, 0, 0, 0};
        say("Find all 12 maths sheets. Learn, practise, and stay quiet.");
    };
    auto ctext = [&](const char* s, int y, int sz, Color c) {
        float width=MeasureTextEx(uiFont,s,(float)sz,1.f).x;
        drawText(s,(GetScreenWidth()-width)/2.f,(float)y,(float)sz,c);
    };

    while (!WindowShouldClose()) {
        float dt = std::min(GetFrameTime(), 0.05f);
        float t = (float)GetTime();
        bool moving = false, loud = false;
        float nearest = 1e9f;
         int ROOM_X_POS=LH.x,ROOM_Z_POS=LH.z-4;

        if (state != PLAY) {
            if (IsKeyPressed(KEY_ONE)) diff = 0;
            if (IsKeyPressed(KEY_TWO)) diff = 1;
            if (IsKeyPressed(KEY_THREE)) diff = 2;
            if (IsKeyPressed(KEY_ENTER)) { reset(); state = PLAY; DisableCursor(); }
        } else {
            if (sheetOpen) {
                for (int k=0; k<3; ++k) if (IsKeyPressed((KeyboardKey)(KEY_ONE+k))) {
                    if (k == SHEETS[activeSheet].answer) { sheetOpen=false; say("Correct! The information sheet is in your journal."); }
                    else say("Not quite. Read the sheet and try again.");
                }
            } else if (examOpen) {
                for (int k=0; k<3; ++k) if (IsKeyPressed((KeyboardKey)(KEY_ONE+k))) {
                    if (k == examAnswers[examIndex]) {
                        examScore++; examIndex++;
                        if (examIndex >= NEXAM) {
                            examOpen=false;
                            if (examScore == NEXAM) { state=WIN; EnableCursor(); endMsg="The final maths test is passed. The Diamond of Math is yours!"; }
                            else { examIndex=examScore=0; say("The final test was not perfect. Return to the lighthouse and try again."); }
                        }
                    } else { examOpen=false; examIndex=examScore=0; say("A wrong answer! Return to the lighthouse and press F to retry."); }
                }
            } else {
            if (IsKeyPressed(KEY_ESCAPE)) { state = MENU; EnableCursor(); }
            if (IsKeyPressed(KEY_E) && !mode) {
                stir = std::min(1.f, stir + 0.25f);
                say("Smeagol resists Gollum's outburst. Curiosity grows.");
            }

            // look
            Vector2 md = GetMouseDelta();
            yaw += md.x * 0.0025f;
            pitch = Clamp(pitch - md.y * 0.0025f, -1.f, 1.f);
            if (IsKeyDown(KEY_LEFT))  yaw -= dt * 2;
            if (IsKeyDown(KEY_RIGHT)) yaw += dt * 2;

            // input
            loud = IsKeyDown(KEY_LEFT_SHIFT);
            float fx = 0, fz = 0;
            if (IsKeyDown(KEY_W)) fz += 1;
            if (IsKeyDown(KEY_Q)) fz += 2;

            if (IsKeyDown(KEY_S)) fz -= 0.6f;
            if (IsKeyDown(KEY_D)) fx += 0.7f;
            if (IsKeyDown(KEY_A)) fx -= 0.7f;
            moving = (fx != 0 || fz != 0);

            // ---- the see-saw: the longer one soul prevails, the stronger the other stirs ----
            tDom += dt;
            float mm = mode ? (moving ? 0.8f : 1.5f) : (moving ? 1.4f : 0.8f);
            stir = std::min(1.f, stir + (0.03f + 0.006f * tDom) * CFG[diff].sw * mm *
                                     (mode ? 1 - 0.5f * ech + 0.8f * gol : 1.f) * dt);
            jokeCd -= dt; scoff -= dt; sealT -= dt;
            flash = std::max(0.f, flash - dt * 1.2f);
            if (echoT > 0) {
                echoT -= dt;
                if (echoT <= 0) { SetSoundVolume(st[foot], 0.6f * ech); PlaySound(st[foot]); }
            }
            if (stir >= 1) {                         // Gollum's climax / the swap
                mode ^= 1; stir = 0; tDom = 0; flash = 1; PlaySound(sGrowl);
                if (mode) {                          // consequences of the humor shape Smeagol
                    gol = std::max(0.f, karma); ech = std::max(0.f, -karma); karma = 0;
                    if (gol > 0.2f)      say("Gollum's joke landed too well... a gollumized Smeagol crawls out.");
                    else if (ech > 0.2f) say("The joke cost dearly. Smeagol returns, and he echoes.");
                    else                 say(QUIP[1][GetRandomValue(0, 2)]);
                } else {
                    gol = ech = 0; karma = 0; jokeCd = 0;
                    say(TextFormat("%s  [E: tell a joke]", QUIP[0][GetRandomValue(0, 2)]));
                }
            }
            bal += ((mode ? 1 - 0.35f * stir - 0.4f * gol : 0.35f * stir) - bal) * std::min(1.f, dt * 1.5f);

            // move
            float sp = (loud ? 5.2f : 3.f) * (0.6f + 0.55f * bal) * (mode ? 1 + 0.25f * ech - 0.15f * gol : 1.f);
            if (moving) {
                float fwx = sinf(yaw), fwz = -cosf(yaw), rx = cosf(yaw), rz = sinf(yaw);
                float dx = (rx * fx + fwx * fz) * sp * dt, dz = (rz * fx + fwz * fz) * sp * dt;
                px += dx; pz += dz;
                stepAcc += hypotf(dx, dz);
                if (stepAcc > CFG[diff].stride) {          // one step = swap photo + play sound
                    stepAcc = 0; foot ^= 1;
                    SetSoundVolume(st[foot], loud ? 1.f : 0.55f);
                    PlaySound(st[foot]);
                    if (mode && ech > 0.2f) echoT = 0.2f;  // the echo of a stronger Smeagol
                }
            }
            float rr = hypotf(px, pz);
            if (rr > 66) { px *= 66 / rr; pz *= 66 / rr; }
            for (auto& c : cols) {
                float dx = px - c.x, dz = pz - c.z, d = hypotf(dx, dz);
                if (d < c.r + 0.5f && d > 0.0001f) { px = c.x + dx / d * (c.r + 0.5f); pz = c.z + dz / d * (c.r + 0.5f); }
            }
            float rd=hypotf(px-ROOM_X_POS.x,pz-ROOM_Z_POS.z);

            // Collect all 12 information sheets. Each opens a lesson and an exercise.
            for (int i=0;i<NSHEETS;i++) {
                float sd=hypotf(px-SHEET_POS[i].x,pz-SHEET_POS[i].z);
                if (!got[i] && mode && sd<1.8f+2.f*ech) {
                    got[i]=true; nGot++; activeSheet=i; sheetOpen=true; PlaySound(sChime);
                    say("A maths information sheet discovered!");
                } else if (!got[i] && !mode && sd<2.2f && scoff<=0) {
                    say("Gollum cannot study. Resist the outburst and return as Smeagol."); scoff=3;
                }
            }

            if (hypotf(px-LH.x,pz-LH.z)<2.6f) {
                if (nGot == NSHEETS && mode) {
                    if (!examOpen && IsKeyPressed(KEY_F)) { examOpen=true; examIndex=examScore=0; PlaySound(sChime); }
                    else if (!examOpen && sealT<=0) { say("All twelve sheets found. Press F to begin the final test."); sealT=3; }
                } else if (sealT<=0 && !examOpen) {
                    say(nGot<NSHEETS ? "The final test is sealed. Find all twelve information sheets." :
                         "The Diamond requires Smeagol. Resist Gollum and return.");
                    sealT=3;
                }
            }

            // enemies
            float det = CFG[diff].det * (loud ? 1.6f : 1.f) * (moving ? 1.f : 0.6f) * (0.3f + 0.9f * bal) *
                        (mode ? 1 - 0.5f * gol + 0.4f * ech : 1.f);
            inv -= dt;
            for (auto& e : E) {
                float dx = px - e.p.x, dz = pz - e.p.z, d = hypotf(dx, dz);
                nearest = std::min(nearest, d);
                bool enraged = e.rage > 0, stunned = e.stun > 0;
                bool hunt = d < det || enraged;
                float v = CFG[diff].spd * (e.gamble ? 1.f : 0.75f) * (hunt ? 1.f : 0.35f) * (enraged ? 1.8f : 1.f);
                if (enraged) e.rage -= dt;
                if (stunned) { e.stun -= dt; v = 0; }
                if (e.gamble) {
                    e.blink -= dt; e.spin += dt;
                    if (e.blink <= 0 && hunt && !stunned) {      // the Gamble blinks next to you
                        e.blink = 3.2f;
                        float a = GetRandomValue(0, 628) / 100.f;
                        e.p.x = px + cosf(a) * 8; e.p.z = pz + sinf(a) * 8;
                        PlaySound(sGrowl);
                    }
                }
                if (d > 0.01f) { e.p.x += dx / d * v * dt; e.p.z += dz / d * v * dt; }
                if (d < 1.3f && inv <= 0 && !stunned) {
                    hp--; inv = 2.2f; PlaySound(sGrowl);
                    if (!mode) karma = std::max(-1.f, karma - 0.3f);   // a bad consequence of Gollum's ways
                    e.p.x = px + (GetRandomValue(0, 1) ? -40.f : 40.f);
                    e.p.z = pz + (GetRandomValue(0, 1) ? -30.f : 30.f);
                    say(e.gamble ? "The Gamble takes its cut." : "Solitude closes in...");
                    if (hp <= 0) { state = LOSE; EnableCursor(); endMsg = "The shadows keep you. The Precious stays unclaimed."; }
                }
            }
            } // end normal gameplay update; study and exam overlays pause the world
            if (toastT > 0) toastT -= dt;
        }

        // ---------- draw ----------
        float shake = flash * 0.3f;
        cam.position = {px + GetRandomValue(-100, 100) / 100.f * shake, 1.7f + GetRandomValue(-100, 100) / 100.f * shake * 0.6f, pz};
        cam.fovy = 62 + 12 * bal + 6 * ech;
        Vector3 fwd = {sinf(yaw) * cosf(pitch), sinf(pitch), -cosf(yaw) * cosf(pitch)};
        cam.target = Vector3Add(cam.position, fwd);

        BeginDrawing();
        ClearBackground({244, 201, 143, 255});
        BeginMode3D(cam);
        DrawPlane({0, -0.4f, 0}, {800, 800}, {37, 90, 153, 255});           // sea
        DrawCylinder({0, -0.2f, 0}, 70, 70, 0.2f, 48, {227, 196, 142, 255}); // island
        for (auto& b : blds) {
            DrawCube({b.p.x, b.h / 2, b.p.z}, b.w, b.h, b.w, {251, 246, 234, 255});
            DrawCylinder({b.p.x, b.h, b.p.z}, 0, b.w * 0.8f, 1.4f, 4, {194, 85, 47, 255});
        }
        for (auto& tr : trees) {
            if (tr.cypress) DrawCylinder({tr.p.x, 0, tr.p.z}, 0, 0.7f, 6.f, 8, {47, 90, 51, 255});
            else {
                DrawCylinder({tr.p.x, 0, tr.p.z}, 0.18f, 0.25f, 1.6f, 6, {107, 74, 46, 255});
                DrawSphere({tr.p.x, 2.3f, tr.p.z}, 1.3f, {138, 154, 90, 255});
            }
        }
        //function void DrawCube(Vector3 position, float width, float height, float length, Color color)


                 //Rest door
        DrawCube({LH.x, 0, LH.z - 4},0.9f,3.0f,0.5f, Color{164,102,42});
        DrawCube({LH.x, 0, LH.z - 4},1.1f,3.2f,0.5f, Color{106,58,10});

        DrawCylinder({LH.x, 0, LH.z - 3}, 1.6f, 2.4f, 16, 12, WHITE);          // lighthouse
        DrawCylinder({LH.x, 5.5f, LH.z - 3}, 2.0f, 1.95f, 3, 12, {194, 59, 59, 255});
        float dy = 1.0f + sinf(t * 1.7f) * 0.15f;                              // the Diamond of math
        Color dc = nGot >= NSHEETS ? Color{255, 77, 120, 255} : Color{120, 60, 80, 255};
        DrawCylinder({LH.x, dy, LH.z}, 0.5f, 0, 0.8f, 6, dc);
        DrawCylinder({LH.x, dy + 0.8f, LH.z}, 0, 0.5f, 0.6f, 6, dc);
        for (int i=0;i<NSHEETS;i++) if (!got[i]) {
            Vector3 p={SHEET_POS[i].x,SHEET_POS[i].y+sinf(t*2+i)*0.1f,SHEET_POS[i].z};
            DrawCube(p,0.9f,0.12f,0.65f,{244,225,166,255});
            DrawSphere(Vector3Add(p,{0,0.35f,0}),0.13f,{25,82,106,255});
        }
        for (auto& e : E) {
            float wob = e.stun > 0 ? sinf(t * 40.f) * 0.15f : 0.f;           // stunned = laughing
            Color tint = e.rage > 0 ? Color{120, 10, 10, 255} : Color{13, 10, 20, 255};
            if (!e.gamble) {
                DrawCylinder({e.p.x + wob, 0, e.p.z}, 0, 0.8f, 3.2f, 10, tint);
                float dx = px - e.p.x, dz = pz - e.p.z, d = std::max(0.001f, hypotf(dx, dz));
                float ux = dx / d, uz = dz / d;
                for (int s = -1; s <= 1; s += 2)
                    DrawSphere({e.p.x + ux * 0.4f - uz * 0.2f * s, 2.6f, e.p.z + uz * 0.4f + ux * 0.2f * s}, 0.07f,
                               e.rage > 0 ? Color{255, 80, 60, 255} : Color{154, 208, 255, 255});
            } else {
                rlPushMatrix();
                rlTranslatef(e.p.x + wob, 1.2f + fabsf(sinf(t * 3.f)) * 0.4f, e.p.z);
                rlRotatef(e.spin * 120.f, 1, 1, 0);
                DrawCube({0, 0, 0}, 1, 1, 1, e.rage > 0 ? Color{200, 20, 20, 255} : Color{122, 15, 28, 255});
                DrawCubeWires({0, 0, 0}, 1.02f, 1.02f, 1.02f, GOLD);
                rlPopMatrix();
            }
        }
        EndMode3D();

        int W = GetScreenWidth(), Hs = GetScreenHeight();
        if(state == REST) {
             DrawRectangle(0, 0, W, Hs, Fade({70, 80, 50, 255}, (1 - bal) * 0.45f));
            if (gol > 0) DrawRectangle(0, 0, W, Hs, Fade({150, 110, 60, 255}, 0.25f * gol));
            if (ech > 0) DrawRectangle(0, 0, W, Hs, Fade({255, 120, 160, 255}, 0.12f * ech));
            // photo "body": swaps with each step; hunched when gollumized, ghosted when echoing
            Texture2D b = tex[foot], g2 = tex[foot ^ 1];
            float sc = Hs * 0.5f / b.height;
            float dw = b.width * sc * (1 + 0.08f * ech), dh = b.height * sc * (1 - 0.2f * gol);
            float bob = moving ? sinf(t * 9.f) * 5.f : 0.f;
            float bx = (W - dw) / 2, by = Hs - dh + bob;
            Color body = {255, (unsigned char)(255 - 70 * gol), (unsigned char)(255 - 140 * gol), 255};
            if (ech > 0.05f)
                DrawTexturePro(g2, {0, 0, (float)g2.width, (float)g2.height}, {bx - 30 * ech, by, dw, dh}, {0, 0}, 0, Fade(PINK, 0.35f * ech));
            DrawTexturePro(b, {0, 0, (float)b.width, (float)b.height}, {bx, by, dw, dh}, {0, 0}, 0, body);

            DrawRectangle(0, 0, W, Hs, Fade({42, 0, 16, 255}, std::max(0.f, 1.f - nearest / 14.f) * 0.55f));
                        drawText("Main objective: keep the karma in the green"+"\n"+"while resting doing the activities that better karma bring."+ "\n"+
                        "Once rested competelly you will be redirected to the action",
                         (float)W-230, 12, 22, RAYWHITE);

            
        }
        if (state == PLAY) {
            // soul tints: Gollum = sick green-grey, gollumized = sepia, echo = rosy glow
            DrawRectangle(0, 0, W, Hs, Fade({70, 80, 50, 255}, (1 - bal) * 0.45f));
            if (gol > 0) DrawRectangle(0, 0, W, Hs, Fade({150, 110, 60, 255}, 0.25f * gol));
            if (ech > 0) DrawRectangle(0, 0, W, Hs, Fade({255, 120, 160, 255}, 0.12f * ech));

            // photo "body": swaps with each step; hunched when gollumized, ghosted when echoing
            Texture2D b = tex[foot], g2 = tex[foot ^ 1];
            float sc = Hs * 0.5f / b.height;
            float dw = b.width * sc * (1 + 0.08f * ech), dh = b.height * sc * (1 - 0.2f * gol);
            float bob = moving ? sinf(t * 9.f) * 5.f : 0.f;
            float bx = (W - dw) / 2, by = Hs - dh + bob;
            Color body = {255, (unsigned char)(255 - 70 * gol), (unsigned char)(255 - 140 * gol), 255};
            if (ech > 0.05f)
                DrawTexturePro(g2, {0, 0, (float)g2.width, (float)g2.height}, {bx - 30 * ech, by, dw, dh}, {0, 0}, 0, Fade(PINK, 0.35f * ech));
            DrawTexturePro(b, {0, 0, (float)b.width, (float)b.height}, {bx, by, dw, dh}, {0, 0}, 0, body);

            DrawRectangle(0, 0, W, Hs, Fade({42, 0, 16, 255}, std::max(0.f, 1.f - nearest / 14.f) * 0.55f));
            drawText(nGot < NSHEETS ? TextFormat("Math sheets %d/%d", nGot, NSHEETS) : "Final test unlocked at the lighthouse", 14, 12, 22, RAYWHITE);
            drawText(TextFormat("Lives: %d   [%s]", hp, CFG[diff].name), (float)W-230, 12, 22, RAYWHITE);

            // balance bar: Gollum <-> Smeagol
            int bw = 300, bx0 = (W - bw) / 2;
            drawText("GOLLUM",(float)(bx0-74),38,14,RAYWHITE);
            drawText("SMEAGOL",(float)(bx0+bw+8),38,14,RAYWHITE);
            DrawRectangleGradientH(bx0, 42, bw, 8, {93, 107, 58, 255}, {224, 82, 106, 255});
            DrawRectangle((int)(bx0 + bal * bw) - 3, 38, 6, 16, WHITE);

            if (mode) {
                // Smeagol's curiosity compass: points to the nearest missing sheet, or the Diamond.
                float tx = LH.x, tz = LH.z, bd = 1e9f;
                if (nGot < NSHEETS) for (int i=0;i<NSHEETS;i++) if (!got[i]) {
                    float d=hypotf(SHEET_POS[i].x-px,SHEET_POS[i].z-pz);
                    if (d<bd) { bd=d; tx=SHEET_POS[i].x; tz=SHEET_POS[i].z; }
                }
                float f = atan2f(sinf(yaw), -cosf(yaw));
                float rel = atan2f(tx - px, tz - pz) - f + sinf(t * 3.3f) * gol * 1.4f;  // gollumized = faulty compass
                Vector2 c = {(float)W / 2, 98}, tip = {c.x - sinf(rel) * 26, c.y - cosf(rel) * 26};
                DrawLineEx(c, tip, 4, {224, 82, 106, 255});
                DrawCircleV(tip, 6, {255, 120, 150, 255});
                const char* nm = gol > 0.2f ? "Gollumized Smeagol" : ech > 0.2f ? "Echoing Smeagol" : "Smeagol";
                ctext(TextFormat("%s  %dm  |  Gollum creeps %d%%", nm, (int)hypotf(tx - px, tz - pz), (int)(stir * 100)), 130, 18, RAYWHITE);
            } else {
                ctext(TextFormat("Gollum: sluggish, shadows ignore you  |  [E] resist outburst  |  Smeagol stirs %d%%",
                                 (int)(stir*100)),70,18,RAYWHITE);
            }
            if (sheetOpen || examOpen) {
                DrawRectangle(0,0,W,Hs,Fade(BLACK,0.78f));
                DrawRectangle(W/2-390,Hs/2-205,780,410,{35,29,24,245});
                DrawRectangleLines(W/2-390,Hs/2-205,780,410,GOLD);
                if (sheetOpen) {
                    const MathSheet& sh=SHEETS[activeSheet];
                    ctext(sh.title,Hs/2-175,25,GOLD);
                    ctext(sh.lesson,Hs/2-125,18,RAYWHITE);
                    ctext(sh.formula,Hs/2-88,23,{120,220,220,255});
                    ctext(sh.code,Hs/2-52,18,{180,220,170,255});
                    ctext(sh.exercise,Hs/2-12,18,RAYWHITE);
                    for(int k=0;k<3;k++) ctext(TextFormat("%d. %s",k+1,sh.option[k]),Hs/2+35+k*34,18,{246,231,200,255});
                    ctext("Press 1, 2 or 3 to answer. The world is paused while you study.",Hs/2+155,16,GRAY);
                } else {
                    ctext("THE DIAMOND OF MATH: FINAL TEST",Hs/2-175,25,GOLD);
                    ctext(TextFormat("Question %d of %d   (score %d)", examIndex + 1, NEXAM, examScore), Hs/2-135, 18, GOLD);
                    ctext(examQuestions[examIndex],Hs/2-95,20,RAYWHITE);
                    for(int k=0;k<3;k++) ctext(TextFormat("%d. %s",k+1,examOptions[examIndex][k]),Hs/2-25+k*42,20,{246,231,200,255});
                    ctext("Answer all six correctly. A failed attempt can be restarted with F.",Hs/2+145,17,GRAY);
                }
            }
            if (toastT>0 && !sheetOpen && !examOpen) ctext(toast.c_str(),Hs/4,24,RAYWHITE);
            if (flash > 0) DrawRectangle(0, 0, W, Hs, Fade(WHITE, flash * 0.8f));
        } else {
            DrawRectangle(0, 0, W, Hs, Fade(BLACK, 0.8f));
            int y = Hs / 2 - 190;
            ctext("SMEAGOL'S PRECIOUS", y, 44, {246, 231, 200, 255});
            if (state != MENU) ctext(endMsg.c_str(), y + 60, 22, {224, 82, 106, 255});
            ctext("Find all 12 maths information sheets, complete their exercises, then take the final test.",y+100,20,RAYWHITE);
            ctext("Smeagol's curiosity helps him study. Gollum is quieter and safer, but cannot collect knowledge.",y+128,18,RAYWHITE);
            ctext("When Gollum prevails, press E to resist his humorous outburst and hasten Smeagol's return.",y+154,18,RAYWHITE);
            ctext("The sheets teach geometry, trigonometry, vectors, matrices, statistics,",y+178,18,RAYWHITE);
            ctext("logarithms, modulo, interpolation and collision detection in programming.",y+200,18,RAYWHITE);
            ctext(TextFormat("Difficulty (1/2/3):  %s", CFG[diff].name), y + 240, 26, GOLD);
            ctext("Put step1.png and step2.png beside the game for your own photos.", y + 280, 18, GRAY);
            ctext("ENTER = start    W/A/S/D move    SHIFT sprint    E = resist outburst    F = final test",y+310,20,RAYWHITE);
        }
        EndDrawing();
    }
    UnloadTexture(tex[0]); UnloadTexture(tex[1]);
    for (auto& s : st) UnloadSound(s);
    UnloadSound(sChime); UnloadSound(sGrowl);
    if (ownsUiFont) UnloadFont(uiFont);
    CloseAudioDevice(); CloseWindow();
    return 0;
}
