#include <stdio.h>
#include <stdlib.h>
#include "NoteManager.h"
#include "graphics.h"
#include "imgui.h"
#include "extgraph.h"
#include "linkedlist.h"
#include <time.h>
#include <windows.h>
#include <shlobj.h>
#include <tchar.h>
#include <io.h>


#define MAX_PATH_LEN 1024

// 全局变量
NoteManager noteManager; // 笔记管理器实例
char currentTitle[MAX_TITLE_LENGTH] = ""; // 当前输入的笔记标题
char currentContent[MAX_CONTENT_LENGTH] = ""; // 当前输入的笔记内容
char currentCategory[MAX_CATEGORY_LENGTH] = ""; // 当前输入的笔记类别
char searchTitle[MAX_TITLE_LENGTH] = ""; // 搜索笔记的标题
char searchContent[MAX_CONTENT_LENGTH] = ""; // 搜索笔记的标题
char newTitle[MAX_TITLE_LENGTH] = ""; // 修改后的笔记标题
char newContent[MAX_CONTENT_LENGTH] = ""; // 修改后的笔记内容
char newCategory[MAX_CATEGORY_LENGTH] = ""; // 修改后的笔记类别
int showHelp = 0; // 用于控制是否显示帮助信息
int showCategory = 0; // 用于控制是否显示按类别展示笔记
int showTitleList = 0;
int showDateList = 0;
static char notebookName[MAX_TITLE_LENGTH] = "";
static char notebookNamePath[MAX_PATH_LEN] = "";
int listFilesCount(const char* dir)
{
    string searchPath = dir;
    strcat(searchPath, "/*");
    HANDLE hFind;
    WIN32_FIND_DATA findData;
    LARGE_INTEGER size;
    hFind = FindFirstFile(searchPath, &findData);
    int count = 0;
    string filenames = "";
    if (hFind == INVALID_HANDLE_VALUE)
    {
        return -1;
    }
    do
    {
        // 忽略"."和".."两个结果 
        if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0)
            continue;
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)    // 是否是目录 
        {
            count += 1;
        }
    } while (FindNextFile(hFind, &findData));
    return count;
}
char filenames[8192] = "";
void listFilesNames(const char* dir)
{
    string searchPath = dir;
    HANDLE hFind;
    WIN32_FIND_DATA findData;
    LARGE_INTEGER size;
    hFind = FindFirstFile(searchPath, &findData);
    int count = 0;
    memset(filenames, 0, sizeof(filenames));
    string dunhao = ",";
    if (hFind == INVALID_HANDLE_VALUE)
    {
        return;
    }
    do
    {
        // 忽略"."和".."两个结果 
        if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0)
            continue;
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)    // 是否是目录 
        {
            string test = findData.cFileName;
            strcat(filenames, test);
            strcat(filenames, dunhao);
        }
    } while (FindNextFile(hFind, &findData));
    return;
}
// 绘制帮助信息
void drawHelp() {
    double fH = GetFontHeight(); // 获取字体高度
    double h = fH * 2; // 行高
    double w = GetWindowWidth() * 0.8; // 宽度占窗口的80%
    double x = (GetWindowWidth() - w) * 0.1; // 居中显示
    double y = GetWindowHeight() - h * 2; // 初始Y坐标

    SetPenColor("Black"); // 设置笔的颜色为黑色
    drawLabel(x, y, "帮助信息:"); // 绘制帮助信息标题
    drawLabel(x, y - h, "1. 使用 'Add Note' 按钮添加新笔记.");
    drawLabel(x, y - h * 2, "2. 使用 'Save Notes' 按钮保存笔记到文件.");
    drawLabel(x, y - h * 3, "3. 使用 'Load Notes' 按钮从文件加载笔记.");
    drawLabel(x, y - h * 4, "4. 使用文本框输入笔记标题和内容.");
    drawLabel(x, y - h * 5, "5. 按 'Help' 按钮查看此帮助信息.");
    drawLabel(x, y - h * 6, "6. 使用 'Search Note' 按钮搜索笔记.");
    drawLabel(x, y - h * 7, "7. 使用 'Modify Note' 按钮修改笔记.");
    drawLabel(x, y - h * 8, "8. 使用 'Delete Note' 按钮删除笔记.");
    char Summary[8192*2]; // 假设100个字符足够存储整个字符串
    char path[MAX_PATH_LEN];
    SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL, 0, path);
    strcat(path, "\\notebook");
    CreateDirectoryA(path, NULL);
    int filecount = listFilesCount(path);
    listFilesNames(path);
    strcat(path, "\\");
    strcat(path, notebookName);
    CreateDirectoryA(path, NULL);
    sprintf(Summary, "当前笔记本目前拥有%d个笔记.本笔记管理系统总共有%d个笔记本.当前所在笔记本", noteManager.count, filecount);
    // 将filenames添加到Summary中
    strcat(Summary, notebookName);
    strcat(Summary, ", fileLists:");
    strcat(Summary, filenames);
    drawLabel(x, y - h * 9, Summary); // 绘制笔记数量
}

