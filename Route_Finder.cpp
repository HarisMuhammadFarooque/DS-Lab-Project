#include <iostream>
#include <fstream>
#include <string>
#include <cctype>
#include <cstdlib>
using namespace std;

const int MAXC    = 100;
const int MAXE    = 2000;
const int INF     = 1000000000;
const int MAXSORT = 5000;

class Graph;
int readCity(const Graph& g, const string& prompt);

string trimStr(const string& s) {
    size_t a = 0, b = s.size();
    while (a < b && isspace((unsigned char)s[a])) a++;
    while (b > a && isspace((unsigned char)s[b - 1])) b--;
    return s.substr(a, b - a);
}

string toLowerStr(string s) {
    for (size_t i = 0; i < s.size(); i++) s[i] = (char)tolower((unsigned char)s[i]);
    return s;
}

string readLine(const string& prompt) {
    cout << prompt;
    string s;
    if (!getline(cin, s)) {
        cout << "\n[Input ended - exiting]\n";
        exit(0);
    }
    return trimStr(s);
}

bool isNumber(const string& s) {
    if (s.empty() || s.size() > 9) return false;
    for (size_t i = 0; i < s.size(); i++)
        if (!isdigit((unsigned char)s[i])) return false;
    return true;
}

string numStr(long long v) {
    if (v == 0) return "0";
    bool neg = v < 0;
    if (neg) v = -v;
    string r = "";
    while (v > 0) { r = (char)('0' + (int)(v % 10)) + r; v /= 10; }
    return neg ? "-" + r : r;
}
string padR(const string& s, int w) {
    string r = s;
    while ((int)r.size() < w) r += ' ';
    return r;
}
string padL(const string& s, int w) {
    string r = s;
    while ((int)r.size() < w) r = " " + r;
    return r;
}

int readInt(const string& prompt, int lo, int hi) {
    while (true) {
        string s = readLine(prompt);
        if (isNumber(s)) {
            int v = atoi(s.c_str());
            if (v >= lo && v <= hi) return v;
        }
        cout << "  Please enter a whole number between " << lo << " and " << hi << ".\n";
    }
}

bool askYesNo(const string& prompt) {
    while (true) {
        string s = toLowerStr(readLine(prompt));
        if (s == "y" || s == "yes") return true;
        if (s == "n" || s == "no") return false;
        cout << "  Please answer y or n.\n";
    }
}

class CircularQueue {
    int data[MAXC + 1];
    int head, tail, cnt;
public:
    CircularQueue() : head(0), tail(0), cnt(0) {}
    bool empty() const { return cnt == 0; }
    bool full() const { return cnt == MAXC + 1; }
    void enqueue(int x) {
        if (full()) return;
        data[tail] = x;
        tail = (tail + 1) % (MAXC + 1);
        cnt++;
    }
    int dequeue() {
        int x = data[head];
        head = (head + 1) % (MAXC + 1);
        cnt--;
        return x;
    }
};

class Stack {
    int data[MAXC + 1];
    int topIdx;
public:
    Stack() : topIdx(-1) {}
    bool empty() const { return topIdx == -1; }
    void push(int x) { if (topIdx < MAXC) data[++topIdx] = x; }
    int pop() { return data[topIdx--]; }
};

// Min-heap with decrease-key; pos[] gives each city's heap index (-1 = not in heap)
class MinHeap {
    int heap[MAXC];
    int pos[MAXC];
    int key[MAXC];
    int sz;

    bool lessThan(int a, int b) const {
        return key[a] < key[b] || (key[a] == key[b] && a < b);
    }
    void swapAt(int i, int j) {
        int t = heap[i]; heap[i] = heap[j]; heap[j] = t;
        pos[heap[i]] = i;
        pos[heap[j]] = j;
    }
    void siftUp(int i) {
        while (i > 0) {
            int p = (i - 1) / 2;
            if (lessThan(heap[i], heap[p])) { swapAt(i, p); i = p; }
            else break;
        }
    }
    void siftDown(int i) {
        while (true) {
            int l = 2 * i + 1, r = 2 * i + 2, m = i;
            if (l < sz && lessThan(heap[l], heap[m])) m = l;
            if (r < sz && lessThan(heap[r], heap[m])) m = r;
            if (m == i) break;
            swapAt(i, m);
            i = m;
        }
    }
public:
    MinHeap() : sz(0) { for (int i = 0; i < MAXC; i++) { pos[i] = -1; key[i] = INF; } }
    bool empty() const { return sz == 0; }
    bool contains(int v) const { return pos[v] != -1; }
    void push(int v, int k) {
        key[v] = k;
        heap[sz] = v;
        pos[v] = sz;
        sz++;
        siftUp(sz - 1);
    }
    void decreaseKey(int v, int k) {
        key[v] = k;
        siftUp(pos[v]);
    }
    int popMin() {
        int v = heap[0];
        sz--;
        if (sz > 0) {
            heap[0] = heap[sz];
            pos[heap[0]] = 0;
            siftDown(0);
        }
        pos[v] = -1;
        return v;
    }
};

