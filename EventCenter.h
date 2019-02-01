#ifndef _EVENTCENTER_H_
#define _EVENTCENTER_H_

#include <boost/function.hpp>
#include <utility>
#include <vector>

class EventCenter {
public:
    static const int EV_CONFIG_CHANGED = 1;

    static EventCenter& Instance();

    void Post(int event);

    void Bind(int event, boost::function<void()> c);

private:
    EventCenter() {}
    ~EventCenter() {}
    EventCenter(EventCenter const&) {}
    void operator=(EventCenter const&) {}

private:
    std::vector<std::pair<int, boost::function<void()>>> m_vector;
};

#endif  // !_EVENTCENTER_H_
