CPPFLAGS = -std=c++11 -ggdb -O0 -I/usr/local/include -fPIC
LDFLAGS = -L. -L/usr/local/lib -lev
SOURCES	= events.cc tcpclient.cc tcpserver.cc
OBJECTS	= $(foreach x, $(basename $(SOURCES)), $(x).o)

TARGET = libevents.so

all: $(OBJECTS) link test

link:
	@echo [LD]
	@$(CXX) -shared $(OBJECTS) -o $(TARGET) $(LDFLAGS)

%.o: %.cc
	@echo [CC] $@
	@$(CXX) $(CPPFLAGS) -o $@ -c $<

test:
	@echo [TEST]
	@$(CXX) test.cc $(CPPFLAGS) $(LDFLAGS) -levents -o test

clean:
	@rm -f $(TARGET)
	@rm -f $(OBJECTS)
	@rm -f test
	@rm -f *~
	@rm -f \#*\#
