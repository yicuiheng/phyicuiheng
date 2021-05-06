TARGET_NAME = phyicuiheng
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++20 -pedantic -DGLEW_STATIC
DEBUG_CXXFLAGS = -O0 -g3 -D_DEBUG -UNDEBUG
RELEASE_CXXFLAGS = -O3 -s -flto -DNDEBUG -U_DEBUG

LDFLAGS = `pkg-config --libs freetype2`
LIBS = -lglfw -lGLEW -lGL -lX11 -lXi
INCLUDE = -I./include `pkg-config --cflags freetype2`

SRCDIR = ./src
SRC = $(wildcard $(SRCDIR)/*.cpp)

BUILD_DIR = ./build
TARGET = $(BUILD_DIR)/$(TARGET_NAME)

OBJ = $(addprefix $(BUILD_DIR)/obj/, $(notdir $(SRC:.cpp=.o)))
DEPEND = $(OBJ:.o=.d)

.PHONY: debug
debug: CXXFLAGS+=$(DEBUG_CXXFLAGS)
debug: $(TARGET)

.PHONY: release
release: CXXFLAGS+=$(RELEASE_CXXFLAGS)
release: $(TARGET)

-include $(DEPEND)

$(TARGET): $(OBJ)
	$(CXX) -o $@ $^ $(LDFLAGS) $(LIBS)


$(BUILD_DIR)/obj/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE)  -o $@ -c -MMD -MP $<


.PHONY: clean
clean:
	-rm -f $(OBJ) $(DEPEND) $(TARGET)

.PHONY: run
run: $(TARGET)
	$(TARGET)