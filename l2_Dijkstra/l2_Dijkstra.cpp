#include <iostream>
#include <vector>
#include <queue>
#include <utility>
#include <algorithm>

using namespace std;
using ll = long long;
const ll INF = (1LL<<60);

// Dijkstra with path reconstruction on a weighted directed/undirected graph
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    // Input format:
    // n m
    // next m lines: u v w   (1-based node indices; for undirected link add both directions)
    // source
    if (!(cin >> n >> m)) {
        cerr << "Expected: n m\n";
        return 0;
    }

    vector<vector<pair<int,ll>>> adj(n+1);
    for (int i = 0; i < m; ++i) {
        int u, v;
        ll w;
        cin >> u >> v >> w;
        if (u < 1 || u > n || v < 1 || v > n) {
            cerr << "Node index out of range\n";
            return 0;
        }
        // add directed edge u -> v with weight w
        adj[u].push_back({v, w});
        // If links are bidirectional (common in routing), also add v->u:
        adj[v].push_back({u, w});
    }

    int src;
    cin >> src;
    if (src < 1 || src > n) {
        cerr << "Source out of range\n";
        return 0;
    }

    // Dijkstra
    vector<ll> dist(n+1, INF);
    vector<int> parent(n+1, -1);
    dist[src] = 0;

    // min-heap of (dist, node)
    using pli = pair<ll,int>;
    priority_queue<pli, vector<pli>, greater<pli>> pq;
    pq.push({0, src});

    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (d != dist[u]) continue; // stale entry
        for (auto &e : adj[u]) {
            int v = e.first;
            ll w = e.second;
            if (dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                parent[v] = u;
                pq.push({dist[v], v});
            }
        }
    }

    // Output distances and paths
    cout << "Node\tDistance\tPath\n";
    for (int v = 1; v <= n; ++v) {
        cout << v << "\t";
        if (dist[v] == INF) {
            cout << "INF\t\t-\n";
            continue;
        }
        cout << dist[v] << "\t\t";
        // reconstruct path from src to v
        vector<int> path;
        int cur = v;
        while (cur != -1) { path.push_back(cur); cur = parent[cur]; }
        reverse(path.begin(), path.end());
        // If first node is not src then unreachable (shouldn't happen if dist finite)
        for (size_t i = 0; i < path.size(); ++i) {
            if (i) cout << "->";
            cout << path[i];
        }
        cout << "\n";
    }

    return 0;
}

//for routing table, consider each node as source in turn and store next hop for each destination