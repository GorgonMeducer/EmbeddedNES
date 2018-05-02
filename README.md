# EmbeddedNES

A portable NES simulator for embedded processors. 


## Update Log

- 03-05-2018 PPU refactory

    * Improve the PPU performance and readabiligy. 
    
    * Add dedicated jeg_cfg.h for configuration purpose. 
    
    * Improve the display quality - solve the blinking-sprite issue.

- 20-04-2018 Support both LiteNES and JEG
    
    You can switching two emulator in MDK projects:
    
    * V2M-MPS2-jeg for JEG

        PPU looks good, but 6502 emulation part need to be rewritten. PPU performance could also be improved. 

    * V2M-MPS2-LiteNES.
    
        The performance has been improved. The PPU looks buggy and I decide to not use it any more.
    
   
## How to test it

    Download the latest version of jeg from https://github.com/semiversus/jeg and unzip all the files under EmbeddedNES\mps2_template\example\jeg folder.
    
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
    
    
    Enjoy.