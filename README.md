# ⬡ ECU Smart Home — NODE-NEXUS

<div align="center">

![Language](https://img.shields.io/badge/Language-C%2B%2B17-cyan?style=for-the-badge&logo=cplusplus&logoColor=white)
![Framework](https://img.shields.io/badge/GUI-Dear%20ImGui%201.91.9b-blueviolet?style=for-the-badge)
![Renderer](https://img.shields.io/badge/Renderer-OpenGL%203.3-green?style=for-the-badge&logo=opengl)
![Platform](https://img.shields.io/badge/Platform-Linux-informational?style=for-the-badge&logo=linux&logoColor=white)
![Build](https://img.shields.io/badge/Build-Passing%20✔-brightgreen?style=for-the-badge)
![License](https://img.shields.io/badge/License-Academic-orange?style=for-the-badge)

> *"The home doesn't just respond — it thinks, speaks, and secures itself."*

**A fully modular, cyberpunk-aesthetic Smart Home control system built in C++17.**
Real-time device control. Secure login. Mobile simulator. Female AI voice.

</div>

---

## 🌐 Project Vision

NODE-NEXUS is not a demo — it is a production-grade embedded control architecture disguised as a university project. Every design decision prioritises **clean separation**, **zero coupling**, and **architectural integrity**.

The interface draws inspiration from high-tech military HUDs and cyberpunk dashboards: deep black backgrounds, pulsing cyan (`#00F3FF`) and electric purple (`#BC13FE`) accents, neon glow borders, and scanline animations — all rendered in real-time via OpenGL.

The system controls **8 live devices** simultaneously:

| Device Class | Instances | Capabilities |
|---|---|---|
| 🚪 `SmartDoor` | Front Door, Back Door | Lock / Unlock, Power, Fail-Secure |
| 💡 `SmartLight` | Living Room, Bedroom, Kitchen | Brightness 0–100%, Power, Wattage |
| ❄️ `SmartAC` | Main AC | Temperature 16–30°C, Mode (Cool/Heat/Fan) |
| 📷 `SmartCamera` | Front Camera, Back Camera | Record, Motion Alert, Power |

---

## 🏛️ Core Architecture

### The Bridge Pattern — GUI ↔ Core Decoupling

The most critical design principle: **the core logic never knows the GUI exists.**

```
┌─────────────────────────────────────────────────────────┐
│                    APPLICATION LAYER                    │
│  src/main.cpp  →  GuiManager::init()  →  run()         │
└────────────────────────┬────────────────────────────────┘
                         │  BRIDGE (one-way dependency)
        ┌────────────────▼────────────────────┐
        │         gui/GuiManager.h/.cpp        │
        │   The ONLY class that #includes      │
        │   device headers from include/       │
        │                                      │
        │  ┌──────────┐  ┌──────────────────┐  │
        │  │DoorPanel │  │  LightPanel      │  │
        │  │ACPanel   │  │  CameraPanel     │  │
        │  └──────────┘  └──────────────────┘  │
        └────────────────┬────────────────────┘
                         │  read-only calls
        ┌────────────────▼────────────────────┐
        │         include/ + src/              │
        │  SmartDoor  SmartLight  SmartAC      │
        │  SmartCamera  MotionSensor           │
        │  AutomationRuleEngine  VoiceService  │
        │                                      │
        │  ← ZERO GUI KNOWLEDGE INSIDE HERE →  │
        └──────────────────────────────────────┘
```

### Separation of Concerns — Directory Map

```
ECU_SHA/
├── include/          ← Pure C++ device abstractions (NO GUI code)
│   ├── SmartDevice.h        — Base class (OOP root)
│   ├── SmartDoor.h
│   ├── SmartLight.h
│   ├── SmartAC.h
│   ├── SmartCamera.h
│   ├── MotionSensor.h
│   ├── AutomationRuleEngine.h   (Qt signals/slots)
│   └── VoiceService.h
│
├── src/              ← Core logic implementations (NO GUI code)
│   ├── main.cpp             — Entry point (only creates GuiManager)
│   ├── SmartDoor.cpp
│   ├── SmartLight.cpp
│   ├── SmartAC.cpp
│   ├── SmartCamera.cpp
│   ├── AutomationRuleEngine.cpp
│   └── ...
│
├── gui/              ← ENTIRE UI sandbox (core is UNAWARE of this)
│   ├── GuiManager.h/.cpp    — Bridge controller + all render logic
│   ├── DoorPanel.h/.cpp     — Door UI widget
│   ├── LightPanel.h/.cpp    — Light UI widget
│   ├── ACPanel.h/.cpp       — AC UI widget
│   ├── CameraPanel.h/.cpp   — Camera UI widget
│   └── imgui/               — Vendored Dear ImGui (SDL2 + OpenGL3)
│
├── Team_Task_Sheets/ ← Sprint assignment sheets (HTML)
├── Makefile
└── README.md
```

### OOP Design Principles Applied

| Principle | Implementation |
|---|---|
| **Abstraction** | `SmartDevice` base class — all devices share a common interface |
| **Inheritance** | `SmartDoor`, `SmartLight`, `SmartAC`, `SmartCamera` all extend `SmartDevice` |
| **Polymorphism** | `GuiManager` calls virtual methods via `shared_ptr<SmartDevice>` |
| **Encapsulation** | UI state (sliders, toggles) never bleeds into device classes |
| **Bridge Pattern** | `GuiManager` decouples abstraction (devices) from implementation (UI) |
| **Observer** | `AutomationRuleEngine` uses Qt signals/slots for event-driven rules |
| **Singleton** | System-wide `Singleton` ensures one controller instance |

---

## ✨ Features Showcase

### 🔐 Task 1 — Secure Access Terminal (Login)

The application boots into a locked login screen. The main dashboard is **completely hidden** until valid credentials are entered.

- Animated scanline grid backdrop with neon glow pulse
- Username + PIN fields (password-masked)
- `Enter` key support for fast login
- 3-second blinking red error on wrong credentials
- Female AI voice feedback on success/failure

> **Default credentials:** `Username: Admin` | `PIN: 1234`

---

### 📱 Task 2 — Mobile Phone Simulator

Toggle a smartphone overlay (`[M] MOBILE` button in the top bar) to see a **390×780 phone-shaped UI** rendered on top of the dashboard:

- Realistic phone chrome with notch, bezels, and home bar
- Neon border pulse animation around the phone shell
- Compact scrollable device control view (Doors / Lights / AC / Cameras)
- One-tap Lock All / Unlock All door controls
- Exit button to return to desktop view

---

### 🖥️ Live Dashboard

A fully reactive 2×2 panel grid rendered at 60fps:

| Panel | Controls |
|---|---|
| 🚪 Door Control | Lock/Unlock per door, Power toggle, Live status badge |
| 💡 Light Control | Brightness slider (0–100%) per room, Power toggle, Wattage readout |
| ❄️ HVAC | Temperature slider (16–30°C), Mode dropdown (Cool/Heat/Fan), Power toggle |
| 📷 Cameras | Record toggle, Motion alert raise, Power toggle, Live REC indicator |

Plus a full-width **Event Log** panel with timestamped history and a Clear button.

---

### 🔊 Task 4 — Female AI Voice

All system events trigger `espeak` with a **female voice profile**:

```cpp
// espeak -v en+f3 -s 145 "Access granted. Welcome to ECU Smart Home."
```

Voice controls in the sidebar:
- ✅ **Enable / Disable** voice globally
- ✅ **Female / Male** voice toggle (`en+f3` vs `en`)
- ✅ **"Test Voice"** button for live preview

---

### 👥 Task 3 — Team Credits Grid

A floating modal window (toggle with `[TEAM]` button in the top bar) displays the full team roster in a 4-column grid with alternating row shading and lead/member colour distinction.

---

## ⚙️ Installation & Run Guide

### Prerequisites

```bash
# Ubuntu / Debian
sudo apt install build-essential libsdl2-dev libgl-dev libqt5core5a qt5-qmake espeak
```

### Step 1 — Clone the Repository

```bash
git clone <repository-url>
cd ECU_SHA
```

### Step 2 — Download Dear ImGui (one-time setup)

The `gui/setup_imgui.sh` script automatically downloads Dear ImGui v1.91.9b and all required STB headers directly from the official GitHub repository into `gui/imgui/`:

```bash
bash gui/setup_imgui.sh
```

This downloads:
- `imgui.h`, `imgui.cpp`, `imgui_draw.cpp`, `imgui_widgets.cpp`, `imgui_tables.cpp`
- `imstb_truetype.h`, `imstb_textedit.h`, `imstb_rectpack.h`
- SDL2 + OpenGL3 backends

### Step 3 — Build

```bash
make clean && make -j$(nproc)
```

The Makefile automatically:
1. Compiles all `src/*.cpp` core files with Qt5 flags
2. Compiles all `gui/*.cpp` panel files with SDL2/OpenGL flags
3. Runs Qt MOC on `AutomationRuleEngine.h`
4. Links everything into a single `ecu_smart_home` binary

### Step 4 — Run

```bash
./ecu_smart_home
```

**Login with:** `Username: Admin` | `PIN: 1234`

---

## 👨‍💻 The Architects

| # | Name | Student ID | Role | Module |
|---|---|---|---|---|
| 🌟 | **Eng Zain** | `692500788` | **Lead Architect** | GuiManager Bridge + Integration |
| ⭐ | **Ahmed Sherif** | `692500790` | **Co-Lead & Reviewer** | QA / Code Review / Architecture |
| 1 | Hamza Ali | `692500812` | Developer | Smart Door Panel |
| 2 | Omar Hosam | `692500809` | Developer | Smart Light Panel |
| 3 | Mohab Shaaban | `692500813` | Developer | Smart AC Panel |
| 4 | Adham Mohamed | `692500606` | Developer | Smart Camera Panel |
| 5 | Bassel Mohamed | `692501345` | Developer | Event Log System |
| 6 | Mohamed Sameh | `692500817` | UI/UX Developer | Layout & Dark Neon Theming |
| 7 | Yassin Amgad | `692500850` | Audio Engineer | Voice Service & TTS |

---

## 🎨 Design System

| Token | Value | Usage |
|---|---|---|
| `CYAN` | `#00F3FF` | Primary accent, section headers, sliders |
| `PURPLE` | `#BC13FE` | Secondary accent, active states, buttons |
| `BG_DARKEST` | `#050510` | Window background |
| `GREEN_ON` | `#1AFF73` | Device online / locked status |
| `AMBER` | `#FFA600` | Warnings, wattage, motion alerts |
| `RED_OFF` | `#FF2E2E` | Device offline / error state |

---

## 📋 Build Targets

```bash
make              # Build all
make clean        # Remove build/ and binary
make -j$(nproc)   # Parallel build (recommended)
```

---

<div align="center">

**ECU Smart Home — NODE-NEXUS**
*Faculty of Engineering · Spring 2026*

`حسبنا الله ونعم الوكيل`

</div>
