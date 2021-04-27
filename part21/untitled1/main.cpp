#include <iostream>
#include <unordered_set>
#include <vector>


double calc(std::vector<long double>& arr, size_t l, size_t r) {
    if (l > 0) {
        return (r - l + 1) * 1.0 / (arr[r] - arr[l - 1]);
    } else {
        return (r - l + 1) * 1.0 / (arr[r]);
    }
}

int main() {
    size_t n;
    std::cin >> n;
    std::vector<long double> arr;
    long double sum = 0;
    for (size_t i = 0; i < n; ++i) {
        long double a;
        std::cin >> a;
        sum += 1.0 / a;
        arr.push_back(sum);
    }
    arr.push_back(sum);
    size_t m;
    std::cin >> m;
    for (size_t i = 0; i < m; ++i) {
        size_t l, r;
        std::cin >> l >> r;
        double answer = calc(arr, l, r);
        printf("%.6f\n", answer);
    }
    return 0;
}
