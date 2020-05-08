#include "BehaviorNode.h"

//权重选择
int BehaviorPrioritySelector::_do_tick(const BehaviorNodeParam &input,
                BehaviorNodeParam &output) {
    int return_value = BEHAVIOR_FINISH;

    if(_check_index(m_current_index)) {
        if(m_last_select_index != m_current_index) {
            if(_check_index(m_last_select_index)) {
                auto node = m_child_lists[m_last_select_index];
                node->do_transition(input);
            }

            m_last_select_index = m_current_index;
        }

        if(_check_index(m_last_select_index)) {
            auto node = m_child_lists[m_last_select_index];
            return_value = node->do_tick(input,output);
            if(return_value != BEHAVIOR_RUNNING) {
                m_last_select_index = BEHAVIOR_NODE_MAX_COUNT;
            }
        }
    }

    return return_value;
}

void BehaviorPrioritySelector::_do_transition(const BehaviorNodeParam &input) {

    if(_check_index(m_last_select_index)) {
        auto node = m_child_lists[m_last_select_index];
        node->_do_transition(input);
    }
}

bool BehaviorPrioritySelector::_do_evaluate(const BehaviorNodeParam &input) {
    m_current_index = BEHAVIOR_NODE_MAX_COUNT;
    for(int i = 0; i < m_child_count;i++) {

        auto node = m_child_lists[i];
        if(node->do_evaluate(input)) {
            m_current_index = i;
            return true;
        }
    }
    return false;
}

//无权重 选择器
bool BehaviorNonePrioritySelector::_do_evaluate(const BehaviorNodeParam &input) {

    if(_check_index(m_current_index)) {
        auto node = m_child_lists[m_current_index];
        if(node->do_evaluate(input)) {
            return true;
        }
    }

    return BehaviorPrioritySelector::_do_evaluate(input);
}

int BehaviorSequeue::_do_tick(const BehaviorNodeParam &input,BehaviorNodeParam &output) {
    int return_val = BEHAVIOR_FINISH;

    if(m_current_index == BEHAVIOR_NODE_MAX_COUNT)
        m_current_index = 0;

    auto node = m_child_lists[m_current_index];
    return_val = node->_do_tick(input,output);
    if(return_val == BEHAVIOR_FINISH) {
        m_current_index++;
        if(m_current_index == m_child_count) {
            m_current_index = BEHAVIOR_NODE_MAX_COUNT;
        } else {
            return_val = BEHAVIOR_RUNNING;
        }
    }

    if(return_val == BEHAVIOR_ERROR_TRANSITION) {
        m_current_index = BEHAVIOR_NODE_MAX_COUNT;
    }

    return return_val;
}

void BehaviorSequeue::_do_transition(const BehaviorNodeParam &input) {
    if(_check_index(m_current_index)) {
        auto node = m_child_lists[m_current_index];
        node->do_transition(input);
    }

    m_current_index = BEHAVIOR_NODE_MAX_COUNT;
}

bool BehaviorSequeue::_do_evaluate(const BehaviorNodeParam &input) {

    int tmp_index;
    if(m_current_index == BEHAVIOR_NODE_MAX_COUNT)
        tmp_index = 0;
    else
        tmp_index = m_current_index;

    if(_check_index(tmp_index)) {
        auto node = m_child_lists[tmp_index];
        if(node->do_evaluate(input))
            return true;
    }

    return false;
}

int BehaviorTerminal::_do_tick(const BehaviorNodeParam &input,BehaviorNodeParam &output) {
    int status = BEHAVIOR_FINISH;

    if(m_status == BEHAVIOR_TERMINAL_READY) {
        _do_enter(input);
        m_need_exit = true;
        m_status = BEHAVIOR_TERMINAL_RUNNING;

        set_active_node(this);
    }

    if(m_status == BEHAVIOR_TERMINAL_RUNNING) {
        status = _do_execute(input,output);
        set_active_node(this);
        if(status != BEHAVIOR_RUNNING) {
            m_status = BEHAVIOR_TERMINAL_FINISH;
        }
    }

    if(m_status == BEHAVIOR_TERMINAL_FINISH) {
        if(m_need_exit) {
            _do_exit(input,status);
            m_need_exit = false;
        }

        set_active_node(nullptr);
        m_status = BEHAVIOR_TERMINAL_READY;
        return status;
    }

    return status;
}

