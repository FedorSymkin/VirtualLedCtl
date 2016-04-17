#ifndef COMMON_H_
#define COMMON_H_

#include <pthread.h>
#include <time.h>
#include <vector>
#include <string>
		

/*
 *I use my own classes for Mutex/Lock/Thread for independence from any libs and C++11
 *In real project I'd use C++11 or boost
 */

class MyMutex
{
	pthread_mutex_t m;

public:
	MyMutex() 		{ pthread_mutex_init(&m, NULL); }
	~MyMutex() 		{ pthread_mutex_destroy(&m); } 
	void lock() 	{ pthread_mutex_lock(&m); } 
	void unlock() 	{ pthread_mutex_unlock(&m); }
	
	friend class MyCond;
};

class MyLock
{
	MyMutex *m;
	
public:
	MyLock(MyMutex &mutex) { m = &mutex; m->lock(); }
	~MyLock() { m->unlock(); }
};

class MyCond
{
	pthread_cond_t c;

public:
	MyCond() 				{ pthread_cond_init(&c, NULL); }
	~MyCond() 				{ pthread_cond_destroy(&c); }
	void signal()			{ pthread_cond_signal(&c); }
	void wait(MyMutex &mtx)	{ pthread_cond_wait(&c, &mtx.m);  }
	
    void timedwait(MyMutex &mtx, timespec until)
	{ 
        pthread_cond_timedwait(&c, &mtx.m, &until);
	}
};

enum LedColor
{
    LED_RED = 0,
    LED_GREEN,
    LED_BLUE,
    LED_UNKNOWN
};

std::string ColorToStr(LedColor color);
LedColor StrToColor(std::string str);
bool IsCorrectRate(int rate);
void split(const std::string &str, std::vector<std::string> &tokens);
long elapsed_ms(timespec &ts);

#endif

