//Copyright (C) Rayonix, LLC
#include <stdio.h>
#include <unistd.h>

#include "RayonixHsCamera.h"

int main() {
   int desiredFrameCount = 10;
   
   lima::RayonixHs::Camera camera;

   printf("Setting exposure time...");
   camera.setExpTime(0);
   printf("done.\n");

   printf("Setting num frames...");
   camera.setNbFrames(desiredFrameCount);
   printf("done.\n");
   
   printf("Preparing acquisition...\n");
   camera.prepareAcq();
   
   printf("Starting acquisition...\n");
   camera.startAcq();
   while(!camera.acquiring()) {}
   
   printf("Waiting on completion...\n");
   while(camera.acquiring()) {
      usleep(100 * 1000);
      //std::cout << "Still acquring..." << std::endl;
   }
   
   printf("All done.\n");

   return 0;
}
