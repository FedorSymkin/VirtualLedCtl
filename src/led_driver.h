#ifndef LED_DRIVER_
#define LED_DRIVER_

#include "stdio.h"
#include <pthread.h>
#include <iostream>
#include <time.h>
#include <sys/time.h>
#include "common.h"

class Led
{	
	bool _on;
	LedColor _color;
	
	void show()
	{
		char c = '?';
        if (_on)
		{
			switch (_color)
			{
				case LED_RED: 	c = 'R'; break;
				case LED_GREEN: c = 'G'; break;
				case LED_BLUE: 	c = 'B'; break;
				default: c = '?'; break;
			}
        } else c = ' ';
	
        std::cout << '\r' << "LED: (" << c << ")";
        std::cout.flush();
	}
	
public:
	Led() : 
        _on(false),
		_color(LED_RED) 
	{
		show();
	}
	
	bool getOn() 		{ return _on; }
	LedColor getColor() { return _color; }
	
	void on()						{ _on = true; show(); }
	void off()						{ _on = false; show(); }
    void toggle()					{ _on = !_on; show(); }
	void setColor(LedColor color)	{ _color = color; show(); }
};

class LedDriver
{
	Led				_led;
		
	bool 			_state;
	unsigned int 	_rate;
	bool			_thread_running;
	
	pthread_t 	_thread;
	MyMutex 	_mutex;
	MyCond 		_wake;
	bool		_was_wakeup;
	
	void wakeup_thread()
	{
		_was_wakeup = true;
		_wake.signal();
	}

    static void wakeable_sleep(LedDriver *pthis, int ms)
	{
        timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
		
		while (true)
		{
			if (ms != -1)
            {
                timespec tsUntil;
                tsUntil.tv_sec = ts.tv_sec + ms / 1000;
                tsUntil.tv_nsec = ts.tv_nsec + (ms % 1000) * 1000000;
                pthis->_wake.timedwait(pthis->_mutex, tsUntil);
            }
			else
				pthis->_wake.wait(pthis->_mutex);
			
            if (pthis->_was_wakeup)
			{
                pthis->_was_wakeup = false;
				return;
			}
			
            if ((ms != -1) && (elapsed_ms(ts) >= ms)) return;
		}
	}
	
	
	static void *thread_func(void *arg)
	{
        LedDriver *pthis = (LedDriver*)arg;
		
		MyLock _l(pthis->_mutex);
		while (pthis->_thread_running)
		{
            int sleep_time = -1;

            if (pthis->_state)
            {
                if (pthis->_rate)
                {
                    pthis->_led.toggle();
                    sleep_time = 1000 / pthis->_rate;
                }
                else
                {
                    pthis->_led.on();
                    sleep_time = -1;
                }
            }
            else
            {
                pthis->_led.off();
                sleep_time = -1;
            }

            wakeable_sleep(pthis, sleep_time);
		}
        pthis->_led.off();
		
		return 0;
	}
	

	
public:
    LedDriver():
        _state(true),
        _rate(1),
		_thread_running(false),
		_was_wakeup(false)
	{}
	
    ~LedDriver()
	{
		stop_thread();
	}

    bool get_state() 			{ MyLock _l(_mutex); return _state; }
    LedColor get_color() 		{ MyLock _l(_mutex); return _led.getColor(); }
    unsigned int get_rate() 	{ MyLock _l(_mutex); return _rate; }
    bool is_thread_running() 	{ MyLock _l(_mutex); return _thread_running; }
	
	void set_state(bool st) 
	{
		MyLock _l(_mutex);
	
		bool prev = _state;
		_state =  st;
        if (st != prev) wakeup_thread();
	}
	
	void set_color(LedColor color) 
	{
		MyLock _l(_mutex);
        LedColor prev = _led.getColor();
		_led.setColor(color);
        if (color != prev) wakeup_thread();
	}
	
	void set_rate(unsigned int rate) 
	{
		MyLock _l(_mutex);
        unsigned int prev = _rate;
		_rate = rate;
        if (rate != prev) wakeup_thread();
	}
	
	int start_thread()
	{
		if (!_thread_running)
		{
            MyLock _l(_mutex);
			int ret = pthread_create(&_thread, NULL, thread_func, (void *)this);	  
			if (!ret) _thread_running = true;
			return ret;
		}
		return 0;
	}
	
	void stop_thread()
	{ 
        if (_thread_running)
        {
            {
                MyLock _l(_mutex);
                _thread_running = false;
                wakeup_thread();
            }
            pthread_join(_thread, 0);
        }
	}
};

#endif

