CXX = g++
SDL_LIB = -L/usr/lib -lSDL2 -lSDL2_mixer -Wl,-rpath=/usr/lib
SDL_INCLUDE = -I/usr/include
CXXFLAGS = -Wall -c -std=c++11 $(SDL_INCLUDE)
LDFLAGS = $(SDL_LIB)
VPATH = src
BUILDDIR = build
BIN = yacht

all: $(BIN)

$(BIN): $(BUILDDIR)/main.o $(BUILDDIR)/chip8core.o $(BUILDDIR)/sdlhelper.o
	$(CXX) $(BUILDDIR)/main.o $(BUILDDIR)/chip8core.o $(BUILDDIR)/sdlhelper.o $(LDFLAGS) -o $(BUILDDIR)/$@

$(BUILDDIR)/main.o: main.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

$(BUILDDIR)/chip8core.o: chip8core.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

$(BUILDDIR)/sdlhelper.o: sdlhelper.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

test:
	cd $(BUILDDIR) && \
	./$(BIN)

clean:
	rm $(BUILDDIR)/*.o && rm $(BUILDDIR)/$(BIN)
