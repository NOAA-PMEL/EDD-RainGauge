#include "../inc/includes.h"

int __low_level_init(void)
{
	 WDTCTL = WDTPW | WDTHOLD;	
	// Configure MPU
//	MPUCTL0 = MPUPW;
//	MPUSEGB1 = 0x05A6;
//	MPUSEGB2 = 0x05A4;
//
//	MPUSAM = (MPUSEG1WE | MPUSEG1RE | MPUSEG2RE | MPUSEG3RE | MPUSEG3XE);
//	MPUCTL0 = MPUPW | MPUENA | MPUSEGIE;
    
    return 1;
}