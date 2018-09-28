# DClient Makefile

//CC		:= gcc
//CXX		:= g++

CFLAGS	+= -std=c99
CXXFLAGS += -w -std=c++11
LIBS += -lpthread -lpcap -lcurl -ljpeg

DIR_ENCRYPT	= ./encrypt
DIR_CONV	= ./conv

OBJECTS = main.o tprocess.o adapter.o dpcap.o msgpass.o eappacket.o \
dcurl.o docr.o dir.o docrkeys.o encrypt.o dlog.o daemon.o\
lock.o darg.o $(DIR_ENCRYPT)/md5.o $(DIR_ENCRYPT)/rc4.o $(DIR_ENCRYPT)/crc32.o\
$(DIR_ENCRYPT)/base64.o $(DIR_CONV)/conv.o

dclient : $(OBJECTS)
	$(CXX) $(LDFLAGS) -o dclient $(OBJECTS) $(LIBS)

main.o : main.cpp
	$(CXX) $(CXXFLAGS) -c $<

tprocess.o : tprocess.cpp tprocess.h
	$(CXX) $(CXXFLAGS) -c $<

adapter.o : adapter.cpp adapter.h
	$(CXX) $(CXXFLAGS) -c $<

dpcap.o : dpcap.cpp dpcap.h
	$(CXX) $(CXXFLAGS) -c $<

msgpass.o : msgpass.cpp msgpass.h
	$(CXX) $(CXXFLAGS) -c $<

eappacket.o : eappacket.cpp eappacket.h
	$(CXX) $(CXXFLAGS) -c $<

dcurl.o : dcurl.cpp dcurl.h
	$(CXX) $(CXXFLAGS) -c $<
	
docr.o : docr.cpp docr.h
	$(CXX) $(CXXFLAGS) -c $<

dir.o : dir.cpp dir.h
	$(CXX) $(CXXFLAGS) -c $<

docrkeys.o : docrkeys.cpp
	$(CXX) $(CXXFLAGS) -c $<

encrypt.o : encrypt.cpp encrypt.h
	$(CXX) $(CXXFLAGS) -c $<
	
dlog.o : dlog.cpp dlog.h
	$(CXX) $(CXXFLAGS) -c $<

lock.o : lock.cpp lock.h
	$(CXX) $(CXXFLAGS) -c $<

darg.o : darg.cpp darg.h
	$(CXX) $(CXXFLAGS) -c $<

daemon.o : daemon.cpp daemon.h
	$(CXX) $(CXXFLAGS) -c $<

$(DIR_ENCRYPT)/%.o : $(DIR_ENCRYPT)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(DIR_CONV)/%.o : $(DIR_CONV)/%.c
	$(CC) $(CFLAGS) -c $< -o $@
	

.PHONY : clean

clean : 
	-rm ./*.o ./$(DIR_ENCRYPT)/*.o ./$(DIR_CONV)/*.o dclient
