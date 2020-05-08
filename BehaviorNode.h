/***
 *  行为树 的行为节点定义
 * */
#ifndef BTREE_BEHAVIOR_NODE_H
#define BTREE_BEHAVIOR_NODE_H

#include <string>

enum PARALLEL_FINSH_CONDITION {
    E_PFC_OR = 1,
    E_PFC_AND
};

enum BEHAVIOR_RUNNING_STATE {
    BEHAVIOR_RUNNING = 0,
    BEHAVIOR_FINISH,
    BEHAVIOR_ERROR_TRANSITION = -1,
};

enum BEHAVIOR_TERMINAL_STATUS {
    BEHAVIOR_TERMINAL_READY = 1,
    BEHAVIOR_TERMINAL_RUNNING,
    BEHAVIOR_TERMINAL_FINISH,
};

class BehaviorNodeParam {
public:
    BehaviorNodeParam() {}
    ~BehaviorNodeParam() {}

    float feed_back_vel;            //反馈的速度
};

class BehaviorPrecondition {
public:
    virtual bool extern_condition(const BehaviorNodeParam& input) const = 0;
};

class BehaviorPreconditionTRUE : public BehaviorPrecondition {
public:
    virtual bool extern_condition(const BehaviorNodeParam& input) const {
        return true;
    }
};

class BehaviorPreconditionFALSE : public BehaviorPrecondition {
public:
    virtual bool extern_condition(const BehaviorNodeParam& input) const {
        return false;
    }
};

class BehaviorPreconditionNOT : public BehaviorPrecondition {
public:
    BehaviorPreconditionNOT(BehaviorPrecondition* hs) : m_not_condition(hs) {}

    ~BehaviorPreconditionNOT() {
        delete m_not_condition;
    }

    virtual bool extern_condition(const BehaviorNodeParam& input) const {
        return !m_not_condition->extern_condition(input);
    }
private:
    BehaviorPrecondition *m_not_condition;
};

class BehaviorPreconditionAND : public BehaviorPrecondition {
public:
    BehaviorPreconditionAND(BehaviorPrecondition *l_pre,BehaviorPrecondition *r_pre) :
            left_pre(l_pre),right_pre(r_pre) {
    }
    ~BehaviorPreconditionAND() {
        delete left_pre;
        delete right_pre;
    }

    virtual bool extern_condition(const BehaviorNodeParam& input) const {
        return left_pre->extern_condition(input) && right_pre->extern_condition(input);
    }
private:
    BehaviorPrecondition *left_pre;
    BehaviorPrecondition *right_pre;
};

class BehaviorPreconditionOR : public BehaviorPrecondition {
public:
    BehaviorPreconditionOR(BehaviorPrecondition *l_pre,BehaviorPrecondition *r_pre) :
            left_pre(l_pre),right_pre(r_pre) {
    }
    ~BehaviorPreconditionOR() {
        delete left_pre;
        delete right_pre;
    }

    virtual bool extern_condition(const BehaviorNodeParam& input) const {
        return left_pre->extern_condition(input) || right_pre->extern_condition(input);
    }
private:
    BehaviorPrecondition *left_pre;
    BehaviorPrecondition *right_pre;
};

class BehaviorPreconditionXOR : public BehaviorPrecondition {
public:
    BehaviorPreconditionXOR(BehaviorPrecondition *l_pre,BehaviorPrecondition *r_pre) :
            left_pre(l_pre),right_pre(r_pre) {
    }
    ~BehaviorPreconditionXOR() {
        delete left_pre;
        delete right_pre;
    }

    virtual bool extern_condition(const BehaviorNodeParam& input) const {
        return left_pre->extern_condition(input) ^ right_pre->extern_condition(input);
    }
private:
    BehaviorPrecondition *left_pre;
    BehaviorPrecondition *right_pre;
};

#define BEHAVIOR_NODE_MAX_COUNT         (16)

class BehaviorNode {

public:
    BehaviorNode(BehaviorNode *parent,BehaviorPrecondition *script = nullptr):debug_name("behavior"),
                m_child_count(0),m_parent_node(nullptr),m_active_node(nullptr),m_last_active_node(nullptr),
                m_precond(nullptr) {
                    for(int i = 0; i < BEHAVIOR_NODE_MAX_COUNT;i++) {
                        m_child_lists[i] = nullptr;
                    }
                    _set_parent_node(parent);
                    set_node_precondition(script);
                }
    virtual ~BehaviorNode() {}

