# EmbeddedNES

A portable NES simulator for embedded processors. 


## Update Log

- 18-05-2018 PPU performance improvement

    * Add buffers for individual sprites
    
    * Clean code, remove unused macro switch, i.e. JEG_USE_DIRTY_MATRIX

- 16-05-2018 Add more default roms

    * Add two default roms
        - ROM1: City Tanks
        - ROM2: Road Fighter
        - ROM3: Super Mario Bro
        - ROM4: Contra (Need implementing new mapper)

- 15-05-2018 Add support for STM32F746G-Discovery
    
    * Add dedicated BSP folders for V2M-MPS2 and STM32F746G-Discovery
    
    * Add dedicated macro switch to disable FILE-IO 
        
        Disable it so you can port and test the NES emulator with default City Tank rom

- 07-05-2008 PPU improvement 

    * Seperate PPU into two versions, one for scanline based and one for partial-scanline based (while another part is pixel based)
    
    * Apply compact dual pixels to background buffer
    
        TODO: Fix debug mode
        
    * Improve the readability

- 05-05-2018 PPU improvement
    
    * Add background buffer for name tables.
    
    * Add Dirty Matrix to background buffer refreshing scheme. Only changed tile will be updated. 
    
    * Add debug mode to show background buffer. 
    
    * Gain > 10% performance improvement.

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