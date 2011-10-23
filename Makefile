HFILES = color.h config.h drawing.h Explosion.h FluidGrid.h GameMode.h \
         HighScoreSheet.h Input.h Menu.h Particle.h Player.h Texture.h \
         Timer.h tweak.h util.h vec.h

CXXFILES = drawing.cc DuelMode.cc GalagaMode.cc HighScoreMode.cc main.cc \
		   Menu.cc Texture.cc util.cc

CXXFLAGS = -O2 `sdl-config --cflags` -DNDEBUG
LDLIBS = `sdl-config --libs` -lSDL_ttf -lSDL_image -lGL -lGLU -lglut

ichor: $(HFILES) $(CXXFILES)
	g++ -o $@ $(CXXFLAGS) $(LDLIBS) $(CXXFILES)
	
