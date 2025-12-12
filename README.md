# WinLicense
Code to extract Windows license information including the activation key

This simple project demonstrates how to programmatically check the licensing/activation status of Windows.  It also shows the Windows license key embedded in the device's firmware, as well as the actual key used to activate Windows (in case the activation key differs from the embedded key).

This project demonstrates the use of the Software Licensing API (SLAPI), which Microsoft does not fully document how to correctly call the APIs to extract license key information.

Sample output:

`Installation status: genuine`  
`Firmware embedded key: ABCDE-01234-FGHIJ-56789-KLMNO`  
`Windows activation key: ABCDE-01234-FGHIJ-56789-KLMNO`  
`Status: licensed`  
`Channel: OEM:DM`  
`DigitalPID: 03612-03259-661-699065-02-1033-26200.0000-3372025`  
`DigitalPID2: 00325-96616-99065-AAOEM`  
`Partial key: #####-#####-#####-#####-KLMNO`
