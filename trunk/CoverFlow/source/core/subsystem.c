#include <stdio.h>
#include <ogcsys.h>

#include "core/fat.h"
#include "core/wpad.h"


void Subsystem_Init(void)
{
	/* Initialize Wiimote subsystem */
	Wpad_Init();

	/* Mount SDHC */
	Fat_MountSDHC();
}

void Subsystem_Close(void)
{
	/* Disconnect Wiimotes */
	Wpad_Disconnect();

	/* Unmount SDHC */
	Fat_UnmountSDHC();
}
