Description
-----------
Compilation of various basic programs about computer vision. It uses [OpenCV](http://opencv.org/).


Installation
------------
* Linux: `sudo apt-get install libopencv-dev libcv-dev libcvaux-dev libhighgui-dev`.
    * for a manual installation, see [there](https://help.ubuntu.com/community/OpenCV).
* Mac OS X: `brew tap homebrew/science && brew install opencv`.


Compilation
-----------
Don't forget to link the OpenCV library: `` `pkg-config --cflags --libs opencv` `` (**WARNING**: order matters, it must be at the end of the compilation command line, not between `g++` and `test.c -o test`).

Example: ``g++ test.c -o test `pkg-config --cflags --libs opencv` ``.

