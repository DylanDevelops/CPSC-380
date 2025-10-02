//===================================================== file = checksum.c =====
//=  Program to compute 16-bit Internet checksum                              =
//=============================================================================
//=  Notes: 1) Based on the C-code given in RFC 1071 (Computing the Internet  =
//=            Checksum by R. Braden, D. Borman, and C. Partridge, 1988).     =
//=         2) Streamlined for 32-bit machines.  All overflows are added-in   =
//=            after the summing loop and not within the summing loop on a    =
//=            an overflow-by-overflow basis.                                 =
//=---------------------------------------------------------------------------=
//=  Build:  bcc32 checksum.c, gcc checksum.c                                 =
//=---------------------------------------------------------------------------=
//=  History:  KJC (8/25/00) - Genesis                                        =
//=============================================================================
//----- Include files ---------------------------------------------------------
#include <stdio.h>                  // Needed for printf()
#include <stdint.h>                 // Needed for int sizes
#include <string.h>                 // Needed for strlen()

//=============================================================================
//=  Compute Internet Checksum for count bytes beginning at location addr     =
//=============================================================================
uint16_t checksum(char *addr, uint32_t count)
{
    register uint32_t sum = 0;

    uint16_t *buf = (uint16_t *) addr;

    // Main summing loop
    while(count > 1)
    {
        sum = sum + *(buf)++;
        count = count - 2;
    }

    // Add left-over byte, if any
    if (count > 0)
        sum = sum + *addr;

    // Fold 32-bit sum to 16 bits
    while (sum>>16)
        sum = (sum & 0xFFFF) + (sum >> 16);

    return(~sum);

}

