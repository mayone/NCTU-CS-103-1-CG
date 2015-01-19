CC = clang++
CFLAGS =  -I /opt/local/include
LDFLAGS = -framework GLUT -framework OpenGL -framework Cocoa /opt/local/lib/libGLEW.a /opt/local/lib/libfreeimage.a
SOURCES = main.cpp mesh.cpp light.cpp view.cpp scene.cpp
OBJECTS = $(SOURCES:.cpp=.o)
EXECUTE = main

all: $(OBJECTS) $(EXECUTE)

$(EXECUTE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@
$(OBJECTS): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -c

run:
	./$(EXECUTE)
clean:
	rm -rf *~ *.o $(EXECUTE)
