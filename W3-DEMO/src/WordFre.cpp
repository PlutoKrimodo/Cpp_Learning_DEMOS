#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>
#include <cctype>
#include <filesystem>

namespace fs = std::filesystem;
char safeToLower(char ch) {
    return static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
}

bool safeIsPunct(char ch) {
    return std::ispunct(static_cast<unsigned char>(ch)) != 0;
}

std::string cleanWord(const std::string& raw) {
    std::string w = raw;
    //转小写
    std::transform(w.begin(), w.end(), w.begin(), safeToLower);
    //去掉标点
    auto first = std::find_if_not(w.begin(), w.end(), safeIsPunct);
    auto last = std::find_if_not(w.rbegin(), w.rend(), safeIsPunct).base();
    //说明全是标点
    if (first >= last) return "";
    return std::string(first,last);
}

int main(int argc,char *argv[])
{
    std::string filename;
    if (argc >= 2) {
        filename = argv[1];
    }
    else {
        //如果一开始没有输入文件地址，提示手动输入一次
        std::cout << "Please Input the File Path:";
        std::getline(std::cin, filename);
    }

    if (filename.empty()) {
        std::cerr << "No FILE PATH Provided!\n";
        return 1;
    }

    if (!fs::exists(filename)) {
        std::cerr << "FILE NOT FOUND!\n";
        return 1;
    }

    // 以二进制模式打开文件，只读前 3 个字节，探测编码
    std::ifstream checker(filename, std::ios::binary);
    if (checker.is_open()) {
        unsigned char header[3] = { 0, 0, 0 }; // 用 unsigned char 防止符号扩展
        checker.read(reinterpret_cast<char*>(header), 3); // 尝试读 3 个字节
        std::streamsize bytesRead = checker.gcount(); // 实际读到了几个字节

        // 检查 UTF-16 LE (小端序)：FF FE
        if (bytesRead >= 2 && header[0] == 0xFF && header[1] == 0xFE) {
            std::cerr << "Error: UTF-16 LE (Little Endian) encoding detected.\n"
                << "This program only supports UTF-8 or plain ASCII.\n"
                << "Please use Notepad++ / VS Code to convert the file to UTF-8 without BOM.\n";
            return 1;
        }

        // 检查 UTF-16 BE (大端序)：FE FF
        if (bytesRead >= 2 && header[0] == 0xFE && header[1] == 0xFF) {
            std::cerr << "Error: UTF-16 BE (Big Endian) encoding detected.\n"
                << "This program only supports UTF-8 or plain ASCII.\n"
                << "Please convert the file to UTF-8 without BOM.\n";
            return 1;
        }

        // 检查 UTF-8 带 BOM (EF BB BF) 
        //    因为 Windows 记事本默认存 UTF-8 时会带这个头，会导致第一个单词统计出错。
        if (bytesRead >= 3 && header[0] == 0xEF && header[1] == 0xBB && header[2] == 0xBF) {
            std::cerr << "Error: UTF-8 with BOM (Byte Order Mark) detected.\n"
                << "The first word would be read incorrectly.\n"
                << "Please save the file as 'UTF-8 without BOM' (In VS Code: bottom bar -> 'Save with Encoding' -> 'UTF-8').\n";
            return 1;
        }

        checker.close();
    }
   

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Can not OPEN the FILE!\n";
        return 1;
    }

    std::unordered_map<std::string, int> freq;
    std::string rawWord;

    while (file >> rawWord) {
        std::string word = cleanWord(rawWord);
        if (!word.empty()) {
            //word右值插入，避免拷贝
            freq[std::move(word)]++;
        }
    }

    std::vector<std::pair<std::string, int>> sorted_fre(freq.begin(), freq.end());
    //降序，同频次按字母升序
    std::sort(sorted_fre.begin(), sorted_fre.end(),
        [](const auto& a,const auto& b) {
            if (a.second != b.second) return a.second > b.second;
            return a.first < b.first;
        });

    //前20
    std::cout << "\nFrequency Top 20\n";
    size_t cnt = 0;
    for (const auto& [word, times] : sorted_fre) {
        std::cout << word << ": " << times << std::endl;
        cnt++;
        if (cnt >= 20) break;
    }

    std::cout << "\nTotal Unique Words: " << freq.size() << std::endl;

    return 0;
}

