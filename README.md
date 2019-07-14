# Realtime Webcam

Grab and display webcam images with minimal latency.

## Motivation

This design was chosen to minimize the latency for grabbing and displaying
images so that more time can be spent processing them in interesting ways. 

This design was made after becoming frustrated with the limitations of OpenCV,
which are described below:

*cv::VideoCapture* only exposes an API for grabbing decoded frames. It does not
allow clients to read encoded frames. It would be more efficient to grab an
encoded frame and pass it directly to the GPU where it can be decoded and
further processed.

*cv::imshow* has 10ms latency on my machine. If we are processing a 30FPS webcam
stream, then we are spending roughly a third of our runtime budget just
displaying the frame.

## Requirements

### Linux

This was only tested on linux (Ubuntu 18.04).

It may or may not work on other OS.

### ffmpeg libraries 

We use ffmpeg libraries to grab frames from the webcam.

```bash
sudo apt-get install libavcodec-dev libavdevice-dev libavformat-dev libavutil-dev
```

Tested with these versions:
libavcodec: 57.107.100
libavdevice: 57.10.100
libavformat: 57.83.100
libavutil: 55.78.100

### SDL2

We use SDL2 to render frames to the screen.

```bash
sudo apt-get install libsdl2-dev libsdl2-image-dev
```

Tested with these versions:
lbsd2l: 2.0-0
libsdl2-image: 2.0-0
