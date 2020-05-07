#ifndef _GAME_CLIENT_H_
#define _GAME_CLIENT_H_

#include <stdint.h>
#include <string>
#include "json.hpp"

typedef struct {
    std::string question;
    std::string choice_a;
    std::string choice_b;
    std::string choice_c;
    std::string choice_d;
    std::string correct_answer;
}QuestionInfo;

typedef struct {
    uint8_t category;
    QuestionInfo questions[5];
}TourInfo;

typedef struct {
    uint8_t category = 1;
    int right = 0;
    int wrong = 0;
}ResultInfo;

typedef struct {
    uint64_t uid = 0;
    ResultInfo tour_result[3];
    uint8_t tour_wins = 0;
}GameUser;

class GameClient {
    public:
    GameClient();
    ~GameClient();
    
    static GameClient *get_instance();
    
    void set_questions(nlohmann::json questions);
    QuestionInfo get_next_question();
    bool check_answer(uint64_t uid, std::string answer);
    void show_next_question();
    bool finish_question();
    bool finish_tour();
    void set_uid(uint8_t who, uint64_t uid);
    void set_game_id(int game_id);
    int get_game_id();
    uint64_t get_op_uid();
    uint64_t get_my_uid();
    void finish_game();
    uint64_t get_winner();
    
    GameUser get_results(uint8_t who);
    
    private:
    static GameClient *_ps_instance;
    int _game_id;
    TourInfo _tours[3];
    int _current_question;
    int _current_tour;
    GameUser _me;
    GameUser _op;
    uint64_t _winner;
    
    std::string get_current_correct_answer();
    
};

#endif // _GAME_CLIENT_H_