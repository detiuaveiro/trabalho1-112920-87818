// imageTest - A program that performs some image processing.
//
// This program is an example use of the image8bit module,
// a programming project for the course AED, DETI / UA.PT
//
// You may freely use and modify this code, NO WARRANTY, blah blah,
// as long as you give proper credit to the original and subsequent authors.
//
// João Manuel Rodrigues <jmr@ua.pt>
// 2023

#include <assert.h>
#include <errno.h>
#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "image8bit.h"
#include "instrumentation.h"

int main(int argc, char* argv[]) {
  program_name = argv[0];
  if (argc != 3) {
    error(1, 0, "Usage: imageTest input.pgm output.pgm");
  }

  ImageInit();
  
  printf("# LOAD Image 1");
  InstrReset(); // to reset instrumentation
  Image img1 = ImageLoad("test/paste.pgm");
  if (img1 == NULL) {
    error(2, errno, "Loading %s: %s", "test/paste.pgm", ImageErrMsg());
  }
  InstrPrint(); // to print instrumentation

  printf("# LOAD Image 2");
  InstrReset(); // to reset instrumentation
  Image img2 = ImageLoad("test/small.pgm");
  if (img2 == NULL) {
    error(2, errno, "Loading %s: %s", "test/small.pgm", ImageErrMsg());
  }
  InstrPrint(); // to print instrumentation

  // Try changing the behaviour of the program by commenting/uncommenting
  // the appropriate lines.

  //img2 = ImageCrop(img1, ImageWidth(img1)/4, ImageHeight(img1)/4, ImageWidth(img1)/2, ImageHeight(img1)/2);
  //Image img2 = ImageRotate(img1);

  printf("# LOCATE image");
  InstrReset(); // to reset instrumentation
  int pxTemp = 0;
	int pyTemp = 0;
	int* px = &pxTemp;
	int* py = &pyTemp;
  int success = ImageLocateSubImage(img1,px,py,img2);
  if (success == 0) {
    error(2, errno, "LOCATE img: %s", ImageErrMsg());
  }
  InstrPrint(); // to print instrumentation

  printf("# BLUR image");
  InstrReset(); // to reset instrumentation
  ImageBlur(img1,2,2);
  if (img1 == NULL) {
    error(2, errno, "Blur img: %s", ImageErrMsg());
  }
  InstrPrint(); // to print instrumentation

  // gcc imageTest.c error.c instrumentation.c image8bit.c -o imageTest.exe

  //ImageNegative(img2);
  //ImageThreshold(img2, 100);
  //ImageBrighten(img2, 1.3);

  //if (ImageSave(img2, argv[2]) == 0) {
  //  error(2, errno, "%s: %s", argv[2], ImageErrMsg());
  //}

  ImageDestroy(&img1);
  ImageDestroy(&img2);
  return 0;
}

