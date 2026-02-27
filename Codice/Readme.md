## Requisiti per la compilazione

### Windows

- Windows 10 o 11 a 64 bit;
- Visual Studio 2022 con tool-set v143;
- CMake 4.x;
- Conan 2.x;
- Vulkan SDK 1.4.309.0:
    - nota: il programma utilizza le Vulkan 1.1 quindi probabilmente anche una versione del SDK più vecchia va bene;
- Scheda video e driver compatibili con Vulkan 1.1.

### Linux

- Debian 13 a 64 bit o distribuzione equivalente;
- CMake 4.x;
- Conan 2.x;
- GCC 12.x, 13.x o 14.x;
- Ninja build system;
- Vulkan SDK 1.4.309.0 (libvulkan-dev e vulkan-utility-libraries-dev):
    - nota: il programma utilizza le Vulkan 1.1 quindi probabilmente anche una versione del SDK più vecchia va bene).
- Scheda video e driver compatibili con Vulkan 1.1.

NOTA: Conan potrebbe installare dei pacchetti tramite il gestore dei pacchetti di sistema al fine di compilare alcune
delle dipendenze, in tal caso Conan utilizzerà sudo per eseguire il gestore dei pacchetti e di conseguenza potrebbe
esser necessario digitare la password sul terminale.
