# OpenCV-bokeh
Using OpenCV to add a depth-of-field bokeh effect to 3D scenes rendered by OpenGL. Uses some optimizations and tricks such as dividing the Z-axis into slices that get coarser the further they are from focus, and some faux HDR by spreading very bight points more.

##Demo
![example1](https://i.imgur.com/vZSiCRD.png)

![example2](https://i.imgur.com/vK0xHGI.png)

##Behind the scenes
Here is the plain render of the scene, and a sample of the depth buffer.

![example3](https://i.imgur.com/LXU2FtW.png)

The program slices the image into different depths based on the pixel information in the depth buffer, and applies variable blur to different depths. Optionally, very bright spots can be blurred more to achieve a more natural effect.
