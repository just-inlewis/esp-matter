#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <unistd.h>

#define INT_GPIO 5
#define SWITCH_ADDR 0x20
#define RELAY_ADDR 0x21

static unsigned int vol;
static unsigned char mute = 0x00;
static int swFd, rlyFd;

void log_message(const char *message, unsigned short data) {
    printf("%s - Bits sent to device: 0x%04x\n", message, data);
}

void ra_write(int fd, unsigned short data) {
    wiringPiI2CWrite(fd, 0x3f)
    usleep(600);
    log_message("Writing to hardware", data);
    if (wiringPiI2CWrite(fd, data) < 0) {
        perror("Error: Writing on I2C");
    }
}

void ra_mute(int fd) {
    mute = (~mute) & 0x1;
    unsigned short data = mute ? ((~vol) & 0xBF) : ((~vol) | 0x40);
    ra_write(fd, data);
    log_message("Mute toggled", data);
}

void ra_set_volume(int fd, unsigned int new_vol) {
    if (new_vol == 0) {
        ra_mute(fd);
    } else if (new_vol <= 0x3F) {
        vol = new_vol;
        unsigned short data = (~vol) | 0x40;
        ra_write(fd, data);
        mute = 0;
        log_message("Volume set", data);
    } else {
        log_message("Invalid volume level", 0);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <volume>\n", argv[0]);
        return EXIT_FAILURE;
    }

    vol = atoi(argv[1]);

    if (wiringPiSetup() < 0) {
        perror("Error: Unable to setup wiringPi");
        return EXIT_FAILURE;
    }

    swFd = wiringPiI2CSetup(SWITCH_ADDR);
    rlyFd = wiringPiI2CSetup(RELAY_ADDR);

    if (swFd < 0 || rlyFd < 0) {
        perror("Error opening I2C channels");
        return EXIT_FAILURE;
    }

    ra_set_volume(rlyFd, vol);

    return 0;
}