// 显示笔记按类别
void displayByCategory() {
    double fH = GetFontHeight(); // 获取字体高度
    double h = fH * 2; // 行高
    double w = GetWindowWidth() * 0.8; // 宽度占窗口的80%
    double x = (GetWindowWidth() - w) * 0.1; // 居中显示
    double y = GetWindowHeight() - h * 2; // 初始Y坐标
    SetPenColor("Black"); // 设置笔的颜色为黑色

    // 使用链表结构存储笔记分类
    typedef struct CategoryNode {
        char category[MAX_TITLE_LENGTH];
        NoteNode* notes;
        struct CategoryNode* next;
    } CategoryNode;

    CategoryNode* categoryHead = NULL; // 分类链表的头指针

    // 将笔记分类存储到链表结构中
    NoteNode* current = noteManager.head;
    while (current) {
        CategoryNode* categoryCurrent = categoryHead;
        CategoryNode* categoryPrev = NULL;
        // 查找当前笔记的类别节点
        while (categoryCurrent && strcmp(categoryCurrent->category, current->note.category) != 0) {
            categoryPrev = categoryCurrent;
            categoryCurrent = categoryCurrent->next;
        }

        // 如果该类别节点不存在，则创建新的类别节点
        if (!categoryCurrent) {
            categoryCurrent = (CategoryNode*)malloc(sizeof(CategoryNode));
            strcpy(categoryCurrent->category, current->note.category);
            categoryCurrent->notes = NULL;
            categoryCurrent->next = NULL;
            if (categoryPrev) {
                categoryPrev->next = categoryCurrent;
            }
            else {
                categoryHead = categoryCurrent;
            }
        }

        // 创建新的笔记节点并添加到类别节点的笔记链表中
        NoteNode* newNote = (NoteNode*)malloc(sizeof(NoteNode));
        newNote->note = current->note;
        newNote->next = categoryCurrent->notes;
        categoryCurrent->notes = newNote;

        current = current->next;
    }

    // 按照层次结构展示笔记
    CategoryNode* categoryCurrent = categoryHead;
    int i = 0;
    while (categoryCurrent) {
        char categoryLabel[150];
        sprintf(categoryLabel, "Category: %s", categoryCurrent->category);
        drawLabel(x, y - i * h, categoryLabel); // 绘制类别标签
        i++;

        NoteNode* noteCurrent = categoryCurrent->notes;
        while (noteCurrent) {
            char noteLabel[200];
            struct tm localTime;
            localtime_s(&localTime, &noteCurrent->note.createTime);
            sprintf(noteLabel, "  - Title: %s, Category: %s, CreatedTime: %04d:%02d:%02d %02d:%02d:%02d, Total Words: %d",
                noteCurrent->note.title, noteCurrent->note.category,
                localTime.tm_year + 1900, localTime.tm_mon + 1, localTime.tm_mday,
                localTime.tm_hour, localTime.tm_min, localTime.tm_sec, noteCurrent->note.noteWordsCount);
            drawLabel(x, y - i * h, noteLabel); // 绘制笔记标签
            i++;
            noteCurrent = noteCurrent->next;
        }

        categoryCurrent = categoryCurrent->next;
    }

    // 释放内存
    while (categoryHead) {
        CategoryNode* tmpCategory = categoryHead;
        categoryHead = categoryHead->next;
        while (tmpCategory->notes) {
            NoteNode* tmpNote = tmpCategory->notes;
            tmpCategory->notes = tmpCategory->notes->next;
            free(tmpNote);
        }
        free(tmpCategory);
    }
}

