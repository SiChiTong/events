CPPFLAGS = -std=c++11 -ggdb -O0 -I/usr/local/include -fPIC
LDFLAGS = -L. -L/usr/local/lib -lev
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

test:
	@echo [TEST]
	@$(CXX) test.cpp $(CPPFLAGS) $(LDFLAGS) -levents -o test

clean:
	@rm -f $(TARGET)
	@rm -f $(OBJECTS)
	@rm -f test
	@rm -f *~
	@rm -f \#*\#
