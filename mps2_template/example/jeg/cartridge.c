#include "cartridge.h"
#include "common.h"
#include "jeg_cfg.h"

nes_err_t cartridge_setup(cartridge_t *ptCartridge, uint8_t *pchData, uint_fast32_t wSize) 
{
    iNES_t *ptHeader = (iNES_t *)pchData;
    uint_fast32_t wPRGSize, wCHRSize;
    
    if (NULL == ptCartridge || NULL == pchData) {
        return nes_err_illegal_pointer;
    }

    // check minimum size (header is 16 bytes)
    if (wSize < sizeof(iNES_t)) {
        return nes_err_illegal_size;
    }

    if (strncmp("NES\x1A", (const char *)&ptHeader->chSign, 4)) {
        return nes_err_invalid_rom;
    }

    wPRGSize = 0x4000 * ptHeader->chPRGROMBankCount;
    wCHRSize = 0x2000 * ptHeader->chCHRROMBankCount;

    ptCartridge->chMirror =     ptHeader->VerticalMirroring 
                            +  (ptHeader->FourScreenMirroring << 1);

    if (strncmp("DiskDude!", (const char *)ptHeader->chDiskDudeString, 9)) {
        //! if "DiskDude!" string is present, ignore upper 4 bits for mapper
        ptCartridge->chMapper = ptHeader->MapperLow;
    } else {
        ptCartridge->chMapper = ptHeader->MapperLow | (ptHeader->MapperHigh << 4);
    }


    if (ptCartridge->chMapper !=0 && ptCartridge->chMapper !=3 ) { // ines #3 is just a quick fix for a test rom
        return nes_err_unsupported_mapper;
    }

    if (wSize < (    wPRGSize 
                +    wCHRSize 
                +    sizeof(iNES_t)                                             //!< should be 16 bytes
                +    (ptHeader->Trainer ? 512 : 0 ))) {
        return nes_err_imcomplete_rom;
    }

    memset(ptCartridge->chIOData, 0, 0x2000);

    //! skip header and trainer data
    ptCartridge->pchPRGMemory =   pchData 
                            +   sizeof(iNES_t)                                  //!< should be 16 bytes
                            +   (ptHeader->Trainer ? 512 : 0 ); 
    
    if (wCHRSize) {
        ptCartridge->pchCHRMemory = ptCartridge->pchPRGMemory + wPRGSize;
        
    } else {
        ptCartridge->pchCHRMemory = ptCartridge->chCHRData;
        wCHRSize = 0x2000;

        memset(ptCartridge->pchCHRMemory, 0, 0x2000);
    }

    //! generate mask
    ptCartridge->wCHRAddressMask = wCHRSize - 1;
    ptCartridge->wPRGAddressMask = wPRGSize - 1;
    
    return nes_ok;
}

#if JEG_USE_DMA_MEMORY_COPY_ACCELERATION == ENABLED
uint8_t *cartridge_get_prg_src_address(cartridge_t *cartridge, uint_fast16_t hwAddress)
{
    if (hwAddress>=0x8000) {
        return &(cartridge->pchPRGMemory[ hwAddress & cartridge->wPRGAddressMask]);
    } 
    return  &(cartridge->chIOData[hwAddress & 0x1fff]);
}
#endif

//! \brief access cpu memory bus
uint_fast8_t cartridge_read_prg(cartridge_t *cartridge, uint_fast16_t hwAddress) {

    if (hwAddress >= 0x8000) {
        return cartridge->pchPRGMemory[hwAddress & cartridge->wPRGAddressMask];
    }    
    
    return cartridge->chIOData[hwAddress & 0x1FFF];
}

#if JEG_USE_EXTRA_16BIT_BUS_ACCESS       == ENABLED
uint_fast16_t cartridge_readw_prg(cartridge_t *cartridge, uint_fast16_t hwAddress)
{
    if (hwAddress >= 0x8000) {
        return *(uint16_t *)&(cartridge->pchPRGMemory[hwAddress & cartridge->wPRGAddressMask]);
    }    
    
    return cartridge->chIOData[hwAddress & 0x1FFF];
}
#endif

void cartridge_write_prg(cartridge_t *cartridge, uint_fast16_t hwAddress, uint_fast8_t value) 
{

    if (hwAddress >= 0x8000) {
        cartridge->pchPRGMemory[ hwAddress & cartridge->wPRGAddressMask] = value;
    } else {
        cartridge->chIOData[hwAddress & 0x1fff] = value;
    }
  
}

//! \brief access ppu memory bus
uint_fast8_t cartridge_read_chr(cartridge_t *cartridge, uint_fast16_t hwAddress) 
{
    return cartridge->pchCHRMemory[hwAddress & cartridge->wCHRAddressMask];
}

void cartridge_write_chr(cartridge_t *cartridge, uint_fast16_t hwAddress, uint_fast8_t value) 
{
    cartridge->pchCHRMemory[hwAddress & cartridge->wCHRAddressMask ] = value;
}