void displayByTitle() {
    double fH = GetFontHeight(); // 获取字体高度
    double h = fH * 2; // 行高
    double w = GetWindowWidth() * 0.8; // 宽度占窗口的80%
    double x = (GetWindowWidth() - w) * 0.1; // 居中显示
    double y = GetWindowHeight() - h * 2; // 初始Y坐标
    SetPenColor("Black"); // 设置笔的颜色为黑色

    // 创建一个笔记数组
    NoteNode* current = noteManager.head;
    int noteCount = 0;
    while (current) {
        noteCount++;
        current = current->next;
    }

    Note* notesArray = (Note*)malloc(noteCount * sizeof(Note));
    current = noteManager.head;
    for (int i = 0; i < noteCount; i++) {
        notesArray[i] = current->note;
        current = current->next;
    }

    // 对笔记数组按标题进行排序
    for (int i = 0; i < noteCount - 1; i++) {
        for (int j = i + 1; j < noteCount; j++) {
            if (strcmp(notesArray[i].title, notesArray[j].title) > 0) {
                Note temp = notesArray[i];
                notesArray[i] = notesArray[j];
                notesArray[j] = temp;
            }
        }
    }

    // 显示排序后的笔记
    for (int i = 0; i < noteCount; i++) {
        char noteLabel[200];
        struct tm localTime;
        localtime_s(&localTime, &notesArray[i].createTime);
        sprintf(noteLabel, "Title: %s, Category: %s, CreatedTime: %04d:%02d:%02d %02d:%02d:%02d, Total Words: %d",
            notesArray[i].title, notesArray[i].category,
            localTime.tm_year + 1900, localTime.tm_mon + 1, localTime.tm_mday,
            localTime.tm_hour, localTime.tm_min, localTime.tm_sec, notesArray[i].noteWordsCount);
        drawLabel(x, y - i * h, noteLabel);
    }

    free(notesArray);
}

void displayByDate() {
    double fH = GetFontHeight(); // 获取字体高度
    double h = fH * 2; // 行高
    double w = GetWindowWidth() * 0.8; // 宽度占窗口的80%
    double x = (GetWindowWidth() - w) * 0.1; // 居中显示
    double y = GetWindowHeight() - h * 2; // 初始Y坐标
    SetPenColor("Black"); // 设置笔的颜色为黑色

    // 创建一个笔记数组
    NoteNode* current = noteManager.head;
    int noteCount = 0;
    while (current) {
        noteCount++;
        current = current->next;
    }

    Note* notesArray = (Note*)malloc(noteCount * sizeof(Note));
    current = noteManager.head;
    for (int i = 0; i < noteCount; i++) {
        notesArray[i] = current->note;
        current = current->next;
    }

    // 对笔记数组按创建时间进行排序
    for (int i = 0; i < noteCount - 1; i++) {
        for (int j = i + 1; j < noteCount; j++) {
            if (difftime(notesArray[i].createTime, notesArray[j].createTime) > 0) {
                Note temp = notesArray[i];
                notesArray[i] = notesArray[j];
                notesArray[j] = temp;
            }
        }
    }

    // 显示排序后的笔记
    for (int i = 0; i < noteCount; i++) {
        char noteLabel[200];
        struct tm localTime;
        localtime_s(&localTime, &notesArray[i].createTime);
        sprintf(noteLabel, "Title: %s, Category: %s, CreatedTime: %04d:%02d:%02d %02d:%02d:%02d, Total Words: %d",
            notesArray[i].title, notesArray[i].category,
            localTime.tm_year + 1900, localTime.tm_mon + 1, localTime.tm_mday,
            localTime.tm_hour, localTime.tm_min, localTime.tm_sec, notesArray[i].noteWordsCount);
        drawLabel(x, y - i * h, noteLabel);
    }

    free(notesArray);
}

