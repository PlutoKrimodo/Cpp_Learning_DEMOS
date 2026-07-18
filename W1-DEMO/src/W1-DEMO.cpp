#include <iostream>
#include<vector>
#include<algorithm>
#include<string>
#include<cstring>

void PrintNums(const std::vector<int>& nums) {
    for (const int& val : nums) {
        std::cout << val << " ";
    }
    std::cout << std::endl;
}

bool safeParseInt(const char* str, int& outVal) {
    char* endPtr = nullptr;
    errno = 0;
    long val = std::strtol(str, &endPtr, 10);

    if (endPtr == str || *endPtr != '\0') return false; // 没数字 或 后面有脏数据
    if (errno == ERANGE || val < INT_MIN || val > INT_MAX) return false;

    outVal = static_cast<int>(val);
    return true;
}

int main(int argc,char* argv[])
{
    try {
        bool descending = false;    //递减
        bool oddFirst = false;      //奇数在前，偶数在后
        std::vector<int> numbers;
        
        //注意，字符数组的第一个元素为程序名称
        for (int i = 1; i < argc; ++i) {
            if (std::strcmp(argv[i], "-r") == 0) {
                descending = true;
            }
            else if (std::strcmp(argv[i], "-l") == 0) {
                oddFirst = true;
            }
            else {
                int val;
                if (safeParseInt(argv[i], val)) {
                    numbers.push_back(val);
                } else {
                    std::cerr << "警告: 忽略无效数字 '" << argv[i] << "'" << std::endl;
                }
            }
        }

        if (numbers.empty()) {
            std::cerr << "必须输入至少一个数值！" << std::endl;
            return 1;
        }

        auto asc = [](auto a, auto b) {return a < b; };
        auto desc = [](auto a, auto b) {return a > b; };
        auto oddfirst = [](auto a, auto b) {
            bool aOdd = (a % 2 != 0);
            bool bOdd = (b % 2 != 0);
            if (aOdd && !bOdd) return true;
            if (!aOdd && bOdd) return false;
            return a < b;
            };

        if (descending) {
            std::sort(numbers.begin(), numbers.end(), desc);
        }
        else if (oddFirst) {
            std::sort(numbers.begin(), numbers.end(), oddfirst);
        }
        else {
            std::sort(numbers.begin(), numbers.end());
        }

        std::cout << "排序结果如下：";
        PrintNums(numbers);
    }
    catch (const std::exception&e) {
        std::cerr << "程序致命错误：" << e.what() << std::endl;
        return 2;
    }
    catch (...) {
        std::cerr << "未知致命错误！" << std::endl;
        return 3;
    }
    return 0;
}
