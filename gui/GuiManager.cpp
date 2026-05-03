/*
 * =============================================================================
 * FILE:    gui/GuiManager.cpp
 * PROJECT: ECU Smart Home — GUI Layer (Dark Neon / Cyberpunk)
 * AUTHOR:  Eng. Zain  (Lead Architect — ID 692500788)
 * =============================================================================
 * Architecture: Bridge Pattern — GuiManager is the ONLY class allowed to
 * include device headers. src/ and include/ are completely unaware of the GUI.
 * =============================================================================
 */

#include "GuiManager.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <chrono>
#include <sstream>
#include <iomanip>

// ============================================================================
// CONSTRUCTOR / DESTRUCTOR
// ============================================================================
GuiManager::GuiManager()
{
    // --- Instantiate core devices (these never know the GUI exists) ----------
    m_frontDoor    = std::make_shared<SmartDoor>("DOOR-01",  "Front Door",  true);
    m_backDoor     = std::make_shared<SmartDoor>("DOOR-02",  "Back Door",   true);
    m_livingLight  = std::make_shared<SmartLight>("LIGHT-01","Living Room",  80);
    m_bedroomLight = std::make_shared<SmartLight>("LIGHT-02","Bedroom",      60);
    m_kitchenLight = std::make_shared<SmartLight>("LIGHT-03","Kitchen",      70);
    m_mainAC       = std::make_shared<SmartAC>("AC-01", "Main AC",
                                               SmartAC::Mode::COOLING, 22.0);
    m_frontCamera  = std::make_shared<SmartCamera>("CAM-01", "Front Camera");
    m_backCamera   = std::make_shared<SmartCamera>("CAM-02", "Back Camera");

    // --- Power on -----------------------------------------------------------
    m_frontDoor->turnOn();    m_backDoor->turnOn();
    m_livingLight->turnOn();  m_bedroomLight->turnOn(); m_kitchenLight->turnOn();
    m_mainAC->turnOn();
    m_frontCamera->turnOn();  m_backCamera->turnOn();

    // --- Wire up UI panels --------------------------------------------------
    m_doorPanel   = std::make_unique<DoorPanel>(m_frontDoor, m_backDoor);
    m_lightPanel  = std::make_unique<LightPanel>(m_livingLight, m_bedroomLight, m_kitchenLight);
    m_acPanel     = std::make_unique<ACPanel>(m_mainAC);
    m_cameraPanel = std::make_unique<CameraPanel>(m_frontCamera, m_backCamera);

    // --- Initialise credential buffers to empty -----------------------------
    std::memset(m_usernameInput, 0, sizeof(m_usernameInput));
    std::memset(m_pinInput,      0, sizeof(m_pinInput));
}

GuiManager::~GuiManager() { shutdown(); }

// ============================================================================
// init()  —  SDL2 window + OpenGL context + Dear ImGui
// ============================================================================
bool GuiManager::init()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "[GuiManager] SDL_Init: %s\n", SDL_GetError());
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,        0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,  1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,   24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE,  8);

    m_window = SDL_CreateWindow(
        "ECU SMART HOME  //  NEON DASHBOARD  v1.0",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        m_winW, m_winH,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
    );
    if (!m_window) {
        fprintf(stderr, "[GuiManager] SDL_CreateWindow: %s\n", SDL_GetError());
        return false;
    }

    m_glContext = SDL_GL_CreateContext(m_window);
    if (!m_glContext) {
        fprintf(stderr, "[GuiManager] GL context: %s\n", SDL_GetError());
        return false;
    }
    SDL_GL_MakeCurrent(m_window, m_glContext);
    SDL_GL_SetSwapInterval(1); // vsync

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename  = nullptr; // no imgui.ini clutter

    applyTheme();

    ImGui_ImplSDL2_InitForOpenGL(m_window, m_glContext);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    logEvent("System initialised — ECU Smart Home v1.0");
    logEvent("8 devices online. Awaiting authentication...");
    m_running = true;
    return true;
}

// ============================================================================
// applyTheme()  —  Dark Neon / Cyberpunk colour palette
// ============================================================================
void GuiManager::applyTheme()
{
    ImGuiStyle& s = ImGui::GetStyle();
    s.WindowRounding     = 8.0f;
    s.ChildRounding      = 8.0f;
    s.FrameRounding      = 5.0f;
    s.GrabRounding       = 5.0f;
    s.ScrollbarRounding  = 6.0f;
    s.PopupRounding      = 5.0f;
    s.TabRounding        = 5.0f;
    s.WindowBorderSize   = 1.5f;
    s.FrameBorderSize    = 1.0f;
    s.ItemSpacing        = { 8.0f, 7.0f };
    s.FramePadding       = { 8.0f, 5.0f };
    s.WindowPadding      = { 14.0f, 14.0f };
    s.ScrollbarSize      = 12.0f;
    s.GrabMinSize        = 10.0f;

    ImVec4* c = s.Colors;
    using namespace CyberpunkTheme;

    c[ImGuiCol_WindowBg]             = BG_DARKEST;
    c[ImGuiCol_ChildBg]              = BG_PANEL;
    c[ImGuiCol_PopupBg]              = BG_PANEL;
    c[ImGuiCol_Border]               = CYAN_DIM;
    c[ImGuiCol_BorderShadow]         = { 0,0,0,0 };
    c[ImGuiCol_FrameBg]              = BG_WIDGET;
    c[ImGuiCol_FrameBgHovered]       = { 0.10f, 0.10f, 0.22f, 1.0f };
    c[ImGuiCol_FrameBgActive]        = { 0.14f, 0.14f, 0.30f, 1.0f };
    c[ImGuiCol_TitleBg]              = { 0.03f, 0.03f, 0.08f, 1.0f };
    c[ImGuiCol_TitleBgActive]        = { 0.00f, 0.25f, 0.35f, 1.0f };
    c[ImGuiCol_ScrollbarBg]          = BG_DARKEST;
    c[ImGuiCol_ScrollbarGrab]        = CYAN_DIM;
    c[ImGuiCol_ScrollbarGrabHovered] = CYAN;
    c[ImGuiCol_ScrollbarGrabActive]  = PURPLE;
    c[ImGuiCol_CheckMark]            = CYAN;
    c[ImGuiCol_SliderGrab]           = CYAN;
    c[ImGuiCol_SliderGrabActive]     = PURPLE;
    c[ImGuiCol_Button]               = { 0.05f, 0.12f, 0.20f, 1.0f };
    c[ImGuiCol_ButtonHovered]        = { 0.00f, 0.55f, 0.75f, 1.0f };
    c[ImGuiCol_ButtonActive]         = PURPLE;
    c[ImGuiCol_Header]               = { 0.10f, 0.18f, 0.28f, 1.0f };
    c[ImGuiCol_HeaderHovered]        = CYAN_DIM;
    c[ImGuiCol_HeaderActive]         = PURPLE_DIM;
    c[ImGuiCol_Tab]                  = { 0.05f, 0.08f, 0.15f, 1.0f };
    c[ImGuiCol_TabHovered]           = CYAN_DIM;
    c[ImGuiCol_TabActive]            = { 0.05f, 0.30f, 0.42f, 1.0f };
    c[ImGuiCol_TabUnfocused]         = { 0.03f, 0.05f, 0.10f, 1.0f };
    c[ImGuiCol_TabUnfocusedActive]   = { 0.05f, 0.18f, 0.25f, 1.0f };
    c[ImGuiCol_PlotLines]            = CYAN;
    c[ImGuiCol_PlotHistogram]        = PURPLE;
    c[ImGuiCol_Text]                 = TEXT_PRIMARY;
    c[ImGuiCol_TextDisabled]         = TEXT_DIM;
    c[ImGuiCol_Separator]            = CYAN_DIM;
    c[ImGuiCol_SeparatorHovered]     = CYAN;
    c[ImGuiCol_SeparatorActive]      = PURPLE;
}