// 绘制笔记管理界面
void drawNoteManager() {
    double fH = GetFontHeight(); // 获取字体高度
    double h = fH * 2; // 行高
    double w = GetWindowWidth() * 0.4; // 宽度占窗口的40%
    double x = GetWindowWidth() * 0.05; // 初始X坐标
    double y = GetWindowHeight() - h * 1; // 初始Y坐标

    NoteNode* current = noteManager.head;
    int i = 1;
    if (showHelp == 0 && showCategory == 0 && showTitleList == 0 && showDateList == 0) {
        // 绘制所有笔记的标题和类别
        while (current != NULL) {
            char noteLabel[200];
            struct tm localTime;
            localtime_s(&localTime, &current->note.createTime);
            sprintf(noteLabel, "Title: %s, Category: %s, CreatedTime: %04d:%02d:%02d %02d:%02d:%02d, Total Words: %d",
                current->note.title, current->note.category,
                localTime.tm_year + 1900, localTime.tm_mon + 1, localTime.tm_mday,
                localTime.tm_hour, localTime.tm_min, localTime.tm_sec, current->note.noteWordsCount);
            drawLabel(x, y - i * h, noteLabel); // 绘制笔记标签
            current = current->next;
            i++;
        }
    }

    double inputX = GetWindowWidth() * 0.55; // 输入框的X坐标
    double inputY = y; // 输入框的Y坐标

    // 绘制新笔记输入框和按钮
    drawLabel(inputX, inputY - h, "New Note Title:");
    textbox(GenUIID(0), inputX, inputY - h * 2, w, h, currentTitle, sizeof(currentTitle));
    drawLabel(inputX, inputY - h * 3, "New Note Content:");
    textbox(GenUIID(0), inputX, inputY - h * 4, w, h, currentContent, sizeof(currentContent));
    drawLabel(inputX, inputY - h * 5, "New Note Category:");
    textbox(GenUIID(0), inputX, inputY - h * 6, w, h, currentCategory, sizeof(currentCategory));

    if (button(GenUIID(0), inputX, inputY - h * 7, w, h, "Add Note")) {
        // 添加新笔记
        addNote(&noteManager, currentTitle, currentContent, currentCategory);
        currentTitle[0] = '\0'; // 清空输入框
        currentContent[0] = '\0';
        currentCategory[0] = '\0';
    }

    if (button(GenUIID(0), inputX, inputY - h * 8, w, h, "Save Notes")) {
        // 保存笔记到文件
        saveNotesToFile(&noteManager, notebookNamePath);
    }

    if (button(GenUIID(0), inputX, inputY - h * 9, w, h, "Load Notes")) {
        // 从文件加载笔记
        loadNotesFromFile(&noteManager, notebookNamePath);
    }

    // 绘制搜索笔记输入框和按钮
    drawLabel(inputX, inputY - h * 10, "Search Note by Title:");
    textbox(GenUIID(0), inputX, inputY - h * 11, w, h, searchTitle, sizeof(searchTitle));

    if (button(GenUIID(0), inputX, inputY - h * 12, w, h, "Search Note by Title")) {
        // 搜索笔记
        Note* foundNote = findNoteByTitle(&noteManager, searchTitle);
        if (foundNote != NULL) {
            // 如果找到笔记，填充修改输入框
            strncpy(newTitle, foundNote->title, MAX_TITLE_LENGTH);
            strncpy(newContent, foundNote->content, MAX_CONTENT_LENGTH);
            strncpy(newCategory, foundNote->category, MAX_CATEGORY_LENGTH);
        }
        else {
            // 如果未找到笔记，清空修改输入框
            newTitle[0] = '\0';
            newContent[0] = '\0';
            newCategory[0] = '\0';
        }
    }

    // 绘制修改笔记输入框和按钮
    drawLabel(inputX, inputY - h * 13, "Modify Note Title:");
    textbox(GenUIID(0), inputX, inputY - h * 14, w, h, newTitle, sizeof(newTitle));
    drawLabel(inputX, inputY - h * 15, "Modify Note Content:");
    textbox(GenUIID(0), inputX, inputY - h * 16, w, h, newContent, sizeof(newContent));
    drawLabel(inputX, inputY - h * 17, "Modify Note Category:");
    textbox(GenUIID(0), inputX, inputY - h * 18, w, h, newCategory, sizeof(newCategory));
    if (button(GenUIID(0), inputX, inputY - h * 19, w, h, "Modify Note")) {
        // 修改笔记
        Note* foundNote = findNoteByTitle(&noteManager, searchTitle);
        if (foundNote != NULL) {
            modifyNote(&noteManager, foundNote, newTitle, newContent, newCategory);
        }
    }

    if (button(GenUIID(0), inputX, inputY - h * 20, w, h, "Delete Note")) {
        // 删除笔记
        deleteNoteByTitle(&noteManager, searchTitle);
        newTitle[0] = '\0'; // 清空修改输入框
        newContent[0] = '\0';
        newCategory[0] = '\0';
    }

    if (button(GenUIID(0), inputX, inputY - h * 21, w, h, "Display By category!")) {
        // 切换显示按类别展示笔记
        showCategory = !showCategory;
    }

    if (button(GenUIID(0), inputX, inputY - h * 22, w, h, "Help")) {
        // 切换显示帮助信息
        showHelp = !showHelp;
    }
    // 绘制搜索笔记输入框和按钮
    drawLabel(inputX, inputY - h * 23, "Search Note by Content:");
    textbox(GenUIID(0), inputX, inputY - h * 24, w, h, searchContent, sizeof(searchContent));

    if (button(GenUIID(0), inputX, inputY - h * 25, w, h, "Search Note by Content")) {
        // 搜索笔记
        Note* foundNote = findNoteByContent(&noteManager, searchContent);
        if (foundNote != NULL) {
            // 如果找到笔记，填充修改输入框
            strncpy(newTitle, foundNote->title, MAX_TITLE_LENGTH);
            strncpy(newContent, foundNote->content, MAX_CONTENT_LENGTH);
            strncpy(newCategory, foundNote->category, MAX_CATEGORY_LENGTH);
        }
        else {
            // 如果未找到笔记，清空修改输入框
            newTitle[0] = '\0';
            newContent[0] = '\0';
            newCategory[0] = '\0';
        }
    }
    if (button(GenUIID(0), inputX, inputY - h * 26, w, h, "Display By Title!")) {
        // 切换显示按类别展示笔记
        showTitleList = !showTitleList;
    }
    if (button(GenUIID(0), inputX, inputY - h * 27, w, h, "Display By Date!")) {
        // 切换显示按类别展示笔记
        showDateList = !showDateList;
    }
    if (button(GenUIID(0), inputX, inputY - h * 28, w, h, "Exit")) {
        // 退出程序
        exit(0);
    }
}

