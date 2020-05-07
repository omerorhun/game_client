#include "GameClient.h"
#include "debug.h"

using namespace std;

GameClient *GameClient::_ps_instance = NULL;

GameClient::GameClient() {
    if (_ps_instance == NULL) {
        _ps_instance = this;
        _current_question = 0;
        _current_tour = 0;
        _winner = 0;
    }
}

GameClient::~GameClient() {
    
}

GameClient *GameClient::get_instance() {
    if (_ps_instance == NULL) {
        _ps_instance = new GameClient();
    }
    
    return _ps_instance;
}

void GameClient::set_questions(nlohmann::json questions) {
    
    for (int i = 0; i < 3; i++) {
        _tours[i].category = questions["tours"].at(i)["category"];
        _me.tour_result[i].category = _tours[i].category;
        _op.tour_result[i].category = _tours[i].category;
        for (int j = 0; j < 5; j++) {
            _tours[i].questions[j].question = questions["tours"].at(i)["questions"].at(j)["question"];
            _tours[i].questions[j].choice_a = questions["tours"].at(i)["questions"].at(j)["a"];
            _tours[i].questions[j].choice_b = questions["tours"].at(i)["questions"].at(j)["b"];
            _tours[i].questions[j].choice_c = questions["tours"].at(i)["questions"].at(j)["c"];
            _tours[i].questions[j].choice_d = questions["tours"].at(i)["questions"].at(j)["d"];
            _tours[i].questions[j].correct_answer = questions["tours"].at(i)["questions"].at(j)["answer"];
        }
    }
}

bool GameClient::check_answer(uint64_t uid, string answer) {
    
    mlog.log_debug("check answer uid : %lu (%lu-%lu)", uid, _me.uid, _op.uid);
    
    if (answer.compare(get_current_correct_answer()) == 0) {
        if (_me.uid == uid) 
            _me.tour_result[_current_tour].right++;
        else
            _op.tour_result[_current_tour].right++;
        
        return true;
    }
    
    if (_me.uid == uid)
        _me.tour_result[_current_tour].wrong++;
    else
        _op.tour_result[_current_tour].wrong++;
    
    return false;
}

string GameClient::get_current_correct_answer() {
    string ret;
    
    ret = _tours[_current_tour].questions[_current_question%5].correct_answer;
    
    return ret;
}

QuestionInfo GameClient::get_next_question() {
    return _tours[_current_tour].questions[_current_question%5];
}

void GameClient::show_next_question() {
    QuestionInfo q = get_next_question();
    
    mlog.log_info("%s", q.question.c_str());
    mlog.log_info("A: %s", q.choice_a.c_str());
    mlog.log_info("B: %s", q.choice_b.c_str());
    mlog.log_info("C: %s", q.choice_c.c_str());
    mlog.log_info("D: %s", q.choice_d.c_str());
}

bool GameClient::finish_question() {
    bool ret = false;
    _current_question++;
    
    if ((_current_question % 5) == 0)
        ret = finish_tour();
    
    return ret;
}

bool GameClient::finish_tour() {
    ResultInfo me = _me.tour_result[_current_tour]; // for readability
    ResultInfo op = _op.tour_result[_current_tour];
    
    if (me.right > op.right)
        _me.tour_wins++;
    else if (me.right < op.right)
        _op.tour_wins++;
    
    _current_tour++;
    _current_question = 0;
    
    if ((_me.tour_wins == 2) || (_op.tour_wins == 2) || (_current_tour == 3))
    {
        if (_me.tour_wins > _op.tour_wins)
            _winner = _me.uid;
        else
            _winner = _op.uid;
        
        return true;
    }
    
    return false;
}

void GameClient::finish_game() {
    delete _ps_instance;
    _ps_instance = NULL;
}

void GameClient::set_uid(uint8_t who, uint64_t uid) {
    if (who)
        _me.uid = uid;
    else
        _op.uid = uid;
}

void GameClient::set_game_id(int game_id) {
    _game_id = game_id;
}

int GameClient::get_game_id() {
    return _game_id;
}

uint64_t GameClient::get_my_uid() {
    return _me.uid;
}

uint64_t GameClient::get_op_uid() {
    return _op.uid;
}

GameUser GameClient::get_results(uint8_t who) {
    if (who) {
        return _me;
    }
    
    return _op;
}

uint64_t GameClient::get_winner() {
    return _winner;
}
