/*
 * define_printouts.h
 *
 * Created: 27/03/2015 15:52:16
 *  Author: limpkin
 */ 


#ifndef DEFINE_PRINTOUTS_H_
#define DEFINE_PRINTOUTS_H_


// Memory printouts
//#define MEMORY_LAYOUT_PRINTOUT
#ifdef MEMORY_LAYOUT_PRINTOUT
    #pragma message "Bootkey address: " XSTR(EEP_BOOTKEY_ADDR)
    #pragma message "Boot pwd set bool address: " XSTR(EEP_BOOT_PWD_SET)
    #pragma message "Boot pwd address: " XSTR(EEP_BOOT_PWD)
    #pragma message "User data start address: " XSTR(EEP_USER_DATA_START_ADDR)
    #pragma message "Mass prod bool address: " XSTR(EEP_MASS_PROD_FBOOT_BOOL_ADDR)
    #pragma message "User_id <> SMC_UID & Nonce start address: " XSTR(EEP_SMC_IC_USER_MATCH_START_ADDR)
    #pragma message "Number of possible LUT entries:" XSTR(NB_MAX_SMCID_UID_MATCH_ENTRIES)
    #pragma message "UID request key address:" XSTR(EEP_UID_REQUEST_KEY_ADDR)
    #pragma message "UID address:" XSTR(EEP_UID_ADDR)
    #ifdef MINI_VERSION
        #pragma message "Last two bytes of aes key2 address:" XSTR(EEP_LAST_AES_KEY2_2BYTES_ADDR)
    #else
        #pragma message "Bootkey copy address:" XSTR(EEP_BACKUP_BOOTKEY_ADDR)
    #endif
    #pragma message "Number of user parameters:" XSTR(USER_RESERVED_SPACE_IN_EEP)
#endif

// Check bytes left in eeprom
#ifdef MINI_VERSION
    #if (EEP_LAST_AES_KEY2_2BYTES_ADDR + BOOTKEY_SIZE) == EEPROM_SIZE
        #ifdef MEMORY_LAYOUT_PRINTOUT
            #pragma message "EEPROM memory full"
        #endif
    #else
        #error "Wrong EEPROM memory layout"
    #endif
#else
    #if (EEP_BACKUP_BOOTKEY_ADDR + BOOTKEY_SIZE) == EEPROM_SIZE
        #ifdef MEMORY_LAYOUT_PRINTOUT
            #pragma message "EEPROM memory full"
        #endif
    #else
        #error "Wrong EEPROM memory layout"
    #endif
#endif

#endif /* DEFINE_PRINTOUTS_H_ */