struct Edge {
    int to;
    int w;
    bool blocked;
    Edge* next;
};

class Graph {
    string names[MAXC];
    bool active[MAXC];
    Edge* head[MAXC];
    int n;
    int cityCount;

    void appendEdge(int u, int v, int w) {
        Edge* e = new Edge;
        e->to = v; e->w = w; e->blocked = false; e->next = NULL;
        if (!head[u]) head[u] = e;
        else {
            Edge* t = head[u];
            while (t->next) t = t->next;
            t->next = e;
        }
    }
    void removeDirected(int u, int v) {
        Edge* prev = NULL;
        Edge* cur = head[u];
        while (cur) {
            if (cur->to == v) {
                if (prev) prev->next = cur->next; else head[u] = cur->next;
                delete cur;
                return;
            }
            prev = cur;
            cur = cur->next;
        }
    }
public:
    Graph() : n(0), cityCount(0) {
        for (int i = 0; i < MAXC; i++) { head[i] = NULL; active[i] = false; }
    }
    ~Graph() { clear(); }

    void clear() {
        for (int i = 0; i < n; i++) {
            Edge* e = head[i];
            while (e) { Edge* nx = e->next; delete e; e = nx; }
            head[i] = NULL;
            active[i] = false;
            names[i] = "";
        }
        n = 0;
        cityCount = 0;
    }

    int size() const { return n; }
    int count() const { return cityCount; }
    bool isActive(int u) const { return u >= 0 && u < n && active[u]; }
    const string& name(int u) const { return names[u]; }
    Edge* edges(int u) const { return head[u]; }

    int addCity(const string& nm) {
        int slot = -1;
        for (int i = 0; i < n; i++) if (!active[i]) { slot = i; break; }
        if (slot == -1) {
            if (n >= MAXC) return -1;
            slot = n++;
        }
        names[slot] = nm;
        active[slot] = true;
        head[slot] = NULL;
        cityCount++;
        return slot;
    }

    void removeCity(int u) {
        if (!isActive(u)) return;
        Edge* e = head[u];
        while (e) {
            removeDirected(e->to, u);
            Edge* nx = e->next;
            delete e;
            e = nx;
        }
        head[u] = NULL;
        active[u] = false;
        names[u] = "";
        cityCount--;
        while (n > 0 && !active[n - 1]) n--;
    }

    Edge* findEdge(int u, int v) const {
        if (!isActive(u)) return NULL;
        for (Edge* e = head[u]; e; e = e->next) if (e->to == v) return e;
        return NULL;
    }

    // returns 0 = added, 1 = already exists, 2 = invalid
    int addRoad(int u, int v, int w) {
        if (!isActive(u) || !isActive(v) || u == v || w <= 0) return 2;
        if (findEdge(u, v)) return 1;
        appendEdge(u, v, w);
        appendEdge(v, u, w);
        return 0;
    }

    bool removeRoad(int u, int v) {
        if (!findEdge(u, v)) return false;
        removeDirected(u, v);
        removeDirected(v, u);
        return true;
    }

    bool setBlocked(int u, int v, bool b) {
        Edge* a = findEdge(u, v);
        Edge* c = findEdge(v, u);
        if (!a || !c) return false;
        a->blocked = b;
        c->blocked = b;
        return true;
    }

    int openDegree(int u) const {
        int d = 0;
        for (Edge* e = head[u]; e; e = e->next) if (!e->blocked) d++;
        return d;
    }

    int roadCount() const {
        int total = 0;
        for (int u = 0; u < n; u++)
            if (active[u]) for (Edge* e = head[u]; e; e = e->next) if (u < e->to) total++;
        return total;
    }

    int findExact(const string& nm) const {
        string k = toLowerStr(nm);
        for (int i = 0; i < n; i++)
            if (active[i] && toLowerStr(names[i]) == k) return i;
        return -1;
    }

    // exact match first, then unique partial match; -1 = none, -2 = ambiguous
    int findByName(const string& nm) const {
        int e = findExact(nm);
        if (e != -1) return e;
        string k = toLowerStr(nm);
        int found = -1, hits = 0;
        for (int i = 0; i < n; i++)
            if (active[i] && toLowerStr(names[i]).find(k) != string::npos) { found = i; hits++; }
        if (hits == 1) return found;
        if (hits > 1) return -2;
        return -1;
    }
};

const int DEF_CITIES = 25;
const char* defNames[DEF_CITIES] = {
    "Saddar", "Clifton", "DHA", "Korangi", "Landhi",
    "Malir", "Jinnah Airport", "Gulshan-e-Iqbal", "Gulistan-e-Johar", "North Nazimabad",
    "Nazimabad", "Orangi Town", "SITE", "Lyari", "Keamari",
    "PECHS", "Tariq Road", "Bahadurabad", "Federal B Area", "North Karachi",
    "Surjani Town", "Scheme 33", "Gulshan-e-Maymar", "Liaquatabad", "Model Colony"
};

