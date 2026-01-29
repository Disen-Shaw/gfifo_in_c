
#include "sfifo.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

DECLARE_SFIFO_TYPE(byte, uint8_t, 1024);

#define CIRCULAR_LOOP (50000)
#define IO_DATA_BYTE_COUNT (17)

uint8_t push_data[IO_DATA_BYTE_COUNT];
uint8_t pop_data[IO_DATA_BYTE_COUNT];

sfifo_byte_1024_t fifo;

const char *file_name = "demo_sfifo.txt";

int main(int argc, char *argv[])
{
    FILE *fd = fopen(file_name, "w");
    sfifo_byte_1024_init(&fifo);

    if (fd != NULL)
    {
        fprintf(fd, "file open successfully\n");

        for (int i = 0; i < CIRCULAR_LOOP; ++i)
        {
            // generate random data
            int random_data = 10 * rand();
            for (int j = 0; j < IO_DATA_BYTE_COUNT; ++j)
            {
                push_data[j] = random_data * i;
            }

            // push data
            if (!sfifo_byte_1024_push_array(&fifo, push_data, IO_DATA_BYTE_COUNT))
            {
                fprintf(fd, "push data failed: %d\n", i);
                fclose(fd);
                return -1;
            }

            // pop data
            if (!sfifo_byte_1024_pop_array(&fifo, pop_data, IO_DATA_BYTE_COUNT))
            {
                fprintf(fd, "pop data failed: %d\n", i);
                fclose(fd);
                return -2;
            }

            // check data
            if (memcmp(push_data, pop_data, IO_DATA_BYTE_COUNT) != 0)
            {
                fprintf(fd, "memcpm failed\n");
                fclose(fd);
                return -3;
            }
        }

        fprintf(fd, "io data check no error");
        fclose(fd);
        return 0;
    }
    printf("file open failed\n");
    return -1;
}