// ============================================================================
// speakFemale()  —  text-to-speech via espeak (female voice en+f3)
// ============================================================================
void GuiManager::speakFemale(const char* text)
{
    if (!m_voiceEnabled) return;
    // Use espeak with a female voice profile; -s 145 = slightly slower pacing
    char cmd[512];
    const char* voice = m_femaleVoice ? "en+f3" : "en";
    snprintf(cmd, sizeof(cmd),
             "espeak -v %s -s 145 \"%s\" &", voice, text);
    (void)system(cmd);
}

// ============================================================================
// logEvent()  —  timestamped ring buffer
// ============================================================================
void GuiManager::logEvent(const std::string& msg)
{
    auto now = std::chrono::system_clock::now();
    auto t   = std::chrono::system_clock::to_time_t(now);
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&t), "[%H:%M:%S] ") << msg;
    m_logLines[m_logHead % LOG_CAPACITY] = oss.str();
    ++m_logHead;
    if (m_logCount < LOG_CAPACITY) ++m_logCount;
}

// ============================================================================
// renderGlowRect()  —  neon border glow effect
// ============================================================================
void GuiManager::renderGlowRect(const ImVec2& pos, const ImVec2& size,
                                 const ImVec4& colour, float thickness)
{
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec4 glow = colour; glow.w = 0.22f;
    dl->AddRect({ pos.x-3, pos.y-3 }, { pos.x+size.x+3, pos.y+size.y+3 },
                ImGui::ColorConvertFloat4ToU32(glow), 8.0f, 0, thickness+4.0f);
    dl->AddRect(pos, { pos.x+size.x, pos.y+size.y },
                ImGui::ColorConvertFloat4ToU32(colour), 8.0f, 0, thickness);
}

// ============================================================================
// statusBadge()  —  coloured ● indicator
// ============================================================================
void GuiManager::statusBadge(bool on)
{
    using namespace CyberpunkTheme;
    ImGui::TextColored(on ? GREEN_ON : RED_OFF, on ? "● ON " : "● OFF");
}

