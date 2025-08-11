#!/bin/sh
cd "${0%/*}"

# build command line executable
if [ `uname` = "Darwin" ]; then
  sudo port install SDL3 +universal libiconv +universal;
  clang++ "ImFileDialog/ImFileDialog.cpp" "imgui/imgui.cpp" "imgui/imgui_impl_sdl3.cpp" "imgui/imgui_impl_sdlgpu3.cpp" "imgui/imgui_impl_sdlrenderer3.cpp" "imgui/imgui_draw.cpp" "imgui/imgui_tables.cpp" "imgui/imgui_widgets.cpp" "filesystem.cpp" "filedialogs.cpp" "msgbox/imguial_msgbox.cpp" "main.cpp" -o "filedialogs" -std=c++17 -Wno-format-security -I. -DIMGUI_USE_WCHAR32 -I/opt/local/include -I/opt/local/include -ObjC++ `pkg-config --cflags --libs sdl3 --static` -liconv -Wl,-framework,CoreAudio -Wl,-framework,AudioToolbox -Wl,-weak_framework,CoreHaptics -Wl,-weak_framework,GameController -Wl,-framework,ForceFeedback -lobjc -Wl,-framework,CoreVideo -Wl,-framework,Cocoa -Wl,-framework,Carbon -Wl,-framework,IOKit -Wl,-weak_framework,QuartzCore -Wl,-weak_framework,Metal -fPIC -arch arm64 -arch x86_64 -fPIC;
  cp -fr "/opt/local/lib/libSDL3.dylib" "./libSDL3.dylib"
  install_name_tool -id @loader_path/libSDL3.dylib ./libSDL3.dylib
  install_name_tool -change /opt/local/lib/libSDL3.0.dylib @loader_path/libSDL3.dylib ./filedialogs
  install_name_tool -change /opt/local/lib/libiconv.2.dylib /usr/lib/libiconv.2.dylib ./filedialogs
  cp -fr "./filedialogs" "../filedialogs.app/Contents/MacOS/filedialogs";
  cp -fr "./libSDL3.dylib" "../filedialogs.app/Contents/MacOS/libSDL3.dylib"
elif [ $(uname) = "Linux" ]; then
  cd "lunasvg";
  rm -f "CMakeCache.txt";
  cmake .;
  make;
  cd ..;
  git clone -b release-3.2.x https://github.com/libsdl-org/SDL;
  cd SDL;
  mkdir build;
  cd build;
  cmake ..;
  make;
  sudo make install;
  cd ../..;
  ln -s "/usr/local/lib/libSDL3.so" "./libSDL3.so"
  ln -s "/usr/local/lib/libSDL3.so.0" "./libSDL3.so.0"
  g++ "ImFileDialog/ImFileDialog.cpp" "imgui/imgui.cpp" "imgui/imgui_impl_sdl3.cpp" "imgui/imgui_impl_sdlgpu3.cpp" "imgui/imgui_impl_sdlrenderer3.cpp" "imgui/imgui_draw.cpp" "imgui/imgui_tables.cpp" "imgui/imgui_widgets.cpp" "filesystem.cpp" "filedialogs.cpp" "msgbox/imguial_msgbox.cpp" "main.cpp" -o "filedialogs" -std=c++17 -Wno-format-security -I. -Ilunasvg/include "lunasvg/liblunasvg.a" "lunasvg/plutovg/libplutovg.a" -DIMGUI_USE_WCHAR32 -static-libgcc -static-libstdc++ `pkg-config --cflags sdl3 --static` `pkg-config --cflags --libs gtk+-3.0` `pkg-config --cflags --libs gio-2.0` `pkg-config --cflags --libs glib-2.0` -lX11 -lpthread -no-pie -Wl,--rpath="\$ORIGIN" -lSDL3 -fPIC;