const int DEF_ROADS = 39;
const int defRoads[DEF_ROADS][3] = {
    {0, 1, 6},   {0, 13, 4},  {0, 15, 6},  {0, 14, 9},  {1, 2, 4},
    {2, 3, 12},  {2, 15, 8},  {13, 14, 6}, {13, 12, 7}, {13, 10, 6},
    {15, 16, 2}, {16, 7, 7},  {16, 23, 4}, {15, 6, 9},  {6, 24, 4},
    {24, 5, 4},  {16, 17, 2}, {17, 7, 6},  {7, 8, 4},   {8, 6, 7},
    {8, 21, 8},  {21, 22, 9}, {7, 9, 9},   {9, 10, 4},  {9, 19, 7},
    {19, 20, 8}, {19, 11, 9}, {11, 12, 7}, {10, 12, 6}, {10, 23, 4},
    {18, 23, 3}, {18, 9, 5},  {18, 7, 4},  {3, 4, 7},   {3, 24, 9},
    {4, 5, 5},   {5, 21, 14}, {3, 6, 10},  {12, 14, 8}
};

void loadDefaultMap(Graph& g) {
    g.clear();
    for (int i = 0; i < DEF_CITIES; i++) g.addCity(defNames[i]);
    for (int i = 0; i < DEF_ROADS; i++) g.addRoad(defRoads[i][0], defRoads[i][1], defRoads[i][2]);
}

// Map file: N, N city names, M, then M lines "idA idB km" (0-based ids); # starts a comment line
bool nextLine(ifstream& in, string& line) {
    while (getline(in, line)) {
        line = trimStr(line);
        if (!line.empty() && line[0] != '#') return true;
    }
    return false;
}

bool parseThreeInts(const string& line, int& a, int& b, int& c) {
    const char* p = line.c_str();
    char* end;
    long x = strtol(p, &end, 10); if (end == p) return false; p = end;
    long y = strtol(p, &end, 10); if (end == p) return false; p = end;
    long z = strtol(p, &end, 10); if (end == p) return false;
    if (x < -1000000 || x > 1000000 || y < -1000000 || y > 1000000 || z < -1000000 || z > 1000000) return false;
    a = (int)x; b = (int)y; c = (int)z;
    return true;
}

bool saveMap(const Graph& g, const string& fname) {
    ofstream out(fname.c_str());
    if (!out) return false;
    int newId[MAXC];
    int cnt = 0;
    for (int i = 0; i < g.size(); i++) newId[i] = g.isActive(i) ? cnt++ : -1;
    out << cnt << "\n";
    for (int i = 0; i < g.size(); i++) if (g.isActive(i)) out << g.name(i) << "\n";
    out << g.roadCount() << "\n";
    for (int u = 0; u < g.size(); u++) {
        if (!g.isActive(u)) continue;
        for (Edge* e = g.edges(u); e; e = e->next)
            if (u < e->to) out << newId[u] << " " << newId[e->to] << " " << e->w << "\n";
    }
    return true;
}

bool loadMap(Graph& g, const string& fname, string& err) {
    ifstream in(fname.c_str());
    if (!in) { err = "Cannot open the file."; return false; }
    string line;
    if (!nextLine(in, line) || !isNumber(line)) { err = "First line must be the number of cities."; return false; }
    int n = atoi(line.c_str());
    if (n < 1 || n > MAXC) { err = "Number of cities must be between 1 and 100."; return false; }

    string nm[MAXC];
    for (int i = 0; i < n; i++) {
        if (!nextLine(in, line)) { err = "File ended while reading city names."; return false; }
        nm[i] = line;
    }
    if (!nextLine(in, line) || !isNumber(line)) { err = "Number of roads is missing."; return false; }
    int m = atoi(line.c_str());
    if (m > MAXE) { err = "Too many roads (max 2000)."; return false; }

    static int eu[MAXE], ev[MAXE], ew[MAXE];
    for (int j = 0; j < m; j++) {
        if (!nextLine(in, line)) { err = "File ended while reading roads."; return false; }
        int u, v, w;
        if (!parseThreeInts(line, u, v, w) || u < 0 || u >= n || v < 0 || v >= n || u == v || w <= 0) {
            err = "Invalid road entry: \"" + line + "\"";
            return false;
        }
        eu[j] = u; ev[j] = v; ew[j] = w;
    }
    g.clear();
    for (int i = 0; i < n; i++) g.addCity(nm[i]);
    for (int j = 0; j < m; j++) g.addRoad(eu[j], ev[j], ew[j]);
    return true;
}

struct Route {
    bool found;
    int dist;
    int hops;
    int len;
    int path[MAXC];
    int explored;
};

// Push the path onto a stack from dst back to src, then pop it to get src -> dst order
void buildPath(const int parent[], int dst, Route& r) {
    Stack st;
    for (int v = dst; v != -1; v = parent[v]) st.push(v);
    r.len = 0;
    while (!st.empty()) r.path[r.len++] = st.pop();
    r.hops = r.len - 1;
}

