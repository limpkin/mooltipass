/* CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at src/license_cddl-1.0.txt
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at src/license_cddl-1.0.txt
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*!  \file     userhandling.c
*    \brief    Logic for user handling
*    Created:  09/6/2014
*    Author:   Mathieu Stephan
*/
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/io.h>
#include <string.h>
#include "eeprom_addresses.h"
#include "userhandling.h"
#include "smartcard.h"
#include "defines.h"
#include "oledmp.h"
#include "usb.h"
#include "aes.h"

// Credential timer valid flag
volatile uint8_t credential_timer_valid = FALSE;
// Credential timer value
volatile uint16_t credential_timer = 0;
// Current nonce
uint8_t current_nonce[AES256_CTR_LENGTH];
// Selected login flag (the plugin selected a login)
uint8_t selected_login_flag = FALSE;
// Context valid flag (eg we know the current service / website)
uint8_t context_valid_flag = FALSE;
// AES256 context variable
aes256CtrCtx_t aesctx;
// TO REMOVE AFTER TESTS
//////////////////////////////////////////////////////////////////////////
char temp_login[64] = {0,};
char temp_pass[64] = {0,};
//////////////////////////////////////////////////////////////////////////


/*! \fn     setCurrentContext(uint8_t* name, uint8_t length)
*   \brief  Set our current context
*   \param  name    Name of the desired service / website
*   \param  length  Length of the string
*   \return If we found the context
*/
RET_TYPE setCurrentContext(uint8_t* name, uint8_t length)
{
    uint8_t reg = SREG;
    
    // Look for name inside our flash
    if (strcmp((char*)name, "accounts.google.com") == 0)    // should limit to the len of name?
    {
        USBOLEDDPRINTF_P(PSTR("Active: %s\n"), name);
        // TO ABSOLUTELY REMOVE!!!!
        //////////////////////////////////////////////////////////////////////////
        credential_timer_valid = TRUE;
        //////////////////////////////////////////////////////////////////////////
        context_valid_flag = TRUE;
        temp_login[0] = 0;
        temp_pass[0] = 0;
        return RETURN_OK;
    } 
    else
    {
        USBOLEDDPRINTF_P(PSTR("Fail: %s\n"), name);
        cli();
        credential_timer = 0;
        context_valid_flag = FALSE;
        selected_login_flag = FALSE;
        credential_timer_valid = FALSE;
        SREG = reg;                     // restore original interrupt state (may already be disabled)
        return RETURN_NOK;
    }
}

/*! \fn     getLoginForContext(uint8_t* buffer)
*   \brief  Get login for current context
*   \param  buffer  Buffer to store the login
*   \return If login was entered
*/
RET_TYPE getLoginForContext(char* buffer)
{
    if (context_valid_flag == FALSE)
    {
        // Context invalid
        return RETURN_NOK;
    } 
    else
    {
        if (credential_timer_valid == FALSE)
        {
            // Ask the user for approval
            return RETURN_NOK;
        } 
        else
        {
            // Fetch the login and send it
            // bla bla bla
            // Send it to the computer via HID
            printf_P(PSTR("getLogin\n"));
            if (temp_login[0] != 0) {
                strncpy(buffer, temp_login, 64);
                buffer[63] = 0;
            } else {
                strncpy_P(buffer, PSTR("test@gmail.com"), 32);
                buffer[31] = 0;
            }
            //usbKeybPutStr((char*)buffer); 
            return RETURN_OK;
        }
    }
}

/*! \fn     getPasswordForContext(void)
*   \brief  Get password for current context
*   \return If password was entered
*/
RET_TYPE getPasswordForContext(char* buffer)
{
    uint8_t reg = SREG;

    if ((context_valid_flag == TRUE) && (credential_timer_valid == TRUE))
    {
        // Fetch password and send it over USB
        if (temp_pass[0] != 0) {
            printf_P(PSTR("getPassword temp\n"));
            strncpy(buffer, temp_pass, 64);
            buffer[63] = 0;
        } else {
            printf_P(PSTR("getPassword password123\n"));
            strncpy_P(buffer, PSTR("password123"), 32);
            buffer[31] = 0;
        }
        //usbKeybPutStr((char*)buffer);     // XXX
        // Clear credential timer
        cli();
        credential_timer = 0;
        // TO UNCOMMENT
        //////////////////////////////////////////////////////////////////////////
        //credential_timer_valid = FALSE;
        //////////////////////////////////////////////////////////////////////////
        SREG = reg;                     // restore original interrupt state (may already be disabled)
        return RETURN_OK; 
    } 
    else
    {
        return RETURN_NOK;
    }
}

