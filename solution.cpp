#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <cstring>

using namespace std;

class MaxFlowSolver {
private:
    struct Edge {
        int to, cap, rev;
    };
    
    vector<vector<Edge>> graph;
    vector<int> level, iter;
    vector<int> parent;
    int n;
    
    void add_edge(int from, int to, int cap) {
        graph[from].push_back({to, cap, (int)graph[to].size()});
        graph[to].push_back({from, 0, (int)graph[from].size() - 1});
    }
    
    bool bfs(int s, int t) {
        level.assign(n, -1);
        queue<int> q;
        level[s] = 0;
        q.push(s);
        
        while (!q.empty()) {
            int v = q.front();
            q.pop();
            for (auto &e : graph[v]) {
                if (e.cap > 0 && level[e.to] < 0) {
                    level[e.to] = level[v] + 1;
                    q.push(e.to);
                }
            }
        }
        return level[t] >= 0;
    }
    
    int dfs(int v, int t, int f) {
        if (v == t) return f;
        for (int &i = iter[v]; i < graph[v].size(); i++) {
            Edge &e = graph[v][i];
            if (e.cap > 0 && level[v] < level[e.to]) {
                int d = dfs(e.to, t, min(f, e.cap));
                if (d > 0) {
                    e.cap -= d;
                    graph[e.to][e.rev].cap += d;
                    return d;
                }
            }
        }
        return 0;
    }
    
    void bfs_min_cut(int s, vector<bool>& in_S) {
        in_S.assign(n, false);
        queue<int> q;
        in_S[s] = true;
        q.push(s);
        
        while (!q.empty()) {
            int v = q.front();
            q.pop();
            for (auto &e : graph[v]) {
                if (e.cap > 0 && !in_S[e.to]) {
                    in_S[e.to] = true;
                    q.push(e.to);
                }
            }
        }
    }
    
public:
    MaxFlowSolver(int n) : n(n) {
        graph.resize(n);
        parent.resize(n);
    }
    
    void build_graph(const vector<pair<int, int>>& edges) {
        for (int i = 0; i < n; i++) {
            graph[i].clear();
        }
        
        for (auto [u, v] : edges) {
            add_edge(u, v, 1);
            add_edge(v, u, 1);
        }
    }
    
    int max_flow(int s, int t) {
        // Reset capacities
        for (int i = 0; i < n; i++) {
            for (auto &e : graph[i]) {
                if (i < e.to) {
                    e.cap = 1;
                    graph[e.to][e.rev].cap = 0;
                }
            }
        }
        
        int flow = 0;
        while (bfs(s, t)) {
            iter.assign(n, 0);
            int f;
            while ((f = dfs(s, t, 1e9)) > 0) {
                flow += f;
            }
        }
        return flow;
    }
    
    int max_flow(int s, int t, const vector<pair<int, int>>& edges) {
        // Clear and rebuild graph
        for (int i = 0; i < n; i++) {
            graph[i].clear();
        }
        
        for (auto [u, v] : edges) {
            add_edge(u, v, 1);
            add_edge(v, u, 1);
        }
        
        int flow = 0;
        while (bfs(s, t)) {
            iter.assign(n, 0);
            int f;
            while ((f = dfs(s, t, 1e9)) > 0) {
                flow += f;
            }
        }
        return flow;
    }
    
    // Build Gomory-Hu tree and return all-pairs max flows
    vector<vector<int>> gomory_hu_tree(const vector<pair<int, int>>& edges) {
        build_graph(edges);
        
        vector<vector<int>> tree_weights(n, vector<int>(n, 0));
        vector<int> parent(n, -1);
        
        for (int s = 1; s < n; s++) {
            int t = parent[s];
            if (t == -1) t = 0;
            
            int flow = max_flow(s, t);
            
            vector<bool> in_S;
            bfs_min_cut(s, in_S);
            
            // Update tree weights
            for (int i = 0; i < n; i++) {
                if (i != s && parent[i] == t && in_S[i]) {
                    parent[i] = s;
                }
            }
            parent[s] = t;
            tree_weights[s][t] = tree_weights[t][s] = flow;
        }
        
        // Compute all-pairs max flows from tree
        vector<vector<int>> max_flows(n, vector<int>(n, 0));
        for (int i = 0; i < n; i++) {
            for (int j = i + 1; j < n; j++) {
                // Find minimum edge weight on path from i to j in tree
                int min_weight = 1e9;
                int curr = i;
                while (curr != j) {
                    if (curr < j) {
                        min_weight = min(min_weight, tree_weights[curr][j]);
                        curr = j;
                    } else {
                        min_weight = min(min_weight, tree_weights[curr][parent[curr]]);
                        curr = parent[curr];
                    }
                }
                max_flows[i][j] = max_flows[j][i] = min_weight;
            }
        }
        
        return max_flows;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    
    int n, m;
    cin >> n >> m;
    
    vector<pair<int, int>> edges;
    vector<vector<int>> adj(n);
    
    for (int i = 0; i < m; i++) {
        int a, b;
        cin >> a >> b;
        a--; b--;
        edges.emplace_back(a, b);
        adj[a].push_back(b);
        adj[b].push_back(a);
    }
    
    MaxFlowSolver solver(n);
    
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
        
        // Map original nodes to component nodes
        vector<int> node_map(n, -1);
        vector<pair<int, int>> comp_edges;
        
        for (int i = 0; i < comp.size(); i++) {
            node_map[comp[i]] = i;
        }
        
        for (auto [u, v] : edges) {
            if (node_map[u] != -1 && node_map[v] != -1) {
                comp_edges.emplace_back(node_map[u], node_map[v]);
            }
        }
        
        // If component is small, use direct approach
        if (comp.size() <= 50) {
            MaxFlowSolver comp_solver(comp.size());
            for (int i = 0; i < comp.size(); i++) {
                for (int j = i + 1; j < comp.size(); j++) {
                    int flow = comp_solver.max_flow(i, j, comp_edges);
                    total += flow;
                }
            }
        } else {
            // Use Gomory-Hu tree for larger components
            MaxFlowSolver comp_solver(comp.size());
            auto max_flows = comp_solver.gomory_hu_tree(comp_edges);
            
            for (int i = 0; i < comp.size(); i++) {
                for (int j = i + 1; j < comp.size(); j++) {
                    total += max_flows[i][j];
                }
            }
        }
    }
    
    cout << total << endl;
    return 0;
}