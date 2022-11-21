/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>

extern const uint8_t hello_world_start[] asm("_binary_hello_world_start");
extern const uint8_t hello_world_end[] asm("_binary_hello_world_end");

void app_main()
{
    printf("\r\n");
    for (int i = 0; i < hello_world_end - hello_world_start; i++)
    {
        printf("%c", *(hello_world_start + i));
    }
}
