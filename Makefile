
CC = arm-linux-gnueabi-g++

INCLUDES 	 = -I./include/live555/usageEnvironment/ \
		   -I./include/live555/groupsock/ \
		   -I./include/live555/liveMedia/ \
		   -I./include/live555/basicUsageEnvironment \
		   -I./include/x264 \
		   -I./include/ \
		   -I/usr/local/include/opencv -I/usr/local/include

LIVE555_LIBS 	=  ./lib/livelib/libliveMedia.a ./lib/livelib/libgroupsock.a \
		./lib/livelib/libBasicUsageEnvironment.a ./lib/livelib/libUsageEnvironment.a
X264_LIBS	=  ./lib/x264lib/libx264.so
opencv_LIBS 	= -L/usr/local/lib -lopencv_dnn -lopencv_highgui -lopencv_ml -lopencv_objdetect -lopencv_shape -lopencv_stitching -lopencv_superres -lopencv_videostab -lopencv_calib3d -lopencv_videoio -lopencv_imgcodecs -lopencv_features2d -lopencv_video -lopencv_photo -lopencv_imgproc -lopencv_flann -lopencv_core
LIBS = $(LIVE555_LIBS) $(X264_LIBS) $(opencv_LIBS)


arm_LIVE555_LIBS 	=  ./armlib/livelib/libliveMedia.a ./armlib/livelib/libgroupsock.a \
		./armlib/livelib/libBasicUsageEnvironment.a ./armlib/livelib/libUsageEnvironment.a
arm_X264_LIBS	=  ./armlib/x264lib/libx264.so
arm_opencv_LIBS 	= -L./armlib/opencv -lopencv_dnn -lopencv_highgui -lopencv_ml -lopencv_objdetect -lopencv_shape -lopencv_stitching -lopencv_superres -lopencv_videostab -lopencv_calib3d -lopencv_videoio -lopencv_imgcodecs -lopencv_features2d -lopencv_video -lopencv_photo -lopencv_imgproc -lopencv_flann -lopencv_core
arm_LIBS = $(arm_LIVE555_LIBS) $(arm_X264_LIBS) $(arm_opencv_LIBS)


all:
	g++ $(INCLUDES)  main.cpp videoMonitor.cpp x264Encoder.cpp -lpthread $(LIBS)
arm:
	$(CC) $(INCLUDES)  main.cpp videoMonitor.cpp x264Encoder.cpp -lpthread $(arm_LIBS)

clean:
	rm -rf ./*.o
