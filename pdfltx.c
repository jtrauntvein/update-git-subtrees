/* pdfltx.c

   Copyright (C) 1998, Campbell Scientific, Inc

   Written by: Jon Trauntvein
   Date Begun: Thursday 26 August 1999
   Last Change: Monday 02 July 2001
   Last Commit: $Date: 2009-12-22 13:09:23 -0600 (Tue, 22 Dec 2009) $ (UTC)
   
*/

#include <stdio.h>
#include <string.h>

int main(int argc, char const *argv[])
{
   /* check the arguments */
   int rtn;
   if(argc >= 2)
   {
      /* form the command to be sent to the shell */
      static char command[2048];
      FILE *results;
      int build_complete = 0;
      int arg_no = 2;
      int execute_once = 0;
      int echo_output = 1;
      int pass_count = 0;

      strcpy(command,"pdflatex '\\nonstopmode");
      while(arg_no < argc)
      {
         if((strcmp(argv[arg_no],"-d") == 0 || strcmp(argv[arg_no],"--define") == 0) &&
            arg_no + 1 < argc)
         {
            /* separate the define name from the definition */
            char define_name[128];
            char define_body[128];
            char *def_pos;

            memset(define_name,0,sizeof(define_name));
            memset(define_body,0,sizeof(define_body));
            strncpy(define_name,argv[arg_no + 1],sizeof(define_name) - 1);
            def_pos = strchr(define_name,'=');
            if(def_pos)
            {
               strcpy(define_body,def_pos + 1);
               *def_pos = 0;
            }

            /* generate the macro code */
            strcat(command,"\\def\\");
            strcat(command,define_name);
            strcat(command,"{");
            strcat(command,define_body);
            strcat(command,"}");
            arg_no += 2;
         }
         else if(strcmp(argv[arg_no],"-o") == 0 || strcmp(argv[arg_no],"--once") == 0)
         {
            execute_once = 1;
            arg_no++;
         }
         else if(strcmp(argv[arg_no],"-q") == 0 || strcmp(argv[arg_no],"--quiet") == 0)
         {
            echo_output = 0;
            arg_no++;
         }
         else
         {
            puts("Invalid switch or invalid number of arguments");
            return 1;
         }
      }
      strcat(command,"\\input{");
      strcat(command,argv[1]);
      strcat(command,"}'");

      /* execute the command multiple times until the rerun message goes away or an error shows up */
      rtn = 0;
      if(echo_output)
         puts("Output will be echoed to stdout");
      while(rtn == 0 && !build_complete)
      {
         printf("Executing pass %d\n",++pass_count);
         if((results = popen(command,"r")) != NULL)
         {
            /* read the output from the process */
            build_complete = 1;
            while(!feof(results))
            {
               static char buffer[1024];
               if(fgets(buffer,sizeof(buffer),results))
               {
                  if(echo_output)
                     fputs(buffer,stdout);
                  if(strstr(buffer,"Rerun"))
                     build_complete = 0;
                  if(strstr(buffer,"Error"))
                  {
                     if(!echo_output)
                        fputs(buffer,stdout);
                     rtn = 3;
                  }
                  if(strstr(buffer,"Warning") && !echo_output)
                     fputs(buffer,stdout);
               }
            }
            fclose(results);
            if(execute_once)
               build_complete = 1;
         }
         else
         {
            printf("Failed to execute command: %s\n",command);
            rtn = 2;
         }
      }
   }
   else
   {
      puts("Usage:\n    pdfltx source_file_name {option}");
      rtn = 1;
   }
   return rtn;
} /* main */

