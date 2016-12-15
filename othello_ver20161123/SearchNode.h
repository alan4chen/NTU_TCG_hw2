#define __searchnode__
#include <iostream>
#include <csignal>
#include <signal.h>
#include <unistd.h>
#include <cmath>
#include <list>

#ifndef __bitboard__
#include "bitboard.h"
#endif

#ifndef __mytime__
#include "mytime.h"
#endif


/*******PARAMETERS*******/
double c = 1.18;
int SimulationNum = 1000;

int LoopNum = 100;
int Timeout = 9;

double Rd = 2;
double PruningSigma = 0.4;
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
    double best_UCB = -1;
    Node* best_UCBchild_pointer = NULL;
    double best_win = -1;
    Node* best_winchild_pointer = NULL;
    // Node *nodes[64];
    std::list<Node*> children_pointers;
    int ChildNum = 0;

    // As a child node
    double W = 0;
    double N = 0; // Total Simulation in this Node;
    struct bitboard current_board;

    // UTC propogation
    double W_o = 0; // Origin Simulation Win
    double N_o = 0; // Origin Simulation Total Simulation Number

    // Progressive Pruning
    double bestMl = 0;

    // Temperal Parameters
    int win_num;
    double ucb_score;
    double win_score;

    double mean= 0;
    double sigma= 0;
    double Mr= 0;
    double Ml = 0;
    double sum= 0;
    double square_sum= 0;

    Node(struct bitboard this_bitboard){
        current_board = this_bitboard;
    }

    void make_children(){
        u64 valid_moves = bitboard_controller.get_valid_moves(current_board);
        u64 move;

        for(int i = 0; i < 64; i++){
            move = (valid_moves & (One << i));
            if(move){
                struct bitboard new_board = current_board;
                bitboard_controller.update(new_board, move);
                Node* child_node = new Node(new_board);
                children_pointers.push_back(child_node);
                ChildNum ++;
            }

        }
    }

    void make_simulation(){
        for(int i = 0; i < SimulationNum; i++){
            // if(ret_flag){
            //     if(N == 0){
            //         N = 1;
            //     }
            //     return;
            // }
            if(simulator()){
                W++;
            }
            N++;
        }
        return;
    }

    // return true if win
    bool simulator(){
        // DEBUG_SIMULATION ++;
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

    void UCT_SESB(bool &ProgressivePruning){
        /* Step 1: Selection */
        bool isSelected = false;
        if(ChildNum == 0){
            isSelected = true;
        }
        else{
            best_UCBchild_pointer->UCT_SESB(ProgressivePruning);
        }

        /* Step 2: Expansion */
        if(isSelected){
            W_o = W; // Save the original win rate & simulation number
            N_o = N;
            make_children();
            if(ChildNum == 0){ // No child means no valid moves, give a pass move
                struct bitboard new_board = current_board;
                u64 move = 0;
                bitboard_controller.update(new_board, move);
                Node* child_node = new Node(new_board);
                children_pointers.push_back(child_node);
                ChildNum ++;
            }
        }

        /* Step 3: Simulation */
        if(isSelected){
            for(std::list<Node*>::iterator it=children_pointers.begin(); it != children_pointers.end(); ++it){
                (*it)->make_simulation();
            }
            
        }
        /* Step 4: Backpropagation*/
        W = W_o;
        N = N_o;
        for(std::list<Node*>::iterator it=children_pointers.begin(); it != children_pointers.end(); ++it){
            N += (*it)->N;
            W += (*it)->N - (*it)->W;
        }

        /* Progressive Pruning */
        if(ProgressivePruning){
            doProgressivePruning();
        }


        // Find best ucb score in children
        best_UCB = 0;
        for(std::list<Node*>::iterator it=children_pointers.begin(); it != children_pointers.end(); ++it){
            win_num = (*it)->N - (*it)->W;
            win_score = win_num / (*it)->N;
            ucb_score = win_score + c * sqrt(log(N)/(*it)->N);
            if(ucb_score > best_UCB){
                best_UCB = ucb_score;
                best_UCBchild_pointer = *it;
            }
        }
        
        return;
    }

    void doProgressivePruning(){
        // update mean, sd in this node
            sum = 0;
            square_sum = 0;
            for(std::list<Node*>::iterator it=children_pointers.begin(); it != children_pointers.end(); ++it){
                win_num = (*it)->N - (*it)->W;
                win_score = win_num / (*it)->N;
                sum += win_score;
                square_sum += win_score * win_score;
            }
            mean = sum/ChildNum;
            sigma = sqrt(square_sum/ChildNum-mean*mean);

            if(ChildNum > 2){
                // Find Best Ml among children
                bestMl = 0;
                for(std::list<Node*>::iterator it=children_pointers.begin(); it != children_pointers.end(); ++it){
                    if((*it)->ChildNum == 0){
                        continue;
                    }
                    Ml = 1 - (*it)->mean - Rd * (*it)->sigma;
                    // std::cout << "(*it)->Ml is: " << Ml << " sigma is: " << (*it)->sigma << " mean is: " << (*it)->mean <<  " childnum: " << (*it)->ChildNum << std::endl;
                    if((*it)->sigma < PruningSigma && Ml > bestMl){
                        bestMl = Ml;
                    }
                    // if(Ml == 1 && ChildNum > 1){
                        // std::cout << "error!\n";
                        // std::cout << " before show_board -- (*it)->Ml is: " << Ml << " sigma is: " << (*it)->sigma << std::endl;
                        // bitboard_controller.show_board(current_board, stdout);
                        // exit(1);
                    // }
                }

                // Cut a child if child.Mr < bestMl
                for(std::list<Node*>::iterator it=children_pointers.begin(); it != children_pointers.end(); ++it){
                    if((*it)->ChildNum == 0){
                        continue;
                    }
                    Mr = 1 - (*it)->mean + Rd * (*it)->sigma;
                    if((*it)->sigma < PruningSigma && Mr < bestMl){
                        // std::cout << "--------CUT!!!" << std::endl;
                        delete (*it);
                        children_pointers.erase(it);
                        ChildNum --;
                        --it;
                    }
                }
            }
        return;
    }

    // Not Used
    void doProgressivePruning_Bonomial(){
        // exit(1);
        // update mean, sd in this node
            sum = 0;
            square_sum = 0;

            // find bestMl
            bestMl = 0;
            for(std::list<Node*>::iterator it=children_pointers.begin(); it != children_pointers.end(); ++it){
                win_num = (*it)->N - (*it)->W;
                mean = win_num / (*it)->N;
                sigma = sqrt(mean*(1-mean));
                if(sigma < PruningSigma && mean - Rd * sigma > bestMl){
                    bestMl = mean - Rd * sigma;
                }
            }

            if(ChildNum > 2){
                // Find Best Ml among children
                bestMl = 0;
                for(std::list<Node*>::iterator it=children_pointers.begin(); it != children_pointers.end(); ++it){
                    Ml = 1 - (*it)->mean - Rd * (*it)->sigma;
                    // std::cout << "(*it)->Ml is: " << Ml << " sigma is: " << (*it)->sigma << " mean is: " << (*it)->mean <<  " childnum: " << (*it)->ChildNum << std::endl;
                    if((*it)->sigma < PruningSigma && Ml > bestMl){
                        bestMl = Ml;
                    }
                }
                // Cut a child if child.Mr < bestMl
                for(std::list<Node*>::iterator it=children_pointers.begin(); it != children_pointers.end(); ++it){
                    win_num = (*it)->N - (*it)->W;
                    mean = win_num / (*it)->N;
                    sigma = sqrt(mean*(1-mean));
                    if(sigma < PruningSigma && mean + Rd * sigma < bestMl){
                        // std::cout << "--------CUT!!!" << std::endl;
                        delete *it;
                        children_pointers.erase(it);
                        ChildNum --;
                        --it;
                    }
                }
            }
        
    }

    u64 UCT(bool ProgressivePruning=false){
        // DEBUG_SIMULATION = 0;
        ret_flag = false;
        signal(SIGALRM, alarm_handler);
        alarm(Timeout);

        while(ret_flag!=true){
            // std::cout << "call UCT_SESB from root" << std::endl;
            UCT_SESB(ProgressivePruning);
        }

        /* find the child with best win rate */
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
        
        // std::cout << "DEBUG_SIMULATION: " << DEBUG_SIMULATION << std::endl;

        return bitboard_controller.get_filled_board(best_winchild_pointer->current_board) ^\
            bitboard_controller.get_filled_board(current_board);
    }   


    // return move in u64
    u64 UCB(){
        // DEBUG_SIMULATION = 0;
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
        while(ret_flag!=true){
            // Calculate children's UCB Score
            // Add N
            N = 0;
            for(std::list<Node*>::iterator it=children_pointers.begin(); it != children_pointers.end(); ++it){
                N += (*it)->N;
            }
            // Find best ucb score in children
            best_UCB = 0;
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
       
        // std::cout << "DEBUG_SIMULATION: " << DEBUG_SIMULATION << std::endl;

        // std::cout << "return blackboard: " <<nodes[best_winchild]->current_board.blackboard << std::endl;
        // std::cout << "return whiteboard: " <<nodes[best_winchild]->current_board.whiteboard << std::endl;
        return bitboard_controller.get_filled_board(best_winchild_pointer->current_board) ^\
            bitboard_controller.get_filled_board(current_board);

    }

    ~Node(){
        // std::cout << "delete Node" << std::endl;
        for(std::list<Node*>::iterator it=children_pointers.begin(); it != children_pointers.end(); ++it){
            delete *it;
        }
    }
};