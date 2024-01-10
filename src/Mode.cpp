#include <Mode.h>
#include <global.h>

uint32_t Mode::toHex(const char *hex)
{
    // char hex_substring[7] = "";

    // strncpy(hex_substring, , sizeof(hex_substring));

    // // hex_substring[6] = 0;

    long a = strtol(&hex[1], 0, 16);

    return (uint32_t)a;
}

float Mode::removeNegatives(float num)
{
    return num <= 0.0F ? 0.0F : num;
}
int Mode::removeNegatives(int num)
{
    return num <= 0 ? 0 : num;
}

void Mode::printCRGB(CRGB color)
{
    char msg[32];

    snprintf(msg, sizeof(msg), "RGB(%i,%i,%i)", color.r, color.g, color.b);

    sprintln(msg);
}