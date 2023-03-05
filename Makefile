
define SourcePaths
GraphicsEngine
GraphicsEngine/GUI
GraphicsEngine/GUI/Layout
GraphicsEngine/Input
src
endef

define IncludePaths
/usr/include/freetype2
.
endef

define ExcludeFiles
endef

_CPPFILES=$(foreach dir,$(SourcePaths),$(wildcard $(dir)/*.cpp))

CPPFILES=$(filter-out $(ExcludeFiles),$(_CPPFILES))

_OBJ=$(foreach files,$(CPPFILES),$(patsubst %.cpp,%.o,$(files)))

OBJ=$(patsubst %,./obj/%,$(_OBJ))

INCLUDE=$(foreach dir,$(IncludePaths),-I$(dir))

LFLAGS=-lGLEW -lglfw -lGL -lfreetype 

CXXFLAGS= $(LFLAGS) $(INCLUDE) -O2

example: $(OBJ)
	g++ $(OBJ) -o example $(CXXFLAGS)


init:
	mkdir -p src obj obj/dep $(foreach dir,$(SourcePaths),obj/$(dir))  $(foreach dir,$(SourcePaths),obj/dep/$(dir))

clean:
	rm -f ./example ./obj/*.o ./obj/dep/*.d $(foreach dir,$(SourcePaths),obj/$(dir)/*.o) $(foreach dir,$(SourcePaths),obj/dep/$(dir)/*.d)

./obj/dep/%.d: %.cpp
	@rm -f $@
	@g++ -MM $< $(CXXFLAGS) > $@.tmp
	@sed 's,\($(@F:.d=.o)\)[ :]*,$(patsubst obj/dep/%.d,./obj/%.o,$@) :,g' < $@.tmp > $@
	@sed -i '$$a \\tg++ -c $< -o $(patsubst obj/dep/%.d,./obj/%.o,$@) $(CXXFLAGS)' $@
	@rm -f $@.tmp

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),init)
-include $(patsubst %,./obj/dep/%,$(_OBJ:.o=.d))
endif
endif
