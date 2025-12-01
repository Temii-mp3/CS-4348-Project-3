#include <stdint.h>
#include <iostream>
#include <iomanip>
int isBigEndian();
uint64_t reverseBytes(uint64_t x);

int main(int argc, char **argv)
{

    uint64_t num = 5;
    std::cout << "Original: 0x" << std::hex << std::setw(16) << std::setfill('0') << num << std::endl;

    if (!isBigEndian())
    {
        num = reverseBytes(num);
        std::cout << "Reversed: 0x" << std::hex << std::setw(16) << std::setfill('0') << num << std::endl;
    }
    else
    {
        std::cout << "System is big-endian; no reversal needed." << std::endl;
    }
    return 0;
}

int isBigEndian()
{
    int num = 1;
    return (*(char *)&num == 0);
}

uint64_t reverseBytes(uint64_t x)
{
    uint8_t dest[sizeof(uint64_t)];
    uint8_t *src = (uint8_t *)&x;
    for (int c = 0; c < sizeof(uint64_t); c++)
    {
        dest[c] = src[sizeof(uint64_t) - c - 1];
    }

    return *(uint64_t *)dest;
}