#!/bin/sh
cd "${0%/*}";
if [ `uname` = "Darwin" ]; then
  sudo xcode-select --install;
  if [ -f "/opt/local/bin/port" ]; then
    "./filedialogs/build.sh";
  else
    echo "MacPorts installation not found! Please download and install MacPorts first from www.macports.org";
  fi
elif [ $(uname) = "Linux" ]; then
  "./filedialogs/build.sh";
elif [ $(uname) = "FreeBSD" ]; then
  "./filedialogs/build.sh";
elif [ $(uname) = "DragonFly" ]; then
  "./filedialogs/build.sh";
elif [ $(uname) = "NetBSD" ]; then
  "./filedialogs/build.sh";
elif [ $(uname) = "OpenBSD" ]; then
  "./filedialogs/build.sh";
elif [ $(uname) = "SunOS" ]; then
  "./filedialogs/build.sh";
else
  "./filedialogs/build.sh";
fi
