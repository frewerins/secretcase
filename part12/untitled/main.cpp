#include <iostream>
#include <vector>

int main() {
    std::vector<int> digit_ = {1, 2, 3};
    for (auto rit = digit_.rbegin(); rit <= digit_.rend(); ++rit) {
        digit_.push_back(*rit - '0');
        std::cout << (*rit - '0') << " ";
    }
    return 0;
}
