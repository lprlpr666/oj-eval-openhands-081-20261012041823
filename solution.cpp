#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <cstring>

using namespace std;

// Optimized max flow using multiple BFS from each source
int compute_max_flow(int s, int t, const vector<vector<int>>& adj, int n) {
    if (s == t) return 0;
    
    // Quick check: if not connected, flow is 0
    vector<int> dist(n, -1);
    queue<int> q;
    q.push(s);
    dist[s] = 0;
    
    while (!q.empty()) {
        int v = q.front();
        q.pop();
        if (v == t) break;
        for (int u : adj[v]) {
            if (dist[u] == -1) {
                dist[u] = dist[v] + 1;
                q.push(u);
            }
        }
    }
    
    if (dist[t] == -1) return 0; // Not connected
    
    // For small components, use simple approach
    if (n <= 100) {
        int flow = 0;
        vector<bool> edge_used(n * 4, false);
        
        for (int attempt = 0; attempt < 3; attempt++) {
            vector<int> parent(n, -1);
            vector<int> edge_idx(n, -1);
            queue<int> q;
            q.push(s);
            parent[s] = s;
            
            bool found = false;
            while (!q.empty() && !found) {
                int v = q.front();
                q.pop();
                
                for (int i = 0; i < adj[v].size() && !found; i++) {
                    int u = adj[v][i];
                    int edge_id = v * 4 + i;
                    
                    if (!edge_used[edge_id] && parent[u] == -1) {
                        parent[u] = v;
                        edge_idx[u] = edge_id;
                        if (u == t) {
                            found = true;
                            break;
                        }
                        q.push(u);
                    }
                }
            }
            
            if (!found) break;
            
            int curr = t;
            while (curr != s) {
                int edge_id = edge_idx[curr];
                edge_used[edge_id] = true;
                curr = parent[curr];
            }
            
            flow++;
        }
        
        return flow;
    }
    
    // For larger components, use degree-based heuristics
    // Since degree ≤ 3, max flow is limited by min(deg(s), deg(t))
    int max_possible = min(3, min((int)adj[s].size(), (int)adj[t].size()));
    
    // Try to find edge-disjoint paths
    vector<vector<bool>> edge_used(n, vector<bool>(adj.size(), false));
    int flow = 0;
    
    for (int attempt = 0; attempt < max_possible; attempt++) {
        vector<int> parent(n, -1);
        queue<int> q;
        q.push(s);
        parent[s] = s;
        
        bool found = false;
        while (!q.empty() && !found) {
            int v = q.front();
            q.pop();
            
            for (int i = 0; i < adj[v].size() && !found; i++) {
                int u = adj[v][i];
                if (!edge_used[v][i] && parent[u] == -1) {
                    parent[u] = v;
                    if (u == t) {
                        found = true;
                        break;
                    }
                    q.push(u);
                }
            }
        }
        
        if (!found) break;
        
        // Mark edges as used
        int curr = t;
        while (curr != s) {
            int prev = parent[curr];
            for (int i = 0; i < adj[prev].size(); i++) {
                if (adj[prev][i] == curr) {
                    edge_used[prev][i] = true;
                    break;
                }
            }
            curr = prev;
        }
        
        flow++;
    }
    
    return flow;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    
    int n, m;
    cin >> n >> m;
    
    vector<vector<int>> adj(n);
    
    for (int i = 0; i < m; i++) {
        int a, b;
        cin >> a >> b;
        a--; b--;
        adj[a].push_back(b);
        adj[b].push_back(a);
    }
    
    // Find connected components
    vector<int> component(n, -1);
    vector<vector<int>> components;
    int comp_count = 0;
    
    for (int i = 0; i < n; i++) {
        if (component[i] == -1) {
            vector<int> comp;
            queue<int> q;
            q.push(i);
            component[i] = comp_count;
            comp.push_back(i);
            
            while (!q.empty()) {
                int v = q.front();
                q.pop();
                for (int u : adj[v]) {
                    if (component[u] == -1) {
                        component[u] = comp_count;
                        comp.push_back(u);
                        q.push(u);
                    }
                }
            }
            components.push_back(comp);
            comp_count++;
        }
    }
    
    long long total = 0;
    
    // Process each connected component separately
    for (auto& comp : components) {
        if (comp.size() <= 1) continue;
        
        // Build component adjacency
        vector<vector<int>> comp_adj(comp.size());
        vector<int> node_map(n, -1);
        
        for (int i = 0; i < comp.size(); i++) {
            node_map[comp[i]] = i;
        }
        
        for (int original_node : comp) {
            int new_node = node_map[original_node];
            for (int neighbor : adj[original_node]) {
                if (node_map[neighbor] != -1) {
                    comp_adj[new_node].push_back(node_map[neighbor]);
                }
            }
        }
        
        // For each pair in component, compute max flow
        for (int i = 0; i < comp.size(); i++) {
            for (int j = i + 1; j < comp.size(); j++) {
                int flow = compute_max_flow(i, j, comp_adj, comp.size());
                total += flow;
            }
        }
    }
    
    cout << total << endl;
    return 0;
}