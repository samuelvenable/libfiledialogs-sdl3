/*

 MIT License

 Copyright © 2021-2025 Samuel Venable

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.

*/

#if (defined(_WIN32) && defined(_MSC_VER))
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#endif

#include <climits>
#include <cstdlib>
#include <sstream>
#include <vector>
#include <map>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl3.h>
#include <imgui/imgui_impl_sdlrenderer3.h>

#include <ImFileDialog/ImFileDialog.h>
#include <ImFileDialog/ImFileDialogMacros.h>

#include <msgbox/imguial_msgbox.h>

#include <ghc/filesystem.hpp>
#include <filesystem.hpp>

#include "filedialogs.hpp"

#include <SDL3/SDL_video.h>
#include <sys/stat.h>
#if defined(_WIN32)
#include <windows.h>
#define STR_SLASH "\\"
#define CHR_SLASH '\\'
#define HOME_PATH "USERPROFILE"
#else
#if (defined(__APPLE__) && defined(__MACH__))
#include <AppKit/AppKit.h>
#elif ((defined(__linux__) && !defined(__ANDROID__)) || (defined(__FreeBSD__) || defined(__DragonFly__) || defined(__NetBSD__) || defined(__OpenBSD__)) || defined(__sun))
#include <X11/Xlib.h>
#endif
#include <unistd.h>
#define STR_SLASH "/"
#define CHR_SLASH '/'
#define HOME_PATH "HOME"
#endif
#define DIGITS_MIN -999999999999999
#define DIGITS_MAX  999999999999999
#if (defined(_WIN32) && defined(_MSC_VER))
#pragma comment(lib, "SDL3.lib")
#endif

using std::string;
using std::wstring;
using std::vector;

SDL_Window *dialog = nullptr;

namespace {

  void message_pump() {
    #if defined(_WIN32)
    MSG msg; while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    #endif
  }
  
