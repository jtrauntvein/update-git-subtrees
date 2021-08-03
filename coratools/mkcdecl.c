/* mkcdecl.c

   Copyright (C) 2005, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Saturday 30 July 2005
   Last Change: Saturday 13 January 2018
   Last Commit: $Date: 2019-11-15 16:56:05 -0700 (Fri, 15 Nov 2019) $ 
   Last Changed by: $Author: maddae-kagyah $

*/
/*
   This file and its content are copyright of Campbell Scientific, Inc. - © 2016, 2019 Campbell Scientific, Inc.

   This file is protected by copyright and has been copied and distributed solely for the operational purposes of
   Campbell Scientific products under license. You may not sell, alter or further reproduce or distribute any part
   of this file to any other person or entity. Where provided to you in electronic format, you may only use it for
   work and research involving the use of Campbell Scientific dataloggers and affiliated products. Failure to comply
   with the terms of this warning may expose you to penalties under the law or legal action for copyright infringement."
*/

#pragma hdrstop               // stop creation of precompiled header
#include <stdio.h>
#include <time.h>


char const *help_string =
"This program is a simple simple filter that takes its standard input and\n"
"produces in its standard output an initialised string variable that contains\n"
"content of its standard input.  This output will be suitable for including in\n"
"a \"C\" header file.  The first argument specifies the name of the variable.\n"
"Subsequent arguments specify the namespace in which the variable will be\n"
"declared.  The second is the root namespace while the third will lie within\n"
"the first and so forth.";


/**
 * Main
 */
int main(int argc, char const *argv[])
{
   int rtn = 0;
   if(argc >= 4)
   {
      int namespace_count = 0;
      int i;
      int ch;
      int count = 0;
      FILE *input, *output;
      time_t now;
      char formatted_time[56];
      
      // we need to format the current GMT time as an HTTP date 
      time(&now);
      strftime(formatted_time, sizeof(formatted_time), "%Y-%m-%d %H:%M:%S", gmtime(&now));
      
      // we need to attempt to open the input and output files.
      input = fopen(argv[2],"rb");
      if(input == 0)
      {
         fprintf(stderr,"Unable to open input: %s\n",argv[2]);
         return -2;
      }
      output = fopen(argv[1],"wb");
      if(output == 0)
      {
         fclose(input);
         fprintf(stderr,"Unable to open output: %s\n",argv[1]);
         return -3;
      }
      
      // We need to write the namespaces in which the variable will be
      // contained. 
      while(namespace_count + 4 < argc)
      {
         for(i = 0; i < namespace_count; ++i)
            fprintf(output,"   ");
         fprintf(output,"namespace %s\n",argv[4 + namespace_count]);
         for(i = 0; i < namespace_count; ++i)
            fprintf(output,"   ");
         fprintf(output, "{\n");
         ++namespace_count;
      }

      // we will now transform the input into a "C" variable declaration.
      // First we need to write the declaration
      for(i = 0; i < namespace_count; ++i)
         fprintf(output,"   ");
      fprintf(output, "char const %s_date[] = \"%s\";\n", argv[3], formatted_time); 
      for(i = 0; i < namespace_count; ++i)
         fprintf(output,"   ");
      fprintf(output,"unsigned char const %s[] =\n",argv[3]);
      for(i = 0; i < namespace_count; ++i)
         fprintf(output, "   ");
      fprintf(output, "{\n");

      // we will now output each character in the file as an element in the array
      while((ch = fgetc(input)) != EOF)
      {
         if(count % 16 == 0)
         {
            fprintf(output, "\n");
            for(i = 0; i < namespace_count + 1; ++i)
               fprintf(output, "   ");
         }
         fprintf(output,"0x%02x, ",ch);
         ++count; 
      }

      // we will now output a null terminator for the array and close the array definition
      fprintf(output, "\n");
      for(i = 0; i < namespace_count; ++i)
         fprintf(output, "   ");
      fprintf(output, "};\n");

      // finally, we need to close the namespaces
      for(i = namespace_count; i > 0; --i)
      {
         int j;
         for(j = 1; j < i; ++j)
            fprintf(output,"   ");
         fprintf(output,"};\n");
      }
   }
   else
   {
      fputs(help_string,stderr);
      rtn = -1;
   }
   return rtn;
} /* main */
