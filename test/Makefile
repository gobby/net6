WARNINGS = -ansi -Wall -Wfloat-equal -Wpointer-arith -Werror -Wnon-template-friend -Woverloaded-virtual -Wsign-promo -Wpmf-conversions -Wsynth -Wcast-qual
COMP_FLAGS = $(CXXFLAGS) $(WARNINGS) `pkg-config --cflags net6-1.3`
LINK_FLAGS = $(LIBS) `pkg-config --libs net6-1.3`

SELECT = select
CONN = conn
SERCLI = sercli
TIMEOUT = timeout

APPS = $(SELECT) $(CONN) $(SERCLI) $(TIMEOUT)

all: $(APPS)

$(SELECT): select.cpp
	g++ select.cpp $(COMP_FLAGS) $(LINK_FLAGS) -o $(SELECT)
$(CONN): conn.cpp
	g++ conn.cpp $(COMP_FLAGS) $(LINK_FLAGS) -o $(CONN)
$(SERCLI): sercli.cpp
	g++ sercli.cpp $(COMP_FLAGS) $(LINK_FLAGS) -o $(SERCLI)
$(TIMEOUT): timeout.cpp
	g++ timeout.cpp $(COMP_FLAGS) $(LINK_FLAGS) -o $(TIMEOUT)

clean:
	rm -f $(APPS)

