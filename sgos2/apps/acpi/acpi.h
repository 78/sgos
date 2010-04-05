#ifndef _ACPI_H
#define _ACPI_H

#define Acpi_PowerOff	0x80000000

int InitializeAcpi();
void AcpiPowerOff(void);

#endif