int pathDistance(const Graph& g, const Route& r) {
    int total = 0;
    for (int i = 0; i + 1 < r.len; i++) {
        Edge* e = g.findEdge(r.path[i], r.path[i + 1]);
        if (e) total += e->w;
    }
    return total;
}

Route emptyRoute() {
    Route r;
    r.found = false; r.dist = 0; r.hops = 0; r.len = 0; r.explored = 0;
    return r;
}

Route bfsRoute(const Graph& g, int src, int dst) {
    Route r = emptyRoute();
    int n = g.size();
    bool vis[MAXC];
    int parent[MAXC];
    for (int i = 0; i < n; i++) { vis[i] = false; parent[i] = -1; }

    CircularQueue q;
    q.enqueue(src);
    vis[src] = true;
    while (!q.empty()) {
        int u = q.dequeue();
        r.explored++;
        if (u == dst) { r.found = true; break; }
        for (Edge* e = g.edges(u); e; e = e->next) {
            if (e->blocked) continue;
            if (!vis[e->to]) {
                vis[e->to] = true;
                parent[e->to] = u;
                q.enqueue(e->to);
            }
        }
    }
    if (r.found) {
        buildPath(parent, dst, r);
        r.dist = pathDistance(g, r);
    }
    return r;
}

// dst = -1 computes distances to every city
void dijkstra(const Graph& g, int src, int dst, int dist[], int parent[], int& explored) {
    int n = g.size();
    bool done[MAXC];
    for (int i = 0; i < n; i++) { dist[i] = INF; parent[i] = -1; done[i] = false; }
    MinHeap h;
    dist[src] = 0;
    h.push(src, 0);
    explored = 0;
    while (!h.empty()) {
        int u = h.popMin();
        done[u] = true;
        explored++;
        if (u == dst) break;
        for (Edge* e = g.edges(u); e; e = e->next) {
            if (e->blocked) continue;
            int v = e->to;
            if (done[v]) continue;
            int nd = dist[u] + e->w;
            if (nd < dist[v]) {
                bool inHeap = h.contains(v);
                dist[v] = nd;
                parent[v] = u;
                if (inHeap) h.decreaseKey(v, nd);
                else h.push(v, nd);
            }
        }
    }
}

Route dijkstraRoute(const Graph& g, int src, int dst) {
    Route r = emptyRoute();
    int dist[MAXC], parent[MAXC];
    dijkstra(g, src, dst, dist, parent, r.explored);
    if (dist[dst] < INF) {
        r.found = true;
        buildPath(parent, dst, r);
        r.dist = dist[dst];
    }
    return r;
}

void printRoute(const Graph& g, const Route& r, const string& title) {
    cout << "\n--- " << title << " ---\n";
    if (!r.found) { cout << "  No route available (the cities are not connected).\n"; return; }
    cout << "  ";
    for (int i = 0; i < r.len; i++) {
        cout << g.name(r.path[i]);
        if (i + 1 < r.len) {
            Edge* e = g.findEdge(r.path[i], r.path[i + 1]);
            cout << " -(" << (e ? e->w : 0) << " km)-> ";
        }
    }
    cout << "\n  Roads travelled : " << r.hops
         << "\n  Total distance  : " << r.dist << " km"
         << "\n  Cities explored : " << r.explored << "\n";
}

void printComparison(const Graph& g, const Route& b, const Route& d) {
    printRoute(g, b, "Fewest stops (BFS)");
    printRoute(g, d, "Shortest distance (Dijkstra)");
    if (!b.found || !d.found) return;
    cout << "\n--- Comparison ---\n";
    cout << padR("", 24) << padR("BFS", 16) << padR("Dijkstra", 16) << "\n";
    cout << padR("Roads travelled", 24) << padR(numStr(b.hops), 16) << padR(numStr(d.hops), 16) << "\n";
    cout << padR("Distance (km)", 24) << padR(numStr(b.dist), 16) << padR(numStr(d.dist), 16) << "\n";
    cout << padR("Cities explored", 24) << padR(numStr(b.explored), 16) << padR(numStr(d.explored), 16) << "\n";
    bool same = (b.len == d.len);
    for (int i = 0; same && i < b.len; i++) if (b.path[i] != d.path[i]) same = false;
    if (same) cout << "  Both algorithms chose the same route.\n";
    else {
        cout << "  The routes differ: ";
        if (d.dist < b.dist) cout << "Dijkstra saves " << (b.dist - d.dist) << " km";
        else cout << "equal distance";
        if (b.hops < d.hops) cout << " but BFS uses " << (d.hops - b.hops) << " fewer road(s).";
        else cout << " with the same number of roads.";
        cout << "\n";
    }
}

void dfsVisit(const Graph& g, int u, int comp[], int id) {
    comp[u] = id;
    for (Edge* e = g.edges(u); e; e = e->next)
        if (!e->blocked && comp[e->to] == -1) dfsVisit(g, e->to, comp, id);
}