// ============================================================================
// renderLoginScreen()  —  TASK 1: Secure Access Terminal
// ============================================================================
void GuiManager::renderLoginScreen()
{
    using namespace CyberpunkTheme;

    // Full-screen darkened backdrop
    ImGui::SetNextWindowPos({ 0, 0 });
    ImGui::SetNextWindowSize({ (float)m_winW, (float)m_winH });
    ImGui::SetNextWindowBgAlpha(1.0f);
    ImGui::Begin("##loginbg", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus);

    // ---- Animated scanline grid backdrop -----------------------------------
    ImDrawList* bgDL = ImGui::GetWindowDrawList();
    float t = m_animTime;
    for (int i = 0; i < m_winH; i += 32) {
        float alpha = 0.04f + 0.02f * std::sin(t * 1.5f + i * 0.02f);
        bgDL->AddLine({ 0, (float)i }, { (float)m_winW, (float)i },
                      IM_COL32(0, 243, 255, (int)(alpha * 255)), 1.0f);
    }
    for (int i = 0; i < m_winW; i += 48) {
        bgDL->AddLine({ (float)i, 0 }, { (float)i, (float)m_winH },
                      IM_COL32(0, 243, 255, 7), 1.0f);
    }

    // ---- Centre login card -------------------------------------------------
    const float cardW = 480.0f;
    const float cardH = 400.0f;
    float cx = ((float)m_winW - cardW) * 0.5f;
    float cy = ((float)m_winH - cardH) * 0.5f;

    ImGui::SetNextWindowPos({ cx, cy });
    ImGui::SetNextWindowSize({ cardW, cardH });
    ImGui::SetNextWindowBgAlpha(0.97f);
    ImGui::Begin("##logincard", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings);

    // Neon border glow
    ImVec2 cardPos = ImGui::GetWindowPos();
    ImVec2 cardSize = ImGui::GetWindowSize();
    float pulse = 0.6f + 0.4f * std::sin(t * 2.8f);
    ImVec4 glowCol = { 0.0f, pulse * 0.95f, pulse, 1.0f };
    renderGlowRect(cardPos, cardSize, glowCol, 2.0f);

    // ---- Header ------------------------------------------------------------
    ImGui::SetCursorPosY(24.0f);
    float titleW = cardW * 0.72f;
    ImGui::SetCursorPosX((cardW - titleW) * 0.5f);
    ImGui::PushStyleColor(ImGuiCol_Text, CYAN);
    ImGui::TextWrapped("  SECURE ACCESS TERMINAL");
    ImGui::PopStyleColor();

    ImGui::Spacing();
    ImGui::SetCursorPosX(20.0f);
    ImGui::TextColored(AMBER, "  SYSTEM LOCKED. ENTER CREDENTIALS.");
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // ---- ECU Logo badge ----------------------------------------------------
    float badgeX = (cardW - 200.0f) * 0.5f;
    ImGui::SetCursorPosX(badgeX);
    ImGui::TextColored(PURPLE, "[ ECU SMART HOME  //  NODE-NEXUS ]");
    ImGui::Spacing();
    ImGui::Spacing();

    // ---- Credentials form --------------------------------------------------
    ImGui::SetCursorPosX(40.0f);
    ImGui::TextColored(TEXT_DIM, "USERNAME");
    ImGui::SetCursorPosX(40.0f);
    ImGui::SetNextItemWidth(cardW - 80.0f);
    ImGui::InputText("##username", m_usernameInput, sizeof(m_usernameInput));

    ImGui::Spacing();
    ImGui::SetCursorPosX(40.0f);
    ImGui::TextColored(TEXT_DIM, "PIN  (4-digit)");
    ImGui::SetCursorPosX(40.0f);
    ImGui::SetNextItemWidth(cardW - 80.0f);
    ImGui::InputText("##pin", m_pinInput, sizeof(m_pinInput),
                     ImGuiInputTextFlags_Password | ImGuiInputTextFlags_CharsDecimal);

    ImGui::Spacing();
    ImGui::Spacing();

    // ---- Login error feedback ----------------------------------------------
    if (m_loginFailed && m_loginFailTimer > 0.0f) {
        ImGui::SetCursorPosX(40.0f);
        float blinkAlpha = std::fmod(m_loginFailTimer * 4.0f, 2.0f) < 1.0f ? 1.0f : 0.4f;
        ImGui::TextColored({ 1.0f, 0.18f, 0.18f, blinkAlpha },
                           "  ACCESS DENIED — INVALID CREDENTIALS");
        ImGui::Spacing();
    }

    // ---- AUTHENTICATE button -----------------------------------------------
    ImGui::SetCursorPosX(40.0f);
    ImGui::PushStyleColor(ImGuiCol_Button,        { 0.00f, 0.25f, 0.35f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.00f, 0.55f, 0.75f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  { 0.74f, 0.07f, 0.99f, 1.0f });

    bool enterPressed = ImGui::IsKeyPressed(ImGuiKey_Enter) ||
                        ImGui::IsKeyPressed(ImGuiKey_KeypadEnter);
    bool btnClicked   = ImGui::Button("[ AUTHENTICATE ]", { cardW - 80.0f, 40.0f });

    ImGui::PopStyleColor(3);

    if (btnClicked || enterPressed) {
        // Credentials: Username = Admin, PIN = 1234
        bool okUser = (std::string(m_usernameInput) == "Admin");
        bool okPin  = (std::string(m_pinInput)      == "1234");
        if (okUser && okPin) {
            m_loggedIn      = true;
            m_loginFailed   = false;
            m_loginFailTimer = 0.0f;
            logEvent("AUTHENTICATED — Welcome, Administrator.");
            speakFemale("Access granted. Welcome to ECU Smart Home.");
        } else {
            m_loginFailed    = true;
            m_loginFailTimer = 3.0f;
            speakFemale("Access denied. Invalid credentials.");
        }
    }

    // ---- Footer hint -------------------------------------------------------
    ImGui::Spacing();
    ImGui::SetCursorPosX(40.0f);
    ImGui::TextColored(TEXT_DIM, "  Default: Admin / 1234");

    ImGui::End(); // login card
    ImGui::End(); // login bg
}

// ============================================================================
// renderTopBar()  —  Fixed header strip with animated pulse
// ============================================================================
void GuiManager::renderTopBar()
{
    using namespace CyberpunkTheme;
    ImGuiIO& io = ImGui::GetIO();

    ImGui::SetNextWindowPos({ 0, 0 });
    ImGui::SetNextWindowSize({ (float)m_winW, 58.0f });
    ImGui::SetNextWindowBgAlpha(0.97f);
    ImGui::Begin("##topbar", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar  | ImGuiWindowFlags_NoSavedSettings);

    float pulse = 0.55f + 0.45f * std::sin(m_animTime * 2.5f);
    ImVec4 accent = { 0.0f, pulse * 0.95f, pulse, 1.0f };

    ImGui::SetCursorPosY(10.0f);
    ImGui::TextColored(accent, "  ECU SMART HOME");
    ImGui::SameLine();
    ImGui::TextColored(TEXT_DIM, "  //  NEON DASHBOARD v1.0");

    // Right side — controls
    float rightX = (float)m_winW - 440.0f;
    ImGui::SameLine(rightX);

    // Mobile view toggle
    ImGui::PushStyleColor(ImGuiCol_Button,
        m_mobileView ? ImVec4{0.00f,0.45f,0.60f,1.0f}
                     : ImVec4{0.05f,0.12f,0.20f,1.0f});
    if (ImGui::Button(m_mobileView ? "[M] MOBILE ON" : "[M] MOBILE OFF"))
        m_mobileView = !m_mobileView;
    ImGui::PopStyleColor();

    ImGui::SameLine();

    // Credits toggle
    ImGui::PushStyleColor(ImGuiCol_Button,
        m_showCredits ? ImVec4{0.45f,0.04f,0.60f,1.0f}
                      : ImVec4{0.05f,0.12f,0.20f,1.0f});
    if (ImGui::Button("[TEAM]"))
        m_showCredits = !m_showCredits;
    ImGui::PopStyleColor();

    ImGui::SameLine();
    ImGui::TextColored(PURPLE, "FPS %.0f", io.Framerate);
    ImGui::SameLine();
    ImGui::TextColored(TEXT_DIM, "|");
    ImGui::SameLine();
    ImGui::TextColored(GREEN_ON, "8 DEVICES ONLINE");

    // Bottom glow line
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 lp = ImGui::GetWindowPos();
    lp.y += 56.0f;
    dl->AddLine(lp, { lp.x + (float)m_winW, lp.y },
                ImGui::ColorConvertFloat4ToU32(accent), 2.0f);

    ImGui::End();
}

// ============================================================================
// renderSidebar()  —  Navigation + quick stats
// ============================================================================
void GuiManager::renderSidebar()
{
    using namespace CyberpunkTheme;
    const float TOP = 58.0f;
    const float SBW = 188.0f;

    ImGui::SetNextWindowPos({ 0, TOP });
    ImGui::SetNextWindowSize({ SBW, (float)m_winH - TOP });
    ImGui::SetNextWindowBgAlpha(0.97f);
    ImGui::Begin("##sidebar", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar  | ImGuiWindowFlags_NoSavedSettings);

    ImGui::SetCursorPosY(14.0f);
    ImGui::TextColored(TEXT_DIM, "  SYSTEM MODULES");
    ImGui::Spacing();

    auto navItem = [&](const char* icon, const char* label) {
        ImGui::SetCursorPosX(10.0f);
        ImGui::TextColored(CYAN, "%s", icon);
        ImGui::SameLine(38.0f);
        ImGui::TextColored(TEXT_PRIMARY, "%s", label);
    };

    navItem("U", "Doors");
    navItem("*", "Lights");
    navItem("~", "HVAC / AC");
    navItem("@", "Cameras");
    navItem("#", "Event Log");

    ImGui::Separator();
    ImGui::Spacing();
    ImGui::TextColored(TEXT_DIM, "  POWER LOAD");
    ImGui::Spacing();

    double totalW = m_frontDoor->ratedWattage()   + m_backDoor->ratedWattage()
                  + m_livingLight->ratedWattage()  + m_bedroomLight->ratedWattage()
                  + m_kitchenLight->ratedWattage() + m_mainAC->ratedWattage()
                  + m_frontCamera->ratedWattage()  + m_backCamera->ratedWattage();

    ImGui::SetCursorPosX(10.0f);
    ImGui::TextColored(AMBER, "%.1f W", totalW);
    ImGui::SetCursorPosX(10.0f);
    ImGui::TextColored(TEXT_DIM, "Total Active Load");

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::TextColored(TEXT_DIM, "  SECURITY");
    ImGui::Spacing();

    bool anyMotion = m_frontCamera->isRecording() || m_backCamera->isRecording();
    ImGui::SetCursorPosX(10.0f);
    ImGui::TextColored(anyMotion ? AMBER : GREEN_ON,
                       anyMotion ? "! MOTION DETECTED" : "OK  ALL CLEAR");

    bool frontLocked = m_frontDoor->getLockStatus();
    bool backLocked  = m_backDoor->getLockStatus();
    ImGui::SetCursorPosX(10.0f);
    ImGui::TextColored((frontLocked && backLocked) ? GREEN_ON : AMBER,
                       (frontLocked && backLocked) ? "OK  DOORS SECURED"
                                                   : "!  DOOR UNLOCKED");

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // ---- Voice toggle ------------------------------------------------------
    ImGui::TextColored(TEXT_DIM, "  AI VOICE");
    ImGui::Spacing();
    ImGui::SetCursorPosX(10.0f);
    ImGui::Checkbox("Enable##voice", &m_voiceEnabled);
    ImGui::SetCursorPosX(10.0f);
    ImGui::Checkbox("Female##voice", &m_femaleVoice);

    if (ImGui::Button("Test Voice##sidebar", { SBW - 20.0f, 0 }))
        speakFemale("ECU Smart Home online. All systems nominal.");

    ImGui::End();
}

// ============================================================================
// renderCreditsWindow()  —  TASK 3: Team Credits & Roster Grid
// ============================================================================
void GuiManager::renderCreditsWindow()
{
    using namespace CyberpunkTheme;
    if (!m_showCredits) return;

    ImGui::SetNextWindowSize({ 860.0f, 520.0f }, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(
        { ((float)m_winW - 860.0f) * 0.5f, ((float)m_winH - 520.0f) * 0.5f },
        ImGuiCond_FirstUseEver);
    ImGui::Begin("  SYSTEM ARCHITECTS  //  ECU SMART HOME TEAM",
                 &m_showCredits, ImGuiWindowFlags_NoSavedSettings);

    // Glass header
    ImGui::TextColored(CYAN,    "  ECU SMART HOME  —  NODE-NEXUS");
    ImGui::SameLine();
    ImGui::TextColored(TEXT_DIM,"   Spring 2026  |  C++ OOP Project");
    ImGui::Separator(); ImGui::Spacing();

    ImGui::PushStyleColor(ImGuiCol_Text, PURPLE);
    ImGui::Columns(4, "teamcols", true);
    ImGui::SetColumnWidth(0, 190.0f);
    ImGui::SetColumnWidth(1, 120.0f);
    ImGui::SetColumnWidth(2, 180.0f);
    ImGui::SetColumnWidth(3, 330.0f);
    ImGui::Text("  NAME");      ImGui::NextColumn();
    ImGui::Text("  ID");        ImGui::NextColumn();
    ImGui::Text("  ROLE");      ImGui::NextColumn();
    ImGui::Text("  C++ BACKEND MODULE");  ImGui::NextColumn();
    ImGui::PopStyleColor();
    ImGui::Separator();

    struct Member { const char* name; const char* id; const char* role; const char* module; };
    static const Member team[] = {
        { "Eng Zain",       "692500788", "Lead Architect",    "GuiManager Bridge + Full System Integration" },
        { "Ahmed Sherif",   "692500790", "Co-Lead / QA",      "Architecture Review & Memory Safety"         },
        { "Hamza Ali",      "692500812", "Developer",         "Core SmartDoor & Lock Logic"                 },
        { "Omar Hosam",     "692500809", "Developer",         "Core SmartLight & Wattage Math"              },
        { "Mohab Shaaban",  "692500813", "Developer",         "Core SmartAC & Temperature Clamp Logic"      },
        { "Adham Mohamed",  "692500606", "Developer",         "Core SmartCamera & Edge Detection"           },
        { "Bassel Mohamed", "692501345", "Developer",         "Observer Pattern & Event Log"                },
        { "Mohamed Sameh",  "692500817", "Developer",         "Bridge Architecture & System Integration"    },
        { "Yassin Amgad",   "692500850", "Audio Engineer",    "Audio Singleton & WAV Processing"            },
    };

    for (int i = 0; i < 9; ++i) {
        const Member& m = team[i];
        bool isLead = (i < 2);
        if (i % 2 == 0) {
            ImVec2 rMin = ImGui::GetCursorScreenPos();
            ImGui::GetWindowDrawList()->AddRectFilled(
                { rMin.x - 4, rMin.y },
                { rMin.x + 800.0f, rMin.y + ImGui::GetTextLineHeightWithSpacing() },
                IM_COL32(0, 30, 45, 110));
        }
        ImGui::TextColored(isLead ? CYAN : TEXT_PRIMARY, "  %s", m.name);   ImGui::NextColumn();
        ImGui::TextColored(TEXT_DIM,  "  %s", m.id);                         ImGui::NextColumn();
        ImGui::TextColored(isLead ? PURPLE : CYAN_DIM, "  %s", m.role);     ImGui::NextColumn();
        ImGui::TextColored(TEXT_PRIMARY, "  %s", m.module);                  ImGui::NextColumn();
    }

    ImGui::Columns(1);
    ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
    ImGui::TextColored(AMBER,    "  Faculty of Engineering, ECU");
    ImGui::SameLine(500.0f);
    ImGui::TextColored(GREEN_ON, "  Status: COMPLETE");
    ImGui::Spacing();
    if (ImGui::Button("  Close  ", { 120.0f, 0 })) m_showCredits = false;
    ImGui::End();
}


// ============================================================================
// renderMobileSimulator()  —  TASK 2: Smartphone aspect-ratio overlay
// ============================================================================
void GuiManager::renderMobileSimulator()
{
    using namespace CyberpunkTheme;
    if (!m_mobileView) return;

    const float phoneW = 400.0f, phoneH = 820.0f;
    const float phoneX = ((float)m_winW - phoneW) * 0.5f;
    const float phoneY = ((float)m_winH - phoneH) * 0.5f;
    float t = m_animTime;
    float gp = 0.5f + 0.5f * std::sin(t * 2.0f);

    ImDrawList* bg = ImGui::GetBackgroundDrawList();

    // ── Phone chassis ────────────────────────────────────────────────────────
    // Drop shadow
    bg->AddRectFilled({ phoneX-10, phoneY-10 },
                      { phoneX+phoneW+10, phoneY+phoneH+10 },
                      IM_COL32(0,0,0,200), 38.0f);
    // Body — dark titanium
    bg->AddRectFilled({ phoneX, phoneY },
                      { phoneX+phoneW, phoneY+phoneH },
                      IM_COL32(12, 14, 22, 255), 35.0f);
    // Outer neon glow ring
    bg->AddRect({ phoneX, phoneY }, { phoneX+phoneW, phoneY+phoneH },
                IM_COL32(0, (int)(gp*220), (int)(gp*255), 210), 35.0f, 0, 2.5f);
    // Inner metallic rim
    bg->AddRect({ phoneX+3, phoneY+3 }, { phoneX+phoneW-3, phoneY+phoneH-3 },
                IM_COL32(40, 50, 80, 160), 33.0f, 0, 1.0f);

    // ── Physical side buttons ────────────────────────────────────────────────
    // Volume Up
    bg->AddRectFilled({ phoneX-5, phoneY+120 }, { phoneX-1, phoneY+160 },
                      IM_COL32(30, 38, 60, 255), 3.0f);
    bg->AddRect(      { phoneX-5, phoneY+120 }, { phoneX-1, phoneY+160 },
                      IM_COL32(60, 80, 120, 200), 3.0f, 0, 1.0f);
    // Volume Down
    bg->AddRectFilled({ phoneX-5, phoneY+175 }, { phoneX-1, phoneY+215 },
                      IM_COL32(30, 38, 60, 255), 3.0f);
    bg->AddRect(      { phoneX-5, phoneY+175 }, { phoneX-1, phoneY+215 },
                      IM_COL32(60, 80, 120, 200), 3.0f, 0, 1.0f);
    // Silent toggle
    bg->AddRectFilled({ phoneX-5, phoneY+85  }, { phoneX-1, phoneY+110 },
                      IM_COL32(30, 38, 60, 255), 3.0f);
    // Power button (right side)
    bg->AddRectFilled({ phoneX+phoneW+1, phoneY+160 },
                      { phoneX+phoneW+5, phoneY+220 },
                      IM_COL32(30, 38, 60, 255), 3.0f);
    bg->AddRect(      { phoneX+phoneW+1, phoneY+160 },
                      { phoneX+phoneW+5, phoneY+220 },
                      IM_COL32(0,(int)(gp*180),(int)(gp*200), 200), 3.0f, 0, 1.0f);

    // ── Screen bezel ─────────────────────────────────────────────────────────
    const float sx = phoneX + 10.0f;
    const float sy = phoneY + 14.0f;
    const float sw = phoneW - 20.0f;
    const float sh = phoneH - 36.0f;
    // Screen glass — deep blue-black
    bg->AddRectFilled({ sx, sy }, { sx+sw, sy+sh },
                      IM_COL32(5, 8, 18, 255), 26.0f);
    // Screen edge glow
    bg->AddRect({ sx, sy }, { sx+sw, sy+sh },
                IM_COL32(0,(int)(gp*180),(int)(gp*200),120), 26.0f, 0, 1.2f);

    // ── Dynamic Island (pill notch) ───────────────────────────────────────────
    float islandCx = phoneX + phoneW * 0.5f;
    bg->AddRectFilled({ islandCx-52, phoneY+20 },
                      { islandCx+52, phoneY+42 },
                      IM_COL32(4, 4, 10, 255), 12.0f);
    bg->AddRect(      { islandCx-52, phoneY+20 },
                      { islandCx+52, phoneY+42 },
                      IM_COL32(40, 50, 80, 160), 12.0f, 0, 0.8f);
    // Camera dot inside island
    bg->AddCircleFilled({ islandCx+30, phoneY+31 }, 5,
                        IM_COL32(20, 20, 35, 255));
    bg->AddCircle(      { islandCx+30, phoneY+31 }, 5,
                        IM_COL32(40, 60, 90, 180), 0, 0.8f);
    bg->AddCircleFilled({ islandCx+30, phoneY+31 }, 2,
                        IM_COL32(0,(int)(gp*200),(int)(gp*220), 200));
    // Face ID dot grid (subtle)
    for (int r = 0; r < 2; ++r)
        for (int c = 0; c < 4; ++c)
            bg->AddCircleFilled({ islandCx - 40.0f + c*12, phoneY+26.0f + r*10 }, 1,
                                IM_COL32(60,80,100,80));

    // ── Home indicator bar ────────────────────────────────────────────────────
    bg->AddRectFilled({ islandCx-55, phoneY+phoneH-20 },
                      { islandCx+55, phoneY+phoneH-12 },
                      IM_COL32(120,140,180,160), 5.0f);

    // ── App ImGui window ──────────────────────────────────────────────────────
    const float appX = sx + 2.0f;
    const float appY = sy + 34.0f;
    const float appW = sw - 4.0f;
    const float appH = sh - 54.0f;

    ImGui::SetNextWindowPos({ appX, appY });
    ImGui::SetNextWindowSize({ appW, appH });
    ImGui::SetNextWindowBgAlpha(0.0f);
    ImGui::Begin("##mobileapp", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollWithMouse);

    // Status bar
    ImGui::TextColored({ 0.0f, gp*0.95f, gp, 1.0f }, " ECU SmartHome");
    ImGui::SameLine(appW - 55.0f);
    ImGui::TextColored(GREEN_ON, "LIVE");
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::BeginChild("##mscroll", { 0, appH - 58.0f }, false);

    // Glass mini-card helper
    auto miniCard = [&](const char* cid, ImVec4 accent) {
        ImVec2 p = ImGui::GetCursorScreenPos();
        float cw = appW - 8.0f;
        ImDrawList* dl = ImGui::GetWindowDrawList();
        dl->AddRectFilled(p, { p.x+cw, p.y+64.0f },
                          IM_COL32(8,14,28,180), 10.0f);
        dl->AddRect(p, { p.x+cw, p.y+64.0f },
                    IM_COL32((int)(accent.x*255),(int)(accent.y*255),
                             (int)(accent.z*255), 200), 10.0f, 0, 1.5f);
        ImGui::BeginChild(cid, { cw, 64.0f }, false,
            ImGuiWindowFlags_NoScrollbar);
    };

    // Doors mini-card
    miniCard("##mDoor", CYAN);
    ImGui::TextColored(CYAN, "DOORS");
    ImGui::SameLine(appW - 120.0f);
    if (ImGui::SmallButton("Lock All##m"))  { m_frontDoor->lock(true); m_backDoor->lock(true); }
    ImGui::SameLine();
    if (ImGui::SmallButton("Unlock##m"))    { m_frontDoor->unlock(true); m_backDoor->unlock(true); }
    ImGui::Text("Front:"); ImGui::SameLine(70.0f);
    ImGui::TextColored(m_frontDoor->getLockStatus() ? GREEN_ON : AMBER,
                       m_frontDoor->getLockStatus() ? "LOCKED" : "OPEN");
    ImGui::SameLine(150.0f);
    ImGui::Text("Back:"); ImGui::SameLine(200.0f);
    ImGui::TextColored(m_backDoor->getLockStatus() ? GREEN_ON : AMBER,
                       m_backDoor->getLockStatus() ? "LOCKED" : "OPEN");
    ImGui::EndChild();
    ImGui::Spacing();

    // Lights mini-card
    miniCard("##mLight", { 1.0f, 0.87f, 0.0f, 1.0f });
    ImGui::TextColored(AMBER, "LIGHTS");
    ImGui::Text("Living:"); ImGui::SameLine(70.0f);
    ImGui::TextColored(GREEN_ON, "%d%%", m_livingLight->getBrightness());
    ImGui::SameLine(130.0f);
    ImGui::Text("Bed:"); ImGui::SameLine(165.0f);
    ImGui::TextColored(GREEN_ON, "%d%%", m_bedroomLight->getBrightness());
    ImGui::SameLine(220.0f);
    ImGui::Text("Kit:"); ImGui::SameLine(255.0f);
    ImGui::TextColored(GREEN_ON, "%d%%", m_kitchenLight->getBrightness());
    if (ImGui::SmallButton("All OFF##ml")) { m_livingLight->turnOff(); m_bedroomLight->turnOff(); m_kitchenLight->turnOff(); }
    ImGui::SameLine();
    if (ImGui::SmallButton("All ON##ml"))  { m_livingLight->turnOn();  m_bedroomLight->turnOn();  m_kitchenLight->turnOn(); }
    ImGui::EndChild();
    ImGui::Spacing();

    // AC mini-card
    miniCard("##mAC", { 0.0f, 0.80f, 1.0f, 1.0f });
    ImGui::TextColored(CYAN, "HVAC");
    ImGui::SameLine(80.0f);
    ImGui::TextColored(m_mainAC->getPowerStatus() ? GREEN_ON : RED_OFF,
                       m_mainAC->getPowerStatus() ? "ON" : "OFF");
    ImGui::SameLine(130.0f);
    ImGui::TextColored(AMBER, "%.1f C", m_mainAC->getTemperature());
    if (ImGui::SmallButton("AC On##m"))  m_mainAC->turnOn();
    ImGui::SameLine();
    if (ImGui::SmallButton("AC Off##m")) m_mainAC->turnOff();
    ImGui::EndChild();
    ImGui::Spacing();

    // Camera mini-card
    bool rec = m_frontCamera->isRecording() || m_backCamera->isRecording();
    miniCard("##mCam", rec ? AMBER : GREEN_ON);
    ImGui::TextColored(CYAN, "CAMERAS");
    ImGui::SameLine(100.0f);
    ImGui::TextColored(rec ? AMBER : GREEN_ON, rec ? "MOTION!" : "ALL CLEAR");
    ImGui::Text("Front:"); ImGui::SameLine(70.0f);
    ImGui::TextColored(m_frontCamera->isRecording() ? AMBER : TEXT_DIM,
                       m_frontCamera->isRecording() ? "REC" : "IDLE");
    ImGui::SameLine(140.0f);
    ImGui::Text("Back:"); ImGui::SameLine(180.0f);
    ImGui::TextColored(m_backCamera->isRecording() ? AMBER : TEXT_DIM,
                       m_backCamera->isRecording() ? "REC" : "IDLE");
    ImGui::EndChild();
    ImGui::Spacing();

    ImGui::Separator();
    float btnX = (appW - 150.0f) * 0.5f;
    ImGui::SetCursorPosX(btnX);
    ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(30,8,55,220));
    if (ImGui::Button("Exit Mobile View", { 150.0f, 0 })) m_mobileView = false;
    ImGui::PopStyleColor();

    ImGui::EndChild();
    ImGui::End();
}


// ============================================================================
// renderLogPanel()  —  Timestamped event ring buffer
// ============================================================================
void GuiManager::renderLogPanel()
{
    using namespace CyberpunkTheme;
    ImGui::BeginChild("LogPanel", { 0, 0 }, true);
    ImGui::TextColored(PURPLE, "#  EVENT LOG");
    ImGui::SameLine();
    ImGui::TextColored(TEXT_DIM, "  (newest first)");
    ImGui::Separator();
    ImGui::Spacing();

    int count = m_logCount;
    for (int i = 0; i < count; ++i) {
        int idx = (m_logHead - 1 - i + LOG_CAPACITY) % LOG_CAPACITY;
        ImGui::TextColored(i == 0 ? TEXT_PRIMARY : TEXT_DIM,
                           "%s", m_logLines[idx].c_str());
    }

    if (m_logCount > 0) {
        ImGui::Spacing();
        if (ImGui::SmallButton("Clear Log")) {
            m_logHead = 0; m_logCount = 0;
        }
    }
    ImGui::EndChild();
}

// ============================================================================
// renderDashboard()  —  Main grid: device panels + log
// ============================================================================
void GuiManager::renderDashboard()
{
    using namespace CyberpunkTheme;
    const float TOP = 58.0f, SBW = 188.0f;
    ImGui::SetNextWindowPos({ SBW, TOP });
    ImGui::SetNextWindowSize({ (float)m_winW - SBW, (float)m_winH - TOP });
    ImGui::SetNextWindowBgAlpha(0.0f);
    ImGui::Begin("##dashboard", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus);

    float avail   = (float)m_winW - SBW - 28.0f;
    float halfW   = (avail - 8.0f) * 0.5f;
    float cardH   = 160.0f;

    // Helper: draw a liquid-glass card background
    auto glassCard = [&](const char* id, float w, float h) -> bool {
        bool vis = ImGui::BeginChild(id, { w, h }, false,
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        if (vis) {
            ImVec2 p  = ImGui::GetWindowPos();
            ImVec2 sz = ImGui::GetWindowSize();
            ImDrawList* dl = ImGui::GetWindowDrawList();
            // frosted glass fill
            dl->AddRectFilled(p, { p.x+sz.x, p.y+sz.y },
                              IM_COL32(8, 14, 28, 172), 12.0f);
            // cyan glow border
            float gp = 0.55f + 0.45f * std::sin(m_animTime * 2.2f);
            dl->AddRect(p, { p.x+sz.x, p.y+sz.y },
                        IM_COL32(0, (int)(gp*220), (int)(gp*255), 200), 12.0f, 0, 1.8f);
            // subtle inner highlight
            dl->AddRectFilled({ p.x+1, p.y+1 }, { p.x+sz.x-1, p.y+18 },
                              IM_COL32(255,255,255,8), 12.0f);
        }
        return vis;
    };

    // ── ROW 1 ────────────────────────────────────────────────────────────────
    // DOOR CARD
    if (glassCard("##cDoor", halfW, cardH)) {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 cp = ImGui::GetWindowPos();
        // Door icon: rectangle with knob
        dl->AddRect({ cp.x+14, cp.y+14 }, { cp.x+44, cp.y+56 },
                    IM_COL32(0,230,255,220), 3.0f, 0, 2.0f);
        dl->AddCircleFilled({ cp.x+40, cp.y+36 }, 3,
                            IM_COL32(0,230,255,255));
        ImGui::SetCursorPos({ 56.0f, 12.0f });
        ImGui::TextColored(CYAN, "DOOR CONTROL");
        ImGui::SetCursorPos({ 56.0f, 30.0f });
        ImGui::TextColored(TEXT_DIM, "2 doors monitored");
        ImGui::Separator();
        ImGui::Spacing();
        if (m_doorPanel) m_doorPanel->render();
        ImGui::EndChild();
    }

    ImGui::SameLine(0, 8.0f);

    // LIGHT CARD
    if (glassCard("##cLight", halfW, cardH)) {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 cp = ImGui::GetWindowPos();
        // Bulb icon: circle + base
        dl->AddCircle({ cp.x+29, cp.y+28 }, 14,
                      IM_COL32(255,220,0,220), 0, 2.0f);
        dl->AddRectFilled({ cp.x+24, cp.y+42 }, { cp.x+34, cp.y+50 },
                          IM_COL32(255,200,0,180), 2.0f);
        // glow rays
        for (int a = 0; a < 8; ++a) {
            float ang = a * 3.14159f / 4.0f + m_animTime;
            dl->AddLine({ cp.x+29 + 16*std::cos(ang), cp.y+28 + 16*std::sin(ang) },
                        { cp.x+29 + 22*std::cos(ang), cp.y+28 + 22*std::sin(ang) },
                        IM_COL32(255,220,0,120), 1.5f);
        }
        ImGui::SetCursorPos({ 56.0f, 12.0f });
        ImGui::TextColored(CYAN, "LIGHT CONTROL");
        ImGui::SetCursorPos({ 56.0f, 30.0f });
        double lw = m_livingLight->ratedWattage() + m_bedroomLight->ratedWattage()
                  + m_kitchenLight->ratedWattage();
        ImGui::TextColored(AMBER, "Draw: %.1f W", lw);
        ImGui::Separator();
        ImGui::Spacing();
        if (m_lightPanel) m_lightPanel->render();
        ImGui::EndChild();
    }

    ImGui::Spacing();

    // ── ROW 2 ────────────────────────────────────────────────────────────────
    // AC CARD
    if (glassCard("##cAC", halfW, cardH)) {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 cp = ImGui::GetWindowPos();
        // Fan icon: spinning spokes
        float cx = cp.x + 29, cy = cp.y + 32;
        for (int i = 0; i < 6; ++i) {
            float a = i * 3.14159f / 3.0f + m_animTime * 1.8f;
            dl->AddLine({ cx, cy },
                        { cx + 14*std::cos(a), cy + 14*std::sin(a) },
                        IM_COL32(0,200,255,200), 2.2f);
        }
        dl->AddCircleFilled({ cx, cy }, 4, IM_COL32(0,230,255,255));
        ImGui::SetCursorPos({ 56.0f, 12.0f });
        ImGui::TextColored(CYAN, "HVAC / AC");
        ImGui::SetCursorPos({ 56.0f, 30.0f });
        ImGui::TextColored(AMBER, "Draw: %.1f W", m_mainAC->ratedWattage());
        ImGui::Separator();
        ImGui::Spacing();
        if (m_acPanel) m_acPanel->render();
        ImGui::EndChild();
    }

    ImGui::SameLine(0, 8.0f);

    // CAMERA CARD
    if (glassCard("##cCam", halfW, cardH)) {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 cp = ImGui::GetWindowPos();
        // Camera icon: body + lens
        dl->AddRectFilled({ cp.x+12, cp.y+22 }, { cp.x+44, cp.y+46 },
                          IM_COL32(0,200,255,60), 4.0f);
        dl->AddRect({ cp.x+12, cp.y+22 }, { cp.x+44, cp.y+46 },
                    IM_COL32(0,230,255,220), 4.0f, 0, 1.5f);
        dl->AddCircle({ cp.x+28, cp.y+34 }, 7, IM_COL32(0,230,255,255), 0, 1.5f);
        dl->AddCircleFilled({ cp.x+28, cp.y+34 }, 3, IM_COL32(0,230,255,200));
        // viewfinder bump
        dl->AddRectFilled({ cp.x+22, cp.y+16 }, { cp.x+32, cp.y+22 },
                          IM_COL32(0,200,255,140), 3.0f);
        bool rec = m_frontCamera->isRecording() || m_backCamera->isRecording();
        ImGui::SetCursorPos({ 56.0f, 12.0f });
        ImGui::TextColored(CYAN, "CAMERAS");
        ImGui::SetCursorPos({ 56.0f, 30.0f });
        ImGui::TextColored(rec ? AMBER : GREEN_ON, rec ? "REC ACTIVE" : "ALL CLEAR");
        ImGui::Separator();
        ImGui::Spacing();
        if (m_cameraPanel) m_cameraPanel->render();
        ImGui::EndChild();
    }

    ImGui::Spacing();
    renderLogPanel();
    ImGui::End();
}


// ============================================================================
// run()  —  Main SDL2 + ImGui render loop
// ============================================================================
void GuiManager::run()
{
    ImGuiIO& io = ImGui::GetIO();

    while (m_running) {
        // ---- Event pump -----------------------------------------------------
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            ImGui_ImplSDL2_ProcessEvent(&ev);
            if (ev.type == SDL_QUIT) m_running = false;
            if (ev.type == SDL_WINDOWEVENT &&
                ev.window.event == SDL_WINDOWEVENT_RESIZED) {
                m_winW = ev.window.data1;
                m_winH = ev.window.data2;
            }
        }

        // ---- New ImGui frame ------------------------------------------------
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        m_animTime += io.DeltaTime;

        // ---- Tick login failure timer ----------------------------------------
        if (m_loginFailTimer > 0.0f)
            m_loginFailTimer -= io.DeltaTime;

        // ---- Route to login screen or full dashboard ------------------------
        if (!m_loggedIn) {
            renderLoginScreen();
        } else {
            renderTopBar();
            renderSidebar();
            renderDashboard();

            // ---- Floating overlays (always on top) --------------------------
            renderCreditsWindow();
            renderMobileSimulator();
        }

        // ---- GL render ------------------------------------------------------
        ImGui::Render();
        glViewport(0, 0, m_winW, m_winH);
        glClearColor(0.02f, 0.02f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(m_window);
    }
}

// ============================================================================
// shutdown()  —  Graceful teardown
// ============================================================================
void GuiManager::shutdown()
{
    if (m_glContext) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
        SDL_GL_DeleteContext(m_glContext);
        m_glContext = nullptr;
    }
    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }
    SDL_Quit();
}
