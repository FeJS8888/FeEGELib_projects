#include "FeEGELib.h"
#include <fstream>
#include <random>
#include <string>
#include <vector>

using namespace std;
using namespace FeEGE;

int main() {
    SetProcessDPIAware();
    _setmode(_fileno(stdout), _O_WTEXT);
    setcaption("点名器");
    init(800, 600, INIT_NOFORCEEXIT | INIT_RENDERMANUAL);
    setbkcolor(EGERGB(30, 30, 30));

    random_device rd;
    mt19937 rng(rd());
    vector<wstring> names;
    string configPath = resolvePath(".\\config\\names.txt");

    bool rolling = false;
    wstring currentName;
    int remainingRolls = 0;
    int selectedIdx = -1;
    constexpr int NAME_FONT_SIZE = 80;

    // 从配置文件加载成员列表（UTF-8/ANSI 自动转换为 wstring）
    auto reloadNames = [&]() {
        names.clear();
        ifstream in(configPath);
        if (!in.is_open()) return;
        string line;
        while (getline(in, line)) {
            size_t first = line.find_first_not_of(" \t\r\n");
            if (first == string::npos) continue;
            if (line[first] == '#') continue;
            size_t last = line.find_last_not_of(" \t\r\n");
            names.push_back(autoToWString(line.substr(first, last - first + 1)));
        }
    };

    reloadNames();

    // ===== Widget 构建 =====

    // 标题
    Text* titleText = TextBuilder()
        .setPosition(400, 40)
        .setContent(L"点名器")
        .setFont(50, L"宋体")
        .setColor(EGERGB(0, 200, 255))
        .setAlign(TextAlign::Center)
        .build();

    // 成员数量提示
    Text* countText = TextBuilder()
        .setPosition(400, 108)
        .setContent(L"成员数量：" + to_wstring(names.size()))
        .setFont(22, L"宋体")
        .setColor(EGERGB(150, 150, 150))
        .setAlign(TextAlign::Center)
        .build();

    // 中央名字显示区
    Text* nameText = TextBuilder()
        .setPosition(400, 240)
        .setContent(L"点击下方按钮开始点名")
        .setFont(28, L"宋体")
        .setColor(EGERGB(80, 80, 80))
        .setAlign(TextAlign::Center)
        .build();

    // 开始点名按钮
    Button* rollBtn = ButtonBuilder()
        .setIdentifier(L"rollButton")
        .setSize(180, 60)
        .setColor(EGERGBA(64, 128, 255, 255))
        .setRadius(12)
        .setContent(L"开始点名")
        .build();

    // 配置成员按钮
    Button* settingsBtn = ButtonBuilder()
        .setIdentifier(L"settingsButton")
        .setSize(150, 60)
        .setColor(EGERGBA(119, 136, 153, 255))
        .setRadius(12)
        .setContent(L"配置成员")
        .build();

    // 按钮行 Box（Row 布局，居中对齐）
    Box* btnBox = BoxBuilder()
        .setCenter(400, 510)
        .setSize(600, 80)
        .setDirection(LayoutDirection::Row)
        .setAlign(LayoutAlign::Center)
        .setSpacing(20)
        .addChild({rollBtn, settingsBtn})
        .build();

    assignOrder({titleText, countText, nameText, btnBox});

    // ===== 界面更新 =====
    auto updateUI = [&]() {
        countText->setContent(L"成员数量：" + to_wstring(names.size()));
        if (currentName.empty()) {
            nameText->setContent(L"点击下方按钮开始点名");
            nameText->setFont(28, L"宋体");
            nameText->setColor(EGERGB(80, 80, 80));
        } else {
            nameText->setContent(currentName);
            nameText->setFont(NAME_FONT_SIZE, L"宋体");
            nameText->setColor(rolling ? EGERGB(255, 220, 50) : WHITE);
        }
    };

    // ===== 滚动动画（在 main 作用域声明，保证引用有效）=====
    function<void()> doRoll;
    doRoll = [&]() {
        if (remainingRolls > 0) {
            currentName = names[uniform_int_distribution<size_t>(0, names.size() - 1)(rng)];
            updateUI();
            int elapsed = 20 - remainingRolls;
            remainingRolls--;
            setTimeOut(doRoll, 50 + elapsed * 10);
        } else {
            currentName = names[selectedIdx];
            rolling = false;
            updateUI();
        }
    };

    // ===== 按钮事件绑定 =====
    rollBtn->setOnClickEvent([&]() {
        if (names.empty() || rolling) return;
        rolling = true;
        selectedIdx = static_cast<int>(uniform_int_distribution<size_t>(0, names.size() - 1)(rng));
        remainingRolls = 20;
        doRoll();
    });

    settingsBtn->setOnClickEvent([&]() {
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
            updateUI();
        }
    });

    start();
    return 0;
}

