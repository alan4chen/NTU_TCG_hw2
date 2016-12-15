
all: clean judge ui search


judge: judge.cpp 
	g++ -std=c++11 judge.cpp -o judge

ui: simple_http_UI.cpp
	g++ -std=c++11 simple_http_UI.cpp -o simple_http_UI

search: 
	g++ search.cpp -std=c++11 -O2 -Wall -o search
	g++ search_ucb.cpp -std=c++11 -O2 -Wall -o search_ucb
	g++ search_uct.cpp -std=c++11 -O2 -Wall -o search_uct
	g++ search_progressivepruning.cpp -std=c++11 -O2 -Wall -o search_progressivepruning
	g++ search_ucts.cpp -std=c++11 -O2 -Wall -o search_ucts

clean:
	rm -f ./simple_http_UI || true
	rm -f ./judge || true
	rm -f ./search || true
	rm -f ./search_ucb || true
	rm -f ./search_uct || true
	rm -f ./search_progressivepruning || true
	rm -f ./search_ucts || true