  #if defined(_WIN32)
  wstring widen(string str) {
    if (str.empty()) return L"";
    size_t wchar_count = str.size() + 1;
    vector<wchar_t> buf(wchar_count);
    return wstring { buf.data(), (size_t)MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, buf.data(), (int)wchar_count) };
  }

  string narrow(wstring wstr) {
    if (wstr.empty()) return "";
    int nbytes = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.length(), nullptr, 0, nullptr, nullptr);
    vector<char> buf(nbytes);
    return string { buf.data(), (size_t)WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.length(), buf.data(), nbytes, nullptr, nullptr) };
  }

  HICON GetIcon(HWND hWnd) {
    HICON icon = (HICON)SendMessageW(hWnd, WM_GETICON, ICON_SMALL, 0);
    if (icon == nullptr)
      icon = (HICON)GetClassLongPtrW(hWnd, GCLP_HICONSM);
    if (icon == nullptr)
      icon = LoadIconW((HINSTANCE)GetWindowLongPtrW(hWnd, GWLP_HINSTANCE), MAKEINTRESOURCEW(0));
    if (icon == nullptr)
      icon = LoadIconW(nullptr, MAKEINTRESOURCEW(32512));
    return icon;
  }
  #endif

  string string_replace_all(string str, string substr, string nstr) {
    size_t pos = 0;
    while ((pos = str.find(substr, pos)) != string::npos) {
      message_pump();
      str.replace(pos, substr.length(), nstr);
      pos += nstr.length();
    }
    return str;
  }

  vector<string> string_split(string str, char delimiter) {
    vector<string> vec;
    std::stringstream sstr(str); string tmp;
    while (std::getline(sstr, tmp, delimiter)) {
       message_pump(); vec.push_back(tmp);
    }
    return vec;
  }

  string imgui_filter(string input, bool is_folder) {
    input = string_replace_all(input, "\r", "");
    input = string_replace_all(input, "\n", "");
    input = string_replace_all(input, "{", "");
    input = string_replace_all(input, "}", "");
    input = string_replace_all(input, ",", "");
    vector<string> stringVec = string_split(input, '|');
    string string_output;
    unsigned index = 0;
    for (string str : stringVec) {
      message_pump();
      if (index % 2 == 0)
        string_output += str + string("{");
      else {
        std::replace(str.begin(), str.end(), ';', ',');
        string_output += string_replace_all(str, "*.", ".") + string("},");
      }
      index += 1;
    }
    if (!is_folder) {
      string_output += ".*";
    }
    return string_output;
  }

  enum {
    openFile,
    openFiles,
    saveFile,
    selectFolder,
    oneButton,
    twoButtons,
    threeButtons,
    stringInputBox,
    numberInputBox
  };

  string expand_without_trailing_slash(string dname) {
    std::error_code ec;
    dname = ngs::fs::environment_expand_variables(dname);
    ghc::filesystem::path p = ghc::filesystem::path(dname);
    p = ghc::filesystem::absolute(p, ec);
    if (ec.value() != 0) return "";
    dname = p.string();
    #if defined(_WIN32)
    while ((dname.back() == '\\' || dname.back() == '/') &&
      (p.root_name().string() + "\\" != dname && p.root_name().string() + "/" != dname)) {
      message_pump();
      p = ghc::filesystem::path(dname); dname.pop_back();
    }
    #else
    while (dname.back() == '/' && (!dname.empty() && dname[0] != '/' && dname.length() != 1)) {
      dname.pop_back();
    }
    #endif
    return dname;
  }

  string remove_trailing_zeros(double numb) {
    string strnumb = std::to_string(numb);
    while (!strnumb.empty() && strnumb.find('.') != string::npos && (strnumb.back() == '.' || strnumb.back() == '0'))
      strnumb.pop_back();
    return strnumb;
  }

  SDL_Surface *SDL_CreateRGBSurfaceFrom(void *pixels, int width, int height, int depth, int pitch, Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask) {
    return SDL_CreateSurfaceFrom(width, height, SDL_GetPixelFormatForMasks(depth, Rmask, Gmask, Bmask, Amask), pixels, pitch);
  }

  struct ImVec3 { float x, y, z; ImVec3(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f) { x = _x; y = _y; z = _z; } };

  void imgui_easy_theming(ImVec3 color_for_text, ImVec3 color_for_head, ImVec3 color_for_area, ImVec3 color_for_body, ImVec3 color_for_pops) {
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_Text] = ImVec4(color_for_text.x, color_for_text.y, color_for_text.z, 1.00f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(color_for_text.x, color_for_text.y, color_for_text.z, 0.58f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(color_for_body.x, color_for_body.y, color_for_body.z, 0.95f);
    style.Colors[ImGuiCol_Border] = ImVec4(color_for_body.x, color_for_body.y, color_for_body.z, 0.00f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(color_for_body.x, color_for_body.y, color_for_body.z, 0.00f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(color_for_area.x, color_for_area.y, color_for_area.z, 1.00f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.78f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(color_for_area.x, color_for_area.y, color_for_area.z, 1.00f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(color_for_area.x, color_for_area.y, color_for_area.z, 0.75f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(color_for_area.x, color_for_area.y, color_for_area.z, 0.47f);
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(color_for_area.x, color_for_area.y, color_for_area.z, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.21f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.78f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.80f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.50f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
    style.Colors[ImGuiCol_Button] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.50f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.86f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
    style.Colors[ImGuiCol_Header] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.76f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.86f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.15f);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.78f);
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
    style.Colors[ImGuiCol_PlotLines] = ImVec4(color_for_text.x, color_for_text.y, color_for_text.z, 0.63f);
    style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(color_for_text.x, color_for_text.y, color_for_text.z, 0.63f);
    style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.43f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(color_for_pops.x, color_for_pops.y, color_for_pops.z, 0.92f);
  }

  void SetupImGuiStyle2() {
    float text_0 = (float)strtod(ngs::fs::environment_get_variable("IMGUI_TEXT_COLOR_0").c_str(), nullptr);
    float text_1 = (float)strtod(ngs::fs::environment_get_variable("IMGUI_TEXT_COLOR_1").c_str(), nullptr);
    float text_2 = (float)strtod(ngs::fs::environment_get_variable("IMGUI_TEXT_COLOR_2").c_str(), nullptr);
    float head_0 = (float)strtod(ngs::fs::environment_get_variable("IMGUI_HEAD_COLOR_0").c_str(), nullptr);
    float head_1 = (float)strtod(ngs::fs::environment_get_variable("IMGUI_HEAD_COLOR_1").c_str(), nullptr);
    float head_2 = (float)strtod(ngs::fs::environment_get_variable("IMGUI_HEAD_COLOR_2").c_str(), nullptr);
    float area_0 = (float)strtod(ngs::fs::environment_get_variable("IMGUI_AREA_COLOR_0").c_str(), nullptr);
    float area_1 = (float)strtod(ngs::fs::environment_get_variable("IMGUI_AREA_COLOR_1").c_str(), nullptr);
    float area_2 = (float)strtod(ngs::fs::environment_get_variable("IMGUI_AREA_COLOR_2").c_str(), nullptr);
    float body_0 = (float)strtod(ngs::fs::environment_get_variable("IMGUI_BODY_COLOR_0").c_str(), nullptr);
    float body_1 = (float)strtod(ngs::fs::environment_get_variable("IMGUI_BODY_COLOR_1").c_str(), nullptr);
    float body_2 = (float)strtod(ngs::fs::environment_get_variable("IMGUI_BODY_COLOR_2").c_str(), nullptr);
    float pops_0 = (float)strtod(ngs::fs::environment_get_variable("IMGUI_POPS_COLOR_0").c_str(), nullptr);
    float pops_1 = (float)strtod(ngs::fs::environment_get_variable("IMGUI_POPS_COLOR_1").c_str(), nullptr);
    float pops_2 = (float)strtod(ngs::fs::environment_get_variable("IMGUI_POPS_COLOR_2").c_str(), nullptr);
    static ImVec3 color_for_text = ImVec3(text_0, text_1, text_2);
    static ImVec3 color_for_head = ImVec3(head_0, head_1, head_2);
    static ImVec3 color_for_area = ImVec3(area_0, area_1, area_2);
    static ImVec3 color_for_body = ImVec3(body_0, body_1, body_2);
    static ImVec3 color_for_pops = ImVec3(pops_0, pops_1, pops_2);
    imgui_easy_theming(color_for_text, color_for_head, color_for_area, color_for_body, color_for_pops);
  }

  vector<string> fonts;
  SDL_Renderer *renderer = nullptr;
  SDL_Surface *surf = nullptr;
  ImFontAtlas *shared_font_atlas = nullptr;

  string file_dialog_helper(string filter, string fname, string dir, string title, int type, string message = "", string def = "") {   
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "direct3d12,direct3d11,direct3d,metal,vulkan,gpu,software");
    #if defined(SDL_VIDEO_DRIVER_X11)
    ngs::fs::environment_set_variable("SDL_VIDEODRIVER", "x11");
    #endif
    SDL_Window *window = nullptr;
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD);
    SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");
    SDL_WindowFlags windowFlags = (SDL_WindowFlags)(
    ((ngs::fs::environment_get_variable("IMGUI_DIALOG_PARENT").empty()) ? SDL_WINDOW_ALWAYS_ON_TOP : 0) |
    SDL_WINDOW_UTILITY | SDL_WINDOW_HIDDEN | SDL_WINDOW_BORDERLESS);
    window = SDL_CreateWindow(title.c_str(), IFD_DIALOG_WIDTH, IFD_DIALOG_HEIGHT, windowFlags);
    if (window == nullptr) return "";
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    dialog = window;
    if (ngs::fs::environment_get_variable("IMGUI_DIALOG_FULLSCREEN") == std::to_string(1)) {
      SDL_SetWindowFullscreenMode(window, nullptr);
      SDL_SetWindowFullscreen(window, true);
    }
    renderer = SDL_CreateRenderer(window, nullptr);
    if (renderer == nullptr) return "";
    const char *rendererName = SDL_GetRendererName(renderer);
    if (rendererName) {
      //printf("Renderer: %s\n", rendererName);
    }
    IMGUI_CHECKVERSION();
    if (!shared_font_atlas)
      shared_font_atlas = new ImFontAtlas();
    ImGui::CreateContext(shared_font_atlas);
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.IniFilename = nullptr;
    if (ngs::fs::environment_get_variable("IMGUI_FONT_LOADED") != std::to_string(1)) {
      ngs::imgui::ifd_load_fonts();
      if (ngs::fs::environment_get_variable("IMGUI_FONT_SIZE").empty())
      ngs::fs::environment_set_variable("IMGUI_FONT_SIZE", std::to_string(20));
      ImFontConfig config;
      config.MergeMode = true; ImFont *font = nullptr; ImWchar ranges[] = { 0x0020, 0xFFFF, 0 };
      float fontSize = (float)strtod(ngs::fs::environment_get_variable("IMGUI_FONT_SIZE").c_str(), nullptr);
      for (unsigned i = 0; i < fonts.size(); i++) {
        message_pump();
        if (ngs::fs::file_exists(fonts[i])) {
          io.Fonts->AddFontFromFileTTF(fonts[i].c_str(), fontSize, (!i) ? nullptr : &config, ranges);
        }
      }
      if (!io.Fonts->Fonts.empty()) io.Fonts->Build();
      ngs::fs::environment_set_variable("IMGUI_FONT_LOADED", std::to_string(1));
    }
    if (ngs::fs::environment_get_variable("IMGUI_DIALOG_THEME").empty()) {
      ngs::fs::environment_set_variable("IMGUI_DIALOG_THEME", std::to_string(0));
    }
    int theme = (int)strtoul(ngs::fs::environment_get_variable("IMGUI_DIALOG_THEME").c_str(), nullptr, 10);
    if (theme == -1) {
      ImGui::StyleColorsClassic();
    } else if (theme == 0) {
      ImGui::StyleColorsDark();
    } else if (theme == 1) {
      ImGui::StyleColorsLight();
    } else if (theme == 2) {
      SetupImGuiStyle2();
    }
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);
    ifd::FileDialog::Instance().CreateTexture = [](uint8_t *data, int w, int h, char fmt) -> void * {
      if (surf) SDL_DestroySurface(surf);
      surf = SDL_CreateRGBSurfaceFrom((void *)data, w, h, 32, w * 4, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
      SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
      return (void *)tex;
    };
    ifd::FileDialog::Instance().DeleteTexture = [](void *tex) {
      SDL_DestroyTexture((SDL_Texture *)tex);
    };
    ImVec4 clear_color = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    string filterNew = imgui_filter(filter, (type == selectFolder));
    SDL_Event e; string result;
    #if defined(_WIN32)
    HWND hWnd = nullptr;
    RECT parentFrame;
    RECT childFrame;
    int parentFrameWidth = 0;
    int parentFrameHeight = 0;
    int childFrameWidth = 0;
    int childFrameHeight = 0;
    #elif ((defined(__linux__) && !defined(__ANDROID__)) || (defined(__FreeBSD__) || defined(__DragonFly__) || defined(__NetBSD__) || defined(__OpenBSD__)) || defined(__sun))
    Display *display = nullptr;
    Window xWnd = 0;
    XWindowAttributes parentWA;
    unsigned childFrameWidth = 0;
    unsigned childFrameHeight = 0;
    unsigned parentFrameWidth = 0;
    unsigned parentFrameHeight = 0;
    #endif
    while (true) {
      while (SDL_PollEvent(&e)) {
        ImGui_ImplSDL3_ProcessEvent(&e);
      }
      ImGui_ImplSDLRenderer3_NewFrame();
      ImGui_ImplSDL3_NewFrame();
      ImGui::NewFrame();
      ImGui::SetNextWindowPos(ImVec2(0, 0));
      ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y)); dir = expand_without_trailing_slash(dir);
      if (type == openFile) ifd::FileDialog::Instance().Open("GetOpenFileName", (title.empty()) ? "Open File" : title.c_str(), filterNew.c_str(), false, fname.c_str(), dir.c_str());
      else if (type == openFiles) ifd::FileDialog::Instance().Open("GetOpenFileNames", (title.empty()) ? "Open Files" : title.c_str(), filterNew.c_str(), true, fname.c_str(), dir.c_str());
      else if (type == selectFolder) ifd::FileDialog::Instance().Open("GetDirectory", (title.empty()) ? "Select Directory" : title.c_str(), "", false, fname.c_str(), dir.c_str());
      else if (type == saveFile) ifd::FileDialog::Instance().Save("GetSaveFileName", (title.empty()) ? "Save As" : title.c_str(), filterNew.c_str(), fname.c_str(), dir.c_str());
      else if (type == oneButton) {
        vector<string> buttons;
        buttons.push_back(IFD_OK);
        ImGuiAl::MsgBox msgbox;
        ImGui::PushID("##msgbox");
        msgbox.Init("##msgbox", message.c_str(), buttons, false);
        msgbox.Open();
        int selected = msgbox.Draw();
        switch (selected) {
          case 0: result = ""; break;
          case 1: result = IFD_OK; break;
        }
        ImGui::PopID();
        if (selected) goto finish;
      } else if (type == twoButtons) {
        vector<string> buttons;
        buttons.push_back(IFD_YES);
        buttons.push_back(IFD_NO);
        ImGuiAl::MsgBox msgbox;
        ImGui::PushID("##msgbox");
        msgbox.Init("##msgbox", message.c_str(), buttons, false);
        msgbox.Open();
        int selected = msgbox.Draw();
        switch (selected) {
          case 0: result = ""; break;
          case 1: result = IFD_YES; break;
          case 2: result = IFD_NO; break;
        }
        ImGui::PopID();
        if (selected) goto finish;
      } else if (type == threeButtons) {
        vector<string> buttons;
        buttons.push_back(IFD_YES);
        buttons.push_back(IFD_NO);
        buttons.push_back(IFD_CANCEL);
        ImGuiAl::MsgBox msgbox;
        ImGui::PushID("##msgbox");
        msgbox.Init("##msgbox", message.c_str(), buttons, false);
        msgbox.Open();
        int selected = msgbox.Draw();
        switch (selected) {
          case 0: result = ""; break;
          case 1: result = IFD_YES; break;
          case 2: result = IFD_NO; break;
          case 3: result = IFD_CANCEL; break;
        }
        ImGui::PopID();
        if (selected) goto finish;
      } else if (type == stringInputBox) {
        vector<string> buttons;
        if (ngs::fs::environment_get_variable("IMGUI_DIALOG_CANCELABLE") == std::to_string(1)) {
          buttons.push_back(IFD_OK);
          buttons.push_back(IFD_CANCEL);
        } else {
          buttons.push_back(IFD_OK);
        }
        ImGuiAl::MsgBox msgbox;
        ImGui::PushID("##msgbox");
        strcpy(msgbox.Default, def.substr(0, 1023).c_str());
        strcpy(msgbox.Value, msgbox.Default);
        msgbox.Init("##msgbox", message.c_str(), buttons, true);
        msgbox.Open();
        int selected = msgbox.Draw();
        switch (selected) {
          case 0: result = ""; break;
          case 1: result = msgbox.Result; break;
          case 2: result = ""; break;
        }
        ImGui::PopID();
        if (selected) goto finish;
      } else if (type == numberInputBox) {
        vector<string> buttons;
        if (ngs::fs::environment_get_variable("IMGUI_DIALOG_CANCELABLE") == std::to_string(1)) {
          buttons.push_back(IFD_OK);
          buttons.push_back(IFD_CANCEL);
        } else {
          buttons.push_back(IFD_OK);
        }
        ImGuiAl::MsgBox msgbox;
        ImGui::PushID("##msgbox");
        double defnum = strtod(def.c_str(), nullptr);
        if (defnum < DIGITS_MIN) defnum = DIGITS_MIN;
        if (defnum > DIGITS_MAX) defnum = DIGITS_MAX;
        def = remove_trailing_zeros(defnum);
        strcpy(msgbox.Default, def.substr(0, 1023).c_str());
        strcpy(msgbox.Value, msgbox.Default);
        msgbox.Init("##msgbox", message.c_str(), buttons, true);
        msgbox.Open();
        int selected = msgbox.Draw();
        switch (selected) {
          case 0: result = remove_trailing_zeros(0); break;
          case 1: result = remove_trailing_zeros(strtod(msgbox.Result.c_str(), nullptr)); break;
          case 2: result = remove_trailing_zeros(0); break;
        }
        ImGui::PopID();
        if (selected) goto finish;
      }
      if (ifd::FileDialog::Instance().IsDone("GetOpenFileName")) {
        if (ifd::FileDialog::Instance().HasResult()) {
          result = ifd::FileDialog::Instance().GetResult().string();
        }
        ifd::FileDialog::Instance().Close();
        goto finish;
      } else if (ifd::FileDialog::Instance().IsDone("GetOpenFileNames")) {
        if (ifd::FileDialog::Instance().HasResult()) {
          const std::vector<ghc::filesystem::path>& res = ifd::FileDialog::Instance().GetResults();
          for (const auto& r : res) { message_pump(); result += r.string() + "\n"; }
          if (!result.empty()) result.pop_back();
        }
        ifd::FileDialog::Instance().Close();
        goto finish;
      } else if (ifd::FileDialog::Instance().IsDone("GetDirectory")) {
        if (ifd::FileDialog::Instance().HasResult()) {
          result = ifd::FileDialog::Instance().GetResult().string();
          if (!result.empty() && result.back() != CHR_SLASH) result.push_back(CHR_SLASH);
        }
        ifd::FileDialog::Instance().Close();
        goto finish;
      } else if (ifd::FileDialog::Instance().IsDone("GetSaveFileName")) {
        if (ifd::FileDialog::Instance().HasResult()) {
          result = ifd::FileDialog::Instance().GetResult().string();
        }
        ifd::FileDialog::Instance().Close();
        goto finish;
      }
      if (dialog) {
        if (ngs::fs::environment_get_variable("IMGUI_DIALOG_WIDTH").empty() &&
          ngs::fs::environment_get_variable("IMGUI_DIALOG_HEIGHT").empty() &&
          (type == openFile || type == openFiles || type == saveFile || type == selectFolder)) {
          SDL_SetWindowSize(window, 720, 382);
          SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        }
        #if defined(_WIN32)
        SDL_PropertiesID propsId = SDL_GetWindowProperties(window);
        if (propsId) {
          hWnd = (HWND)SDL_GetPointerProperty(propsId, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
          if (hWnd) {
            SetWindowLongPtrW(hWnd, GWL_STYLE, GetWindowLongPtrW(hWnd, GWL_STYLE) & ~(WS_MAXIMIZEBOX | WS_MINIMIZEBOX));
            SetWindowLongPtrW(hWnd, GWL_EXSTYLE, GetWindowLongPtrW(hWnd, GWL_EXSTYLE) |
            ((ngs::fs::environment_get_variable("IMGUI_DIALOG_PARENT").empty()) ? WS_EX_TOPMOST : 0));
            SetWindowPos(hWnd, ((ngs::fs::environment_get_variable("IMGUI_DIALOG_PARENT").empty()) ?  HWND_TOPMOST : HWND_TOP),
            0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
            if (!ngs::fs::environment_get_variable("IMGUI_DIALOG_PARENT").empty()) {
              EnableWindow((HWND)(void *)(std::uintptr_t)strtoull(
              ngs::fs::environment_get_variable("IMGUI_DIALOG_PARENT").c_str(), nullptr, 10), FALSE);
              if (IsIconic((HWND)(void *)(std::uintptr_t)strtoull(
              ngs::fs::environment_get_variable("IMGUI_DIALOG_PARENT").c_str(), nullptr, 10)))
              ShowWindow((HWND)(void *)(std::uintptr_t)strtoull(
              ngs::fs::environment_get_variable("IMGUI_DIALOG_PARENT").c_str(), nullptr, 10), SW_RESTORE);
              SetWindowLongPtrW(hWnd, GWLP_HWNDPARENT, (LONG_PTR)(std::uintptr_t)strtoull(
              ngs::fs::environment_get_variable("IMGUI_DIALOG_PARENT").c_str(), nullptr, 10));
              GetWindowRect((HWND)(void *)(std::uintptr_t)strtoull(
              ngs::fs::environment_get_variable("IMGUI_DIALOG_PARENT").c_str(), nullptr, 10), &parentFrame);
              parentFrameWidth = parentFrame.right - parentFrame.left;
              parentFrameHeight = parentFrame.bottom - parentFrame.top;
              GetWindowRect(hWnd, &childFrame);
              childFrameWidth = childFrame.right - childFrame.left;
              childFrameHeight = childFrame.bottom - childFrame.top;
              MoveWindow(hWnd, (parentFrame.left + (parentFrameWidth / 2)) - (childFrameWidth / 2),
              (parentFrame.top + (parentFrameHeight / 2)) - (childFrameHeight / 2), childFrameWidth, childFrameHeight, TRUE);
              PostMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)GetIcon((HWND)(void *)(std::uintptr_t)strtoull(
              ngs::fs::environment_get_variable("IMGUI_DIALOG_PARENT").c_str(), nullptr, 10)));
            }
            SDL_Rect rect;
            bool inside = false;
            int x = 0, y = 0, w = 0, h = 0;
            SDL_GetWindowPosition(window, &x, &y);
            if (SDL_GetCurrentRenderOutputSize(SDL_GetRenderer(window), &w, &h)) {
              int numDisplays = 0;
              SDL_DisplayID *dispId = SDL_GetDisplays(&numDisplays);
              if (dispId) {
                if (numDisplays >= 1) {
                  for (int i = 0; i < numDisplays; i++) {
                    message_pump();
                    if (SDL_GetDisplayBounds(dispId[i], &rect)) {
                      if (x >= rect.x && y >= rect.y &&
                      x + w <= rect.x + rect.w && y + h <= rect.y + rect.h) {
                        inside = true;
                        break;
                      }
                    }
                  }
                }
                SDL_free(dispId);
              }
            }
            if (!inside) {
              SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
            }
          }
        }
        #elif (defined(__APPLE__) && defined(__MACH__))
        SDL_PropertiesID propsId = SDL_GetWindowProperties(window);
        if (propsId) {
          NSWindow *nsWnd = (NSWindow *)SDL_GetPointerProperty(propsId, SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, nullptr);
          if (nsWnd) {
            [[nsWnd standardWindowButton:NSWindowCloseButton] setHidden:NO];
            [[nsWnd standardWindowButton:NSWindowMiniaturizeButton] setHidden:YES];
            [[nsWnd standardWindowButton:NSWindowZoomButton] setHidden:YES];
            [[nsWnd standardWindowButton:NSWindowCloseButton] setEnabled:YES];
            [[nsWnd standardWindowButton:NSWindowMiniaturizeButton] setEnabled:NO];
            [[nsWnd standardWindowButton:NSWindowZoomButton] setEnabled:NO];
            if (!ngs::fs::environment_get_variable("IMGUI_DIALOG_PARENT").empty()) {
              [(NSWindow *)(void *)(std::uintptr_t)strtoull(
              ngs::fs::environment_get_variable("IMGUI_DIALOG_PARENT").c_str(), nullptr, 10)
              addChildWindow:nsWnd ordered:NSWindowAbove];
              NSRect parentFrame = [(NSWindow *)(void *)(std::uintptr_t)strtoull(
              ngs::fs::environment_get_variable("IMGUI_DIALOG_PARENT").c_str(), nullptr, 10) frame];
              NSRect childFrame = [nsWnd frame]; [nsWnd setFrame:NSMakeRect(
              (parentFrame.origin.x + (parentFrame.size.width / 2)) - (childFrame.size.width / 2),
              (parentFrame.origin.y + (parentFrame.size.height / 2)) - (childFrame.size.height / 2),
              childFrame.size.width, childFrame.size.height) display:YES];
              [nsWnd makeKeyAndOrderFront:nil];
            }
          }
        }
        #elif ((defined(__linux__) && !defined(__ANDROID__)) || (defined(__FreeBSD__) || defined(__DragonFly__) || defined(__NetBSD__) || defined(__OpenBSD__)) || defined(__sun))
        SDL_PropertiesID propsId = SDL_GetWindowProperties(window);
        if (propsId) {
          display = (Display *)SDL_GetPointerProperty(propsId, SDL_PROP_WINDOW_X11_DISPLAY_POINTER, nullptr);
          if (display) {
            xWnd = (Window)SDL_GetNumberProperty(propsId, SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0);
            if (xWnd) {
              if (!ngs::fs::environment_get_variable("IMGUI_DIALOG_PARENT").empty()) {
                Window xwindow = (Window)(std::uintptr_t)strtoull(
                ngs::fs::environment_get_variable("IMGUI_DIALOG_PARENT").c_str(), nullptr, 10);
                XSetTransientForHint(display, xWnd, xwindow);
              }
            }
          }
        }
        #endif
        dialog = nullptr;
      }
      ImGui::Render();
      SDL_SetRenderDrawColor(renderer, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
      SDL_RenderClear(renderer);
      ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
      SDL_RenderPresent(renderer);
      #if ((defined(__linux__) && !defined(__ANDROID__)) || (defined(__FreeBSD__) || defined(__DragonFly__) || defined(__NetBSD__) || defined(__OpenBSD__)) || defined(__sun))
      if (!ngs::fs::environment_get_variable("IMGUI_DIALOG_PARENT").empty()) {
        Window xwindow = (Window)(std::uintptr_t)strtoull(
        ngs::fs::environment_get_variable("IMGUI_DIALOG_PARENT").c_str(), nullptr, 10);
        Window parentFrameRoot = 0; int parentFrameX = 0, parentFrameY = 0;
        Window parentWindow = 0, rootWindow = 0, *childrenWindows = nullptr;
        unsigned numberOfChildren = 0;
        while (true) {
          if (XQueryTree(display, xwindow, &rootWindow, &parentWindow, &childrenWindows, &numberOfChildren) == 0) {
            break;
          }
          if (childrenWindows) {
            XFree(childrenWindows);
          }
          if (xwindow == rootWindow || parentWindow == rootWindow) {
            break;
          } else {
            xwindow = parentWindow;
          }
        }
        XGetWindowAttributes(display, xwindow, &parentWA);
        unsigned parentFrameBorder = 0, parentFrameDepth = 0;
        XGetGeometry(display, xwindow, &parentFrameRoot, &parentFrameX, &parentFrameY,
        &parentFrameWidth, &parentFrameHeight, &parentFrameBorder, &parentFrameDepth);
        Window childFrameRoot = 0; int childFrameX = 0, childFrameY = 0;
        unsigned childFrameBorder = 0, childFrameDepth = 0;
        XGetGeometry(display, xWnd, &childFrameRoot, &childFrameX, &childFrameY,
        &childFrameWidth, &childFrameHeight, &childFrameBorder, &childFrameDepth);
        XMoveWindow(display, xWnd, (parentWA.x + (parentFrameWidth / 2)) - (childFrameWidth / 2),
        (parentWA.y + (parentFrameHeight / 2)) - (childFrameHeight / 2));
      }
      #endif
      if (SDL_GetWindowFlags(window) & SDL_WINDOW_HIDDEN) {
        SDL_ShowWindow(window);
      }
    }
    finish:
    #if defined(_WIN32)
    if (!ngs::fs::environment_get_variable("IMGUI_DIALOG_PARENT").empty()) {
      EnableWindow((HWND)(void *)(std::uintptr_t)strtoull(
      ngs::fs::environment_get_variable("IMGUI_DIALOG_PARENT").c_str(), nullptr, 10), TRUE);
    }
    #endif
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyWindow(window);
    dialog = nullptr;
    window = nullptr;
    return result;
  }

} // anonymous namespace

namespace ngs::imgui {

  void ifd_load_fonts() {
    if (!fonts.empty()) fonts.clear();
    if (ngs::fs::environment_get_variable("IMGUI_FONT_PATH").empty() && ngs::fs::environment_get_variable("IMGUI_FONT_FILES").empty()) {
      fonts.push_back(ngs::fs::directory_contents_first(ngs::fs::executable_get_directory() + "fonts", "*.ttf;*.otf;*.ttc", false, false));
      while (!fonts[fonts.size() - 1].empty()) {
        message_pump();
        fonts.push_back(ngs::fs::directory_contents_next());
      }
      if (!fonts.empty()) fonts.pop_back();
      ngs::fs::directory_contents_close();
    } else if (!ngs::fs::environment_get_variable("IMGUI_FONT_PATH").empty()) {
      fonts.push_back(ngs::fs::directory_contents_first(ngs::fs::environment_get_variable("IMGUI_FONT_PATH"), "*.ttf;*.otf;*.ttc", false, false));
      while (!fonts[fonts.size() - 1].empty()) {
        message_pump();
        fonts.push_back(ngs::fs::directory_contents_next());
      }
      if (!fonts.empty()) fonts.pop_back();
      ngs::fs::directory_contents_close();
    } else if (!ngs::fs::environment_get_variable("IMGUI_FONT_FILES").empty()) {
      fonts = string_split(string_replace_all(ngs::fs::environment_get_variable("IMGUI_FONT_FILES"), "\r", ""), '\n');
      while (!fonts.empty() && fonts[fonts.size() - 1].empty()) {
        message_pump();
        fonts.pop_back();
      }
    }
  }

  string get_open_filename(string filter, string fname) {
    return file_dialog_helper(((filter.empty()) ? "*.*" : filter), fname, ngs::fs::environment_get_variable(HOME_PATH), "Open", openFile);
  }

  string get_open_filename_ext(string filter, string fname, string dir, string title) {
    return file_dialog_helper(((filter.empty()) ? "*.*" : filter), fname, ((dir.empty()) ? ngs::fs::environment_get_variable(HOME_PATH) : dir), title, openFile);
  }

  string get_open_filenames(string filter, string fname) {
    return file_dialog_helper(((filter.empty()) ? "*.*" : filter), fname, ngs::fs::environment_get_variable(HOME_PATH), "Open", openFiles);
  }
 
  string get_open_filenames_ext(string filter, string fname, string dir, string title) {
    return file_dialog_helper(((filter.empty()) ? "*.*" : filter), fname, ((dir.empty()) ? ngs::fs::environment_get_variable(HOME_PATH) : dir), title, openFiles);
  }

  string get_save_filename(string filter, string fname) {
    return file_dialog_helper(((filter.empty()) ? "*.*" : filter), fname, ngs::fs::environment_get_variable(HOME_PATH), "Save As", saveFile);
  }

  string get_save_filename_ext(string filter, string fname, string dir, string title) {
    return file_dialog_helper(((filter.empty()) ? "*.*" : filter), fname, ((dir.empty()) ? ngs::fs::environment_get_variable(HOME_PATH) : dir), title, saveFile);
  }

  string get_directory(string dname) {
    return file_dialog_helper("", "", ((dname.empty()) ? ngs::fs::environment_get_variable(HOME_PATH) : dname), "Select Directory", selectFolder);
  }

  string get_directory_alt(string capt, string root) {
    return file_dialog_helper("", "", ((root.empty()) ? ngs::fs::environment_get_variable(HOME_PATH) : root), capt, selectFolder);
  }
  
  string show_message(string message) {
    return file_dialog_helper("", "", "", ngs::fs::environment_get_variable("IMGUI_DIALOG_CAPTION"), oneButton, message);
  }

  string show_question(string message) {
    return file_dialog_helper("", "", "", ngs::fs::environment_get_variable("IMGUI_DIALOG_CAPTION"), twoButtons, message);
  }

  string show_question_ext(string message) {
    return file_dialog_helper("", "", "", ngs::fs::environment_get_variable("IMGUI_DIALOG_CAPTION"), threeButtons, message);
  }

  string get_string(string message, string defstr) {
    return file_dialog_helper("", "", "", ngs::fs::environment_get_variable("IMGUI_DIALOG_CAPTION"), stringInputBox, message, defstr);
  }

  double get_number(string message, double defnum) {
    double result = strtod(file_dialog_helper("", "", "", ngs::fs::environment_get_variable("IMGUI_DIALOG_CAPTION"), numberInputBox,
    message, remove_trailing_zeros(defnum)).c_str(), nullptr);
    if (result < DIGITS_MIN) result = DIGITS_MIN;
    if (result > DIGITS_MAX) result = DIGITS_MAX;
    return result;
  }

} // namespace ngs::imgui

#if defined(IFD_SHARED_LIBRARY)
void ifd_load_fonts() {
  ngs::imgui::ifd_load_fonts();
}

const char *get_open_filename(const char *filter, const char *fname) {
  static string result;
  result = ngs::imgui::get_open_filename(filter, fname);
  return result.c_str();
}

const char *get_open_filename_ext(const char *filter, const char *fname, const char *dir, const char *title) {
  static string result;
  result = ngs::imgui::get_open_filename_ext(filter, fname, dir, title);
  return result.c_str();
}

const char *get_open_filenames(const char *filter, const char *fname) {
  static string result;
  result = ngs::imgui::get_open_filenames(filter, fname);
  return result.c_str();
}

const char *get_open_filenames_ext(const char *filter, const char *fname, const char *dir, const char *title) {
  static string result;
  result = ngs::imgui::get_open_filenames_ext(filter, fname, dir, title);
  return result.c_str();
}

const char *get_save_filename(const char *filter, const char *fname) {
  static string result;
  result = ngs::imgui::get_save_filename(filter, fname);
  return result.c_str();
}

const char *get_save_filename_ext(const char *filter, const char *fname, const char *dir, const char *title) {
  static string result;
  result = ngs::imgui::get_save_filename_ext(filter, fname, dir, title);
  return result.c_str();
}

const char *get_directory(const char *dname) {
  static string result;
  result = ngs::imgui::get_directory(dname);
  return result.c_str();
}

const char *get_directory_alt(const char *capt, const char *root) {
  static string result;
  result = ngs::imgui::get_directory_alt(capt, root);
  return result.c_str();
}

const char *show_message(const char *message) {
  static string result;
  result = ngs::imgui::show_message(message);
  return result.c_str();
}

const char *show_question(const char *message) {
  static string result;
  result = ngs::imgui::show_question(message);
  return result.c_str();
}

const char *show_question_ext(const char *message) {
  static string result;
  result = ngs::imgui::show_question_ext(message);
  return result.c_str();
}

const char *get_string(const char *message, const char *defstr) {
  static string result;
  result = ngs::imgui::get_string(message, defstr);
  return result.c_str();
}

double get_number(const char *message, double defnum) {
  return ngs::imgui::get_number(message, defnum);
}
#endif
