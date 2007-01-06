WARNINGS = -ansi -Wall -Wfloat-equal -Wpointer-arith -Werror -Wnon-template-friend -Woverloaded-virtual -Wsign-promo -Wpmf-conversions -Wsynth -Wcast-qual
COMP_FLAGS = $(CXXFLAGS) $(WARNINGS) `pkg-config --cflags net6-1.0`
LINK_FLAGS = $(LIBS) `pkg-config --libs net6-1.0`

SELECT = select
CONN = conn
SERCLI = sercli

APPS = $(SELECT) $(CONN) $(SERCLI)

all: $(APPS)

$(SELECT): select.cpp
	g++ select.cpp $(COMP_FLAGS) $(LINK_FLAGS) -o $(SELECT)
$(CONN): conn.cpp
	g++ conn.cpp $(COMP_FLAGS) $(LINK_FLAGS) -o $(CONN)
$(SERCLI): sercli.cpp
	g++ sercli.cpp $(COMP_FLAGS) $(LINK_FLAGS) -o $(SERCLI)

clean:
	rm -f $(APPS)