elif [ $(uname) = "FreeBSD" ]; then
  cd "lunasvg";
  rm -f "CMakeCache.txt";
  cmake .;
  make;
  cd ..;
  clang++ "ImFileDialog/ImFileDialog.cpp" "imgui/imgui.cpp" "imgui/imgui_impl_sdl3.cpp" "imgui/imgui_impl_sdlgpu3.cpp" "imgui/imgui_impl_sdlrenderer3.cpp" "imgui/imgui_draw.cpp" "imgui/imgui_tables.cpp" "imgui/imgui_widgets.cpp" "filesystem.cpp" "filedialogs.cpp" "msgbox/imguial_msgbox.cpp" "main.cpp" -o "filedialogs" -std=c++17 -Wno-format-security -I. -Ilunasvg/include "lunasvg/liblunasvg.a" "lunasvg/plutovg/libplutovg.a" -DIMGUI_USE_WCHAR32 `pkg-config --cflags --libs sdl3 --static` `pkg-config --cflags --libs gtk+-3.0` `pkg-config --cflags --libs gio-2.0` `pkg-config --cflags --libs glib-2.0` -lX11 -lc -lpthread -fPIC;
elif [ $(uname) = "DragonFly" ]; then
  cd "lunasvg";
  rm -f "CMakeCache.txt";
  cmake .;
  make;
  cd ..;
  g++ "ImFileDialog/ImFileDialog.cpp" "imgui/imgui.cpp" "imgui/imgui_impl_sdl3.cpp" "imgui/imgui_impl_sdlgpu3.cpp" "imgui/imgui_impl_sdlrenderer3.cpp" "imgui/imgui_draw.cpp" "imgui/imgui_tables.cpp" "imgui/imgui_widgets.cpp" "filesystem.cpp" "filedialogs.cpp" "msgbox/imguial_msgbox.cpp" "main.cpp" -o "filedialogs" -std=c++17 -Wno-format-security -I. -Ilunasvg/include "lunasvg/liblunasvg.a" "lunasvg/plutovg/libplutovg.a" -DIMGUI_USE_WCHAR32 -static-libgcc -static-libstdc++ `pkg-config --cflags --libs sdl3 --static` `pkg-config --cflags --libs gtk+-3.0` `pkg-config --cflags --libs gio-2.0` `pkg-config --cflags --libs glib-2.0` -lX11 -lc -lpthread -fPIC;
elif [ $(uname) = "NetBSD" ]; then
  cd "lunasvg";
  rm -f "CMakeCache.txt";
  cmake .;
  make;
  cd ..;
  g++ "ImFileDialog/ImFileDialog.cpp" "imgui/imgui.cpp" "imgui/imgui_impl_sdl3.cpp" "imgui/imgui_impl_sdlgpu3.cpp" "imgui/imgui_impl_sdlrenderer3.cpp" "imgui/imgui_draw.cpp" "imgui/imgui_tables.cpp" "imgui/imgui_widgets.cpp" "filesystem.cpp" "filedialogs.cpp" "msgbox/imguial_msgbox.cpp" "main.cpp" -o "filedialogs" -std=c++17 -Wno-format-security -I. -Ilunasvg/include "lunasvg/liblunasvg.a" "lunasvg/plutovg/libplutovg.a" -DIMGUI_USE_WCHAR32 -static-libgcc -static-libstdc++ `pkg-config --cflags --libs sdl3 --static` `pkg-config --cflags --libs gtk+-3.0` `pkg-config --cflags --libs gio-2.0` `pkg-config --cflags --libs glib-2.0` -lX11 -lc -lpthread -fPIC;
elif [ $(uname) = "OpenBSD" ]; then
  cd "lunasvg";
  rm -f "CMakeCache.txt";
  cmake .;
  make;
  cd ..;
  clang++ "ImFileDialog/ImFileDialog.cpp" "imgui/imgui.cpp" "imgui/imgui_impl_sdl3.cpp" "imgui/imgui_impl_sdlgpu3.cpp" "imgui/imgui_impl_sdlrenderer3.cpp" "imgui/imgui_draw.cpp" "imgui/imgui_tables.cpp" "imgui/imgui_widgets.cpp" "filesystem.cpp" "filedialogs.cpp" "msgbox/imguial_msgbox.cpp" "main.cpp" -o "filedialogs" -std=c++17 -Wno-format-security -I. -Ilunasvg/include "lunasvg/liblunasvg.a" "lunasvg/plutovg/libplutovg.a" -DIMGUI_USE_WCHAR32 -I/usr/local/include `pkg-config --cflags --libs sdl3 --static` `pkg-config --cflags --libs gtk+-3.0` `pkg-config --cflags --libs gio-2.0` `pkg-config --cflags --libs glib-2.0` -lX11 -lc -lkvm -lpthread -fPIC;
