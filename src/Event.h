#ifndef EVENT_H_
#define EVENT_H_

#include "Advisor.h"
#include "Node.h"

#include <utility>

struct Event {
    using event_t = double;

    static constexpr event_t UNSET = -1;
    static const Node::wavelength_t WL_UNSET = 0;

    enum Type { START, END, BLOCK, DUMMY };

    /* Constructors, Destructor, and Assignment operators {{{ */
    // Default constructor
    Event();

    Event(Type type, event_t time);

    Event(const std::pair<Advisor::vertex_t, Advisor::vertex_t> src_dst,
          Type type,
          event_t time);

    Event(const Advisor::vertex_t src,
          const Advisor::vertex_t dst,
          Type type,
          event_t time,
          std::vector<Advisor::vertex_t> path,
          Node::wavelength_t wl = WL_UNSET);

    // Copy constructor
    Event(const Event& other);

    // Move constructor
    Event(Event&& other);

    // Destructor
    ~Event();

    // Assignment operator
    Event&
    operator=(const Event& other);

    // Move assignment operator
    Event&
    operator=(Event&& other);
    /* }}} */

    bool
    operator==(const Event& other) const;
    bool
    operator!=(const Event& other) const;
    bool
    operator>(const Event& other) const;
    bool
    operator<(const Event& other) const;

    /* Source and destination nodes */
    Advisor::vertex_t src;
    Advisor::vertex_t dst;
    Type type;
    event_t time;
    std::vector<Advisor::vertex_t> path;
    Node::wavelength_t wavelength;
};

#endif /* end of include guard */
