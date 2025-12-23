#ifndef NOTEMANAGER_H
#define NOTEMANAGER_H

#include "Note.h"
#include<time.h>

// 定义存储笔记的链表节点结构体
typedef struct NoteNode {
    Note note;                   // 笔记内容
    struct NoteNode* next;       // 指向下一个节点的指针
} NoteNode;

// 定义笔记管理器结构体
typedef struct NoteManager {
    NoteNode* head;              // 链表头指针
    int count;                   // 笔记数量
} NoteManager;

// 初始化笔记管理器
void initNoteManager(NoteManager* manager);

// 添加新笔记
void addNote(NoteManager* manager, const char* title, const char* content, const char* category);

// 根据标题删除笔记
void deleteNoteByTitle(NoteManager* manager, const char* title);

// 根据标题查找笔记
Note* findNoteByTitle(NoteManager* manager, const char* title);

Note* findNoteByContent(NoteManager* manager, const char* content);

// 将所有笔记保存到文件
void saveNotesToFile(NoteManager* manager, const char* filename);

// 从文件加载笔记
void loadNotesFromFile(NoteManager* manager, const char* filename);

// 修改笔记
void modifyNote(NoteManager* manager, Note* note, const char* newTitle, const char* newContent, const char* newCategory);

#endif // NOTEMANAGER_H
