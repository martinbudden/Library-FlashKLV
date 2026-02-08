#if defined (LIBRARY_FLASH_KLV_USE_EMPTY_MAIN)

#if defined(FRAMEWORK_RPI_PICO)
int main()
{
    return 0;
}

#elif defined(FRAMEWORK_ESPIDF)

extern "C" void app_main()
{
    return 0;
}

#elif defined(FRAMEWORK_STM32_CUBE)

int main()
{
    return 0;
}

#elif defined(FRAMEWORK_TEST)

int main()
{
    return 0;
}

#else

#include <Arduino.h>

void setup()
{
}

void loop()
{
}

#endif // FRAMEWORK

#endif // LIBRARY_FLASH_KLV_USE_EMPTY_MAIN