void BehaviorTerminal::_do_transition(const BehaviorNodeParam &input) {
    if(m_need_exit) {
        _do_exit(input,BEHAVIOR_ERROR_TRANSITION);
    }
    set_active_node(nullptr);
    m_status = BEHAVIOR_TERMINAL_READY;
    m_need_exit = false;
}

//综合节点
bool BehaviorParallel::_do_evaluate(const BehaviorNodeParam &input) {
    for(int i = 0; i < m_child_count;i++) {
        auto node = m_child_lists[i];
        if(m_childs_status[i] == BEHAVIOR_RUNNING) {
            if(!node->do_evaluate(input)) {
                return false;
            }
        }
    }

    return true;
}

int BehaviorParallel::_do_tick(const BehaviorNodeParam &input,BehaviorNodeParam &output) {

    int finshed_child_count = 0;

    for(int i = 0; i < m_child_count;i++) {
        auto node = m_child_lists[i];
        if(m_parallel_condition == E_PFC_OR) {
            if(m_childs_status[i] == BEHAVIOR_RUNNING) {
                m_childs_status[i] = m_child_lists[i]->do_tick(input,output);
            }
            if(m_childs_status[i] != BEHAVIOR_RUNNING) {
                for(int j = 0; j < BEHAVIOR_NODE_MAX_COUNT;j++)
                    m_childs_status[j] = BEHAVIOR_RUNNING;

                return BEHAVIOR_FINISH;
            }
        } else if (m_parallel_condition == E_PFC_AND) {
            if(m_childs_status[i] == BEHAVIOR_RUNNING) {
                m_childs_status[i] = m_child_lists[i]->do_tick(input,output);
            }

            if(m_childs_status[i] != BEHAVIOR_RUNNING) {
                finshed_child_count++;
            }
        }
    }

    if(finshed_child_count == m_child_count) {
        for(int j = 0; j < BEHAVIOR_NODE_MAX_COUNT;j++)
            m_childs_status[j] = BEHAVIOR_RUNNING;
        return BEHAVIOR_FINISH;
    }
    return BEHAVIOR_RUNNING;
}

void BehaviorParallel::_do_transition(const BehaviorNodeParam &input) {
    for(int i = 0; i < BEHAVIOR_NODE_MAX_COUNT; i++) {
        m_childs_status[i] = BEHAVIOR_RUNNING;
    }

    for(int i = 0; i < m_child_count;i++) {
        auto node = m_child_lists[i];
        node->do_transition(input);
    }
}

BehaviorParallel& BehaviorParallel::set_finsh_condition(int condition) {
    m_parallel_condition = condition;
    return (*this);
}

bool BehaviorLoop::_do_evaluate(const BehaviorNodeParam &input) {
    bool check_loop_count = (m_loop_count == defaut_loop) || (m_current_count < m_loop_count);

    if(!check_loop_count)
        return false;

    if(_check_index(0)) {
        auto node = m_child_lists[0];

        return node->do_evaluate(input);
    }

    return false;
}

int BehaviorLoop::_do_tick(const BehaviorNodeParam &input,BehaviorNodeParam &output) {

    int run_status = BEHAVIOR_FINISH;

    if(_check_index(0)) {
        auto node = m_child_lists[0];

        run_status = node->do_tick(input,output);
        if(run_status == BEHAVIOR_FINISH) {
            if(m_loop_count != m_current_count) {
                m_current_count++;

                if(m_current_count == m_loop_count) {
                    run_status = BEHAVIOR_RUNNING;
                }
            } else {
                run_status = BEHAVIOR_RUNNING;
            }
        }
    }

    if(run_status != BEHAVIOR_RUNNING) {
        m_current_count = 0;
    }

    return run_status;
}

void BehaviorLoop::_do_transition(const BehaviorNodeParam &input) {
    if(_check_index(0)) {
        auto node = m_child_lists[0];
        node->do_transition(input);
    }

    m_current_count = 0;
}




