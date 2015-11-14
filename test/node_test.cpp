#define BOOST_TEST_MODULE NodeTest
#include <boost/test/unit_test.hpp>

#include "Node.h"

#include <vector>
#include <unordered_set>

BOOST_AUTO_TEST_CASE(node_ctor_test) {
    const Node::id_t id = 42;
    Node n1{id, 5};
    BOOST_CHECK_EQUAL(n1.num_wavelengths(), 5);
    BOOST_CHECK_EQUAL(n1.has_converter(), false);

    Node n2{id, 3, true};
    BOOST_CHECK_EQUAL(n2.has_converter(), true);

    Node n3{id, std::vector<Node::wavelength_t>{4, 8, 1, 10}};
    std::unordered_set<Node::wavelength_t> correct = {1, 4, 8, 10};
    BOOST_CHECK_EQUAL(n3.num_wavelengths(), 4);
    bool ok = n3.wavelengths() == correct;
    BOOST_CHECK(ok);

    Node n4{n3};
    BOOST_CHECK_EQUAL(n4.num_wavelengths(), 4);
}

BOOST_AUTO_TEST_CASE(node_can_use_test) {
    Node n{42, std::vector<Node::wavelength_t>{1}};

    BOOST_CHECK_EQUAL(n.can_use(1), true);
    BOOST_CHECK_EQUAL(n.can_use(2), false);
}

BOOST_AUTO_TEST_CASE(node_available_wavelengths_test) {
    Node n{42, std::vector<Node::wavelength_t>{1, 2, 3}};

    std::unordered_set<Node::wavelength_t> correct = {1, 2, 3};
    bool ok = n.available_wavelengths() == correct;
    BOOST_CHECK(ok);
}

BOOST_AUTO_TEST_CASE(node_used_wavelengths_test) {
    Node n{42, std::vector<Node::wavelength_t>{1, 2, 3}};

    BOOST_CHECK_EQUAL(n.used_wavelengths().empty(), true);
}

BOOST_AUTO_TEST_CASE(node_lock_test) {
    const Node::id_t id = 42;
    Node n1{id, std::vector<Node::wavelength_t>{5, 1, 8}};

    // Can use any?
    BOOST_CHECK_EQUAL(n1.available_wavelengths().empty(), false);
    // Doesn't exist
    BOOST_CHECK_EQUAL(n1.can_use(4), false);
    // Not used
    BOOST_CHECK_EQUAL(n1.can_use(5), true);

    // Lock it!
    BOOST_CHECK_EQUAL(n1.lock(5), true);

    // Now it's used
    BOOST_CHECK_EQUAL(n1.can_use(5), false);
    // But there's something that we can use
    BOOST_CHECK_EQUAL(n1.available_wavelengths().empty(), false);

    // Already used so can't lock
    BOOST_CHECK_EQUAL(n1.lock(5), false);

    // Keep locking
    while (!n1.available_wavelengths().empty()) {
        auto wl = *n1.available_wavelengths().begin();
        n1.lock(wl);
    }
    BOOST_CHECK_EQUAL(n1.available_wavelengths().empty(), true);
}

BOOST_AUTO_TEST_CASE(node_release_test) {
    const Node::id_t id = 42;
    Node n{id, std::vector<Node::wavelength_t>{5, 1, 8}};

    BOOST_REQUIRE_EQUAL(n.available_wavelengths().size(), 3);
    n.lock(5);
    BOOST_REQUIRE_EQUAL(n.available_wavelengths().size(), 2);

    // Nothing happens
    n.release(1);
    BOOST_CHECK_EQUAL(n.available_wavelengths().size(), 2);

    // Release a wavelength that is locked
    n.release(5);
    BOOST_CHECK_EQUAL(n.available_wavelengths().size(), 3);

    n.lock(1);
    n.lock(5);
    n.lock(8);
    BOOST_REQUIRE_EQUAL(n.available_wavelengths().size(), 0);
    // Release when there are multiple wavelengths in use
    n.release(8);
    std::unordered_set<Node::wavelength_t> correct = {1, 5};
    bool ok = n.used_wavelengths() == correct;
    BOOST_CHECK(ok);
    correct = std::unordered_set<Node::wavelength_t>{8};
    ok = n.available_wavelengths() == correct;
    BOOST_CHECK(ok);
}
