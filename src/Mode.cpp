#include <Mode.h>
#include <global.h>

uint32_t Mode::toHex(const char *hex)
{
    char hex_substring[7] = "";

    strncpy(hex_substring, &hex[1], sizeof(hex_substring));

    hex_substring[6] = 0;

    long a = strtol(hex_substring, 0, 16);

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
    char msg[32] = "";

    strcat(msg, "r:");
    itoa(color.r, strchr(msg, 0), DEC);
    strcat(msg, "g:");
    itoa(color.g, strchr(msg, 0), DEC);
    strcat(msg, "b:");
    itoa(color.b, strchr(msg, 0), DEC);

    sprintln(msg);
}