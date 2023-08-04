#include <Mode.h>
#include <global.h>

uint32_t Mode::toHex(const char *hex)
{
    char _hex[10] = "";
    strcpy(_hex, hex + 1);
    long a = strtol(hex, 0, 16);
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
    itoa(color.r, strchr(msg, NULL), DEC);
    strcat(msg, "g:");
    itoa(color.g, strchr(msg, NULL), DEC);
    strcat(msg, "b:");
    itoa(color.b, strchr(msg, NULL), DEC);

    sprintln(msg);
}