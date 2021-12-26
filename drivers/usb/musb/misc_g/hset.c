/******************************************************************
 * Copyright 2006 Mentor Graphics Corporation
 *
 * This file is part of the Inventra Controller Driver for Linux.
 * 
 * The Inventra Controller Driver for Linux is free software; you 
 * can redistribute it and/or modify it under the terms of the GNU 
 * General Public License version 2 as published by the Free Software 
 * Foundation.
 * 
 * The Inventra Controller Driver for Linux is distributed in 
 * the hope that it will be useful, but WITHOUT ANY WARRANTY; 
 * without even the implied warranty of MERCHANTABILITY or 
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public 
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with The Inventra Controller Driver for Linux ; if not, 
 * write to the Free Software Foundation, Inc., 59 Temple Place, 
 * Suite 330, Boston, MA  02111-1307  USA
 * 
 * ANY DOWNLOAD, USE, REPRODUCTION, MODIFICATION OR DISTRIBUTION 
 * OF THIS DRIVER INDICATES YOUR COMPLETE AND UNCONDITIONAL ACCEPTANCE 
 * OF THOSE TERMS.THIS DRIVER IS PROVIDED "AS IS" AND MENTOR GRAPHICS 
 * MAKES NO WARRANTIES, EXPRESS OR IMPLIED, RELATED TO THIS DRIVER.  
 * MENTOR GRAPHICS SPECIFICALLY DISCLAIMS ALL IMPLIED WARRANTIES 
 * OF MERCHANTABILITY; FITNESS FOR A PARTICULAR PURPOSE AND 
 * NON-INFRINGEMENT.  MENTOR GRAPHICS DOES NOT PROVIDE SUPPORT 
 * SERVICES OR UPDATES FOR THIS DRIVER, EVEN IF YOU ARE A MENTOR 
 * GRAPHICS SUPPORT CUSTOMER. 
 ******************************************************************/

/*
 * High-Speed Electrical Test (HSET) tool for the 
 * Inventra Controller Driver (ICD) for Linux.
 * $Revision: 1.1.1.1 $
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PROCFILENAME "/proc/testmusbhdrc0"
#define DEVICEFILENAME "/proc/bus/usb/devices"

static unsigned long BusNumber = 0;
static char Speeds[128];
static unsigned long Parents[128];
static unsigned long Ports[128];

static void StartSession(unsigned char bHostRequest)
{
    FILE* pFile = fopen("/proc/musbhdrc0", "a");
    if(pFile)
    {
	if(bHostRequest)
	{
	    fprintf(pFile, "H\n");
	    fclose(pFile);
	    pFile = fopen("/proc/musbhdrc0", "a");
	}
	fprintf(pFile, "S\n");
	fclose(pFile);
    }
    else
    {
	fprintf(stderr, "Error: driver not loaded or PROC_FS not configured\n");
    }
}

static void StopSession()
{
    FILE* pFile = fopen("/proc/musbhdrc0", "a");
    if(pFile)
    {
	fprintf(pFile, "s\n");
	fclose(pFile);
    }
    else
    {
	fprintf(stderr, "Error: driver not loaded or PROC_FS not configured\n");
    }
}

static void DropBus()
{
    FILE* pFile = fopen("/proc/musbhdrc0", "a");
    if(pFile)
    {
	fprintf(pFile, "E\n");
	fclose(pFile);
    }
    else
    {
	fprintf(stderr, "Error: driver not loaded or PROC_FS not configured\n");
    }
}

static void ZapDriver()
{
    FILE* pFile = fopen("/proc/musbhdrc0", "a");
    if(pFile)
    {
	fprintf(pFile, "Z\n");
	fclose(pFile);
    }
    else
    {
	fprintf(stderr, "Error: driver not loaded or PROC_FS not configured\n");
    }
}

static void SendCommand(const char* pCommand)
{
    FILE* pFile = fopen(PROCFILENAME, "a");
    if(pFile)
    {
	fprintf(pFile, "%s\n", pCommand);
	fclose(pFile);
    }
    else
    {
	fprintf(stderr, "Error: driver not loaded or PROC_FS not configured\n");
    }
}

static unsigned long GetBusNumber()
{
    char* pEnd;
    char* work;
    char line[256];
    unsigned long v = 0;
    FILE* pFile = fopen(PROCFILENAME, "r");

    if(pFile)
    {
	if(fgets(line, 256, pFile))
	{
	    work = strstr(line, "Bus=");
	    if(work == line)
	    {
		v = strtoul(&(line[4]), &pEnd, 16);
	    }
	}
	fclose(pFile);
    }
    else
    {
	fprintf(stderr, "Error: driver not loaded or PROC_FS not configured\n");
    }
    return v;
}

static void ShowStatus()
{
    char line[256];
    FILE* pFile = fopen(PROCFILENAME, "r");

    if(pFile)
    {
	printf("****************** STATUS *********************\n");
	while(!feof(pFile))
	{
	    if(fgets(line, 256, pFile))
	    {
		printf("%s", line);
		continue;
	    }
	    break;
	}
	fclose(pFile);
	printf("**************** END STATUS *******************\n");
    }
    else
    {
	fprintf(stderr, "Error: driver not loaded or PROC_FS not configured\n");
    }
}

static void ShowDevices()
{
    char* work;
    char* pEnd;
    unsigned long bus, lev, addr, parent, port;
    char line[256];
    unsigned char find=1;
    FILE* pFile = fopen(DEVICEFILENAME, "r");

    if(pFile)
    {
	while(!feof(pFile))
	{
	    if(fgets(line, 256, pFile))
	    {
		if(find && ('T' == line[0]))
		{
		    /* new device */
		    work = strstr(line, "Bus=");
		    if(work)
		    {
			/* get bus */
			work += strlen("Bus=");
			bus = strtoul(work, &pEnd, 16);
			work = strstr(pEnd, "Lev=");
			if(work)
			{
			    /* get level */
			    work += strlen("Lev=");
			    lev = strtoul(work, &pEnd, 16);
			    if((bus == BusNumber) && lev)
			    {
				/* get parent address */
				work = strstr(pEnd, "Prnt=");
				if(work)
				{
				    work += strlen("Prnt=");
				    parent = strtoul(work, &pEnd, 16);

				    work = strstr(pEnd, "Port=");
				    if(work)
				    {
					/* get port */
					work += strlen("Port=");
					port = strtoul(work, &pEnd, 16);

					work = strstr(pEnd, "Dev#=");
					if(work)
					{
					    /* get address */
					    work += strlen("Dev#=");
					    while(isspace(*work))
					    {
						work++;
					    }
					    addr = strtoul(work, &pEnd, 0);
					    Parents[addr] = parent;
					    Ports[addr] = port;

					    /* get speed */
					    work = strstr(pEnd, "Spd=");
					    if(work)
					    {
						work += strlen("Spd=");
						if('4' == *work)
						{
						    Speeds[addr] = 'H';
						}
						else if(('1' == *work) && ('2' == work[1]))
						{
						    Speeds[addr] = 'U';
						}
						else
						{
						    Speeds[addr] = 'L';
						}

						/* denote ready for other part */
						find = 0;
					    }
					}
				    }
				}
			    }
			}
		    }
		}
		else if(!find && 'P' == line[0])
		{
		    work = strstr(line, "Vendor");
		    printf("%03d: %s", addr, work);
		    find = 1;
		}
		continue;
	    }
	    break;
	}
	fclose(pFile);
    }
    else
    {
	fprintf(stderr, "Error: driver not loaded or PROC_FS not configured\n");
    }
}

