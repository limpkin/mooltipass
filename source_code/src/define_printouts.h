/*
 * define_printouts.h
 *
 * Created: 27/03/2015 15:52:16
 *  Author: limpkin
 */ 


#ifndef DEFINE_PRINTOUTS_H_
#define DEFINE_PRINTOUTS_H_


// Memory printouts
#define MEMORY_LAYOUT_PRINTOUT
#ifdef MEMORY_LAYOUT_PRINTOUT
    #pragma message "Bootkey address: " XSTR(EEP_BOOTKEY_ADDR)
    #pragma message "Boot pwd set bool address: " XSTR(EEP_BOOT_PWD_SET)
    #pragma message "Boot pwd address: " XSTR(EEP_BOOT_PWD)
    #pragma message "User data start address: " XSTR(EEP_USER_DATA_START_ADDR)
    #pragma message "User_id <> SMC_UID & Nonce start address: " XSTR(EEP_SMC_IC_USER_MATCH_START_ADDR)
    #pragma message "Number of possible LUT entries:" XSTR(NB_MAX_SMCID_UID_MATCH_ENTRIES)
    #pragma message "Reserved space start address:" XSTR(EEP_RESERVED_SPACE_START_ADDR)
#endif

// Check bytes left in eeprom
#if (EEPROM_SIZE - EEP_RESERVED_SPACE_START_ADDR - EEPROM_END_RESERVED) == 0
    #ifdef MEMORY_LAYOUT_PRINTOUT
        #pragma message "EEPROM memory full"
    #endif
#else
    #error "Wrong EEPROM memory layout"
#endif

#endif /* DEFINE_PRINTOUTS_H_ */