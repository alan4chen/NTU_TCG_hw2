killall judge
make judge
echo compile_judge_complete
make search
echo compile_search_complete
rm -rf log_ju.txt log_p1.txt log_p2.txt
./judge 7124 4 > log_ju.txt &
./search_uct 127.0.0.1 7124 > log_p1.txt &
./search_ucb 127.0.0.1 7124 > log_p2.txt &
