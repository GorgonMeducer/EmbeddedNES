# EmbeddedNES

A portable NES simulator for embedded processors

## How to test it
    
    A MDK (https://www.keil.com/demo/eval/arm.htm) project has been provided:
    
    ./mps2_template/example/build/mdk
    
    No real hardware is required but you need a professional license (which you can request an 7-days evaluation professional license) to run the Fast Model.
    
    After you open the project with MDK:
    
    *  Compile it 
    and 
    *  Press the Debug button. 
    
    A Cortex-M7 Fast Model (a.k.a FVP) will be launched. If you don't see anything happened, please make sure "Models Cortex-M Debugger" is selected in the debugger settings by checking following path:
    
    Menu Project->Options for Target->Debug
    
    All the configuration and settings of FVP should be included in this project. If you find nothing, please let me know.
    
    If you are lucky enough to have a MPS2+ board, then you can debug it on the  real hardware. (Currently, it is extremely slow, I am working on it...)
    
    After successfully launching the FVP, please reset the processor and press F5 to start the debug.
    
    Enjoy.