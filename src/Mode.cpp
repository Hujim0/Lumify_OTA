#include <Mode.h>

uint32_t Mode::toHex(String hex)
{
    long a = strtol(hex.substring(1).c_str(), 0, 16);
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
    Serial.print("r: ");
    Serial.println(color.r);
    Serial.print("g: ");
    Serial.println(color.g);
    Serial.print("b: ");
    Serial.println(color.b);
}