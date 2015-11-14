#include "Node.h"

using wavelength_t = Node::wavelength_t;
using Wavelengths = Node::Wavelengths;

const Node::wavelength_t Node::NONE = 0;

/* Constructors, Destructor, and Assignment operators {{{ */
// Default constructor
Node::Node()
    : id_{0}
{ }

Node::Node(const id_t id, const unsigned num_links, bool has_converter)
    : id_{id}
    , has_converter_{has_converter}
{
    for (unsigned i = 1; i <= num_links; i++) {
        wavelengths_.insert(static_cast<wavelength_t>(i));
    }
}

Node::Node(const id_t id, const int num_links, bool has_converter)
    : Node(id, static_cast<unsigned>(num_links), has_converter)
{ }

// Copy constructor
Node::Node(const Node& other)
    : id_{other.id_}
    , wavelengths_{other.wavelengths_}
    , used_{other.used_}
    , has_converter_{other.has_converter_}
{ }

// Move constructor
Node::Node(Node&& other)
    : id_{std::move(other.id_)}
    , wavelengths_{std::move(other.wavelengths_)}
    , used_{std::move(other.used_)}
    , has_converter_{std::move(other.has_converter_)}
{ }

// Destructor
Node::~Node()
{ }

// Assignment operator
Node&
Node::operator=(const Node& other) {
    id_ = other.id_;
    wavelengths_ = other.wavelengths_;
    used_ = other.used_;
    has_converter_ = other.has_converter_;
    return *this;
}

// Move assignment operator
Node&
Node::operator=(Node&& other) {
    id_ = std::move(other.id_);
    wavelengths_ = std::move(other.wavelengths_);
    used_ = std::move(other.used_);
    has_converter_ = std::move(other.has_converter_);
    return *this;
}
/* }}} */

bool
Node::can_use(const wavelength_t wl) const {
    // A wavelength that doesn't exist
    if (wavelengths_.count(wl) == 0) {
        return false;
    }

    if (has_converter_ && used_.size() < wavelengths_.size()) {
        return true;
    }

    return used_.count(wl) == 0;
}

bool
Node::lock(const wavelength_t wl) {
    if (!can_use(wl)) {
        return false;
    }

    used_.insert(wl);
    return true;
}

void
Node::release(const wavelength_t wl) {
    used_.erase(wl);
}

Wavelengths
Node::available_wavelengths() const {
    std::unordered_set<wavelength_t> available;

    for (const wavelength_t wl : wavelengths_) {
        if (used_.count(wl) == 0) {
            available.insert(wl);
        }
    }

    return available;
}
