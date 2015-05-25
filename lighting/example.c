/*********************************************
 * SPI to MAX7221 (using spidev driver)
 * From https://www.raspberrypi.org/forums/viewtopic.php?t=41713
 *********************************************/

#include <math.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

void timestamp(const char *s)
{
   time_t curtime;
   struct tm *loctime;
   char timestr [20];

   curtime = time(NULL);
   loctime = localtime(&curtime);
   strftime(timestr,20,"%H:%M.%S",loctime);
   
    printf("%s - %s",timestr,s);
}


static void pabort(const char *s)
{
   perror(s);
   abort();
}

static const char *device = "/dev/spidev0.0";
static uint8_t mode;
static uint8_t bits = 8;
static uint32_t speed = 200000;
static uint16_t delay;

static void matrixwrite(int fd, unsigned char max_address, unsigned char max_data)
{
   uint8_t tx[] = { max_address, max_data, };
   write(fd, tx, 2);
}

static void initspi(int fd)
{
   int ret = 0;

   /** spi mode **/
   ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
   if (ret == -1)
      pabort("can't set spi mode");

   ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
   if (ret == -1)
      pabort("can't get spi mode");

   /** bits per word **/
   ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
   if (ret == -1)
      pabort("can't set bits per word");

   ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
   if (ret == -1)
      pabort("can't get bits per word");

   /** max speed hz **/
   ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
   if (ret == -1)
      pabort("can't set max speed hz");

   ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
   if (ret == -1)
      pabort("can't get max speed hz");

}


void matrix_single(int fd, int delay, int repeat)
{
   uint8_t col; // Column
   uint8_t row; // Row
   uint8_t z;

   timestamp("2. single leds.\n");
   for (z = 1; z <= repeat; z++) {
      for (col = 1; col <= 8; col++) {
         for (row = 0; row <= 7; row++) {
            z = pow(2,row);
            matrixwrite(fd, col, z);
            usleep(delay);
         }
         matrixwrite(fd, col, 0);
      }
   }
}

void matrix_colwipe(int fd, int delay, int repeat)
{
   uint8_t col; // Column
   uint8_t row; // Row
   uint8_t z;

   timestamp("3. Column wipe.\n");
   for (z = 1; z <= repeat; z++) {
      for (col = 1; col <= 8; col++) {
         matrixwrite(fd, col, 0xFF);
         usleep(delay);
      }
      for (col = 1; col <= 8; col++) {
         matrixwrite(fd, col, 0x00);
         usleep(delay);
      }
   }
   for (z = 1; z <= repeat; z++) {
      for (col = 8; col >= 1; col--) {
         matrixwrite(fd, col, 0xFF);
         usleep(delay);
      }
      for (col = 1; col <= 8; col++) {
         matrixwrite(fd, col, 0x00);
         usleep(delay);
      }
   }
}


void matrix_linewipe(int fd, int delay, int repeat)
{
   uint8_t col; // Column
   uint8_t row; // Row
   uint8_t z;

   timestamp("4. Line wipe.\n");
   for (z = 1; z <= repeat; z++) {
      for (row = 0; row <= 7; row++) {
         z = pow(2,row);
         for (col = 1; col <= 8; col++) {
            matrixwrite(fd, col, z);
            usleep(delay);
         }
         for (col = 8; col >= 1; col--) {
            matrixwrite(fd, col, 0);
            usleep(delay);
         }
      }
   }
}

void matrix_linewipeupdown(int fd, int delay, int repeat)
{
   uint8_t col; // Column
   uint8_t row; // Row
   uint8_t z;

   timestamp("5. Line up/down.\n");
   for (z = 1; z <= repeat; z++) {
      for (col = 1; col <= 8; col++) {
         matrixwrite(fd, col, 0xFF);
         usleep(delay);
         matrixwrite(fd, col, 0);
      }
      for (col = 8; col >= 1; col--) {
         matrixwrite(fd, col, 0xFF);
         usleep(delay);
         matrixwrite(fd, col, 0);
      }
   }
}


void matrix_flash(int fd, int delay1, int delay2, int repeat)
{
   uint8_t col; // Column
   uint8_t row; // Row
   uint8_t z;

   timestamp("6. Flash.\n");
   
   for (z = 1; z <= repeat; z++) {
      for (col = 1; col <= 8; col++) {
         matrixwrite(fd, col, 0xFF);
      }
      usleep(delay1);      
      for (col = 1; col <= 8; col++) {
         matrixwrite(fd, col, 0x00);
      }
      usleep(delay2);      
   }
   
}


int main(int argc, char *argv[])
{
   int fd;

   fd = open(device, O_RDWR);

   if (fd < 0)
      pabort("can't open device");

   initspi(fd);

   printf("---------------------------------------\n");
   printf("spi mode: %d\n", mode);
   printf("bits per word: %d\n", bits);
   printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);
   printf("---------------------------------------\n");
   
   uint8_t col; // Column

   //All leds off.
   for (col = 1; col <= 8; col++) { matrixwrite(fd, col, 0); }
   
   timestamp("1. Self test.\n");
   matrixwrite(fd, 0x0F, 0x01);
   usleep(500000);
   matrixwrite(fd, 0x0F, 0x00);
   usleep(500000);

   // Initialize Matrix
   matrixwrite(fd, 0x0C, 0x01); // Normal operation
   matrixwrite(fd, 0x0B, 0x07); // Scan Limit (all digits)
   matrixwrite(fd, 0x0A, 0x07); // Intensity
   matrixwrite(fd, 0x09, 0x00); // Decode mode (off)

   matrix_single(fd, 10000, 2);
   usleep(1000000);
   matrix_colwipe(fd, 20000, 5);
   usleep(1000000);
   matrix_linewipe(fd, 20000, 5);
   usleep(1000000);
   matrix_linewipeupdown(fd, 40000, 5);
   usleep(1000000);
   matrix_flash(fd, 30000, 70000, 20);

   timestamp("Completed.\n");

   //All leds off.
   for (col = 1; col <= 8; col++) { matrixwrite(fd, col, 0); }
   
   close(fd);

   return;
}

