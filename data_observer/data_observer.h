#ifndef _DATA_OBSERVER_H__
#define _DATA_OBSERVER_H__

#include <stdint.h>
#include <RWLock.h>
#include <bitset>
#include <string.h>
#include <stdlib.h>
#include <map>

namespace data_observer {

class DataNode {
public:
    DataNode(int node_len):node_data(nullptr),_node_rw_lock(true),
                           data_len(node_len),fream_id(0) {
        if(data_len > 0)
            node_data = new char[data_len];
    }
    ~DataNode() {
        if(node_data != nullptr)
            delete node_data;
        node_data = nullptr;
    }

    //检查数据是否有更新
    bool node_check_update(int id) {
        if(id != fream_id) {
            return true;
        }
        return false;
    }

    bool node_resize(int size) {
        if(size < 0 || size == data_len)
            return false;

        _node_rw_lock.writeLock();
        char * tmp;
        if(size > data_len) {
            tmp = new char[size];
            memset(tmp,'\0',size);
            memcpy(tmp,node_data,data_len);
            delete node_data;
            node_data = tmp;
            data_len = size;
        }
        _node_rw_lock.writeUnlock();
        return true;
    }

    //数据拷贝
    uint32_t node_copy(void *node,int node_len = -1) {

        _node_rw_lock.readLock();
        if(node_len == -1 || node_len == data_len)
            memcpy(node,node_data,data_len);
        else {
            if(node_len < data_len)
                memcpy(node,node_data,node_len);
            else
                memcpy(node,node_data,data_len);
        }
        _node_rw_lock.readUnlock();
        return fream_id;
    }

    //数据发布
    void node_publish(void *node,int node_len = -1) {

        printf("%s \n",(char *)node);
        _node_rw_lock.writeLock();
        if(node_len != data_len) {
            if(node_len > data_len || node_len == -1) {
                memcpy(node_data, node, data_len);
            } else {
                memset(node_data,'\0',data_len);
                memcpy(node_data, node, node_len);
            }
        } else {
            memcpy(node, node_data, node_len);
        }
        _node_rw_lock.writeUnlock();
        fream_id++;
    }

private:
    RWLock _node_rw_lock;

    char * node_data;
    uint32_t fream_id;
    uint32_t data_len;
};

typedef struct observer_node_id {
    int fream_id;
    int node_id;
}DORB_node_id_t;

#define NODE_OBSERVER_MAX_COUNT        (20)
class DataObserver {
public:
    using shared_node_t = shared_ptr<DataNode>;
    DataObserver() : data_observer_count(0),bit_map(0),
                     observer_rw_lock(true) {}
    ~DataObserver() {
        node_observer_map.clear();
    }

    //注册观察的数据 return value   0--> 存在  -1 : 不存在
    int RegisterObserver(const char* node_name,DORB_node_id_t &node) {

        std::string name = node_name;
        auto it = node_observer_map.find(name);
        if(it == node_observer_map.end()) {
            return -1;
        }

        node.fream_id = 0;
        node.node_id = it->second;
        return 0;
    }

    // -1 代表已经存在
    int RegisterPublish(const char *node_name,int data_len) {
        std::string name = node_name;
        auto it = node_observer_map.find(name);
        if(it != node_observer_map.end()) {
            return -1;
        } else {
            if(data_observer_count > NODE_OBSERVER_MAX_COUNT) {
                return -1;
            } else {
                put_observer_node(name,data_len);
            }
        }
        return 0;
    }

    int UnRegisterPublish(const char *node_name) {
        std::string name = node_name;

        if(remove_observer_node(name)) {
            return 0;
        } else {
            return -1;
        }
    }

    //发布数据
    // node_len : -1 代表 默认数据长度
    int Publish(const char *node_name,void *node_data,int node_len = -1) {

        printf("it -> name : %s  \n",node_name);
        std::string name = node_name;
        auto it = node_observer_map.find(name);
        if(it != node_observer_map.end()) {
            printf("it -> name : %s it->seconde : %d \n",it->first.c_str(),it->second);
            observer_nodes[it->second]->node_publish(node_data,node_len);
            return 0;
        } else {
            return -1;
        }
    }

    bool CopyData(DORB_node_id_t &input_node,void *node_data,int node_len = -1) {
        int id = input_node.node_id;

        auto p = observer_nodes[id];
        if(p->node_check_update(input_node.fream_id)) {
            input_node.fream_id = p->node_copy(node_data,node_len);
            return true;
        }

        return false;
    }

private:
    int get_free_id() {
        if(data_observer_count >= NODE_OBSERVER_MAX_COUNT) {
            return -2;
        }

        observer_rw_lock.readLock();
        for(int i = 0; i < NODE_OBSERVER_MAX_COUNT;i++ ) {
            if(!bit_map[i]) {
                observer_rw_lock.readUnlock();
                return i + 1;
            }
        }
        observer_rw_lock.readUnlock();

        return -1;
    }

    bool put_observer_node(string name ,int data_len) {
        int id = get_free_id();
        if(id < 0) {
            return false;
        }

        observer_rw_lock.writeLock();
        bit_map[id - 1] = 1;
        data_observer_count++;
        observer_rw_lock.writeUnlock();

//        printf("put name ---> %s data_observer_count : %d \n",name.c_str(),data_observer_count);

//        observer_nodes[id] = new DataNode(data_len);
//        observer_nodes[id] = std::shared_ptr<DataNode>(data_len);

        observer_nodes[id] = std::make_shared<DataNode>(data_len) ;
        node_observer_map.insert(std::pair<std::string,int>(name,id));
        return true;
    }

    bool remove_observer_node(std::string node_name) {
        auto it = node_observer_map.find(node_name);

        if(it != node_observer_map.end()) {
            int id = it->second;
            if(check_subcriber_id(id)) {
                observer_rw_lock.writeLock();
                bit_map[id] = 0;
                data_observer_count--;
                observer_rw_lock.writeUnlock();

                observer_nodes[id].reset();
                node_observer_map.erase(it);
            } else
                return true;
        } else
            return true;
    }

    // id 有没有
    bool check_subcriber_id(int id) {
        if(id >= NODE_OBSERVER_MAX_COUNT) {
            return false;
        }

        observer_rw_lock.readLock();
        if(bit_map[id]) {
            observer_rw_lock.readUnlock();
            return true;
        } else {
            observer_rw_lock.readUnlock();
            return false;
        }
    }

private:
    RWLock observer_rw_lock;

    int data_observer_count;
    std::bitset<NODE_OBSERVER_MAX_COUNT> bit_map;
    shared_node_t observer_nodes[NODE_OBSERVER_MAX_COUNT];
//    DataNode *observer_nodes[NODE_OBSERVER_MAX_COUNT];
    std::map<std::string,int > node_observer_map;
};

};

#endif //_DATA_OBSERVER_H__
