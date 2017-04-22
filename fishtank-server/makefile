CPPFLAGS := -g -c -std=c++11
LFLAGS := -lrt

OBJS := ../jni/network.o main.o Match.o Client.o object/Base.o object/Player.o object/Shell.o object/Platform.o

fishtank-server: $(OBJS)
	g++ -o $@ $(OBJS) $(LFLAGS)

%.o: %.cpp ../jni/network.h fishtank-server.h object.h ../jni/fishnet.h
	g++ -o $@ $(CPPFLAGS) $<

clean:
	rm *.o object/*.o

.PHONY: clean
