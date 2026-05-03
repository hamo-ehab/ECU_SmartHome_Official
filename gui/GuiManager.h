/*
 * =============================================================================
 * FILE:    gui/GuiManager.h
 * PROJECT: ECU Smart Home System — GUI Layer
 * AUTHOR:  Eng. Zain (Lead Architect)
 * =============================================================================
 * GuiManager is the ONLY bridge between the UI layer and the core device
 * classes. It holds shared_ptrs to each managed device and owns the SDL2 +
 * OpenGL + Dear ImGui render loop.
 *
 * ARCHITECTURAL RULE:
 *   - Nothing inside src/ or include/ may include this header.
 *   - This header is allowed to include from ../include/ freely.
 * =============================================================================
 */
// حسبنا الله ونعم الوكيل
#pragma once

// ---- Core device headers (read-only from this layer) -----------------------
#include "../include/SmartDevice.h"
#include "../include/SmartDoor.h"
#include "../include/SmartLight.h"
#include "../include/SmartAC.h"
#include "../include/SmartCamera.h"
#include "../include/MotionSensor.h"
#include "../include/AutomationRuleEngine.h"

// ---- GUI Panels ------------------------------------------------------------
#include "DoorPanel.h"
#include "LightPanel.h"
#include "ACPanel.h"
#include "CameraPanel.h"

// ---- STL -------------------------------------------------------------------
#include <memory>
#include <string>
#include <vector>
#include <array>
#include <cstdint>
#include <cstring>

// ---- SDL2 (window + GL context) --------------------------------------------
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

// ---- Dear ImGui core -------------------------------------------------------
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_sdl2.h"
#include "imgui/backends/imgui_impl_opengl3.h"

// ===========================================================================
// CyberpunkTheme — colour tokens for the "Dark Neon / Cyberpunk" aesthetic
// ===========================================================================
namespace CyberpunkTheme
{
    static constexpr ImVec4 BG_DARKEST   { 0.02f, 0.02f, 0.05f, 1.00f };
    static constexpr ImVec4 BG_PANEL     { 0.05f, 0.05f, 0.10f, 0.95f };
    static constexpr ImVec4 BG_WIDGET    { 0.08f, 0.08f, 0.15f, 1.00f };
    static constexpr ImVec4 CYAN         { 0.00f, 0.95f, 1.00f, 1.00f };
    static constexpr ImVec4 CYAN_DIM     { 0.00f, 0.55f, 0.60f, 1.00f };
    static constexpr ImVec4 PURPLE       { 0.74f, 0.07f, 0.99f, 1.00f };
    static constexpr ImVec4 PURPLE_DIM   { 0.45f, 0.04f, 0.60f, 1.00f };
    static constexpr ImVec4 TEXT_PRIMARY { 0.90f, 0.95f, 1.00f, 1.00f };
    static constexpr ImVec4 TEXT_DIM     { 0.45f, 0.50f, 0.60f, 1.00f };
    static constexpr ImVec4 GREEN_ON     { 0.10f, 1.00f, 0.45f, 1.00f };
    static constexpr ImVec4 RED_OFF      { 1.00f, 0.18f, 0.18f, 1.00f };
    static constexpr ImVec4 AMBER        { 1.00f, 0.65f, 0.00f, 1.00f };

    inline ImVec4 hex(uint32_t rgba)
    {
        return {
            ((rgba >> 24) & 0xFF) / 255.0f,
            ((rgba >> 16) & 0xFF) / 255.0f,
            ((rgba >>  8) & 0xFF) / 255.0f,
            ((rgba      ) & 0xFF) / 255.0f
        };
    }
}

// ===========================================================================
// GuiManager
// ===========================================================================
class GuiManager
{
public:
    GuiManager();
    ~GuiManager();

    GuiManager(const GuiManager&)            = delete;
    GuiManager& operator=(const GuiManager&) = delete;

    bool init();
    void run();
    void shutdown();

private:
    // ---- SDL / GL handles --------------------------------------------------
    SDL_Window*   m_window   { nullptr };
    SDL_GLContext m_glContext { nullptr };
    bool          m_running  { false   };

    int m_winW { 1440 };
    int m_winH { 900  };

    // ---- Device instances --------------------------------------------------
    std::shared_ptr<SmartDoor>   m_frontDoor;
    std::shared_ptr<SmartDoor>   m_backDoor;
    std::shared_ptr<SmartLight>  m_livingLight;
    std::shared_ptr<SmartLight>  m_bedroomLight;
    std::shared_ptr<SmartLight>  m_kitchenLight;
    std::shared_ptr<SmartAC>     m_mainAC;
    std::shared_ptr<SmartCamera> m_frontCamera;
    std::shared_ptr<SmartCamera> m_backCamera;

    // ---- Panel Instances ---------------------------------------------------
    std::unique_ptr<DoorPanel>   m_doorPanel;
    std::unique_ptr<LightPanel>  m_lightPanel;
    std::unique_ptr<ACPanel>     m_acPanel;
    std::unique_ptr<CameraPanel> m_cameraPanel;

    // ---- Log ring buffer ---------------------------------------------------
    static constexpr int LOG_CAPACITY = 64;
    std::array<std::string, LOG_CAPACITY> m_logLines;
    int  m_logHead  { 0 };
    int  m_logCount { 0 };

    float m_animTime { 0.0f };

    // ---- LOGIN STATE -------------------------------------------------------
    bool  m_loggedIn        { false };
    char  m_usernameInput[64] {};
    char  m_pinInput[16]      {};
    bool  m_loginFailed     { false };
    float m_loginFailTimer  { 0.0f };

    // ---- MOBILE SIMULATOR STATE --------------------------------------------
    bool  m_mobileView      { false };

    // ---- CREDITS WINDOW ----------------------------------------------------
    bool  m_showCredits     { false };

    // ---- VOICE SETTINGS ----------------------------------------------------
    bool  m_femaleVoice     { true  };
    bool  m_voiceEnabled    { true  };

    // ---- Private helpers ---------------------------------------------------
    void applyTheme();
    void speakFemale(const char* text);

    // Screens
    void renderLoginScreen();
    void renderDashboard();

    // Dashboard sub-renderers
    void renderTopBar();
    void renderSidebar();
    void renderLogPanel();
    void renderCreditsWindow();
    void renderMobileSimulator();
    void renderGlowRect(const ImVec2& pos, const ImVec2& size,
                        const ImVec4& colour, float thickness = 1.5f);

    void logEvent(const std::string& msg);
    void statusBadge(bool on);
};