elif [ $(uname) = "SunOS" ]; then
  cd "lunasvg";
  rm -f "CMakeCache.txt";
  cmake .;
  make;
  cd ..;
  export PKG_CONFIG_PATH=/usr/lib/64/pkgconfig;
  g++ "ImFileDialog/ImFileDialog.cpp" "imgui/imgui.cpp" "imgui/imgui_impl_sdl3.cpp" "imgui/imgui_impl_sdlgpu3.cpp" "imgui/imgui_impl_sdlrenderer3.cpp" "imgui/imgui_draw.cpp" "imgui/imgui_tables.cpp" "imgui/imgui_widgets.cpp" "filesystem.cpp" "filedialogs.cpp" "msgbox/imguial_msgbox.cpp" "main.cpp" -o "filedialogs" -std=c++17 -Wno-format-security -I. -Ilunasvg/include "lunasvg/liblunasvg.a" "lunasvg/plutovg/libplutovg.a" -DIMGUI_USE_WCHAR32 -static-libgcc `pkg-config --cflags --libs sdl3 --static` `pkg-config --cflags --libs gtk+-3.0` `pkg-config --cflags --libs gio-2.0` `pkg-config --cflags --libs glib-2.0` -lX11 -lc -lpthread -fPIC;
else
  windres "resources.rc" -o "resources.o";
  g++ "ImFileDialog/ImFileDialog.cpp" "imgui/imgui.cpp" "imgui/imgui_impl_sdl3.cpp" "imgui/imgui_impl_sdlgpu3.cpp" "imgui/imgui_impl_sdlrenderer3.cpp" "imgui/imgui_draw.cpp" "imgui/imgui_tables.cpp" "imgui/imgui_widgets.cpp" "filesystem.cpp" "filedialogs.cpp" "msgbox/imguial_msgbox.cpp" "main.cpp" "resources.o" -o "filedialogs.exe" -std=c++17 -I. -D_UNICODE -DUNICODE -DIMGUI_USE_WCHAR32 -static-libgcc -static-libstdc++ -static `pkg-config --cflags --libs sdl3 --static` -lshell32 -mconsole -fPIC;
  rm -f "resources.o";
fi

# build shared library
if [ `uname` = "Darwin" ]; then
  clang++ "ImFileDialog/ImFileDialog.cpp" "imgui/imgui.cpp" "imgui/imgui_impl_sdl3.cpp" "imgui/imgui_impl_sdlgpu3.cpp" "imgui/imgui_impl_sdlrenderer3.cpp" "imgui/imgui_draw.cpp" "imgui/imgui_tables.cpp" "imgui/imgui_widgets.cpp" "filesystem.cpp" "filedialogs.cpp" "msgbox/imguial_msgbox.cpp" -o "libfiledialogs.dylib" -std=c++17 -DIFD_SHARED_LIBRARY -shared -Wno-format-security -I. -DIMGUI_USE_WCHAR32 -I/opt/local/include -I/opt/local/include -ObjC++ `pkg-config --cflags --libs sdl3 --static` -liconv -Wl,-framework,CoreAudio -Wl,-framework,AudioToolbox -Wl,-weak_framework,CoreHaptics -Wl,-weak_framework,GameController -Wl,-framework,ForceFeedback -lobjc -Wl,-framework,CoreVideo -Wl,-framework,Cocoa -Wl,-framework,Carbon -Wl,-framework,IOKit -Wl,-weak_framework,QuartzCore -Wl,-weak_framework,Metal -fPIC -arch arm64 -arch x86_64 -fPIC;
  cp -fr "/opt/local/lib/libSDL3.dylib" "./libSDL3.dylib"
  install_name_tool -id @loader_path/libSDL3.dylib ./libSDL3.dylib
  install_name_tool -change /opt/local/lib/libSDL3.0.dylib @loader_path/libSDL3.dylib ./libfiledialogs.dylib
  install_name_tool -change /opt/local/lib/libiconv.2.dylib /usr/lib/libiconv.2.dylib ./libfiledialogs.dylib
