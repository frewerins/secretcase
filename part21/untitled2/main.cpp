#include <bits/stdc++.h>
struct Point {
    int x1;
    int x2;
    int y1;
    int y2;
    Point(int x1, int x2, int y1, int y2) : x1(x1), x2(x2), y1(y1), y2(y2) {};
};

int BSearch(int w, int h, std::vector<std::vector<int>>& arr) {
    int r = std::min(w, h) + 1;
    int l = 0;
    while (l < r - 1) {
        int m = (l + r) / 2;
        int cur = 0;
        for (int j = 0; j <= w - m; ++j){
            std::vector<std::pair<int, int>> p;
            for (int i = 0; i < arr.size(); ++i) {
                if (std::max(j, arr[i][0]) < std::min(j + m, arr[i][2])) {
                    p.push_back({arr[i][1], 0});
                    p.push_back({arr[i][3], 1});
                }
            }
            p.push_back({0, 0});
            p.push_back({h, 1});
            sort(p.begin(), p.end());
            int bal = 0;
            for (size_t i = 1; i < p.size(); ++i) {
                if (i && bal == 0){
                    cur = std::max(cur, p[i].first - p[i - 1].first);
                }
                if (p[i].second == 0) {
                    ++bal;
                } else {
                    --bal;
                }
            }
        }
        if (cur >= m) {
            l = m;
        }
        else{
            r = m;
        }
    }
    return l;
}
int main(){
    int w, h, a, b;
    std::vector<std::vector<int>> arr;
    std::cin >> w >> h >> a >> b;
    size_t n;
    std::cin >> n;
    for (size_t i = 0; i < n; ++i) {
        int x1, x2, y1, y2;
        std::cin >> x1 >> y1 >> x2 >> y2;
        arr.push_back({x1, y1, x2, y2});
    }
    int f = BSearch(w, h, arr);
    for (int j = 0; j <= w - f; ++j) {
        std::vector<std::pair<int, int>> p;
        for (size_t i = 0; i < n; ++i) {
            if (std::max(j, arr[i][0]) < std::min(j + f, arr[i][2])) {
                p.push_back({arr[i][1], 0});
                p.push_back({arr[i][3], 1});
            }
        }
        p.push_back({0, 0});
        p.push_back({h, 1});
        sort(p.begin(), p.end());
        int bal = 0, cur = 0;
        for (size_t i = 1; i < p.size(); ++i){
            if (i && bal == 0){
                cur = std::max(cur, p[i].first - p[i - 1].first);
                if (cur >= f) {
                    std::cout << j << " " << p[i - 1].first << " " << j + f << " " << p[i - 1].first + f << "\n";
                    return 0;
                }
            }
            if (p[i].second == 0) {
                ++bal;
            }
            else{
                --bal;
            }
        }
    }


    return 0;
}