#ifndef LRU_CACHE_H
#define LRU_CACHE_H

#include <unordered_map>
#include <list>

/**
 * A simple LRU Cache that mimics QCache behavior.
 * It takes ownership of the pointers and deletes them when evicted.
 * For this project, it uses delete[] because it's used for block buffers.
 */
template <typename Key, typename T>
class LRUCache {
public:
    LRUCache(int maxCost = 0) : m_maxCost(maxCost), m_currentCost(0) {}
    ~LRUCache() {
        clear();
    }

    void setMaxCost(int maxCost) {
        m_maxCost = maxCost;
        trim();
    }

    T* take(const Key& key) {
        auto it = m_map.find(key);
        if (it == m_map.end()) return nullptr;
        
        Node& node = it->second;
        T* obj = node.object;
        m_currentCost -= node.cost;
        m_list.erase(node.it);
        m_map.erase(it);
        return obj;
    }

    bool insert(const Key& key, T* object, int cost = 1) {
        if (!object) return false;
        
        // Remove existing if any
        auto it = m_map.find(key);
        if (it != m_map.end()) {
            T* oldObj = it->second.object;
            m_currentCost -= it->second.cost;
            m_list.erase(it->second.it);
            m_map.erase(it);
            delete[] oldObj;
        }
        
        if (m_maxCost > 0 && cost > m_maxCost) {
            delete[] object;
            return false;
        }

        m_list.push_front(key);
        Node node;
        node.object = object;
        node.cost = cost;
        node.it = m_list.begin();
        
        m_map[key] = node;
        m_currentCost += cost;
        trim();
        return true;
    }

    void clear() {
        for (typename std::unordered_map<Key, Node>::iterator it = m_map.begin(); it != m_map.end(); ++it) {
            delete[] it->second.object;
        }
        m_map.clear();
        m_list.clear();
        m_currentCost = 0;
    }

private:
    struct Node {
        T* object;
        int cost;
        typename std::list<Key>::iterator it;
    };

    void trim() {
        if (m_maxCost <= 0) return;
        while (m_currentCost > m_maxCost && !m_list.empty()) {
            Key last = m_list.back();
            T* obj = take(last);
            delete[] obj;
        }
    }

    int m_maxCost;
    int m_currentCost;
    std::list<Key> m_list;
    std::unordered_map<Key, Node> m_map;
};

#endif // LRU_CACHE_H
