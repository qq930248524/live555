#ifndef _VIDEO_MONITOR_H_
#define _VIDEO_MONITOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

//file
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//opencv
#include <cxcore.h>
#include <highgui.h>
#include <cv.h>


//live555
#include <liveMedia.hh>
#include <BasicUsageEnvironment.hh>
#include <GroupsockHelper.hh>
#include <sys/types.h>
#include <sys/stat.h>

#include "x264Encoder.h"

#define FIFO "/tmp/fifo"

class VideoMonitor
{
	public:
		VideoMonitor();
		~VideoMonitor();
		int init();
		int startMonitor();
		int stopMonitor();
		void Destroy();

	private:
		pthread_t threadID_cam;
		pthread_t threadID_live555;
		static void *thread_cam(void *arg);
		static void *thread_live555(void *arg);

		static CvCapture *cap;
		static int camHigh;
		static int camWidth;
		static RTSPServer* rtspServer;
		static void play();
		static void afterPlaying(void *);
};


#endif
