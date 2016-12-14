killall judge
make judge
echo compile_judge_complete
make search
echo compile_search_complete
mkdir uctpp_vs_uct
rm -rf uctpp_vs_uct/log_ju.txt uctpp_vs_uct/log_p1.txt uctpp_vs_uct/log_p2.txt
./judge 27125 60 > uctpp_vs_uct/log_ju.txt &
sleep 1
./search_progressivepruning 127.0.0.1 27125 > uctpp_vs_uct/log_p1.txt &
./search_uct 127.0.0.1 27125 > uctpp_vs_uct/log_p2.txt &
