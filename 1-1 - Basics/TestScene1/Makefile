CC = g++
CFLAGS = -framework GLUT -framework OpenGL -framework Cocoa
SOURCES = main.cpp mesh.cpp light.cpp view.cpp scene.cpp
OBJECTS = $(SOURCES:.cpp=.o)
EXECUTE = main

all: $(OBJECTS) $(EXECUTE)

$(EXECUTE): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@
$(OBJECTS): $(SOURCES)
	$(CC) $(SOURCES) -c

run:
	./$(EXECUTE)
clean:
	rm -rf *.o $(EXECUTE)
