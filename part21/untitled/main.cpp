#include <iostream>

bool find_pr(int n) {
    if (n == 2 || n == 3) {
        return true;
    }
    for (int i = 2; i * i <= n; ++i) {
        if (n % i == 0) {
            return false;
        }
    }
    return true;
}

int main() {
    int n;
    std::cout << sizeof("\r\n\r\n");
    std::cin >> n;
    int i = 0;
    int digit = 2;
    while (i < n) {
        if (find_pr(digit)) {
            std::cout << digit << "\n";
            ++i;
        }
        ++digit;
    }
    return 0;
}