    BehaviorNode& AddChild(BehaviorNode *child) {
        if(m_child_count == BEHAVIOR_NODE_MAX_COUNT) {
            printf("child buffer is full!!\n");
            return (*this);
        }

        m_child_lists[m_child_count++] = child;
        return (*this);
    }

    void set_active_node(BehaviorNode * node) {
        m_last_active_node = m_active_node;
        m_active_node = node;

        if(m_parent_node != nullptr) {
            m_parent_node->set_active_node(node);
        }
    }

    const BehaviorNode* get_last_active_node() const {
        return m_last_active_node;
    }

    int do_tick(const BehaviorNodeParam &input,BehaviorNodeParam &output) {
        return _do_tick(input,output);
    }

    bool do_evaluate(const BehaviorNodeParam &input) {

        if(m_precond == nullptr) {
            return _do_evaluate(input);
        } else {
            return m_precond->extern_condition(input) && _do_evaluate(input);
        }
    }

    void do_transition(const BehaviorNodeParam &input) {
        _do_transition(input);
    }

    BehaviorNode& set_node_precondition(BehaviorPrecondition *script) {
        if(m_precond != script) {
            if(m_precond)
                delete m_precond;
            m_precond = script;
        }

        return (*this);
    }

    BehaviorNode& set_debug_name(const char* name) {
        debug_name = name;
        return (*this);
    }

    const char* get_debug_name() const {
        return debug_name.c_str();
    }

public:
    //运行函数
    virtual int _do_tick(const BehaviorNodeParam &input,BehaviorNodeParam &output) {
        return BEHAVIOR_FINISH;
    }

    virtual void _do_transition(const BehaviorNodeParam &input) {
    }

    virtual bool _do_evaluate(const BehaviorNodeParam &input) {
        return true;
    }

protected:
    bool _check_index(int index) const {
        return (index >= 0) && (index < BEHAVIOR_NODE_MAX_COUNT);
    }

    void _set_parent_node(BehaviorNode *parent) {
        m_parent_node = parent;
    }

protected:
    std::string debug_name;
    BehaviorNode *m_child_lists[BEHAVIOR_NODE_MAX_COUNT];
    int m_child_count;
    BehaviorNode *m_parent_node;
    BehaviorNode *m_active_node;
    BehaviorNode *m_last_active_node;

    BehaviorPrecondition *m_precond;
};


//权重选择器
class BehaviorPrioritySelector : public BehaviorNode {

public:
    BehaviorPrioritySelector(BehaviorNode *parent,BehaviorPrecondition *script = nullptr) :
            BehaviorNode(parent,script),m_current_index(0),m_last_select_index(0) {}

    virtual int _do_tick(const BehaviorNodeParam &input,BehaviorNodeParam &output);
    virtual void _do_transition(const BehaviorNodeParam &input);
    virtual bool _do_evaluate(const BehaviorNodeParam &input);

protected:
    int m_current_index;
    int m_last_select_index;
};

//无权重 选择器
class BehaviorNonePrioritySelector : public BehaviorPrioritySelector {

public:
    BehaviorNonePrioritySelector(BehaviorNode *parent,BehaviorPrecondition *script = nullptr) :
            BehaviorPrioritySelector(parent,script) {}

    virtual bool _do_evaluate(const BehaviorNodeParam &input);
};

//顺序器
class BehaviorSequeue : public BehaviorNode {

public:
    BehaviorSequeue(BehaviorNode *parent,BehaviorPrecondition *script = nullptr) :
            BehaviorNode(parent,script), m_current_index(BEHAVIOR_NODE_MAX_COUNT) {}

    virtual int _do_tick(const BehaviorNodeParam &input,BehaviorNodeParam &output);
    virtual void _do_transition(const BehaviorNodeParam &input);
    virtual bool _do_evaluate(const BehaviorNodeParam &input);

private:
    int m_current_index;
};