void handleFileMenuSelection(int selection, double x, double y, double w, double h) {
    switch (selection) {
    case 1:
        // 创建笔记本
    {
        static char alertionCreateNotebook[MAX_TITLE_LENGTH] = "Created Succes!";
        drawLabel(x, y - h * 10, "Notebook Name:");
        textbox(GenUIID(0), x, y - h * 12, w, h, notebookName, 100);
        if (button(GenUIID(0), x, y - h * 13, w, h, "Create Notebook")) {
            char path[MAX_PATH_LEN];
            SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL, 0, path);
            strcat(path, "\\notebook");
            CreateDirectoryA(path, NULL);
            strcat(path, "\\");
            strcat(path, notebookName);
            CreateDirectoryA(path, NULL);
            textbox(GenUIID(0), x, y - h * 14, w, h, alertionCreateNotebook, 100);
        }
    }
    break;
    case 2:
        // 保存笔记本
    {
        static char alertionStoreNotebook[MAX_TITLE_LENGTH] = "Stored Succes!";
        drawLabel(x, y - h, "Notebook Name:");
        if (textbox(GenUIID(0), x, y - h * 2, w, h, notebookName, sizeof(notebookName))) {
            if (button(GenUIID(0), x, y - h * 3, w, h, "Save Notebook")) {
                char path[MAX_PATH_LEN];
                SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL, 0, path);
                strcat(path, "\\notebook");
                strcat(path, "\\");
                strcat(path, notebookName);
                saveNotesToFile(&noteManager, path);
                textbox(GenUIID(0), x, y - h * 14, w, h, alertionStoreNotebook, 100);
            }
        }
    }
    break;
    case 3:
        // 打开笔记本
    {
        static char alertionLoadNotebook[MAX_TITLE_LENGTH] = "Load Succes!";
        drawLabel(x, y - h, "Notebook Name:");
        if (textbox(GenUIID(0), x, y - h * 2, w, h, notebookName, sizeof(notebookName))) {
            if (button(GenUIID(0), x, y - h * 3, w, h, "Open Notebook")) {
                char path[MAX_PATH_LEN];
                SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL, 0, path);
                strcat(path, "\\notebook");
                strcat(path, "\\");
                strcat(path, notebookName);
                loadNotesFromFile(&noteManager, path);
                memset(notebookNamePath, 0, MAX_PATH_LEN);
                memcpy(notebookNamePath, path, sizeof(path));
                textbox(GenUIID(0), x, y - h * 14, w, h, alertionLoadNotebook, 100);
            }
        }
    }
    break;
    case 4:
		// 删除笔记本
	{
		static char notebookName[MAX_TITLE_LENGTH] = "";
        static char alertionDeleteNotebook[MAX_TITLE_LENGTH] = "Delete Succes!";
		drawLabel(x, y - h, "Notebook Name:");
		if (textbox(GenUIID(0), x, y - h * 2, w, h, notebookName, sizeof(notebookName))) {
			if (button(GenUIID(0), x, y - h * 3, w, h, "Delete Notebook")) {
				char path[MAX_PATH_LEN];
				SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL, 0, path);
                strcat(path, "\\notebook");
				strcat(path, "\\");
				strcat(path, notebookName);
				remove(path);
                textbox(GenUIID(0), x, y - h * 14, w, h, alertionDeleteNotebook, 100);
			}
		}
	}
    case 5:
        // 创建笔记
        addNote(&noteManager, currentTitle, currentContent, currentCategory);
        currentTitle[0] = '\0'; // 清空输入框
        currentContent[0] = '\0';
        currentCategory[0] = '\0';
        break;
    default:
        break;
    }
}
int prefileMenuSelection = -1;
// 绘制菜单界面
void drawMenu() {
    double fH = GetFontHeight(); // 获取字体高度
    double h = fH * 2; // 行高
    double w = GetWindowWidth() * 0.1; // 宽度占窗口的10%
    double x = GetWindowWidth() * 0.05; // 初始X坐标
    double y = GetWindowHeight() - h; // 初始Y坐标

    // 文件菜单选项
    char* fileMenu[] = {
        "File",
        "Create Notebook",
        "Save Notebook",
        "Open Notebook",
        "Delete Notebook",
        "Create Note"
    };

    // 帮助菜单选项
    char* helpMenu[] = {
        "Help"
    };

    // 绘制菜单栏
    drawMenuBar(x, y, w, h); // 绘制菜单栏
    //drawMenuBar(x + w, y, w, h); // 绘制菜单栏

    // 绘制文件菜单
    int fileMenuSelection = menuList(GenUIID(0), x, y, w, 150, h, fileMenu, sizeof(fileMenu) / sizeof(fileMenu[0]));
    if (fileMenuSelection == 0) {
        handleFileMenuSelection(prefileMenuSelection, x, y, w, h);
    }
    if ((fileMenuSelection != prefileMenuSelection && prefileMenuSelection == -1)) {
        handleFileMenuSelection(fileMenuSelection, x, y, w, h);
        prefileMenuSelection = fileMenuSelection;
    }
    else if (fileMenuSelection != prefileMenuSelection && prefileMenuSelection != -1) {
        handleFileMenuSelection(prefileMenuSelection, x, y, w, h);
    }
    else if (fileMenuSelection == prefileMenuSelection) {
        prefileMenuSelection = -1;
    }
}

