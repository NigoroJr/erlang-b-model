#include "Link.h"

using wavelength_t = Link::wavelength_t;
using Wavelengths = Link::Wavelengths;

const Link::wavelength_t Link::NONE = 0;

/* Constructors, Destructor, and Assignment operators {{{ */
// Default constructor
Link::Link()
{ }

Link::Link(const unsigned num_links, bool has_converter)
    : has_converter_{has_converter}
{
    for (unsigned i = 1; i <= num_links; i++) {
        wavelengths_.insert(static_cast<wavelength_t>(i));
    }
}

Link::Link(const int num_links, bool has_converter)
    : Link(static_cast<unsigned>(num_links), has_converter)
{ }

// Copy constructor
Link::Link(const Link& other)
    : wavelengths_{other.wavelengths_}
    , used_{other.used_}
    , has_converter_{other.has_converter_}
{ }

// Move constructor
Link::Link(Link&& other)
    : wavelengths_{std::move(other.wavelengths_)}
    , used_{std::move(other.used_)}
    , has_converter_{std::move(other.has_converter_)}
{ }

// Destructor
Link::~Link()
{ }

// Assignment operator
Link&
Link::operator=(const Link& other) {
    wavelengths_ = other.wavelengths_;
    used_ = other.used_;
    has_converter_ = other.has_converter_;
    return *this;
}

// Move assignment operator
Link&
Link::operator=(Link&& other) {
    wavelengths_ = std::move(other.wavelengths_);
    used_ = std::move(other.used_);
    has_converter_ = std::move(other.has_converter_);
    return *this;
}
/* }}} */

bool
Link::can_use(const wavelength_t wl) const {
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
Link::lock(const wavelength_t wl) {
    if (!can_use(wl)) {
        return false;
    }

    used_.insert(wl);
    return true;
}

void
Link::release(const wavelength_t wl) {
    used_.erase(wl);
}

Wavelengths
Link::available_wavelengths() const {
    std::unordered_set<wavelength_t> available;

    for (const wavelength_t wl : wavelengths_) {
        if (used_.count(wl) == 0) {
            available.insert(wl);
        }
    }

    return available;
}