// 控制行为
class BehaviorTerminal : public BehaviorNode {
public:
    BehaviorTerminal(BehaviorNode *parent,BehaviorPrecondition *script = nullptr) :
            BehaviorNode(parent,script),m_status(),m_need_exit(false) {}

    virtual int _do_tick(const BehaviorNodeParam &input,BehaviorNodeParam &output);
    virtual void _do_transition(const BehaviorNodeParam &input);

protected:
    virtual void _do_enter(const BehaviorNodeParam &input) {}
    virtual int  _do_execute(const BehaviorNodeParam &input,BehaviorNodeParam &output) {
        return BEHAVIOR_FINISH;
    }
    virtual void _do_exit(const BehaviorNodeParam &input,int exit_id) {}

private:
    int m_status;
    bool m_need_exit;
};

//综合节点
class BehaviorParallel : public BehaviorNode {
public:
    BehaviorParallel(BehaviorNode *parent,BehaviorPrecondition *script = nullptr) :
        BehaviorNode(parent,script),m_parallel_condition(0) {
        for(int i = 0; i < BEHAVIOR_NODE_MAX_COUNT;i++) {
            m_childs_status[i] = BEHAVIOR_RUNNING;
        }
    }

    virtual bool _do_evaluate(const BehaviorNodeParam &input);
    virtual int _do_tick(const BehaviorNodeParam &input,BehaviorNodeParam &output);
    virtual void _do_transition(const BehaviorNodeParam &input);

    BehaviorParallel& set_finsh_condition(int condition);

private:
    int m_parallel_condition;
    int m_childs_status[BEHAVIOR_NODE_MAX_COUNT];
};

//循环节点
class BehaviorLoop : public BehaviorNode {
public:
    static const int defaut_loop = -1;
public:
    BehaviorLoop(BehaviorNode *parent,BehaviorPrecondition *script = nullptr,int loop = defaut_loop) :
            BehaviorNode(parent,script),m_loop_count(loop),m_current_count(0) {}

    virtual bool _do_evaluate(const BehaviorNodeParam &input);
    virtual int _do_tick(const BehaviorNodeParam &input,BehaviorNodeParam &output);
    virtual void _do_transition(const BehaviorNodeParam &input);

private:
    int m_loop_count;
    int m_current_count;
};

class BehaviorFactory {
public:
    static BehaviorNode& CreateParallelNode(BehaviorNode * parent,const char *debug_name,int condition) {
        BehaviorParallel *node = new BehaviorParallel(parent);
        node->set_finsh_condition(condition);
        create_node_common(node,parent,debug_name);
        return (*node);
    }

    static BehaviorNode& CreatePrioritySelectorNode(BehaviorNode * parent,const char *debug_name) {
        BehaviorPrioritySelector *node = new BehaviorPrioritySelector(parent);
        create_node_common(node,parent,debug_name);
        return (*node);
    }

    static BehaviorNode& CreateNonePrioritySelectorNode(BehaviorNode * parent,const char *debug_name) {
        BehaviorNonePrioritySelector *node = new BehaviorNonePrioritySelector(parent);
        create_node_common(node,parent,debug_name);
        return (*node);
    }

    static BehaviorNode& CreateSequenceNode(BehaviorNode * parent,const char *debug_name) {
        BehaviorSequeue *node = new BehaviorSequeue(parent);
        create_node_common(node,parent,debug_name);
        return (*node);
    }

    static BehaviorNode& CreateLoopNode(BehaviorNode *parent,const char * debug_name,int loop_count) {
        BehaviorNode *node = new BehaviorLoop(parent,nullptr,loop_count);
        create_node_common(node,parent,debug_name);
        return (*node);
    }

    template<typename T>
    static BehaviorNode& CreateTeminalNode(BehaviorNode *parent,const char *debug_name) {
        BehaviorNode *node = new T(parent);
        create_node_common(node,parent,debug_name);
        return (*node);
    }
private:
    static void create_node_common(BehaviorNode *current,BehaviorNode *parent,const char * debug_name) {
        if(parent) {
            parent->AddChild(current);
        }

        current->set_debug_name(debug_name);
    }
};


#endif //BTREE_BEHAVIOR_NODE_H
