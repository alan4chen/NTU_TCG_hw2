killall judge
make judge
echo compile_judge_complete
make search
echo compile_search_complete
mkdir uct_vs_random
rm -rf uct_vs_random/log_ju.txt uct_vs_random/log_p1.txt uct_vs_random/log_p2.txt
./judge 27123 60 > uct_vs_random/log_ju.txt &
sleep 1
./search_uct 127.0.0.1 27123 > uct_vs_random/log_p1.txt &
./search 127.0.0.1 27123 > uct_vs_random/log_p2.txt &