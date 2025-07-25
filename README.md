# SDL3 ImGui File Dialogs - CLI and Client Library

Contact me on [Discord](https://discord.com) if you have any comments, questions, or concerns, which might not belong in a GitHub issues ticket. My Discord handle is `samuelvenable`. This project is based on [ImFileDialog](https://github.com/dfranx/ImFileDialog) by [dfranx](https://github.com/dfranx), with many bugs/crashes fixed and overall improvements. The 'Quick Access' sidebar actually remembers what favorites were previously saved to it from previous runs of your application now, by saving the settings to a text file in a hidden configuration subfolder of your home folder. Allows for full localization among many other good things you'll find useful. Most of the dialog is customizable via environment variables. To create your own default list of favorites for your application, here is a minimal example:

```
/*
** Alternative main.cpp - replace "SDL3-ImGui-FileDialogs/filedialogs/main.cpp" with this exact code
*/

#include <iostream> // std::cout, std::endl
#include <string>   // std::string, std::to_string
#include <vector>   // std::vector
#include <cstddef>  // std::size_t

#include "ImFileDialog/ImFileDialogMacros.h" // Easy Localization
#include "filedialogs.hpp"                   // NGS File Dialogs
#include "ghc/filesystem.hpp"                // GHC File System
#include "filesystem.hpp"                    // NGS File System

#if (defined(__APPLE__) && defined(__MACH__))
// Compile with: -framework AppKit -ObjC++
#include <AppKit/AppKit.h> // NSApplication
#endif

/* setup home directory
environment variable */
#if !defined(HOME_PATH)
#if defined(_WIN32) // Windows
#define HOME_PATH "USERPROFILE"
#else // Unix-likes
#define HOME_PATH "HOME"
#endif
#endif

// for SDL3
#if defined(_WIN32)
#undef main
#endif

// for MSVC
#if (defined(_WIN32) && defined(_MSC_VER))
#pragma comment(linker, "/subsystem:console /entry:mainCRTStartup")
#endif

namespace {
  void init() {
    #if defined(_WIN32)
    SetConsoleOutputCP(CP_UTF8);
    #elif (defined(__APPLE__) && defined(__MACH__))
    // hide icon from dock on macOS to match all the other platforms
    [[NSApplication sharedApplication] setActivationPolicy:(NSApplicationActivationPolicy)1];
    if (@available(macOS 14.0, *)) {
      [[NSApplication sharedApplication] yieldActivationToApplication:[NSRunningApplication currentApplication]];
      [[NSApplication sharedApplication] activate];
    } else {
      [[NSApplication sharedApplication] activateIgnoringOtherApps:YES];
    }
    #endif

    // set imgui file dialogs window width and height; default is 720x382 pixels
    ngs::fs::environment_set_variable("IMGUI_DIALOG_WIDTH", std::to_string(720));
    ngs::fs::environment_set_variable("IMGUI_DIALOG_HEIGHT", std::to_string(382));
    
    // load all *.ttf and *.otf fonts of varying languages and combine them into one font from directory
    ngs::fs::environment_set_variable("IMGUI_FONT_PATH", ngs::fs::executable_get_directory() + "fonts");
    ngs::fs::environment_set_variable("IMGUI_FONT_SIZE", std::to_string(20)); // font size for dialogbox

    // setup imgui file dialog favorites config file absolute pathname
    ngs::fs::environment_set_variable("IMGUI_CONFIG_HOME", HOME_PATH);
    ngs::fs::environment_set_variable("IMGUI_CONFIG_FOLDER", "imfiledialogs");
    ngs::fs::environment_set_variable("IMGUI_CONFIG_FILE", "fdfavorites.txt");
    
    // if imgui file dialog favorites config file absolute pathname does not exist, then create one
    if (!ngs::fs::file_exists("${IMGUI_CONFIG_HOME}/.config/${IMGUI_CONFIG_FOLDER}/${IMGUI_CONFIG_FILE}")) {
      // setup favorites std::vector
      std::vector<std::string> favorites;
      favorites.push_back(ngs::fs::environment_get_variable(HOME_PATH));
      favorites.push_back(ngs::fs::directory_get_desktop_path());
      favorites.push_back(ngs::fs::directory_get_documents_path());
      favorites.push_back(ngs::fs::directory_get_downloads_path());
      favorites.push_back(ngs::fs::directory_get_music_path());
      favorites.push_back(ngs::fs::directory_get_pictures_path());
      favorites.push_back(ngs::fs::directory_get_videos_path());
      // add custom favorites to config text file; this file is not encrypted and may be easily edited by any software
      int desc = ngs::fs::file_text_open_write("${IMGUI_CONFIG_HOME}/.config/${IMGUI_CONFIG_FOLDER}/${IMGUI_CONFIG_FILE}");
      if (desc != -1) { // success; file can now be written to
        for (std::size_t i = 0; i < favorites.size(); i++) {
          // write favorite from vector at current index "i"
          ngs::fs::file_text_write_string(desc, favorites[i]);
          ngs::fs::file_text_writeln(desc); // write new line
        }
        // close file descriptor    
        ngs::fs::file_text_close(desc);
      }
    }
  }
} // anonymous namespace

int main() {
  init(); // setup all initialization related settings
  std::cout << ngs::imgui::get_open_filename("Portable Network Graphics (*.png)|*.png|" +
  std::string("Graphics Interchange Format (*.gif)|*.gif"), "Untitled.png") << std::endl;
  return 0;
}
```

# Platforms

Supports Windows, macOS, Linux, FreeBSD, DragonFly BSD, NetBSD, OpenBSD, and SunOS.

![win32.png](win32.png)

![macos.png](macos.png)

![linux.png](linux.png)

**Build Dependencies:**
- Windows: Visual Studio (or MSYS2, MinGW, pacman, g++, make, pkg-config), sdl3
- macOS: Xcode command line tools, MacPorts (for universal binaries), clang++, make, sdl3
- Linux: posix-compliant shell at "/bin/sh", g++, make, sdl3, x11, gtk+3.0, gio2.0, glib2.0, pkg-config, cmake
- FreeBSD: clang++, make, sdl3, x11, gtk+3.0, gio2.0, glib2.0, pkg-config, cmake
- DragonFly BSD: g++, make, sdl3, x11, gtk+3.0, gio2.0, glib2.0, pkg-config, cmake
- NetBSD: g++, make, sdl3, x11, gtk+3.0, gio2.0, glib2.0, pkg-config, cmake
- OpenBSD: clang++, make, sdl3, x11, gtk+3.0, gio2.0, glib2.0, pkg-config, cmake
- SunOS: g++, make, sdl3, x11, gtk+3.0, gio2.0, glib2.0, pkg-config, cmake
