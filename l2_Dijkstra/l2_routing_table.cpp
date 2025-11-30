#include <iostream>
#include <vector>
#include <queue>
#include <utility>
#include <algorithm>
#include <string>
#include <iomanip>

using namespace std;
using ll = long long;
const ll INF = (1LL<<60);

struct Edge { int to; ll w; };

pair<vector<ll>, vector<int>> dijkstra(int src, const vector<vector<Edge>>& adj) {
    int n = (int)adj.size() - 1;
    vector<ll> dist(n+1, INF);
    vector<int> parent(n+1, -1);
    dist[src] = 0;
    using pli = pair<ll,int>;
    priority_queue<pli, vector<pli>, greater<pli>> pq;
    pq.push({0, src});
    while (!pq.empty()) {
        auto [d,u] = pq.top(); pq.pop();
        if (d != dist[u]) continue;
        for (auto &e : adj[u]) {
            int v = e.to;
            ll nd = d + e.w;
            if (nd < dist[v]) {
                dist[v] = nd;
                parent[v] = u;
                pq.push({nd, v});
            }
        }
    }
    return {dist, parent};
}

vector<int> reconstruct_path(const vector<int>& parent, int src, int dst) {
    if (dst == src) return {src};
    if (parent[dst] == -1) return {}; // unreachable
    vector<int> path;
    int cur = dst;
    while (cur != -1) {
        path.push_back(cur);
        if (cur == src) break;
        cur = parent[cur];
    }
    reverse(path.begin(), path.end());
    if (path.empty() || path.front() != src) return {};
    return path;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    // Input:
    // n m
    // m lines: u v w   (undirected link)
    if (!(cin >> n >> m)) {
        cerr << "Expected: n m\n";
        return 0;
    }

    vector<vector<Edge>> adj(n+1);
    for (int i = 0; i < m; ++i) {
        int u, v;
        ll w;
        cin >> u >> v >> w;
        if (u < 1 || u > n || v < 1 || v > n) {
            cerr << "Node index out of range\n";
            return 0;
        }
        // OSPF-style: treat as bi-directional
        adj[u].push_back({v, w});
        adj[v].push_back({u, w});
    }

    // For each router as source, compute routing table
    for (int src = 1; src <= n; ++src) {
        auto [dist, parent] = dijkstra(src, adj);

        cout << "===============================================\n";
        cout << "Routing table for Router " << src << " (OSPF-style)\n";
        cout << "===============================================\n";
        cout << left << setw(12) << "Destination"
                     << setw(10) << "NextHop"
                     << setw(10) << "Cost"
                     << "Path\n";
        cout << "-----------------------------------------------\n";

        for (int dst = 1; dst <= n; ++dst) {
            // Reconstruct path and find next hop
            vector<int> path = reconstruct_path(parent, src, dst);
            string nextHop;
            string pathStr;
            if (path.empty()) {
                nextHop = "-";
                pathStr = "-";
            } else {
                if (dst == src) nextHop = "-";
                else nextHop = to_string(path[1]); // first hop after src
                // join path
                for (size_t i = 0; i < path.size(); ++i) {
                    if (i) pathStr += "->";
                    pathStr += to_string(path[i]);
                }
            }

            // Cost representation
            string costStr;
            if (dist[dst] >= INF/2) costStr = "INF";
            else costStr = to_string(dist[dst]);

            cout << left << setw(12) << dst
                         << setw(10) << nextHop
                         << setw(10) << costStr
                         << pathStr << "\n";
        }
        cout << "\n";
    }

    return 0;
}
