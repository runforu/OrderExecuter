#include "EventCenter.h"

EventCenter& EventCenter::Instance() {
    static EventCenter _instance;
    return _instance;
}

void EventCenter::Post(int event) {
    for (std::vector<std::pair<int, boost::function<void()>>>::iterator i = m_vector.begin(); i != m_vector.end(); i++) {
        if (event == (*i).first) {
            (*i).second();
        }
    }
}

void EventCenter::Bind(int event, boost::function<void()> c) {
    m_vector.push_back(std::make_pair(event, c));
}