static unsigned long GetAddress(unsigned int iSetAddr)
{
    char* pEnd;
    char num[16];
    unsigned long v = 0xff;

    printf("\nPlease enter device address (C syntax, 0 for none): ");
    do
    {
	if(fgets(num, 16, stdin))
	{
	    v = strtoul(num, &pEnd, 0);
	    if(isalnum(*pEnd) || (v > 127))
	    {
		v = 0xff;
		printf("That won't do.  Please try again: ");
	    }
	    else if(v && iSetAddr)
	    {
		sprintf(num, "TS%c", Speeds[v]);
		SendCommand(num);
		sprintf(num, "TSB%02x", Parents[v]);
		SendCommand(num);
		sprintf(num, "TSP%02x", Ports[v]);
		SendCommand(num);
	    }
	}
    } while(v > 127);

    return v;
}

static void PrintPortMenu()
{
    printf("Port Control Menu:\n");
    printf("\tQ - Quit Menu/Test\n");
    printf("\tS - Suspend bus\n");
    printf("\tR - Resume bus\n");
    printf("\tE - Reset bus\n");
    printf("\tJ - TEST_J mode\n");
    printf("\tK - TEST_K mode\n");
    printf("\t0 - TEST_SE0_NAK mode\n");
    printf("\tP - TEST_PACKET mode\n");
    printf("\tF - TEST_FORCE_ENABLE\n");
}

static void PortMode()
{
    char input[4];
    char cmd[64];
    char c = 'x';

    while('Q' != toupper(c))
    {
	ShowStatus();
	printf("\n");
	PrintPortMenu();
	printf("Enter the letter corresponding to your choice: ");
	fgets(input, 4, stdin);
	c = toupper(input[0]);
	switch(c)
	{
	case 'S':
	case 'R':
	case 'E':
	    sprintf(cmd, "TN%c", c);
	    SendCommand(cmd);
	    sleep(1);
	    break;
	case 'J':
	case 'K':
	case 'P':
	case 'F':
	    StartSession(0);
	    sprintf(cmd, "TNT%c", c);
	    SendCommand(cmd);
	    break;
	case '0':
	    SendCommand("TNTE");
	    break;
	}
    }
    SendCommand("TN");
}

