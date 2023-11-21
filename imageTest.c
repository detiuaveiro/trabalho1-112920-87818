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
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "image8bit.h"
#include "instrumentation.h"

int main(int argc, char* argv[])
{
  program_name = argv[0];
  if (argc != 3) { error(1, 0, "Usage: imageTest input.pgm output.pgm"); }
  ImageInit();
 
  // cl imageTest.c image8bit.c instrumentation.c error.c
 
  //Image img1 = ImageLoad("ComplexityTests/ImageLocateTests/Small/art4_300x300.pgm");
  //Image img2 = ImageLoad("test/small.pgm");
  //InstrReset();
  //int pxTemp = 0;
	//int pyTemp = 0;
	//int* px = &pxTemp;
	//int* py = &pyTemp;
  //int success = ImageLocateSubImage(img1,px,py,img2);
  //if (success == 0) { error(2, errno, "LOCATE img: %s", ImageErrMsg()); }
  //InstrPrint();

  Image img1 = ImageLoad("ComplexityTests/ImageLocateTests/Small/art3_222x217.pgm");
  InstrReset();
  ImageBlur(img1,2,2);
  InstrPrint();

  ImageDestroy(&img1);
  //ImageDestroy(&img2);
  return 0;
}