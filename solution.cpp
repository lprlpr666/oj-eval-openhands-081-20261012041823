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
    
public:
    MaxFlowSolver(int n) : n(n) {
        graph.resize(n);
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
    
    long long total = 0;
    
    // For each pair (a, b) where a < b, compute max flow
    // Since degree ≤ 3, max flow between any two nodes is at most 3
    for (int a = 0; a < n; a++) {
        for (int b = a + 1; b < n; b++) {
            int flow = solver.max_flow(a, b, edges);
            total += flow;
        }
    }
    
    cout << total << endl;
    return 0;
}