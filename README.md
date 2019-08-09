# EvoXDTSR: What is EvoXDTSR?

EvoXDTSR was one of the early wave of applications designed to integrate with some of the custom homebrew and debug BIOS installs for the original Xbox. Its aim was to facilitate manipulating games running directly on the Xbox from your computer. It allowed you to set breakpoints and manipulate memory.

Eventually more robust homebrew BIOSes hit the scene, and EvoXDTSR was replaced by XDK Assist (<https://github.com/swichers/xdk-assist>).

## Features

* All the standard TSR commands
* Saves your breakpoint info when using set/enable/disable/clear
* Can automatically log to file
* Can freeze/unfreeze the game when doing a search

# Why release the code so late?

I released the code for XDK Assist and thought why not go ahead and release my other well used Xbox project along with it. I doubt anyone is still using this, but I had the code, so here ya go.

# Requirements

Surprisingly compiled without any issue in Visual Studio 2019 Community Edition. You must enable MFC components, and there are a lot of deprecation warnings, but otherwise everything was peachy out of the box.