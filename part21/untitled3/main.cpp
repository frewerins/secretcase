#include <bits/stdc++.h>

using namespace std;

int main() {
    int n;
    cin >> n;
    long double numbers[n];
    for (int i = 0; i < n; ++i) {
        cin >> numbers[i];
    }
    long double pref[n + 1];
    pref[0] = 0.0;
    for (int i = 1; i < n + 1; ++i) {
        pref[i] = pref[i - 1] + 1.0 / numbers[i - 1];
    }
    int q;
    cin >> q;
    for (int i = 0; i < q; ++i) {
        int l,r;
        cin >> l >> r;
        long double x = (r - l + 1) * 1.0 / (pref[r + 1] - pref[l]);
        std::cout << std::fixed << std::setprecision(6) << x <<std::endl;
    }
    return 0;
}