/*! \fn     setLoginForContext(uint8_t* name, uint8_t length)
*   \brief  Set login for current context
*   \param  name    String containing the login
*   \param  length  String length
*   \return Operation success or not
*/
RET_TYPE setLoginForContext(uint8_t* name, uint8_t length)
{
    if (context_valid_flag == FALSE)
    {
        return RETURN_NOK;
    } 
    else
    {
        // Look for given login in the flash
        if (TRUE)
        {
            // Select it
            // TO REMOVE!!!!
            //////////////////////////////////////////////////////////////////////////
            memcpy(temp_login, name, length+1);
            //////////////////////////////////////////////////////////////////////////
            selected_login_flag = TRUE;
            return RETURN_OK;
        } 
        else
        {
            // If doesn't exist, ask user for confirmation to add to flash
            if (TRUE)
            {
                selected_login_flag = TRUE;
                return RETURN_OK;
            } 
            else
            {
                selected_login_flag = FALSE;
                return RETURN_NOK;
            }
        }
    }
}

/*! \fn     setPasswordForContext(uint8_t* password, uint8_t length)
*   \brief  Set password for current context
*   \param  password    String containing the password
*   \param  length      String length
*   \return Operation success or not
*/
RET_TYPE setPasswordForContext(uint8_t* password, uint8_t length)
{
    if ((selected_login_flag == FALSE) || (context_valid_flag == FALSE))
    {
        // Login not set
        return RETURN_NOK;
    } 
    else
    {
        // Ask for password changing approval
        if (TRUE)
        {
            // Store password
            // TO REMOVE !!!
            //////////////////////////////////////////////////////////////////////////
            memcpy(temp_pass, password, length+1);
            //////////////////////////////////////////////////////////////////////////
            selected_login_flag = FALSE;
            return RETURN_OK;
        } 
        else
        {
            return RETURN_NOK;
        }
    }
}

/*! \fn     firstTimeUserHandlingInit(void)
*   \brief  First time required intialization
*/
void firstTimeUserHandlingInit(void)
{
    eeprom_write_byte((uint8_t*)EEP_NB_KNOWN_CARDS_ADDR, 0);
    eeprom_write_byte((uint8_t*)EEP_NB_KNOWN_USERS_ADDR, 0);
}

/*! \fn     getNumberOfKnownUsers(void)
*   \brief  Get the number of know users
*   \return The number of users
*/
uint8_t getNumberOfKnownUsers(void)
{
    return eeprom_read_byte((uint8_t*)EEP_NB_KNOWN_USERS_ADDR);
}

/*! \fn     getNumberOfKnownCards(void)
*   \brief  Get the number of know cards
*   \return The number of cards
*/
uint8_t getNumberOfKnownCards(void)
{
    return eeprom_read_byte((uint8_t*)EEP_NB_KNOWN_CARDS_ADDR);
}

/*! \fn     findUserId(uint8_t userid)
*   \brief  Find a given user ID
*   \param  userid  The user ID
*   \return Yes or No...
*/
RET_TYPE findUserId(uint8_t userid)
{
    uint8_t i;
    
    for (i = 0; i < getNumberOfKnownCards(); i++)
    {
        if (eeprom_read_byte((uint8_t*)EEP_SMC_IC_USER_MATCH_START_ADDR+i*SMCID_UID_MATCH_ENTRY_LENGTH+SMARTCARD_CPZ_LENGTH+AES256_CTR_LENGTH) == userid)
        {
            return RETURN_OK;
        }
    }
    
    return RETURN_NOK;    
}

