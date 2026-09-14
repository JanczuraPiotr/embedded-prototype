# embedded-prototype

Repozytorium zawiera projekty i wspólne zależności pobierane jako podmoduły Git:

```text
embedded-prototype/
├── .gitmodules
├── FreeRTOS/
├── Drivers/STM32CubeF7/
└── generator-1/
```

Podmoduły są przypięte do konkretnych commitów w indeksie Git. Projekty korzystają
z bibliotek w katalogu nadrzędnym, bez kopiowania ich do swoich katalogów.

## Pobieranie zależności

Z katalogu głównego repozytorium:

```bash
git submodule update --init --recursive
```

Przy pierwszym klonowaniu:

```bash
git clone --recurse-submodules <URL-repozytorium>
```

Aby pobrać tylko zależności używane obecnie przez `generator-1`:

```bash
git submodule update --init FreeRTOS Drivers/STM32CubeF7
git -C FreeRTOS submodule update --init FreeRTOS/Source
git -C Drivers/STM32CubeF7 submodule update --init Drivers/CMSIS/Device/ST/STM32F7xx
```

## generator-1

Wymagane narzędzia: CMake, Make i toolchain `arm-none-eabi` z biblioteką C/C++.

```bash
cmake -S generator-1 -B generator-1/build
cmake --build generator-1/build
```

W CLion otwórz katalog `generator-1`. Jego CMakeLists.txt odnajduje wspólne
biblioteki względem położenia projektu. `FreeRTOSConfig.h` pozostaje lokalną
konfiguracją tego projektu.
