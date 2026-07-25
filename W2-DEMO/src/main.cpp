#include "myvector.h"
#include <iostream>
#include<cmath>
//素数判断
bool isPrime(int num) {
	if (num <= 1) {
		return false;
	}
	else if (num == 2) {
		return true;
	}
	else if (num % 2 == 0) {
		return false;
	}

	int max_divisor = (int)(std::sqrt(num) + 1);
	for (int i = 3; i < max_divisor; i += 2) {
		if (num % i == 0) {
			return false;
		}
	}
	return true;
}

//测试MyVector使用
int main(){
	int n;
	std::cin >> n;
	MyVector<int> temp;
	for (int i = 0; i < n; i++) {
		if (isPrime(i)) {
			temp.push_back(i);
		}
	}

	for (auto num : temp) {
		std::cout << num << " ";
	}
	std::cout << std::endl;
	return 0;
}