#include "stm32f4xx.h"

int main(void)
{
    for (;;) {
        __WFI(); /* nothing runs yet, so sleep */
    }
}
