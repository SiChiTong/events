ifeq ($(ASYNC_REDIS), 1)
CPPFLAGS = -I$(HIREDIS_PATH) -std=c++11 -ggdb -O0 -I/usr/local/include -fPIC -pthread -DASYNC_REDIS
LDFLAGS = -L$(HIREDIS_PATH) $(HIREDIS_PATH)/libhiredis.a -L. -L/usr/local/lib -lev -pthread
else
CPPFLAGS = -std=c++11 -ggdb -O0 -I/usr/local/include -fPIC -pthread
LDFLAGS = -L. -L/usr/local/lib -lev -pthread
endif
SOURCES	= events.cpp tcpclient.cpp tcpserver.cpp
OBJECTS	= $(foreach x, $(basename $(SOURCES)), $(x).o)

TARGET = libevents.so

all: $(OBJECTS) link test

link:
	@echo [LD]
	@$(CXX) -shared $(OBJECTS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	@echo [CC] $@
	@$(CXX) $(CPPFLAGS) -o $@ -c $<

test: test.cpp
	@echo [TEST]
	@$(CXX) test.cpp $(CPPFLAGS) $(LDFLAGS) -levents -o test

clean:
	@rm -f $(TARGET)
	@rm -f $(OBJECTS)
	@rm -f test
	@rm -f *~
	@rm -f \#*\#
