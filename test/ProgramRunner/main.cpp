#include "../../Csi.ProgramRunner.h"
#include "../../Csi.StrAscStream.h"
#include "../../Csi.SimpleDispatch.h"
#include <iostream>


////////////////////////////////////////////////////////////
// class Exithandler
////////////////////////////////////////////////////////////
class ExitHandler: public Csi::EventReceiver
{
public:
   ////////////////////////////////////////////////////////////
   // constructor
   ////////////////////////////////////////////////////////////
   ExitHandler():
      exit_code(0)
   { }
   
   ////////////////////////////////////////////////////////////
   // receive
   ////////////////////////////////////////////////////////////
   virtual void receive(Csi::SharedPtr<Csi::Event> &ev)
   {
      if(ev->getType() == Csi::ProgramRunner::event_program_ended::event_id)
      {
         Csi::ProgramRunner::event_program_ended *event(
            static_cast<Csi::ProgramRunner::event_program_ended *>(ev.get_rep()));
         exit_code = event->exit_code;
         Csi::Event::post_quit_message(exit_code);
      }
   } // receive

   ////////////////////////////////////////////////////////////
   // exit_code
   ////////////////////////////////////////////////////////////
   int exit_code;
};


int main(int argc, char *argv[])
{
   // form the command ,line
   Csi::OStrAscStream command_line;
   for(int i = 2; i < argc; ++i)
      command_line << " " << argv[1];

   // initialise the event dispatcher
   Csi::SimpleDispatch *dispatch(new Csi::SimpleDispatch);
   Csi::Event::set_dispatcher(dispatch);
   
   // start the program execution
   ExitHandler handler;
   Csi::ProgramRunner runner(
      &handler,               
      "./test1",
      command_line.str().c_str()); 
   runner.start();

   // we now need to drive the dispatcher
   while(dispatch->do_dispatch())
      0;
   Csi::Event::set_dispatcher(0);
   std::cout << "The process returned " << handler.exit_code << std::endl;
   return handler.exit_code;
} // main
