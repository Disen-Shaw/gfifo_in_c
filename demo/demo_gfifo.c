
#include "gfifo.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define FIFO_SIZE (1024)

DECLARE_GFIFO_TYPE(byte, uint8_t);

#define CIRCULAR_LOOP (50000)
#define IO_DATA_BYTE_COUNT (17)

uint8_t fifo_buf[FIFO_SIZE];
uint8_t push_data[IO_DATA_BYTE_COUNT];
uint8_t pop_data[IO_DATA_BYTE_COUNT];

const char *file_name = "demo_gfifo.txt";

gfifo_byte_t fifo;

int main(int argc, char *argv[])
{
    FILE *fd = fopen(file_name, "w");
    gfifo_byte_init(&fifo, fifo_buf, FIFO_SIZE);

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
            if (!gfifo_byte_push_array(&fifo, push_data, IO_DATA_BYTE_COUNT))
            {
                fprintf(fd, "push data failed: %d\n", i);
                fclose(fd);
                return -1;
            }

            // pop data
            if (!gfifo_byte_pop_array(&fifo, pop_data, IO_DATA_BYTE_COUNT))
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