int findComponents(const Graph& g, int comp[]) {
    int n = g.size(), k = 0;
    for (int i = 0; i < n; i++) comp[i] = -1;
    for (int i = 0; i < n; i++)
        if (g.isActive(i) && comp[i] == -1) dfsVisit(g, i, comp, k++);
    return k;
}

struct Item {
    int id;
    int key;
};

long long cmpCount = 0;
long long moveCount = 0;
static Item tmpBuf[MAXSORT];

inline bool lessItem(const Item& a, const Item& b) {
    cmpCount++;
    return a.key < b.key || (a.key == b.key && a.id < b.id);
}
inline void putItem(Item& dst, const Item& src) { dst = src; moveCount++; }
inline void swapItem(Item& a, Item& b) { Item t = a; a = b; b = t; moveCount += 3; }

void bubbleSort(Item a[], int n) {
    for (int i = 0; i < n - 1; i++) {
        bool swapped = false;
        for (int j = 0; j < n - 1 - i; j++)
            if (lessItem(a[j + 1], a[j])) { swapItem(a[j], a[j + 1]); swapped = true; }
        if (!swapped) break;
    }
}

void selectionSort(Item a[], int n) {
    for (int i = 0; i < n - 1; i++) {
        int m = i;
        for (int j = i + 1; j < n; j++) if (lessItem(a[j], a[m])) m = j;
        if (m != i) swapItem(a[i], a[m]);
    }
}

void insertionSort(Item a[], int n) {
    for (int i = 1; i < n; i++) {
        Item t = a[i];
        int j = i - 1;
        while (j >= 0 && lessItem(t, a[j])) { putItem(a[j + 1], a[j]); j--; }
        putItem(a[j + 1], t);
    }
}

void shellSort(Item a[], int n) {
    for (int gap = n / 2; gap > 0; gap /= 2)
        for (int i = gap; i < n; i++) {
            Item t = a[i];
            int j = i;
            while (j >= gap && lessItem(t, a[j - gap])) { putItem(a[j], a[j - gap]); j -= gap; }
            putItem(a[j], t);
        }
}

void mergeSortRec(Item a[], int lo, int hi) {
    if (lo >= hi) return;
    int mid = lo + (hi - lo) / 2;
    mergeSortRec(a, lo, mid);
    mergeSortRec(a, mid + 1, hi);
    int i = lo, j = mid + 1, k = lo;
    while (i <= mid && j <= hi) {
        if (lessItem(a[j], a[i])) putItem(tmpBuf[k++], a[j++]);
        else putItem(tmpBuf[k++], a[i++]);
    }
    while (i <= mid) putItem(tmpBuf[k++], a[i++]);
    while (j <= hi) putItem(tmpBuf[k++], a[j++]);
    for (k = lo; k <= hi; k++) putItem(a[k], tmpBuf[k]);
}
void mergeSort(Item a[], int n) { mergeSortRec(a, 0, n - 1); }

void quickSortRec(Item a[], int lo, int hi) {
    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        swapItem(a[mid], a[hi]);
        Item p = a[hi];
        int i = lo;
        for (int j = lo; j < hi; j++)
            if (lessItem(a[j], p)) { swapItem(a[i], a[j]); i++; }
        swapItem(a[i], a[hi]);
        quickSortRec(a, lo, i - 1);
        lo = i + 1;
    }
}
void quickSort(Item a[], int n) { quickSortRec(a, 0, n - 1); }

typedef void (*SortFn)(Item[], int);

bool isSortedItems(const Item a[], int n) {
    for (int i = 0; i + 1 < n; i++)
        if (a[i + 1].key < a[i].key || (a[i + 1].key == a[i].key && a[i + 1].id < a[i].id)) return false;
    return true;
}

void compareSorts(const Item base[], int n) {
    static Item work[MAXSORT];
    const char* algoNames[6] = { "Bubble", "Selection", "Insertion", "Shell", "Merge", "Quick" };
    SortFn fns[6] = { bubbleSort, selectionSort, insertionSort, shellSort, mergeSort, quickSort };

    cout << "\n--- Sorting comparison: " << n << " elements ---\n";
    cout << padR("Algorithm", 12) << padL("Comparisons", 14) << padL("Moves", 12) << padL("Sorted", 8) << "\n";
    cout << string(46, '-') << "\n";

    for (int a = 0; a < 6; a++) {
        for (int i = 0; i < n; i++) work[i] = base[i];
        cmpCount = 0;
        moveCount = 0;
        fns[a](work, n);
        cout << padR(algoNames[a], 12) << padL(numStr(cmpCount), 14) << padL(numStr(moveCount), 12)
             << padL(isSortedItems(work, n) ? "yes" : "NO", 8) << "\n";
    }
}

