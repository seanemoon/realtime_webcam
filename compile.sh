g++ -std=c++11 -Wall \
   main.cc \
  `pkg-config --cflags --libs libavcodec libavdevice libavformat libavutil` \
  `sdl2-config --cflags --libs` -lSDL2_image
