CXX=g++
CXXFLAGS=`pkg-config opencv --cflags`
LDLIBS=-lglut -lGL -lGLU `pkg-config opencv --libs`
WITH_OPENGL=ON 
