rc resource.rc
cl /EHsc /std:c++14 *.cpp object/*.cpp ../jni/network.cpp ws2_32.lib resource.res /link /out:fishtank-server.exe
