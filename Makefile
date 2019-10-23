LDFLAGS  = -lstdc++ -m64 -march=native
CPPFLAGS = -march=native -std=c++17 -m64 -O3 -Wextra -Wall -Wshadow

CPPFLAGS += -isystem /home/pseyfert/.local/include
CPPFLAGS += -I.
CPPFLAGS += -fopenmp

LDLIBS   += -L/home/pseyfert/.local/lib -lbenchmark
# LDLIBS   += -ltbb

all: partition
