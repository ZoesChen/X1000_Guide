############################################################
1. How to compile

For 1 mic, please use
    make clean
    make

For 4 mic, please use:
    make use_ula=1 clean
    make use_ula=1

############################################################
2. How to get degree

   Register a callback function to get degree of the speaker
                                     See cloud_sem.c

############################################################
3. How to execute
   Copy bin/ libs/ aiengine-1mic and aiengine-4mic to where
you what to place, for example
   "cp bin/ libs/ aiengine-* /tmp/ -fr" and then execute:
   ./aiengine-1mic or
   ./aiengine-4mic

############################################################
4. Status Diagram

   while(1)
   {
   		Wait for Wakeup
   		Semantic Identify
   		Speech Synthesis
   }