void sortingMenu(const Graph& g) {
    static Item items[MAXSORT];
    while (true) {
        cout << "\n=== Rank / Sort Cities ===\n"
             << "  1. Rank by number of open roads (most connected first)\n"
             << "  2. Rank by distance from a source city (nearest first)\n"
             << "  3. Stress test: compare sorts on random data\n"
             << "  0. Back\n";
        int c = readInt("Choice: ", 0, 3);
        if (c == 0) return;

        if (c == 1) {
            int n = 0;
            for (int i = 0; i < g.size(); i++)
                if (g.isActive(i)) { items[n].id = i; items[n].key = -g.openDegree(i); n++; }
            if (n == 0) { cout << "  The map is empty.\n"; continue; }
            Item ranked[MAXC];
            for (int i = 0; i < n; i++) ranked[i] = items[i];
            mergeSort(ranked, n);
            cout << "\n--- Cities ranked by number of open roads ---\n";
            for (int i = 0; i < n; i++)
                cout << "  " << padL(numStr(i + 1), 2) << ". " << padR(g.name(ranked[i].id), 20)
                     << -ranked[i].key << " road(s)\n";
            compareSorts(items, n);
        }
        else if (c == 2) {
            if (g.count() < 2) { cout << "  Need at least two cities.\n"; continue; }
            int src = readCity(g, "Source city (id or name, blank = cancel): ");
            if (src < 0) continue;
            int dist[MAXC], parent[MAXC], ex;
            dijkstra(g, src, -1, dist, parent, ex);
            int n = 0;
            for (int i = 0; i < g.size(); i++)
                if (g.isActive(i) && i != src) { items[n].id = i; items[n].key = dist[i]; n++; }
            Item ranked[MAXC];
            for (int i = 0; i < n; i++) ranked[i] = items[i];
            mergeSort(ranked, n);
            cout << "\n--- Cities ranked by distance from " << g.name(src) << " ---\n";
            for (int i = 0; i < n; i++) {
                cout << "  " << padL(numStr(i + 1), 2) << ". " << padR(g.name(ranked[i].id), 20);
                if (ranked[i].key >= INF) cout << "unreachable\n";
                else cout << ranked[i].key << " km\n";
            }
            compareSorts(items, n);
        }
        else {
            int n = readInt("Number of random elements (100-5000): ", 100, MAXSORT);
            srand(12345);
            for (int i = 0; i < n; i++) { items[i].id = i; items[i].key = rand() % 100000; }
            compareSorts(items, n);
            cout << "  (Notice how the work of the O(n^2) sorts explodes as n grows.)\n";
        }
    }
}

void showCityList(const Graph& g) {
    cout << "\nCities:\n";
    int col = 0;
    for (int i = 0; i < g.size(); i++) {
        if (!g.isActive(i)) continue;
        cout << "  [" << padL(numStr(i), 2) << "] " << padR(g.name(i), 20);
        if (++col % 3 == 0) cout << "\n";
    }
    if (col % 3 != 0) cout << "\n";
}

int readCity(const Graph& g, const string& prompt) {
    while (true) {
        string s = readLine(prompt);
        if (s.empty()) return -1;
        if (isNumber(s)) {
            int id = atoi(s.c_str());
            if (g.isActive(id)) return id;
            cout << "  No city with id " << s << ".\n";
        } else {
            int id = g.findByName(s);
            if (id >= 0) return id;
            if (id == -2) cout << "  That matches several cities - please be more specific or use the id.\n";
            else cout << "  City not found.\n";
        }
    }
}

bool readTwoCities(const Graph& g, int& a, int& b) {
    showCityList(g);
    a = readCity(g, "\nFrom city (id or name, blank = cancel): ");
    if (a < 0) return false;
    b = readCity(g, "To city   (id or name, blank = cancel): ");
    if (b < 0) return false;
    return true;
}

void displayMap(const Graph& g) {
    cout << "\n=== Road Network: " << g.count() << " cities, " << g.roadCount() << " roads ===\n";
    for (int u = 0; u < g.size(); u++) {
        if (!g.isActive(u)) continue;
        cout << "[" << padL(numStr(u), 2) << "] " << g.name(u) << ":\n";
        if (!g.edges(u)) { cout << "       (no roads)\n"; continue; }
        cout << "       ";
        for (Edge* e = g.edges(u); e; e = e->next) {
            cout << g.name(e->to) << " (" << e->w << " km" << (e->blocked ? ", CLOSED" : "") << ")";
            if (e->next) cout << ", ";
        }
        cout << "\n";
    }
}

void addCityUI(Graph& g) {
    string nm = readLine("New city name (blank = cancel): ");
    if (nm.empty()) return;
    if (g.findExact(nm) != -1) { cout << "  That city already exists.\n"; return; }
    int id = g.addCity(nm);
    if (id < 0) { cout << "  Map is full (max " << MAXC << " cities).\n"; return; }
    cout << "  Added \"" << nm << "\" with id " << id << ".\n";
    cout << "  Now connect it to existing cities (blank = finish).\n";
    while (true) {
        int other = readCity(g, "  Connect to (id or name): ");
        if (other < 0) break;
        if (other == id) { cout << "  Cannot connect a city to itself.\n"; continue; }
        int w = readInt("  Distance in km (1-1000): ", 1, 1000);
        int r = g.addRoad(id, other, w);
        if (r == 0) cout << "  Road added.\n";
        else if (r == 1) cout << "  That road already exists.\n";
    }
}

