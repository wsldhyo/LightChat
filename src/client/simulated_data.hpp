#ifndef SIMULATED_DATA_HPP
#define SIMULATED_DATA_HPP
#include <QString>
#include <vector>
// ------------------ 模拟数据 ------------------
static std::vector<QString> strs = {
    "hello world !", "nice to meet u", "New year,new life",
    "You have to love yourself",
    "My love is written in the wind ever since the whole world is you"};

static std::vector<QString> heads = {":/icons/head_1.jpg", ":/icons/head_2.jpg",
                                     ":/icons/head_3.jpg", ":/icons/head_4.jpg",
                                     ":/icons/head_5.jpg"};

static std::vector<QString> names = {"llfc", "zack",   "golang", "cpp",
                                     "java", "nodejs", "python", "rust"};
#endif // SIMULATED_DATA_HPP
