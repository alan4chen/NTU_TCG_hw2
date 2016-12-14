
all:
	g++ -std=c++11 search.cpp -o serach
	g++ -std=c++11 judge.cpp -o judge
	g++ -std=c++11 simple_http_UI.cpp -o simple_http_UI

judge:
	g++ -std=c++11 judge.cpp -o judge

ui:
	g++ -std=c++11 simple_http_UI.cpp -o simple_http_UI

search: search.cpp search_ucb.cpp clean
	g++ search.cpp -std=c++11 -O2 -Wall -o search
	g++ search_ucb.cpp -std=c++11 -O2 -Wall -o search_ucb
	g++ search_uct.cpp -std=c++11 -O2 -Wall -o search_uct
	g++ search_progressivepruning.cpp -std=c++11 -O2 -Wall -o search_progressivepruning

clean:
	rm -f ./search || true
	rm -f ./search_ucb || true
	rm -f ./search_uct || true
	rm -f ./search_progressivepruning || true