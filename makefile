# CC = gcc

# INCLUDES = -I/home/mrjantz/cs360/include
# LIBDIR = /home/mrjantz/cs360/lib

# CFLAGS = $(INCLUDES)
# LIBS = $(LIBDIR)/libfdr.a

# EXECUTABLES = bin/jshell

# all: $(EXECUTABLES)

# testers: bin/cattostde \
#          bin/strays \
#          bin/strays-files \
#          bin/strays-fsleep \
#          bin/strays-sleep \

# bin/jshell: src/jshell.c
# 	$(CC) $(CFLAGS) -o bin/jshell src/jshell.c $(LIBS)

# bin/strays: src/strays.c
# 	$(CC) -o bin/strays src/strays.c

# bin/cattostde: src/cattostde.c
# 	$(CC) -o bin/cattostde src/cattostde.c

# bin/strays-files: src/strays-files.c
# 	$(CC) -o bin/strays-files src/strays-files.c

# bin/strays-fsleep: src/strays-fsleep.c
# 	$(CC) -o bin/strays-fsleep src/strays-fsleep.c

# bin/strays-sleep: src/strays-sleep.c
# 	$(CC) -o bin/strays-sleep src/strays-sleep.c

CC = gcc

INCLUDES = -I/home/mrjantz/cs360/include
LIBDIR = /home/mrjantz/cs360/lib

# Add -fsanitize=address for AddressSanitizer and -g for debug symbols
CFLAGS = -g -fsanitize=address $(INCLUDES)
LDFLAGS = -fsanitize=address
LIBS = $(LIBDIR)/libfdr.a

EXECUTABLES = bin/jshell

all: $(EXECUTABLES)

testers: bin/cattostde \
         bin/strays \
         bin/strays-files \
         bin/strays-fsleep \
         bin/strays-sleep \

bin/jshell: src/jshell.c
	$(CC) $(CFLAGS) -o bin/jshell src/jshell.c $(LIBS) $(LDFLAGS)

bin/strays: src/strays.c
	$(CC) $(CFLAGS) -o bin/strays src/strays.c $(LDFLAGS)

bin/cattostde: src/cattostde.c
	$(CC) $(CFLAGS) -o bin/cattostde src/cattostde.c $(LDFLAGS)

bin/strays-files: src/strays-files.c
	$(CC) $(CFLAGS) -o bin/strays-files src/strays-files.c $(LDFLAGS)

bin/strays-fsleep: src/strays-fsleep.c
	$(CC) $(CFLAGS) -o bin/strays-fsleep src/strays-fsleep.c $(LDFLAGS)

bin/strays-sleep: src/strays-sleep.c
	$(CC) $(CFLAGS) -o bin/strays-sleep src/strays-sleep.c $(LDFLAGS)