elif [ $(uname) = "Linux" ]; then
  cd "lunasvg";
  rm -f "CMakeCache.txt";
  cmake .;
  make;
  cd ..;
  g++ "ImFileDialog/ImFileDialog.cpp" "imgui/imgui.cpp" "imgui/imgui_impl_sdl3.cpp" "imgui/imgui_impl_sdlgpu3.cpp" "imgui/imgui_impl_sdlrenderer3.cpp" "imgui/imgui_draw.cpp" "imgui/imgui_tables.cpp" "imgui/imgui_widgets.cpp" "filesystem.cpp" "filedialogs.cpp" "msgbox/imguial_msgbox.cpp" -o "libfiledialogs.so" -std=c++17 -DIFD_SHARED_LIBRARY -shared -Wno-format-security -I. -Ilunasvg/include "lunasvg/liblunasvg.a" "lunasvg/plutovg/libplutovg.a" -DIMGUI_USE_WCHAR32 -static-libgcc -static-libstdc++ `pkg-config --cflags sdl3 --static` `pkg-config --cflags --libs gtk+-3.0` `pkg-config --cflags --libs gio-2.0` `pkg-config --cflags --libs glib-2.0` -lX11 -lpthread -Wl,--rpath="\$ORIGIN" -lSDL3 -fPIC;
elif [ $(uname) = "FreeBSD" ]; then
  cd "lunasvg";
  rm -f "CMakeCache.txt";
  cmake .;
  make;
  cd ..;
  clang++ "ImFileDialog/ImFileDialog.cpp" "imgui/imgui.cpp" "imgui/imgui_impl_sdl3.cpp" "imgui/imgui_impl_sdlgpu3.cpp" "imgui/imgui_impl_sdlrenderer3.cpp" "imgui/imgui_draw.cpp" "imgui/imgui_tables.cpp" "imgui/imgui_widgets.cpp" "filesystem.cpp" "filedialogs.cpp" "msgbox/imguial_msgbox.cpp" -o "libfiledialogs.so" -std=c++17 -DIFD_SHARED_LIBRARY -shared -Wno-format-security -I. -Ilunasvg/include "lunasvg/liblunasvg.a" "lunasvg/plutovg/libplutovg.a" -DIMGUI_USE_WCHAR32 `pkg-config --cflags --libs sdl3 --static` `pkg-config --cflags --libs gtk+-3.0` `pkg-config --cflags --libs gio-2.0` `pkg-config --cflags --libs glib-2.0` -lX11 -lc -lpthread -fPIC;
elif [ $(uname) = "DragonFly" ]; then
  cd "lunasvg";
  rm -f "CMakeCache.txt";
  cmake .;
  make;
  cd ..;
  g++ "ImFileDialog/ImFileDialog.cpp" "imgui/imgui.cpp" "imgui/imgui_impl_sdl3.cpp" "imgui/imgui_impl_sdlgpu3.cpp" "imgui/imgui_impl_sdlrenderer3.cpp" "imgui/imgui_draw.cpp" "imgui/imgui_tables.cpp" "imgui/imgui_widgets.cpp" "filesystem.cpp" "filedialogs.cpp" "msgbox/imguial_msgbox.cpp" -o "libfiledialogs.so" -std=c++17 -DIFD_SHARED_LIBRARY -shared -Wno-format-security -I. -Ilunasvg/include "lunasvg/liblunasvg.a" "lunasvg/plutovg/libplutovg.a" -DIMGUI_USE_WCHAR32 -static-libgcc -static-libstdc++ `pkg-config --cflags --libs sdl3 --static` `pkg-config --cflags --libs gtk+-3.0` `pkg-config --cflags --libs gio-2.0` `pkg-config --cflags --libs glib-2.0` -lX11 -lc -lpthread -fPIC;