void removeCityUI(Graph& g) {
    showCityList(g);
    int id = readCity(g, "\nCity to remove (id or name, blank = cancel): ");
    if (id < 0) return;
    if (askYesNo("  Remove " + g.name(id) + " and all its roads? (y/n): ")) {
        string nm = g.name(id);
        g.removeCity(id);
        cout << "  Removed " << nm << ".\n";
    }
}

void addRoadUI(Graph& g) {
    int a, b;
    if (!readTwoCities(g, a, b)) return;
    if (a == b) { cout << "  A road needs two different cities.\n"; return; }
    int w = readInt("Distance in km (1-1000): ", 1, 1000);
    int r = g.addRoad(a, b, w);
    if (r == 0) cout << "  Road added: " << g.name(a) << " <-> " << g.name(b) << " (" << w << " km).\n";
    else if (r == 1) cout << "  A road between these cities already exists.\n";
    else cout << "  Invalid input.\n";
}

void removeRoadUI(Graph& g) {
    int a, b;
    if (!readTwoCities(g, a, b)) return;
    if (g.removeRoad(a, b)) cout << "  Road removed.\n";
    else cout << "  There is no road between these cities.\n";
}

void routeUI(const Graph& g, int mode) {
    int a, b;
    if (!readTwoCities(g, a, b)) return;
    if (a == b) { cout << "  Source and destination are the same city.\n"; return; }
    if (mode == 1) printRoute(g, bfsRoute(g, a, b), "Fewest stops (BFS)");
    else if (mode == 2) printRoute(g, dijkstraRoute(g, a, b), "Shortest distance (Dijkstra)");
    else printComparison(g, bfsRoute(g, a, b), dijkstraRoute(g, a, b));
}

void connectivityUI(const Graph& g) {
    int a, b;
    if (!readTwoCities(g, a, b)) return;
    Route r = bfsRoute(g, a, b);
    cout << "\n";
    if (r.found) cout << "  " << g.name(a) << " and " << g.name(b) << " ARE connected (minimum "
                      << r.hops << " road(s)).\n";
    else cout << "  " << g.name(a) << " and " << g.name(b) << " are NOT connected.\n";

    int comp[MAXC];
    int k = findComponents(g, comp);
    cout << "  The network has " << k << " connected component(s) (found with DFS).\n";
    if (k > 1) {
        for (int c = 0; c < k; c++) {
            cout << "    Component " << (c + 1) << ": ";
            bool first = true;
            for (int i = 0; i < g.size(); i++)
                if (g.isActive(i) && comp[i] == c) {
                    if (!first) cout << ", ";
                    cout << g.name(i);
                    first = false;
                }
            cout << "\n";
        }
    }
}

void withinStopsUI(const Graph& g) {
    showCityList(g);
    int src = readCity(g, "\nStarting city (id or name, blank = cancel): ");
    if (src < 0) return;
    int limit = readInt("Maximum number of stops N (1-99): ", 1, 99);

    int n = g.size();
    int level[MAXC];
    for (int i = 0; i < n; i++) level[i] = -1;
    CircularQueue q;
    q.enqueue(src);
    level[src] = 0;
    while (!q.empty()) {
        int u = q.dequeue();
        if (level[u] == limit) continue;
        for (Edge* e = g.edges(u); e; e = e->next) {
            if (e->blocked || level[e->to] != -1) continue;
            level[e->to] = level[u] + 1;
            q.enqueue(e->to);
        }
    }
    cout << "\n--- Cities reachable from " << g.name(src) << " within " << limit << " stop(s) ---\n";
    int total = 0;
    for (int lv = 1; lv <= limit; lv++) {
        bool any = false;
        for (int i = 0; i < n; i++) {
            if (!g.isActive(i) || level[i] != lv) continue;
            if (!any) { cout << "  " << lv << " stop(s): "; any = true; }
            else cout << ", ";
            cout << g.name(i);
            total++;
        }
        if (any) cout << "\n";
    }
    if (total == 0) cout << "  None.\n";
    else cout << "  Total: " << total << " cities.\n";
}

void listClosedRoads(const Graph& g) {
    cout << "\n--- Closed roads ---\n";
    bool any = false;
    for (int u = 0; u < g.size(); u++) {
        if (!g.isActive(u)) continue;
        for (Edge* e = g.edges(u); e; e = e->next)
            if (u < e->to && e->blocked) {
                cout << "  " << g.name(u) << " <-> " << g.name(e->to) << " (" << e->w << " km)\n";
                any = true;
            }
    }
    if (!any) cout << "  No roads are closed.\n";
}