static void PrintDeviceMenu()
{
    printf("Device Operations Menu:\n");
    printf("\tQ - Quit Menu/Test\n");
    printf("\tJ - TEST_J mode         K - TEST_K mode\n");
    printf("\t0 - TEST_SE0_NAK mode   P - TEST_PACKET mode\n");
    printf("\tH - TEST_FORCE_ENABLE (force a hub into HS mode)\n");
    printf("\tA - Set address for SET_ADDRESS test\n");
    printf("\tG - GET_DESCRIPTOR      S - SET_ADDRESS\n");
    printf("\tL - Loop GET_DESCRIPTOR test\n");
    printf("\tE - Enable remote wake  D - Disable remote wake\n");
    printf("\tF - Step SET_FEATURE    T - Step GET_DESCRIPTOR\n");
    printf("\tN - Next step\n");
}

static void DeviceMode()
{
    char input[4];
    char cmd[64];
    char c = 'x';

    while('Q' != toupper(c))
    {
	/* show current status */
	ShowStatus();
	/* turn off loop mode */
	SendCommand("l");
	/* show menu */
	printf("\n");
	PrintDeviceMenu();
	/* prompt */
	printf("Enter the letter corresponding to your choice: ");
	fgets(input, 4, stdin);
	c = toupper(input[0]);
	switch(c)
	{
	case 'A':
	    sprintf(cmd, "TA%02X", GetAddress(0));
	    SendCommand(cmd);
	    break;
	case 'J':
	    SendCommand("TDJ");
	    sleep(1);
	    break;
	case 'K':
	    SendCommand("TDK");
	    sleep(1);
	    break;
	case '0':
	    SendCommand("TDE");
	    sleep(1);
	    break;
	case 'P':
	    SendCommand("TDP");
	    sleep(1);
	    break;
	case 'H':
	    SendCommand("TDF");
	    sleep(1);
	    break;
	case 'G':
	    SendCommand("TGD");
	    sleep(1);
	    break;
	case 'L':
	    SendCommand("LTGD");
	    sleep(1);
	    break;
	case 'S':
	    SendCommand("TSA");
	    sleep(1);
	    break;
	case 'E':
	    SendCommand("TWE");
	    sleep(1);
	    break;
	case 'D':
	    SendCommand("TWD");
	    sleep(1);
	    break;
	case 'F':
	    SendCommand("TSF");
	    sleep(1);
	    break;
	case 'T':
	    SendCommand("TSG");
	    sleep(1);
	    break;
	case 'N':
	    SendCommand("N");
	    sleep(1);
	    break;
	}
    }
    SendCommand("TN");
}

static void PrintMainMenu()
{
    printf("Main Menu:\n");
    printf("\tQ - Quit\n");
    printf("\tS - Start Session\n");
    printf("\tT - sTop Session\n");
    printf("\tR - Relinquish Bus\n");
    printf("\tZ - Zap driver\n");
    printf("\tP - Port Control\n");
    /*printf("\tB - Enumerate Bus\n");*/
    printf("\tA - Select Device (by address)\n");
    printf("\tD - Device Control\n");
    printf("\tK - Kill Test Mode\n");
}

static void MainMode()
{
    char input[4];
    char cmd[64];
    char c = 'x';

    BusNumber = GetBusNumber();
    if(!BusNumber)
    {
	fprintf(stderr, "Couldn't get bus number\n");
	exit(-1);
    }

    while('Q' != toupper(c))
    {
	ShowStatus();
	printf("\n");
	PrintMainMenu();
	printf("Enter the letter corresponding to your choice: ");
	fgets(input, 4, stdin);
	c = toupper(input[0]);
	switch(c)
	{
	case 'S':
	    StartSession(0);
	    break;
	case 'T':
	    StopSession(1);
	    break;
	case 'R':
	    DropBus(1);
	    break;
	case 'Z':
	    ZapDriver();
	    break;
	case 'P':
	    PortMode();
	    break;
	    /*
	case 'B':
	    SendCommand("B");
	    break;
	    */
	case 'A':
	    printf("\n");
	    ShowDevices();
	    sprintf(cmd, "A%02X", GetAddress(1));
	    SendCommand(cmd);
	    break;
	case 'D':
	    DeviceMode();
	    break;
	case 'K':
	    SendCommand("TN");
	    ZapDriver();
	    break;
	}
    }
}

static void PrintUsage()
{
    printf("Usage: hset [-h] [-s]\n");
    printf("\t-h: display this help message\n");
}

int main(int argc, char* argv[])
{
    if(argc > 1)
    {
	if(0 == strcmp(argv[1], "-h"))
	{
	    PrintUsage();
	    exit(-1);
	}
    }
    MainMode();
}

