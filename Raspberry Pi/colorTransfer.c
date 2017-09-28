// colorTransfer.c
// wballiet@g.hmc.edu 19 Nov 2016
// modified from lab7.c bchasnov@hmc.edu, david_harris@hmc.edu 15 October 2015
//
// Send color data to FPGA over SPI


////////////////////////////////////////////////
// #includes
////////////////////////////////////////////////

#include <stdio.h>
#include <stlib.h>

////////////////////////////////////////////////
// Main
////////////////////////////////////////////////

void main(void) {
  char colors[3];

  // Read in the colors from stdin and skips the junk (assumes values are posted)
  fgets(colors, 7, stdin);

  // Write the new colors to a file
  FILE *fp = fopen("~/.webcolors.txt", "w");
  fputc(colors[0]);
  fputc(colors[1]);
  fputc(colors[2]);
  fclose(fp);

  // HTML header
  printf("%s%c%c\n", "Content-Type:text/html;charset=iso-8859-1",13,10);

  // Redirect to the color picker with no delay
  printf("<META HTTP-EQUIV=\"Refresh\" CONTENT="0;url=/index.html\">");
  return 0;
}