// 显示函数
void display() {
    DisplayClear(); // 清除显示
    drawMenu(); // 绘制菜单界面
    drawNoteManager(); // 绘制笔记管理界面
    if (showHelp) {
        drawHelp(); // 绘制帮助信息
    }
    if (showCategory) {
        displayByCategory(); // 按类别展示笔记
    }
    if (showTitleList) {
        displayByTitle();
    }
    if (showDateList) {
		displayByDate();
	}
}

// 处理字符输入事件
void CharEventProcess(char ch) {
    uiGetChar(ch); // 获取字符
    display(); // 更新显示
}

// 处理键盘事件
void KeyboardEventProcess(int key, int event) {
    uiGetKeyboard(key, event); // 获取键盘事件
    display(); // 更新显示
}

// 处理鼠标事件
void MouseEventProcess(int x, int y, int button, int event) {
    uiGetMouse(x, y, button, event); // 获取鼠标事件
    display(); // 更新显示
}

// 处理定时器事件
void TimerEventProcess(int timerID) {
    display(); // 更新显示
}

// 主函数
void Main() {
    SetWindowTitle("Note Management System"); // 设置窗口标题
    SetWindowSize(GetFullScreenWidth(), GetFullScreenHeight()); // 设置窗口大小
    InitGraphics(); // 初始化图形
    initNoteManager(&noteManager); // 初始化笔记管理器
    registerCharEvent(CharEventProcess); // 注册字符输入事件处理函数
    registerKeyboardEvent(KeyboardEventProcess); // 注册键盘事件处理函数
    registerMouseEvent(MouseEventProcess); // 注册鼠标事件处理函数
    registerTimerEvent(TimerEventProcess); // 注册定时器事件处理函数
}
