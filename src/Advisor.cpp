#include "Advisor.h"

using Graph = Advisor::Graph;
using vertex_t = Advisor::vertex_t;
using edge_t = Advisor::edge_t;

/* Constructors, Destructor, and Assignment operators {{{ */
// Default constructor
Advisor::Advisor()
{ }

Advisor::Advisor(const Graph& nodes,
        const Advisor::event_t lambda,
        const Advisor::event_t duration_mean)
    : lambda{lambda}
    , duration_mean{duration_mean}
    , nodes{nodes}
    , u_dist{0, static_cast<Link::id_t>(boost::num_vertices(nodes) - 1)}
    , arrival_dist{lambda}
    , duration_dist{duration_mean}
{ }

// Copy constructor
Advisor::Advisor(const Advisor& other)
    : lambda{other.lambda}
    , duration_mean{other.duration_mean}
    , nodes{other.nodes}
    , u_dist{other.u_dist}
    , arrival_dist{other.arrival_dist}
    , duration_dist{other.duration_dist}
{ }

// Move constructor
Advisor::Advisor(Advisor&& other)
    : lambda{std::move(other.lambda)}
    , duration_mean{std::move(other.duration_mean)}
    , nodes{std::move(other.nodes)}
    , rgen{std::move(other.rgen)}
    , u_dist{std::move(other.u_dist)}
    , arrival_dist{std::move(other.arrival_dist)}
    , duration_dist{std::move(other.duration_dist)}
{ }

// Destructor
Advisor::~Advisor()
{ }

// Assignment operator
Advisor&
Advisor::operator=(const Advisor& other) {
    lambda = other.lambda;
    duration_mean = other.duration_mean;
    nodes = other.nodes;
    u_dist = other.u_dist;
    arrival_dist = other.arrival_dist;
    duration_dist = other.duration_dist;
    return *this;
}

// Move assignment operator
Advisor&
Advisor::operator=(Advisor&& other) {
    lambda = std::move(other.lambda);
    duration_mean = std::move(other.duration_mean);
    nodes = std::move(other.nodes);
    rgen = std::move(other.rgen);
    u_dist = std::move(other.u_dist);
    arrival_dist = std::move(other.arrival_dist);
    duration_dist = std::move(other.duration_dist);
    return *this;
}
/* }}} */

std::pair<Link::id_t, Link::id_t>
Advisor::get_nodes() {
    const Link::id_t a = u_dist(rgen);
    Link::id_t b = u_dist(rgen);
    while (a == b) {
        b = u_dist(rgen);
    }
    return std::make_pair(a, b);
}

Advisor::event_t
Advisor::get_arrival() {
    return arrival_dist(rgen);
}

Advisor::event_t
Advisor::get_duration() {
    return duration_dist(rgen);
}

std::pair<std::vector<vertex_t>, Link::wavelength_t>
Advisor::path_between(Link::id_t a, Link::id_t b) {
    const unsigned num_nodes = boost::num_vertices(nodes);
    if (a >= num_nodes || b >= num_nodes || num_nodes < 2) {
        return std::make_pair(std::vector<vertex_t>(), Link::NONE);
    }

    using FilteredGraph = boost::filtered_graph<
        Graph,
        EdgeFilter<Graph>,
        boost::keep_all
    >;

    // Search for all available wavelengths that a has
    for (auto&& wl : nodes[a].available_wavelengths()) {
        bool has_path = true;

        std::vector<vertex_t> predecessors;
        predecessors.resize(boost::num_vertices(nodes));
        // Initialize predecessors to the node itself
        boost::graph_traits<Graph>::vertex_iterator v_b, v_e;
        std::tie(v_b, v_e) = boost::vertices(nodes);
        for (auto it = v_b; it != v_e; it++) {
            predecessors[*it] = *it;
        }

        // Make a filtered graph without unavailable nodes
        EdgeFilter<Graph> edge_filter{nodes, wl};
        FilteredGraph fg{nodes, edge_filter, boost::keep_all{}};

        // Do a BFS on the filtered graph, recording parents
        auto vis = boost::visitor(
            boost::make_bfs_visitor(
                boost::record_predecessors(
                    &predecessors[0],
                    boost::on_tree_edge{})
                )
            );
        boost::breadth_first_search(fg, a, vis);

        // No path
        if (predecessors[b] == b) {
            continue;
        }

        std::vector<vertex_t> path;
        vertex_t p = b;
        do {
            path.insert(path.begin(), p);
            p = predecessors[p];
        } while (p != a);
        // Add the source
        path.insert(path.begin(), p);

        for (const vertex_t v : path) {
            if (!nodes[v].can_use(wl)) {
                has_path = false;
                break;
            }
        }

        if (has_path) {
            return std::make_pair(path, wl);
        }
    }

    return std::make_pair(std::vector<vertex_t>(), Link::NONE);
}

bool
Advisor::has_path_between(Link::id_t a, Link::id_t b) {
    auto wl = path_between(a, b).second;
    return wl != Link::NONE;
}

std::pair<std::vector<vertex_t>, Link::wavelength_t>
Advisor::make_connection(Link::id_t a, Link::id_t b) {
    std::vector<vertex_t> path;
    Link::wavelength_t wl;
    std::tie(path, wl) = path_between(a, b);

    // Link::NONE on failure
    if (wl == Link::NONE) {
        return std::make_pair(std::vector<vertex_t>(), Link::NONE);
    }

    for (const vertex_t& vertex : path) {
        nodes[vertex].lock(wl);
    }
    return std::make_pair(path, wl);
}

void
Advisor::remove_connection(const std::vector<vertex_t>& path,
                           const Link::wavelength_t wl) {
    for (const vertex_t& vertex : path) {
        nodes[vertex].release(wl);
    }
}
