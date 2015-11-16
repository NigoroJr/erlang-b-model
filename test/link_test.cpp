#define BOOST_TEST_MODULE LinkTest
#include <boost/test/unit_test.hpp>

#include "Link.h"

#include <vector>
#include <unordered_set>

BOOST_AUTO_TEST_CASE(node_ctor_test) {
    Link l1{5};
    BOOST_CHECK_EQUAL(l1.num_wavelengths(), 5);
    BOOST_CHECK_EQUAL(l1.has_converter(), false);

    Link l2{3, true};
    BOOST_CHECK_EQUAL(l2.has_converter(), true);

    Link l3{std::vector<Link::wavelength_t>{4, 8, 1, 10}};
    std::unordered_set<Link::wavelength_t> correct = {1, 4, 8, 10};
    BOOST_CHECK_EQUAL(l3.num_wavelengths(), 4);
    bool ok = l3.wavelengths() == correct;
    BOOST_CHECK(ok);

    Link l4{l3};
    BOOST_CHECK_EQUAL(l4.num_wavelengths(), 4);
}

BOOST_AUTO_TEST_CASE(node_can_use_test) {
    Link link{std::vector<Link::wavelength_t>{1}};

    BOOST_CHECK_EQUAL(link.can_use(1), true);
    BOOST_CHECK_EQUAL(link.can_use(2), false);
}

BOOST_AUTO_TEST_CASE(node_available_wavelengths_test) {
    Link link{std::vector<Link::wavelength_t>{1, 2, 3}};

    std::unordered_set<Link::wavelength_t> correct = {1, 2, 3};
    bool ok = link.available_wavelengths() == correct;
    BOOST_CHECK(ok);
}

BOOST_AUTO_TEST_CASE(node_used_wavelengths_test) {
    Link link{std::vector<Link::wavelength_t>{1, 2, 3}};

    BOOST_CHECK_EQUAL(link.used_wavelengths().empty(), true);
}

BOOST_AUTO_TEST_CASE(node_lock_test) {
    Link l1{std::vector<Link::wavelength_t>{5, 1, 8}};

    // Can use any?
    BOOST_CHECK_EQUAL(l1.available_wavelengths().empty(), false);
    // Doesn't exist
    BOOST_CHECK_EQUAL(l1.can_use(4), false);
    // Not used
    BOOST_CHECK_EQUAL(l1.can_use(5), true);

    // Lock it!
    BOOST_CHECK_EQUAL(l1.lock(5), true);

    // Now it's used
    BOOST_CHECK_EQUAL(l1.can_use(5), false);
    // But there's something that we can use
    BOOST_CHECK_EQUAL(l1.available_wavelengths().empty(), false);

    // Already used so can't lock
    BOOST_CHECK_EQUAL(l1.lock(5), false);

    // Keep locking
    while (!l1.available_wavelengths().empty()) {
        auto wl = *l1.available_wavelengths().begin();
        l1.lock(wl);
    }
    BOOST_CHECK_EQUAL(l1.available_wavelengths().empty(), true);
}

BOOST_AUTO_TEST_CASE(node_release_test) {
    Link link{std::vector<Link::wavelength_t>{5, 1, 8}};

    BOOST_REQUIRE_EQUAL(link.available_wavelengths().size(), 3);
    link.lock(5);
    BOOST_REQUIRE_EQUAL(link.available_wavelengths().size(), 2);

    // Nothing happens
    link.release(1);
    BOOST_CHECK_EQUAL(link.available_wavelengths().size(), 2);

    // Release a wavelength that is locked
    link.release(5);
    BOOST_CHECK_EQUAL(link.available_wavelengths().size(), 3);

    link.lock(1);
    link.lock(5);
    link.lock(8);
    BOOST_REQUIRE_EQUAL(link.available_wavelengths().size(), 0);
    // Release when there are multiple wavelengths in use
    link.release(8);
    std::unordered_set<Link::wavelength_t> correct = {1, 5};
    bool ok = link.used_wavelengths() == correct;
    BOOST_CHECK(ok);
    correct = std::unordered_set<Link::wavelength_t>{8};
    ok = link.available_wavelengths() == correct;
    BOOST_CHECK(ok);
}
