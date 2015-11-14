#include "Event.h"

/* Constructors, Destructor, and Assignment operators {{{ */
// Default constructor
Event::Event()
    : src{0}
    , dst{0}
    , type{Type::DUMMY}
    , time{UNSET}
    , wavelength{WL_UNSET}
{ }

Event::Event(Type type, event_t time)
    : src{0}
    , dst{0}
    , type{type}
    , time{time}
    , wavelength{WL_UNSET}
{ }

Event::Event(const std::pair<Advisor::vertex_t, Advisor::vertex_t> src_dst,
             Type type,
             event_t time)
    : src{src_dst.first}
    , dst{src_dst.second}
    , type{type}
    , time{time}
    , wavelength{WL_UNSET}
{ }

Event::Event(const Advisor::vertex_t src,
             const Advisor::vertex_t dst,
             Type type,
             event_t time,
             std::vector<Advisor::vertex_t> path,
             Node::wavelength_t wl)
    : src{src}
    , dst{dst}
    , type{type}
    , time{time}
    , path{path}
    , wavelength{wl}
{ }

// Copy constructor
Event::Event(const Event& other)
    : src{other.src}
    , dst{other.dst}
    , type{other.type}
    , time{other.time}
    , path{other.path}
    , wavelength{other.wavelength}
{ }

// Move constructor
Event::Event(Event&& other)
    : src{std::move(other.src)}
    , dst{std::move(other.dst)}
    , type{std::move(other.type)}
    , time{std::move(other.time)}
    , path{std::move(other.path)}
    , wavelength{std::move(other.wavelength)}
{ }

// Destructor
Event::~Event()
{ }

// Assignment operator
Event&
Event::operator=(const Event& other) {
    src = other.src;
    dst = other.dst;
    type = other.type;
    time = other.time;
    path = other.path;
    wavelength = other.wavelength;
    return *this;
}

// Move assignment operator
Event&
Event::operator=(Event&& other) {
    src = std::move(other.src);
    dst = std::move(other.dst);
    type = std::move(other.type);
    time = std::move(other.time);
    path = std::move(other.path);
    wavelength = std::move(other.wavelength);
    return *this;
}
/* }}} */

bool
Event::operator==(const Event& other) const {
    return time == other.time && type == other.type;
}

bool
Event::operator!=(const Event& other) const {
    return time != other.time || type != other.type;
}

bool
Event::operator>(const Event& other) const {
    return time > other.time;
}

bool
Event::operator<(const Event& other) const {
    return time < other.time;
}