/*! \fn     getUserIdFromSmartCardCPZ(uint8_t* buffer, uint8_t* userid)
*   \brief  Get a user ID from card CPZ
*   \param  buffer      Buffer containing the CPZ
*   \param  nonce       pointer to where to store the ctr nonce
*   \param  userid      pointer to where to store the user id
*   \return If we found the CPZ
*/
RET_TYPE getUserIdFromSmartCardCPZ(uint8_t* buffer, uint8_t* nonce, uint8_t* userid)
{
    uint8_t temp_bool;
    uint8_t i,j;
    
    for (i = 0; i < getNumberOfKnownCards(); i++)
    {
        temp_bool = TRUE;
        for (j = 0; j < SMARTCARD_CPZ_LENGTH; j++)
        {
            if (buffer[j] != eeprom_read_byte((uint8_t*)EEP_SMC_IC_USER_MATCH_START_ADDR+i*SMCID_UID_MATCH_ENTRY_LENGTH+j))
            {
                temp_bool = FALSE;
            }
        }
        if (temp_bool == TRUE)
        {
            // We found the CPZ, store the aes ctr value & the user id
            for (j = 0; j < AES256_CTR_LENGTH; j++)
            {
                nonce[j] = eeprom_read_byte((uint8_t*)EEP_SMC_IC_USER_MATCH_START_ADDR+i*SMCID_UID_MATCH_ENTRY_LENGTH+SMARTCARD_CPZ_LENGTH+j);
            }
            *userid = eeprom_read_byte((uint8_t*)EEP_SMC_IC_USER_MATCH_START_ADDR+i*SMCID_UID_MATCH_ENTRY_LENGTH+SMARTCARD_CPZ_LENGTH+AES256_CTR_LENGTH);
            return RETURN_OK;
        }
    }
    
    return RETURN_NOK;
}

/*! \fn     writeSmartCardCPZForUserId(uint8_t* buffer, uint8_t userid)
*   \brief  Add a CPZ<>User id entry
*   \param  buffer      Buffer containing the CPZ
*   \param  nonce       Buffer containing the AES CTR nonce
*   \param  userid      user id
*   \return If we could add the entry
*/
RET_TYPE writeSmartCardCPZForUserId(uint8_t* buffer, uint8_t* nonce, uint8_t userid)
{
    uint8_t temp_buffer[AES256_CTR_LENGTH];
    uint8_t i;
        
    if (((getNumberOfKnownCards()+1)*SMCID_UID_MATCH_ENTRY_LENGTH) + EEP_SMC_IC_USER_MATCH_START_ADDR >= EEPROM_SIZE)
    {
        // Check that we still have space to store
        return RETURN_NOK;
    }
    else if (getUserIdFromSmartCardCPZ(buffer, temp_buffer, &i) == RETURN_OK)
    {
        // Check if we don't already know the smart card
        return RETURN_NOK;
    }
    else
    {
        if (findUserId(userid) == RETURN_NOK)
        {
            // Increment the number of users
            eeprom_write_byte((uint8_t*)EEP_NB_KNOWN_USERS_ADDR, getNumberOfKnownUsers()+1);
        }
        for (i = 0; i < SMARTCARD_CPZ_LENGTH; i++)
        {
            // Store the CPZ
            eeprom_write_byte((uint8_t*)EEP_SMC_IC_USER_MATCH_START_ADDR + getNumberOfKnownCards()*SMCID_UID_MATCH_ENTRY_LENGTH + i, buffer[i]);
        }
        for (i = 0; i < AES256_CTR_LENGTH; i++)
        {
            // Store the AES CTR value
            eeprom_write_byte((uint8_t*)EEP_SMC_IC_USER_MATCH_START_ADDR + getNumberOfKnownCards()*SMCID_UID_MATCH_ENTRY_LENGTH + SMARTCARD_CPZ_LENGTH + i, nonce[i]);
        }
        eeprom_write_byte((uint8_t*)EEP_SMC_IC_USER_MATCH_START_ADDR + getNumberOfKnownCards()*SMCID_UID_MATCH_ENTRY_LENGTH + SMARTCARD_CPZ_LENGTH + AES256_CTR_LENGTH, userid);
        eeprom_write_byte((uint8_t*)EEP_NB_KNOWN_CARDS_ADDR, getNumberOfKnownCards()+1);
        return RETURN_OK;
    }
}

/*! \fn     initEncryptionHandling(uint8_t* aes_key, uint8_t* nonce)
*   \brief  Initialize our encryption/decryption part
*   \param  aes_key     Our AES256 key
*   \param  nonce       The nonce
*/
void initEncryptionHandling(uint8_t* aes_key, uint8_t* nonce)
{
    uint8_t i;
    
    for (i = 0; i < AES256_CTR_LENGTH; i++)
    {
        current_nonce[i] = nonce[i];
    }
    
    aes256CtrInit(&aesctx, aes_key, current_nonce, AES256_CTR_LENGTH);
}