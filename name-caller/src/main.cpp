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
    constexpr int MAX_NAMES = 50;

    // wstring 去除首尾空白 helper
    auto trimW = [](const wstring& s) -> wstring {
        size_t first = s.find_first_not_of(L" \t");
        if (first == wstring::npos) return L"";
        size_t last = s.find_last_not_of(L" \t");
        return s.substr(first, last - first + 1);
    };

    // wstring → UTF-8 helper（用于将 names 写回配置文件）
    auto wstrToUTF8 = [](const wstring& ws) -> string {
        if (ws.empty()) return "";
        int bytes = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), (int)ws.size(),
                                        nullptr, 0, nullptr, nullptr);
        if (bytes <= 0) return "";
        string result(bytes, '\0');
        WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), (int)ws.size(),
                            &result[0], bytes, nullptr, nullptr);
        return result;
    };

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

    // ===== 主界面 Widgets =====

    Text* titleText = TextBuilder()
        .setPosition(400, 40)
        .setContent(L"点名器")
        .setFont(50, L"宋体")
        .setColor(EGERGB(0, 200, 255))
        .setAlign(TextAlign::Center)
        .build();

    Text* countText = TextBuilder()
        .setPosition(400, 108)
        .setContent(L"成员数量：" + to_wstring(names.size()))
        .setFont(22, L"宋体")
        .setColor(EGERGB(150, 150, 150))
        .setAlign(TextAlign::Center)
        .build();

    Text* nameText = TextBuilder()
        .setPosition(400, 260)
        .setContent(L"点击下方按钮开始点名")
        .setFont(28, L"宋体")
        .setColor(EGERGB(80, 80, 80))
        .setAlign(TextAlign::Center)
        .build();

    Button* rollBtn = ButtonBuilder()
        .setIdentifier(L"rollButton")
        .setSize(180, 60)
        .setColor(EGERGBA(64, 128, 255, 255))
        .setRadius(12)
        .setContent(L"开始点名")
        .build();

    Button* settingsBtn = ButtonBuilder()
        .setIdentifier(L"settingsButton")
        .setSize(150, 60)
        .setColor(EGERGBA(119, 136, 153, 255))
        .setRadius(12)
        .setContent(L"配置成员")
        .build();

    Box* btnBox = BoxBuilder()
        .setCenter(400, 510)
        .setSize(600, 80)
        .setDirection(LayoutDirection::Row)
        .setAlign(LayoutAlign::Center)
        .setSpacing(20)
        .addChild({rollBtn, settingsBtn})
        .build();

    // ===== 配置面板 =====

    // 预分配 MAX_NAMES 个输入框
    vector<InputBox*> nameInputBoxes;
    for (int i = 0; i < MAX_NAMES; i++) {
        nameInputBoxes.push_back(
            InputBoxBuilder()
                .setSize(400, 36)
                .setRadius(6)
                .setTextHeight(16)
                .build()
        );
    }

    // 名单滚动面板（Column 布局 + 滚动条）
    vector<Widget*> allInputs(nameInputBoxes.begin(), nameInputBoxes.end());
    Panel* listPanel = PanelBuilder()
        .setCenter(0, 0)
        .setSize(440, 280)
        .setRadius(6)
        .setBackground(EGERGB(45, 45, 45))
        .setLayout(FlexLayoutBuilder()
            .setDirection(LayoutDirection::Column)
            .setAlign(LayoutAlign::Start)
            .setSpacing(6)
            .setPadding(10)
            .build())
        .addChild(allInputs)
        .setScrollBar(true)
        .build();

    Text* configTitle = TextBuilder()
        .setPosition(0, 0)
        .setContent(L"配置成员")
        .setMaxWidth(440)
        .setFont(28, L"Microsoft YaHei")
        .setColor(EGERGB(0, 200, 255))
        .setAlign(TextAlign::Center)
        .build();

    Text* configHint = TextBuilder()
        .setPosition(0, 0)
        .setContent(L"编辑成员名单（空行将被忽略）")
        .setMaxWidth(440)
        .setFont(15, L"Microsoft YaHei")
        .setColor(EGERGB(180, 180, 180))
        .setAlign(TextAlign::Center)
        .build();

    Button* saveBtn = ButtonBuilder()
        .setIdentifier(L"saveButton")
        .setSize(120, 44)
        .setRadius(10)
        .setContent(L"保存")
        .setColor(EGERGB(64, 128, 255))
        .build();

    Button* cancelBtn = ButtonBuilder()
        .setIdentifier(L"cancelButton")
        .setSize(120, 44)
        .setRadius(10)
        .setContent(L"取消")
        .setColor(EGERGB(119, 136, 153))
        .build();

    Box* configBtnBox = BoxBuilder()
        .setCenter(0, 0)
        .setSize(440, 56)
        .setDirection(LayoutDirection::Row)
        .setAlign(LayoutAlign::Center)
        .setSpacing(20)
        .addChild({saveBtn, cancelBtn})
        .build();

    // 配置主面板（Column 布局：标题、提示、名单、按钮行）
    Panel* configPanel = PanelBuilder()
        .setCenter(400, 300)
        .setSize(500, 490)
        .setRadius(15)
        .setBackground(EGERGB(35, 35, 35))
        .setLayout(FlexLayoutBuilder()
            .setDirection(LayoutDirection::Column)
            .setAlign(LayoutAlign::Center)
            .setSpacing(14)
            .setPadding(20)
            .build())
        .addChild({configTitle, configHint, listPanel, configBtnBox})
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

    // ===== 滚动动画 =====
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

    // ===== 按钮事件 =====

    // 开始点名
    rollBtn->setOnClickEvent([&]() {
        if (names.empty() || rolling) return;
        rolling = true;
        selectedIdx = static_cast<int>(
            uniform_int_distribution<size_t>(0, names.size() - 1)(rng));
        remainingRolls = 20;
        doRoll();
    });

    // 打开配置面板：将当前 names 填入输入框，切换到配置视图
    settingsBtn->setOnClickEvent([&]() {
        if (rolling) return;
        for (int i = 0; i < MAX_NAMES; i++) {
            nameInputBoxes[i]->setContent(
                i < (int)names.size() ? names[i] : L"");
        }
        assignOrder({configPanel});
    });

    // 保存配置：收集非空输入框内容，写回 UTF-8 文件，更新主界面
    saveBtn->setOnClickEvent([&]() {
        names.clear();
        for (auto* ib : nameInputBoxes) {
            wstring s = trimW(ib->getContent());
            if (!s.empty()) names.push_back(s);
        }
        ofstream out(configPath);
        if (out.is_open()) {
            for (const auto& name : names) {
                out << wstrToUTF8(name) << '\n';
            }
            out.close();
        }
        updateUI();
        assignOrder({titleText, countText, nameText, btnBox});
    });

    // 取消：放弃更改，返回主界面
    cancelBtn->setOnClickEvent([&]() {
        assignOrder({titleText, countText, nameText, btnBox});
    });

    start();
    return 0;
}

