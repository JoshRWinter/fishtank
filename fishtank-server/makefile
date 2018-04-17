CPPFLAGS := -g -c -std=c++11 -Wall -pedantic
LFLAGS := -lrt

OBJS := ../jni/network.o main.o WebView.o Match.o Client.o object/Base.o object/Player.o object/Shell.o object/Platform.o object/Airstrike.o object/Mine.o object/Grass.o

fishtank-server: $(OBJS) WebViewResources.o
	g++ -o $@ $(OBJS) WebViewResources.o $(LFLAGS)

WebViewResources.o: WebViewResources.cpp WebView.h WebViewResources.h
	g++ -o $@ $(CPPFLAGS) $<

%.o: %.cpp ../jni/network.h fishtank-server.h WebView.h object.h ../jni/fishnet.h
	g++ -o $@ $(CPPFLAGS) $<

clean:
	rm -f *.o object/*.o

release: clean
	g++ -o fishtank-server -O2 -std=c++11 -Wall -pedantic *.cpp ../jni/network.cpp object/*.cpp -s

.PHONY: clean release
