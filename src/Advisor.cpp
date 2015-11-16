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
    , u_dist{0, static_cast<vertex_t>(boost::num_vertices(nodes) - 1)}
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

std::pair<vertex_t, vertex_t>
Advisor::get_nodes() {
    const vertex_t a = u_dist(rgen);
    vertex_t b = u_dist(rgen);
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

std::pair<std::vector<edge_t>, Link::wavelength_t>
Advisor::path_between(vertex_t a, vertex_t b) {
    using FilteredGraph = boost::filtered_graph<
        Graph,
        EdgeFilter<Graph>,
        boost::keep_all
    >;

    // Search for all available wavelengths that a has
    std::unordered_set<Link::wavelength_t> possible_wavelengths;
    boost::graph_traits<Graph>::out_edge_iterator e_b, e_e;
    // Look at all edges connected to a
    std::tie(e_b, e_e) = boost::out_edges(a, nodes);
    for (auto it = e_b; it != e_e; it++) {
        for (const Link::wavelength_t wl : nodes[*it].available_wavelengths()) {
            possible_wavelengths.insert(wl);
        }
    }

    for (const Link::wavelength_t wl : possible_wavelengths) {
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

        // Make path of edges (rather than vertices)
        std::vector<edge_t> path_edges;
        for (unsigned i = 0; i < path.size() - 1; i++) {
            vertex_t src = path[i];
            vertex_t tgt = path[i + 1];

            edge_t edge;
            // Ignoring whether it was found or not since the vertices are
            // based on results from BFS. The edge better exist...
            std::tie(edge, std::ignore) = boost::edge(src, tgt, nodes);
            path_edges.push_back(edge);
            const Link& link = nodes[edge];

            if (!link.can_use(wl)) {
                has_path = false;
                break;
            }
        }

        if (has_path) {
            return std::make_pair(path_edges, wl);
        }
    }

    return std::make_pair(std::vector<edge_t>(), Link::NONE);
}

bool
Advisor::has_path_between(vertex_t a, vertex_t b) {
    auto wl = path_between(a, b).second;
    return wl != Link::NONE;
}

std::pair<std::vector<edge_t>, Link::wavelength_t>
Advisor::make_connection(vertex_t a, vertex_t b) {
    std::vector<edge_t> path;
    Link::wavelength_t wl;
    std::tie(path, wl) = path_between(a, b);

    // Link::NONE on failure
    if (wl == Link::NONE) {
        return std::make_pair(std::vector<edge_t>(), Link::NONE);
    }

    for (const edge_t& edge : path) {
        nodes[edge].lock(wl);
    }
    return std::make_pair(path, wl);
}

void
Advisor::remove_connection(const std::vector<edge_t>& path,
                           const Link::wavelength_t wl) {
    for (const edge_t& edge : path) {
        nodes[edge].release(wl);
    }
}
