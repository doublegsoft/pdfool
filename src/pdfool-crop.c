#include <stdio.h>
#include <pdfio.h>

int main(int argc, const char* argv[])
{
  pdfio_file_t* pdf = pdfioFileOpen("../../data/000233597294.pdf", 
                                    NULL,
                                    NULL, 
                                    NULL,
                                    NULL);
  return 0;
}