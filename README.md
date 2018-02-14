# Vulkan 1.0.61.1 Compute w/ GLFW 3.2.1

N-Body example using Vulkan Computes. 
- [Other examples](https://github.com/SaschaWillems/Vulkan#examples) that can be converted to run on this.
- Works on Windows, should work on Linux, can re-add Android support. 

With:
- GLI
- GLM
- STB
- ImGui
- Assimp

I compiled this on VS 2015, the project files are included.

How:
- Based off Sascha Willems' Example: https://github.com/SaschaWillems/Vulkan
- I also used Niko Kauppi's youtube tutorial: https://www.youtube.com/watch?v=wHt5wcxIPcE&list=PLUXvZMiAqNbK8jd7s52BIDtCbZnKNGp0P
- And, of course, Vulkan's site: https://vulkan-tutorial.com/

![Screenshot](https://i.imgur.com/uGdGT2H.png)

<div>

I used Vulkan 1.0.61.1, and the other additional dependencies are in the the provided file named as such.

Find the working .exe in x64/Debug. You can change the particle and shader behaviors without recompiling by tweaking the particles.comp shader then reloading the SPIRV numbers by clicking the generate-spirv batch file in the shaders folder next to the .exe.

Only real issue is window resizing, as well as the non-working VulkanUIOverlay.cpp (found in Project1/unused/need updating). This will enable overlay menus using ImGui.

Sascha's examples need only be modified to fit the correct VulkanBase class name, and the correct main loop sequence as indicated in the comments below the VulkanBase class in VulkanBase.h. There are hundreds of examples, though some will require the nonworking file.
<div>

Changes:

12/31: Changed the example to N-Body and added all the files that needed updating, plus dependencies. Only one that did not work correctly was VulkanUIOverlay.cpp. 
