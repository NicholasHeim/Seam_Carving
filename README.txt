Project 4 README.txt
Author: Nicholas Heim
Uakron Email: nwh8

Purpose: 
The purpose of this program is for decreasing the size of a photo (grayscale, 256 shades) without losing the main 
points of the piece. This is done by reading the image, calculating the change in energy between pixels within the
image, and then removing the string of pixels (connected by always moving forward one and the connected pixel
being one above, one below, or on exactly the same level) either vertically or horizontally. 

CMake file:
Put the images in the main containing folder (nheim_4) and run cmake in a contained directory called "build"
The image files will be coppied automatically. If you wish to put them directly into build, remove the 
corresponding statements in CMakeLists.txt. Or, you may put them directly into the build folder.

Running the program:
console command inside build directory: "./carve YOUR_IMAGE.pgm VERTICAL_SEAMS_TO_REMOVE HORIZONTAL_SEAMS_TO_REMOVE"