void closureWhatIf(Graph& g) {
    int a, b;
    if (!readTwoCities(g, a, b)) return;
    if (a == b) { cout << "  Source and destination are the same city.\n"; return; }
    Route before = dijkstraRoute(g, a, b);
    printRoute(g, before, "Current best route (Dijkstra)");
    if (!before.found || before.hops < 1) return;

    cout << "\nRoads on this route:\n";
    for (int i = 0; i < before.hops; i++) {
        Edge* e = g.findEdge(before.path[i], before.path[i + 1]);
        cout << "  " << (i + 1) << ". " << g.name(before.path[i]) << " <-> "
             << g.name(before.path[i + 1]) << " (" << (e ? e->w : 0) << " km)\n";
    }
    int k = readInt("Close which road? (0 = cancel): ", 0, before.hops);
    if (k == 0) return;
    int u = before.path[k - 1], v = before.path[k];
    g.setBlocked(u, v, true);
    cout << "\n  >>> Road " << g.name(u) << " <-> " << g.name(v) << " is now CLOSED. Recomputing...\n";
    Route bfsAfter = bfsRoute(g, a, b);
    Route after = dijkstraRoute(g, a, b);
    printComparison(g, bfsAfter, after);
    if (after.found)
        cout << "\n  Detour cost: " << (after.dist - before.dist) << " extra km compared with before.\n";
    if (!askYesNo("\nKeep this road closed? (y/n): ")) {
        g.setBlocked(u, v, false);
        cout << "  Road reopened.\n";
    } else {
        cout << "  Road stays closed.\n";
    }
}

void closureMenu(Graph& g) {
    while (true) {
        cout << "\n=== Road Closures ===\n"
             << "  1. Close a road\n"
             << "  2. Reopen a road\n"
             << "  3. List closed roads\n"
             << "  4. What-if: close a road on a route and recompute\n"
             << "  0. Back\n";
        int c = readInt("Choice: ", 0, 4);
        if (c == 0) return;
        if (c == 1 || c == 2) {
            int a, b;
            if (!readTwoCities(g, a, b)) continue;
            if (g.setBlocked(a, b, c == 1)) cout << "  Road " << (c == 1 ? "closed." : "reopened.") << "\n";
            else cout << "  There is no road between these cities.\n";
        }
        else if (c == 3) listClosedRoads(g);
        else closureWhatIf(g);
    }
}

void loadUI(Graph& g) {
    string f = readLine("File name to load (blank = cancel): ");
    if (f.empty()) return;
    string err;
    if (loadMap(g, f, err)) cout << "  Loaded " << g.count() << " cities and " << g.roadCount() << " roads.\n";
    else cout << "  Load failed: " << err << " (current map kept)\n";
}

void saveUI(const Graph& g) {
    string f = readLine("File name to save (blank = karachi_map.txt): ");
    if (f.empty()) f = "karachi_map.txt";
    if (saveMap(g, f)) cout << "  Map saved to " << f << " (closed roads are saved as open).\n";
    else cout << "  Could not write the file.\n";
}

void printMenu() {
    cout << "\n==================================================\n"
         << "          KARACHI CITY ROUTE FINDER\n"
         << "==================================================\n"
         << "  1. Display map\n"
         << "  2. Add city               3. Remove city\n"
         << "  4. Add road               5. Remove road\n"
         << "  6. Fewest-stops route (BFS)\n"
         << "  7. Shortest-distance route (Dijkstra)\n"
         << "  8. Compare both routes\n"
         << "  9. Connectivity check\n"
         << " 10. Cities within N stops\n"
         << " 11. Rank / sort cities (sorting comparison)\n"
         << " 12. Road closures\n"
         << " 13. Load map from file\n"
         << " 14. Save map to file\n"
         << " 15. Reset to built-in Karachi map\n"
         << "  0. Exit\n";
}

int main() {
    Graph g;
    loadDefaultMap(g);

    while (true) {
        printMenu();
        int c = readInt("Choice: ", 0, 15);
        if (c == 0) break;
        if (g.count() == 0 && c != 2 && c != 13 && c != 15) {
            cout << "  The map is empty - add cities, load a file, or reset first.\n";
            continue;
        }
        switch (c) {
            case 1:  displayMap(g); break;
            case 2:  addCityUI(g); break;
            case 3:  removeCityUI(g); break;
            case 4:  addRoadUI(g); break;
            case 5:  removeRoadUI(g); break;
            case 6:  routeUI(g, 1); break;
            case 7:  routeUI(g, 2); break;
            case 8:  routeUI(g, 3); break;
            case 9:  connectivityUI(g); break;
            case 10: withinStopsUI(g); break;
            case 11: sortingMenu(g); break;
            case 12: closureMenu(g); break;
            case 13: loadUI(g); break;
            case 14: saveUI(g); break;
            case 15:
                if (askYesNo("  Discard the current map and restore the built-in one? (y/n): ")) {
                    loadDefaultMap(g);
                    cout << "  Built-in Karachi map restored.\n";
                }
                break;
        }
    }
    cout << "\nGoodbye!\n";
    return 0;
}