#include "videoMonitor.h"

using namespace cv;

int VideoMonitor::camHigh = 240;
int VideoMonitor::camWidth = 240;
CvCapture *VideoMonitor::cap = NULL;
RTSPServer* VideoMonitor::rtspServer = NULL;


UsageEnvironment *env = NULL;
char * ptr = NULL;
H264VideoStreamFramer *videoSource = NULL;
RTPSink *videoSink = NULL;

EventTriggerId DeviceSource::eventTriggerId = 0;

VideoMonitor::VideoMonitor()
{
}

VideoMonitor::~VideoMonitor()
{}

int VideoMonitor::init()
{
	mkfifo(FIFO, 0777);
	camHigh = 240;
	camWidth = 320;
	return 0;
}

int VideoMonitor::startMonitor()
{
	if(threadID_cam != 0){
		printf("monitor is running !\n");
		return -1;
	}

	if(cap != NULL){
		printf("camera is running !\n");
		return -1;
	}

	cap = cvCreateCameraCapture(-1);
	if(cap == NULL){
		perror("open camera error!\n");
		return -1;
	}

	if(pthread_create(&threadID_cam, NULL, thread_cam, NULL) != 0){
		perror("create thread cam error!\n");
		return -1;
	}

	
	//run live thread, only oncetime
	if(threadID_live555 == 0){
		if(pthread_create(&threadID_live555, NULL, thread_live555, NULL) != 0){
			perror("create thread live555 error!\n");
			return -1;
		}
	}
	return 0;
}

int VideoMonitor::stopMonitor()
{
	pthread_cancel(threadID_cam);
	threadID_cam = 0;

	cvReleaseCapture(&cap);
	cap = NULL;

	return 0;
}

void VideoMonitor::Destroy()
{
}

void *VideoMonitor::thread_cam(void *arg)
{
	IplImage *pFrame = NULL;
	cvNamedWindow("result", 1);

	cvSetCaptureProperty(cap,CV_CAP_PROP_FRAME_WIDTH,320);
	cvSetCaptureProperty(cap,CV_CAP_PROP_FRAME_HEIGHT,240);

	x264Encoder x264(camWidth, camHigh, 0, 33);

	int fd = open(FIFO, O_WRONLY|O_CREAT, 0777);
	if(fd < 0){
		printf("open fifo file error!");
		return 0;
	}

	while(true){
		pFrame = cvQueryFrame(cap);
		if(pFrame == NULL) break;

		cvShowImage("result", pFrame);
		Mat mat = cvarrToMat(pFrame);
		int size = x264.EncodeOneFrame(mat);
		unsigned char *data = x264.GetEncodedFrame();
		write(fd, data, size);
	}
}

void *VideoMonitor::thread_live555(void *arg)
{
	// Begin by setting up our usage environment:
	TaskScheduler* scheduler = BasicTaskScheduler::createNew();
	env = BasicUsageEnvironment::createNew(*scheduler);

	// Create 'groupsocks' for RTP and RTCP:
	struct in_addr destinationAddress;
	destinationAddress.s_addr = chooseRandomIPv4SSMAddress(*env);
	// Note: This is a multicast address.  If you wish instead to stream
	// using unicast, then you should use the "testOnDemandRTSPServer"
	// test program - not this test program - as a model.

	const unsigned short rtpPortNum = 18888;
	const unsigned short rtcpPortNum = rtpPortNum+1;
	const unsigned char ttl = 255;

	const Port rtpPort(rtpPortNum);
	const Port rtcpPort(rtcpPortNum);

	Groupsock rtpGroupsock(*env, destinationAddress, rtpPort, ttl);
	rtpGroupsock.multicastSendOnly(); // we're a SSM source
	Groupsock rtcpGroupsock(*env, destinationAddress, rtcpPort, ttl);
	rtcpGroupsock.multicastSendOnly(); // we're a SSM source

	// Create a 'H264 Video RTP' sink from the RTP 'groupsock':
	OutPacketBuffer::maxSize = 600000;
	videoSink = H264VideoRTPSink::createNew(*env, &rtpGroupsock, 96);

	// Create (and start) a 'RTCP instance' for this RTP sink:
	const unsigned estimatedSessionBandwidth = 10000; // in kbps; for RTCP b/w share
	const unsigned maxCNAMElen = 100;
	unsigned char CNAME[maxCNAMElen+1];
	gethostname((char*)CNAME, maxCNAMElen);
	CNAME[maxCNAMElen] = '\0'; // just in case
	RTCPInstance* rtcp
		= RTCPInstance::createNew(*env, &rtcpGroupsock,
				estimatedSessionBandwidth, CNAME,
				videoSink, NULL /* we're a server */,
				True /* we're a SSM source */);
	// Note: This starts RTCP running automatically

	//RTSPServer* rtspServer = RTSPServer::createNew(*env, 8554);
	rtspServer = RTSPServer::createNew(*env, 8554);
	if (rtspServer == NULL) {
		*env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
		exit(1);
	}
	ServerMediaSession* sms
		= ServerMediaSession::createNew(*env, "testStream", FIFO,
				"Session streamed by \"testH264VideoStreamer\"",
				True /*SSM*/);
	sms->addSubsession(PassiveServerMediaSubsession::createNew(*videoSink, rtcp));
	rtspServer->addServerMediaSession(sms);

	char* url = rtspServer->rtspURL(sms);
	*env << "Play this stream using the URL \"" << url << "\"\n";
	delete[] url;

	// Start the streaming:
	*env << "Beginning streaming...\n";
	play();

	env->taskScheduler().doEventLoop(); // does not return
	return 0;
}




void VideoMonitor::afterPlaying(void* /*clientData*/) {
	*env << "...done reading from file\n";
	videoSink->stopPlaying();
	Medium::close(videoSource);
	//Camera.Destory();
	// Note that this also closes the input file that this source read from.

	// Start playing once again:
	play();
}

void VideoMonitor::play() 
{
	// Open the input file as a 'byte-stream file source':
	ByteStreamFileSource* fileSource
		= ByteStreamFileSource::createNew(*env, FIFO);
	if (fileSource == NULL) {
		*env << "Unable to open file \"" << FIFO
			<< "\" as a byte-stream file source\n";
		exit(1);
	}

	FramedSource* videoES = fileSource;

	// Create a framer for the Video Elementary Stream:
	videoSource = H264VideoStreamFramer::createNew(*env, videoES);

	// Finally, start playing:
	*env << "Beginning to read from file...\n";
	videoSink->startPlaying(*videoSource, afterPlaying, videoSink);
}

