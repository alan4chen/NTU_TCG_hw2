#define __mytime__
#include <ctime>
#include <sys/time.h>
#include <unistd.h>

struct timeval tv;
double getCurrentTimestamp(){
	if(gettimeofday(&tv, NULL) == -1){
    	std::cout << "Error: gettimeofday" << std::endl;
		exit(-1);
   	}
   	double a = tv.tv_usec /10000 /(double)100.0 ;
   	double b = tv.tv_sec%100000;
    return a + b;
}