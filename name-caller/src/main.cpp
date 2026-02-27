#include "FeEGELib.h"
#include <algorithm>
#include <fstream>
#include <random>
#include <string>
#include <vector>

using namespace std;
using namespace FeEGE;

int main() {
    setcaption("点名器");
    init(800, 600, INIT_NOFORCEEXIT | INIT_RENDERMANUAL);
    initPen();
    setbkcolor(EGERGB(30, 30, 30));

    random_device rd;
    mt19937 rng(rd());
    vector<string> names;
    string configPath = resolvePath(".\\config\\names.txt");

    bool rolling = false;
    string currentName = "";
    int remainingRolls = 0;
    int selectedIdx = -1;

    // 从配置文件加载成员列表
    auto reloadNames = [&]() {
        names.clear();
        ifstream in(configPath);
        if (!in.is_open()) return;
        string line;
        while (getline(in, line)) {
            if (detectEncoding(line) == "UTF-8") {
                line = UTF8ToANSI(line);
            }
            size_t first = line.find_first_not_of(" \t\r\n");
            if (first == string::npos) continue;
            if (line[first] == '#') continue;
            size_t last = line.find_last_not_of(" \t\r\n");
            names.push_back(line.substr(first, last - first + 1));
        }
    };

    // 重绘主界面
    auto reDraw = [&]() {
        pen::clearAll();

        // 标题
        pen::font(45, "宋体");
        pen::color(EGERGB(0, 200, 255));
        pen::print(X >> 1, (int)(Y * 0.07), "点名器");

        // 成员数量提示
        pen::font(22, "宋体");
        pen::color(EGERGB(150, 150, 150));
        pen::print(X >> 1, (int)(Y * 0.18), "成员数量：" + to_string(names.size()));

        // 中央名字显示区域
        if (!currentName.empty()) {
            int fontSize = 110;
            while (fontSize > 20 &&
                   textwidth(currentName.c_str(), pen_image) >= (int)(X * 0.8)) {
                fontSize--;
                pen::font(fontSize, "宋体");
            }
            if (rolling) {
                pen::color(EGERGB(255, 220, 50));
            } else {
                pen::color(WHITE);
            }
            pen::print(X >> 1, (int)(Y * 0.45), currentName);
        } else {
            pen::font(30, "宋体");
            pen::color(EGERGB(80, 80, 80));
            pen::print(X >> 1, (int)(Y * 0.45), "点击下方按钮开始点名");
        }
    };

    reloadNames();
    reDraw();

    // 滚动动画函数（在 main 作用域声明，保证引用有效）
    function<void()> doRoll;
    doRoll = [&]() {
        if (remainingRolls > 0) {
            currentName = names[uniform_int_distribution<size_t>(0, names.size() - 1)(rng)];
            reDraw();
            int elapsed = 20 - remainingRolls;
            remainingRolls--;
            int delay = 50 + elapsed * 10;
            setTimeOut(doRoll, delay);
        } else {
            currentName = names[selectedIdx];
            rolling = false;
            reDraw();
        }
    };

    // 开始点名按钮
    ButtonBuilder()
        .setIdentifier(L"rollButton")
        .setCenter(X * 0.5, Y - 75)
        .setColor(EGERGBA(64, 128, 255, 255))
        .setSize(180, 60)
        .setRadius(12)
        .setContent("开始点名")
        .setOnClick([&]() {
            if (names.empty() || rolling) return;
            rolling = true;
            selectedIdx = static_cast<int>(uniform_int_distribution<size_t>(0, names.size() - 1)(rng));
            remainingRolls = 20;
            doRoll();
        })
        .build();

    // 配置成员按钮
    ButtonBuilder()
        .setIdentifier(L"settingsButton")
        .setCenter(X - 100, Y - 75)
        .setColor(EGERGBA(119, 136, 153, 255))
        .setSize(150, 60)
        .setRadius(12)
        .setContent("配置成员")
        .setOnClick([&]() {
            if (rolling) return;
            int result = MessageBox(
                getHWnd(),
                TEXT("即将打开配置文件，关闭配置文件后自动重载，是否继续？"),
                TEXT("提示"),
                MB_YESNO | MB_ICONQUESTION
            );
            if (result == IDYES) {
                runCmdAwait("notepad.exe " + configPath);
                reloadNames();
                reDraw();
            }
        })
        .build();

    start();
    return 0;
}
