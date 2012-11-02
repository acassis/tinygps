#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/signal.h>
#include <sys/types.h>

extern char gps_encode(char c);
extern void gps_f_get_position(float *latitude, float *longitude, unsigned long *fix_age);
extern unsigned short satellites();
extern unsigned long gps_hdop();

#define BAUDRATE B4800
#define FALSE 0
#define TRUE 1
//
volatile int STOP=FALSE;
void signal_handler_IO (int status);
int wait_flag=TRUE;
char devicename[80] = "/dev/ttyUSB0", ch;
int status;

int newdata = 0;

//
int main(int argc, char *argv[])
{
int fd, res, i;
struct termios newtio;
struct sigaction saio;
char buf[255];
//
//open the device in non-blocking way (read will return immediately)
fd = open(devicename, O_RDWR | O_NOCTTY | O_NONBLOCK);
if (fd < 0)
{
perror(devicename);
exit(1);
}
//
//install the serial handler before making the device asynchronous
saio.sa_handler = signal_handler_IO;
sigemptyset(&saio.sa_mask);   //saio.sa_mask = 0;
saio.sa_flags = 0;
saio.sa_restorer = NULL;
sigaction(SIGIO,&saio,NULL);
//
// allow the process to receive SIGIO
fcntl(fd, F_SETOWN, getpid());
//
// make the file descriptor asynchronous
fcntl(fd, F_SETFL, FASYNC);
//
// set new port settings for canonical input processing
newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;
newtio.c_iflag = IGNPAR;
newtio.c_oflag = 0;
newtio.c_lflag = 0;
newtio.c_cc[VMIN]=1;
newtio.c_cc[VTIME]=0;
tcflush(fd, TCIFLUSH);
tcsetattr(fd,TCSANOW,&newtio);
//
// loop while waiting for input. normally we would do something useful here
while (STOP == FALSE)
{
//
// after receiving SIGIO, wait_flag = FALSE, input is available and can be read */
if (wait_flag == FALSE)  //if input is available
{
res = read(fd,buf,255);
if (res > 0)
{
	for (i=0; i < res; i++)  //for all chars in string
	{
		//printf("%c", buf[i]);
		if(gps_encode(buf[i]))
			newdata = 1;
	}

  if (newdata)
  {
    float flat, flon;
    unsigned long age;
    gps_f_get_position(&flat, &flon, &age);
    printf("LAT= %f LON= %f SAT=%d PREC=%d \n", flat, flon, gps_satellites(), gps_hdop());
    newdata = 0;
  }


}
wait_flag = TRUE;      /* wait for new input */
}
}
close(fd);
}
//
/***************************************************************************
* signal handler. sets wait_flag to FALSE, to indicate above loop that     *
* characters have been received.                                           *
***************************************************************************/
//
void signal_handler_IO (int status)
{
//printf("received SIGIO signal.\n");
wait_flag = FALSE;
}


