
        AREA    nes_rom, DATA, READONLY
        EXPORT  NES_ROM_3

; Includes the binary file MyBinFile1.bin from the current source folder
NES_ROM_3
        INCBIN  Super Mario Bros (E).nes
NES_ROM_3_End

; Use a relative or absolute path to other folders if necessary
;       INCBIN  c:\project\MyBinFile1.bin
; Add further binary files to merge them if necessary
;       INCBIN  MyBinFile2.bin

; define a constant which contains the size of the image above
NES_ROM_3_Length
        DCD     NES_ROM_3_End - NES_ROM_3

        EXPORT  NES_ROM_3_Length

        END