elif [ $(uname) = "NetBSD" ]; then
  cd "lunasvg";
  rm -f "CMakeCache.txt";
  cmake .;
  make;
  cd ..;
  g++ "ImFileDialog/ImFileDialog.cpp" "imgui/imgui.cpp" "imgui/imgui_impl_sdl3.cpp" "imgui/imgui_impl_sdlgpu3.cpp" "imgui/imgui_impl_sdlrenderer3.cpp" "imgui/imgui_draw.cpp" "imgui/imgui_tables.cpp" "imgui/imgui_widgets.cpp" "filesystem.cpp" "filedialogs.cpp" "msgbox/imguial_msgbox.cpp" -o "libfiledialogs.so" -std=c++17 -DIFD_SHARED_LIBRARY -shared -Wno-format-security -I. -Ilunasvg/include "lunasvg/liblunasvg.a" "lunasvg/plutovg/libplutovg.a" -DIMGUI_USE_WCHAR32 -static-libgcc `pkg-config --cflags --libs sdl3 --static` `pkg-config --cflags --libs gtk+-3.0` `pkg-config --cflags --libs gio-2.0` `pkg-config --cflags --libs glib-2.0` -lX11 -lc -lpthread -fPIC;
elif [ $(uname) = "OpenBSD" ]; then
  cd "lunasvg";
  rm -f "CMakeCache.txt";
  cmake .;
  make;
  cd ..;
  clang++ "ImFileDialog/ImFileDialog.cpp" "imgui/imgui.cpp" "imgui/imgui_impl_sdl3.cpp" "imgui/imgui_impl_sdlgpu3.cpp" "imgui/imgui_impl_sdlrenderer3.cpp" "imgui/imgui_draw.cpp" "imgui/imgui_tables.cpp" "imgui/imgui_widgets.cpp" "filesystem.cpp" "filedialogs.cpp" "msgbox/imguial_msgbox.cpp" -o "libfiledialogs.so" -std=c++17 -DIFD_SHARED_LIBRARY -shared -Wno-format-security -I. -Ilunasvg/include "lunasvg/liblunasvg.a" "lunasvg/plutovg/libplutovg.a" -DIMGUI_USE_WCHAR32 -I/usr/local/include `pkg-config --cflags --libs sdl3 --static` `pkg-config --cflags --libs gtk+-3.0` `pkg-config --cflags --libs gio-2.0` `pkg-config --cflags --libs glib-2.0` -lX11 -lc -lkvm -lpthread -fPIC;
elif [ $(uname) = "SunOS" ]; then
  cd "lunasvg";
  rm -f "CMakeCache.txt";
  cmake .;
  make;
  cd ..;
  export PKG_CONFIG_PATH=/usr/lib/64/pkgconfig;
  g++ "ImFileDialog/ImFileDialog.cpp" "imgui/imgui.cpp" "imgui/imgui_impl_sdl3.cpp" "imgui/imgui_impl_sdlgpu3.cpp" "imgui/imgui_impl_sdlrenderer3.cpp" "imgui/imgui_draw.cpp" "imgui/imgui_tables.cpp" "imgui/imgui_widgets.cpp" "filesystem.cpp" "filedialogs.cpp" "msgbox/imguial_msgbox.cpp" -o "libfiledialogs.so" -std=c++17 -DIFD_SHARED_LIBRARY -shared -Wno-format-security -I. -Ilunasvg/include "lunasvg/liblunasvg.a" "lunasvg/plutovg/libplutovg.a" -DIMGUI_USE_WCHAR32 -static-libgcc `pkg-config --cflags --libs sdl3 --static` `pkg-config --cflags --libs gtk+-3.0` `pkg-config --cflags --libs gio-2.0` `pkg-config --cflags --libs glib-2.0` -lX11 -lc -lpthread -fPIC;
else
  g++ "ImFileDialog/ImFileDialog.cpp" "imgui/imgui.cpp" "imgui/imgui_impl_sdl3.cpp" "imgui/imgui_impl_sdlgpu3.cpp" "imgui/imgui_impl_sdlrenderer3.cpp" "imgui/imgui_draw.cpp" "imgui/imgui_tables.cpp" "imgui/imgui_widgets.cpp" "filesystem.cpp" "filedialogs.cpp" "msgbox/imguial_msgbox.cpp" -o "libfiledialogs.dll" -std=c++17 -DIFD_SHARED_LIBRARY -shared -I. -D_UNICODE -DUNICODE -DIMGUI_USE_WCHAR32 -static-libgcc -static-libstdc++ -static `pkg-config --cflags --libs sdl3 --static` -lshell32 -fPIC;
fi
