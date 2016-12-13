#define __searchnode__
#include <iostream>
#include <csignal>
#include <signal.h>
#include <unistd.h>
#include <list>

#ifndef __bitboard__
#include "bitboard.h"
#endif

#ifndef __mytime__
#include "mytime.h"
#endif


/*******PARAMETERS*******/
double c = 1.18;
int SimulationNum = 100;

int LoopNum = 100;
int Timeout = 1;
/************************/

double MIN_DOUBLE = std::numeric_limits<double>::lowest();
double MAX_DOUBLE = std::numeric_limits<double>::max();

BitboardController bitboard_controller = BitboardController();
bool ret_flag = false;

// int DEBUG_SIMULATION = 0;

void alarm_handler(int signal){
    ret_flag = true;
    std::cout << "alram handler!!!" << std::endl;
    // std::cout << "DEBUG SimulationNum: " << DEBUG_SIMULATION << std::endl;
    return;
}

class Node{
public:
    // As a parent node
    double best_UCB = 0;
    Node* best_UCBchild_pointer = NULL;
    double best_win = 0;
    Node* best_winchild_pointer = NULL;
    // Node *nodes[64];
    std::list<Node*> children_pointers;
    int ChildNum = 0;

    // As a child node
    double W = 0;
    double N = 0; // Total Simulation in this Node;
    struct bitboard current_board;

    Node(struct bitboard this_bitboard){
        current_board = this_bitboard;
    }

    void make_children(){
        u64 valid_moves = bitboard_controller.get_valid_moves(current_board);
        u64 move;

        //DEBUGG
        // std::cout << "---show valid_moves:" << std::endl;
        // bitboard_controller.show_bit_string(valid_moves);

        for(int i = 0; i < 64; i++){
            move = (valid_moves & (One << i));
            if(move){
                struct bitboard new_board = current_board;
                bitboard_controller.update(new_board, move);
                // DEBUGG
                // std::cout << "--- --- --- after update i: " << i << std::endl;
                // bitboard_controller.show_board(new_board, stderr);

                Node* child_node = new Node(new_board);
                children_pointers.push_back(child_node);
                ChildNum ++;
            }

        }
    }

    void make_simulation(){
        for(int i = 0; i < SimulationNum; i++){
            // std::cout << "--- --- make_simulation i: " << i << std::endl;
            if(simulator()){
                W++;
            }
            N++;

            // if(i > 5){
            //     std::cout << DEBUG_VALID_MOVE << std::endl;
            //     std::cout << DEBUG_RANDOM_PICK << std::endl;
            //     std::cout << DEBUG_UPDATE << std::endl;
            //     exit(1);
            // }
            // DEBUG_SIMULATION ++;

        }
        return;
    }

    // return true if win
    bool simulator(){
        struct bitboard simulation_board = current_board;
        u64 valid_moves;
        u64 tile_to_put;

        while(!bitboard_controller.is_game_over(simulation_board)){
            valid_moves = bitboard_controller.get_valid_moves(simulation_board);
            tile_to_put = bitboard_controller.random_pick_move(valid_moves);
            bitboard_controller.update(simulation_board, tile_to_put);
        }
 
        int score = bitboard_controller.get_score(simulation_board);
        if(((current_board.turn==true) && score > 0) || 
            ((current_board.turn==false) && score < 0)){
            return true;
        }
        return false;
    }

    // return move in u64
    u64 UCB(){
        ret_flag = false;
        signal(SIGALRM, alarm_handler);
        alarm(Timeout);

        /* Step 1: Make Children */
        make_children();
        if(ChildNum == 0){ // No child means no valid moves
            return 0;
        }
        std::cout << "---ChildNum is: " << ChildNum << std::endl;

        /* Step 2: Init Simulation */
        // Simulation Among the child
        for(std::list<Node*>::iterator it=children_pointers.begin(); it != children_pointers.end(); ++it){
            (*it)->make_simulation();
        }

        /* Step 3: Simulation according best UCB child */
        int win_num;
        double ucb_score;
        double win_score;
        while(ret_flag!=true){
            // Calculate children's UCB Score
            // Add N
            N = 0;
            for(std::list<Node*>::iterator it=children_pointers.begin(); it != children_pointers.end(); ++it){
                N += (*it)->N;
            }
            // Find best ucb score in children
            for(std::list<Node*>::iterator it=children_pointers.begin(); it != children_pointers.end(); ++it){
                win_num = (*it)->N - (*it)->W;
                win_score = win_num / (*it)->N;
                ucb_score = win_score + c * sqrt(log(N)/(*it)->N);
                if(ucb_score > best_UCB){
                    best_UCB = ucb_score;
                    best_UCBchild_pointer = *it;
                }
                
            }
            best_UCBchild_pointer->make_simulation();
        }

        /* Step 4: find the child with best win rate */
        for(std::list<Node*>::iterator it=children_pointers.begin(); it != children_pointers.end(); ++it){
            win_num = (*it)->N - (*it)->W;
            win_score = win_num / (*it)->N;
            if(win_score > best_win){
                best_win = win_score;
                best_winchild_pointer = *it;
            }
        }

        // return best move with largest win_rate
        int DEBUG_COUNT = 0;
        for(std::list<Node*>::iterator it=children_pointers.begin(); it != children_pointers.end(); ++it){
            std::cout << "child " << DEBUG_COUNT << " has win score: " << (*it)->W/(*it)->N << std::endl;
            DEBUG_COUNT ++;
        }
        std::cout << "choose child with rate: " << best_winchild_pointer->W/best_winchild_pointer->N << std::endl;
        std::cout << "return board:" << std::endl;
        bitboard_controller.show_bit_string(bitboard_controller.get_filled_board(best_winchild_pointer->current_board) ^\
            bitboard_controller.get_filled_board(current_board));
        // std::cout << "return blackboard: " <<nodes[best_winchild]->current_board.blackboard << std::endl;
        // std::cout << "return whiteboard: " <<nodes[best_winchild]->current_board.whiteboard << std::endl;
        return bitboard_controller.get_filled_board(best_winchild_pointer->current_board) ^\
            bitboard_controller.get_filled_board(current_board);

    }

    ~Node(){
        // cout << "delete:" << a << endl;
        for(std::list<Node*>::iterator it=children_pointers.begin(); it != children_pointers.end(); ++it){
            delete *it;
        }
    }
};