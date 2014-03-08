SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)
CXXFLAGS = -I../common -L../common -g
CXXLIBS = -lcommon -lpthread
CXX = g++

all: $(MAIN)

depend: $(DEPENDS)

clean:
	rm -f *.o *.d $(MAIN)

$(MAIN): $(OBJECTS)
	@$(CXX) $(CXXFLAGS) -o $@ $(OBJECTS) $(CXXLIBS)

%.o: %.c
	@$(CXX) $(CXXFLAGS) -o $@ -c $< $(CXXLIBS)