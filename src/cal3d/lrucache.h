#ifndef IMVU_CAL3D_LRUCACHE_H
#define IMVU_CAL3D_LRUCACHE_H

#include <list>

//not threadsafe

template<class K, class V>
class lrucache {
private:
    typedef std::pair<K, V> pair_t;
    typedef std::list<pair_t> list_t;
    typedef std::map<K, typename list_t::iterator> map_t;

    list_t m_list;
    map_t m_map;
    unsigned int m_maxSize;

public:
    lrucache(unsigned int maxSize = 32) : m_maxSize(maxSize) {}

    void set(const K& k, const V& v) {
        map_t::const_iterator mi = m_map.find(k);
        if(mi != m_map.end()) {
            //key is already present, kill it first
            list_t::iterator li = mi->second;
            m_list.erase(li);
        }
        m_list.push_front(pair_t(k,v));
        m_map[k] = m_list.begin();
        if(size() <= m_maxSize) {
            return;
        }
        //expire the oldest entry
        list_t::iterator last = m_list.end();
        --last;
        const K& old_k = last->first;
        m_map.erase(old_k);
        m_list.erase(last);
    }

    V* get(const K& k) {
        map_t::const_iterator mi = m_map.find(k);
        if(mi == m_map.end()) return 0;
        list_t::iterator li = mi->second;
        //we need to move this entry to the head of the list
        list_t::iterator head = m_list.begin();
        if(li == head) { return &li->second; }

        m_list.insert(head, *li);
        m_list.erase(li);
        head = m_list.begin();
        m_map[k] = head;
        return &head->second;
    }

    bool isset(const K& k) const {
        map_t::const_iterator mi = m_map.find(k);
        return mi != m_map.end();
    }

    size_t size() const {
        return m_map.size();
    }
};

#endif
